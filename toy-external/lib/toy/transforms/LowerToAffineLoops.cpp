//===----------------------------------------------------------------------===//
//
// This file implements a partial lowering of Toy operations to a combination of
// affine loops, memref operations and standard operations. This lowering
// expects that all calls have been inlined, and all shapes have been resolved.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "toy/Passes.h"
#include "toy/ToyDialect.h"
#include "toy/ToyOps.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

using namespace mlir;

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns
//===----------------------------------------------------------------------===//

/// Convert the given RankedTensorType into the corresponding MemRefType.
static MemRefType convertTensorToMemRef(RankedTensorType type) {
  return MemRefType::get(type.getShape(), type.getElementType());
}

/// Insert an allocation and deallocation for the given MemRefType.
/// Inserisco una alloc e una deallo all'inizio e alla fine del blocco corrente
/// rispettivamente (il rewriter mantiene l'insertion point corrente e tutte le
/// istruzioni; la location è relativa al codice .toy che ha generato la
/// allocazione)
static Value insertAllocAndDealloc(MemRefType type, Location loc,
                                   PatternRewriter &rewriter) {
  // AllocOp è una Op-derived class (smart-pointer to Operation instance)
  auto alloc = memref::AllocOp::create(rewriter, loc, type);

  // Make sure to allocate at the beginning of the block.
  auto *parentBlock = alloc->getBlock();
  alloc->moveBefore(&parentBlock->front());

  // Make sure to deallocate this alloc at the end of the block. This is fine
  // as toy functions have no control flow.
  auto dealloc = memref::DeallocOp::create(rewriter, loc, alloc);
  dealloc->moveBefore(&parentBlock->back());
  return alloc;
}

/// This defines the function type used to process an iteration of a lowered
/// loop. It takes as input an OpBuilder and the range of loop induction
/// variables for the iteration. It returns a value to store at the current
/// index of the iteration.
using LoopIterationFn =
    function_ref<Value(OpBuilder &rewriter, ValueRange loopIvs)>;

/// This lowers a source operation to a loop nest where the body of the most
/// internal loop is implemented by the lambda passed in. The uses of the source
/// op are replaced by uses of an purposely allocated memRef; the internal loop
/// stores its results to this memRef'd region
static void lowerOpToLoops(Operation *op, PatternRewriter &rewriter,
                           LoopIterationFn processIteration) {
  auto loc = op->getLoc();

  // Insert an allocation and deallocation for the result of this operation.
  auto tensorType = llvm::cast<RankedTensorType>((*op->result_type_begin()));
  auto memRefType = convertTensorToMemRef(tensorType);
  auto alloc = insertAllocAndDealloc(memRefType, loc, rewriter);

  // Create a nest of affine loops, with one loop per dimension of the shape.
  // The lowerbound of each loop is zero, the upper bound is the respective
  // dimension of that loop level, and the step is 1 for all loops.
  SmallVector<int64_t, 4> lowerBounds(tensorType.getRank(), /*Value=*/0);
  SmallVector<int64_t, 4> steps(tensorType.getRank(), /*Value=*/1);
  // this takes a callback that is used to construct the body of the innermost
  // loop given a: builder, a location and a range of loop induction variables.
  affine::buildAffineLoopNest(
      rewriter, loc, lowerBounds, tensorType.getShape(), steps,
      [&](OpBuilder &nestedBuilder, Location loc, ValueRange ivs) {
        // Call the processing function with the rewriter and the loop
        // induction variables. This function will return the value to store at
        // the current index.
        Value valueToStore = processIteration(nestedBuilder, ivs);
        // actually store the value within the allocated region and with the
        // correct index
        affine::AffineStoreOp::create(nestedBuilder, loc, valueToStore, alloc,
                                      ivs);
      });

  // NB: qua sto utilizzando un 'ConversionPatternRewriter'; questa sottoclasse
  // applica la sostituzioni in maniera lazy ed è cio che popola gli 'OpAdaptor'
  // dei conversion patterns.
  // Replace the source operation with the generated alloc (everyone the used
  // the source op now will use the alloc).
  rewriter.replaceOp(op, alloc);
}

namespace {
//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Binary operations
//===----------------------------------------------------------------------===//

template <typename BinaryOp, typename LoweredBinaryOp>
struct BinaryOpLowering : public OpConversionPattern<BinaryOp> {
  using OpConversionPattern<BinaryOp>::OpConversionPattern;
  // dato che siamo in un template immagino serva usare un alias esplicito
  using OpAdaptor = typename OpConversionPattern<BinaryOp>::OpAdaptor;

  LogicalResult
  matchAndRewrite(BinaryOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto loc = op->getLoc();

    lowerOpToLoops(op, rewriter, [&](OpBuilder &builder, ValueRange loopIvs) {
      // Generate loads for the element of 'lhs' and 'rhs' at the
      // inner loop.
      auto loadedLhs =
          affine::AffineLoadOp::create(builder, loc, adaptor.getLhs(), loopIvs);
      auto loadedRhs =
          affine::AffineLoadOp::create(builder, loc, adaptor.getRhs(), loopIvs);
      // Create the binary operation performed on the loaded
      // values.
      return LoweredBinaryOp::create(builder, loc, loadedLhs, loadedRhs);
    });

    return success();
  }
};
using AddOpLowering = BinaryOpLowering<toy::AddOp, arith::AddFOp>;
using MulOpLowering = BinaryOpLowering<toy::MulOp, arith::MulFOp>;

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Constant operations
//===----------------------------------------------------------------------===//

struct ConstantOpLowering : public OpConversionPattern<toy::ConstantOp> {
  using OpConversionPattern<toy::ConstantOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::ConstantOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    DenseElementsAttr constantValue = op.getValue();
    Location loc = op.getLoc();

    // When lowering the constant operation, we allocate and assign the constant
    // values to a corresponding memref allocation.
    auto tensorType = llvm::cast<RankedTensorType>(op.getType());
    auto memRefType = convertTensorToMemRef(tensorType);
    auto alloc = insertAllocAndDealloc(memRefType, loc, rewriter);

    // We will be generating constant indices up-to the largest dimension.
    // Create these constants up-front to avoid large amounts of redundant
    // operations (riutilizziamo gli stessi indici anche per dimensioni
    // diverse). An indexOp looks like this: `%c0 = arith.constant 0 : index`,
    // but it won't appear in the lowered mlir like that because we're using
    // these indices with affine operations
    auto valueShape = memRefType.getShape();
    SmallVector<Value, 8> constantIndices;
    if (!valueShape.empty()) {
      for (auto i : llvm::seq<int64_t>(0, *llvm::max_element(valueShape)))
        constantIndices.push_back(
            arith::ConstantIndexOp::create(rewriter, loc, i));
    } else {
      // This is the case of a tensor of rank 0 (scalar).
      constantIndices.push_back(
          arith::ConstantIndexOp::create(rewriter, loc, 0));
    }

    // The constant operation represents a multi-dimensional constant, so we
    // will need to generate a store for each of the elements. The following
    // functor recursively walks the dimensions of the constant shape,
    // generating a store when the recursion hits the base case.
    SmallVector<Value, 2> indices;
    auto valueIt = constantValue.value_begin<FloatAttr>();
    std::function<void(uint64_t)> storeElements = [&](uint64_t dimension) {
      // The last dimension is the base case of the recursion, at this point
      // we store the element at the given index.
      if (dimension == valueShape.size()) {
        affine::AffineStoreOp::create(
            rewriter, loc, arith::ConstantOp::create(rewriter, loc, *valueIt++),
            alloc, llvm::ArrayRef(indices));
        return;
      }

      // Otherwise, iterate over the current dimension by exploring recursively
      // the 'children dimensions' having appended the current index first
      for (uint64_t i = 0, e = valueShape[dimension]; i != e; ++i) {
        indices.push_back(constantIndices[i]);
        storeElements(dimension + 1);
        indices.pop_back();
      }
    };

    // Start the element storing recursion from the first dimension.
    storeElements(/*dimension=*/0);

    // Replace this operation with the generated alloc.
    rewriter.replaceOp(op, alloc);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Func operations
//===----------------------------------------------------------------------===//

struct FuncOpLowering : public OpConversionPattern<toy::FuncOp> {
  using OpConversionPattern<toy::FuncOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::FuncOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    // We only lower the main function as we expect that all other functions
    // have been inlined.
    if (op.getName() != "main")
      return failure();

    // Verify that the given main has no inputs and results.
    if (op.getNumArguments() || op.getFunctionType().getNumResults()) {
      return rewriter.notifyMatchFailure(op, [](Diagnostic &diag) {
        diag << "expected 'main' to have 0 inputs and 0 results";
      });
    }

    // Create a new non-toy function, with the same region.
    auto func = mlir::func::FuncOp::create(rewriter, op.getLoc(), op.getName(),
                                           op.getFunctionType());
    rewriter.inlineRegionBefore(op.getRegion(), func.getBody(), func.end());
    rewriter.eraseOp(op);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Print operations
//===----------------------------------------------------------------------===//

struct PrintOpLowering : public OpConversionPattern<toy::PrintOp> {
  using OpConversionPattern<toy::PrintOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::PrintOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    // We don't lower "toy.print" in this pass, but we need to update its
    // operands.
    rewriter.modifyOpInPlace(op,
                             [&] { op->setOperands(adaptor.getOperands()); });
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Return operations
//===----------------------------------------------------------------------===//

struct ReturnOpLowering : public OpConversionPattern<toy::ReturnOp> {
  using OpConversionPattern<toy::ReturnOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::ReturnOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    // During this lowering, we expect that all function calls have been
    // inlined.
    if (op.hasOperand())
      return failure();

    // We lower "toy.return" directly to "func.return".
    rewriter.replaceOpWithNewOp<func::ReturnOp>(op);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: Transpose operations
//===----------------------------------------------------------------------===//

struct TransposeOpLowering : public OpConversionPattern<toy::TransposeOp> {
  using OpConversionPattern<toy::TransposeOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::TransposeOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto loc = op->getLoc();
    lowerOpToLoops(op, rewriter, [&](OpBuilder &builder, ValueRange loopIvs) {
      Value input = adaptor.getInput();

      // Transpose the elements by generating a load from the
      // reverse indices.
      SmallVector<Value, 2> reverseIvs(llvm::reverse(loopIvs));
      return affine::AffineLoadOp::create(builder, loc, input, reverseIvs);
    });
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ToyToAffine Conversion Patterns: MatMul operations
//===----------------------------------------------------------------------===//

// Ricordati come si fa una matmul:
// for (int i=0; i<rows_a; i++) {
//     for (int j=0; j<cols_b; j++) {
//         for (int k=0; k<rows_b; k++) {
//             c[i, j] += a[i, k] * b[k, j];
//         }
//     }
// }
struct MatMulOpLowering : public OpConversionPattern<toy::MatMulOp> {
  using OpConversionPattern<toy::MatMulOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::MatMulOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto loc = op->getLoc();

    // Insert an allocation and deallocation for the result of this operation.
    auto tensorType = llvm::cast<RankedTensorType>((*op->result_type_begin()));
    auto memRefType = convertTensorToMemRef(tensorType);
    auto alloc = insertAllocAndDealloc(memRefType, loc, rewriter);

    // Create a nest of affine loops, with one loop per dimension of the shape.
    // The lowerbound of each loop is zero, the upper bound is the respective
    // dimension of that loop level, and the step is 1 for all loops.
    SmallVector<int64_t, 4> lowerBounds(3, /*Value=*/0);
    // the first two loops scan the output matrix; the last loop scans a row of
    // the lhs (= columns of the rhs) to compute the dot product for the single
    // element.
    auto lhs = llvm::cast<RankedTensorType>(op.getLhs().getType());
    auto rhs = llvm::cast<RankedTensorType>(op.getRhs().getType());
    SmallVector<int64_t, 4> upperBounds{lhs.getDimSize(0), rhs.getDimSize(1),
                                        lhs.getDimSize(1)};
    SmallVector<int64_t, 4> steps(3, /*Value=*/1);

    // TODO: controlla se c'è veramente bisogno di questo; e vedi se magari
    // riesci a fare con linalg dialect.
    // Devo fare attenzione ad inizializzare la matrice risultato (alloc) a zero
    // dato che il loop nest ACCUMULA INCREMENTI invece di fare un assegnamento
    // secco come per le altre binop
    SmallVector<int64_t, 4> initLB(2, /*Value=*/0);
    SmallVector<int64_t, 4> initUB{lhs.getDimSize(0), rhs.getDimSize(1)};
    SmallVector<int64_t, 4> initSteps(2, /*Value=*/1);
    affine::buildAffineLoopNest(
        rewriter, loc, initLB, initUB, initSteps,
        [&](OpBuilder &nestedBuilder, Location loc, ValueRange ivs) {
          auto zero = arith::ConstantOp::create(
              rewriter, loc,
              rewriter.getFloatAttr(tensorType.getElementType(), 0.0));
          affine::AffineStoreOp::create(nestedBuilder, loc, zero, alloc, ivs);
        });

    // qua facciamo la matmul vera e propria
    affine::buildAffineLoopNest(
        rewriter, loc, lowerBounds, upperBounds, steps,
        [&](OpBuilder &nestedBuilder, Location loc, ValueRange ivs) {
          Value i = ivs[0];
          Value j = ivs[1];
          Value k = ivs[2];

          // Generate loads for the element of 'lhs' and 'rhs'
          auto loadedLhs = affine::AffineLoadOp::create(
              nestedBuilder, loc, adaptor.getLhs(), ValueRange{i, k});
          auto loadedRhs = affine::AffineLoadOp::create(
              nestedBuilder, loc, adaptor.getRhs(), ValueRange{k, j});

          // Now we can perform the mac:
          // - first we do the multiplication to get an increment
          // - then we load the accumulated value
          // - we sum the new increment to the accumulator
          // - lastly we store the new accumulator value
          auto increment =
              arith::MulFOp::create(nestedBuilder, loc, loadedLhs, loadedRhs);
          auto accumulator = affine::AffineLoadOp::create(
              nestedBuilder, loc, alloc, ValueRange{i, j});
          auto valueToStore =
              arith::AddFOp::create(nestedBuilder, loc, accumulator, increment);
          affine::AffineStoreOp::create(nestedBuilder, loc, valueToStore, alloc,
                                        ValueRange{i, j});
        });

    // Replace the source operation with the generated alloc (everyone the used
    // the source op now will use the alloc).
    rewriter.replaceOp(op, alloc);

    return success();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// ToyToAffineLoweringPass
//===----------------------------------------------------------------------===//

/// This is a partial lowering to affine loops of the toy operations that are
/// computationally intensive (like matmul for example...) while keeping the
/// rest of the code in the Toy dialect.
namespace {
struct ToyToAffineLoweringPass
    : public PassWrapper<ToyToAffineLoweringPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ToyToAffineLoweringPass)
  StringRef getArgument() const override { return "toy-to-affine"; }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<affine::AffineDialect, func::FuncDialect,
                    memref::MemRefDialect>();
  }
  void runOnOperation() final;
};
} // namespace

void ToyToAffineLoweringPass::runOnOperation() {
  // The first thing to define is the conversion target. This will define the
  // final target for this lowering.
  ConversionTarget target(getContext());

  // We define the specific operations, or dialects, that are legal targets for
  // this lowering. In our case, we are lowering to a combination of the
  // `Affine`, `Arith`, `Func`, and `MemRef` dialects.
  target.addLegalDialect<affine::AffineDialect, BuiltinDialect,
                         arith::ArithDialect, func::FuncDialect,
                         memref::MemRefDialect>();

  // We also define the Toy dialect as Illegal so that the conversion will fail
  // if any of these operations are *not* converted. Given that we actually want
  // a partial lowering, we explicitly mark the Toy operations that don't want
  // to lower, `toy.print`, as `legal`. `toy.print` will still need its operands
  // to be updated though (as we convert from TensorType to MemRefType), so we
  // only treat it as `legal` if its operands are legal.
  target.addIllegalDialect<toy::ToyDialect>();
  target.addDynamicallyLegalOp<toy::PrintOp>([](toy::PrintOp op) {
    return llvm::none_of(op->getOperandTypes(),
                         [](Type type) { return llvm::isa<TensorType>(type); });
  });

  // Now that the conversion target has been defined, we just need to provide
  // the set of patterns that will lower the Toy operations.
  RewritePatternSet patterns(&getContext());
  patterns.add<AddOpLowering, ConstantOpLowering, FuncOpLowering, MulOpLowering,
               PrintOpLowering, ReturnOpLowering, TransposeOpLowering,
               MatMulOpLowering>(&getContext());

  // With the target and rewrite patterns defined, we can now attempt the
  // conversion. The conversion will signal failure if any of our `illegal`
  // operations were not converted successfully.
  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}

/// Create a pass for lowering operations in the `Affine` and `Std` dialects,
/// for a subset of the Toy IR (e.g. matmul).
std::unique_ptr<Pass> mlir::toy::createLowerToAffinePass() {
  return std::make_unique<ToyToAffineLoweringPass>();
}

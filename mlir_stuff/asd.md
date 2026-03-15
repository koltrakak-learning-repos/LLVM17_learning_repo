# What are blocks and regions?

A good hierarchy to remember is:

- Module
  - Operation
    - Region
      - Block
        - Operations

## Regions

A Region is

- a list of blocks
- **owned by an operation**

**Think of a region as the body of an operation** (praticamente una funzione).

Many MLIR operations contain regions to represent **structured control flow.**

- regions are thus a container for the CFG of the operation (eg. i can have an operation for an if)

## Blocks

A Block is similar to a basic block in SSA IR. It contains:

- a linear sequence of operations
- block arguments (substitutes to φ nodes)

LLVM style:

`x = phi [a, bb1], [b, bb2]`

MLIR style:

`^bb3(%x: i32):`

Branches pass values:

`cf.br ^bb3(%a)`
`cf.br ^bb3(%b)`

The last operation in a block must be a terminator operation.

- operation tipo un salto/branch che dice "dove va il controllo dopo questo blocco".
- specifica i successori (notare plurale per operazioni tipo conditional branch) del blocco
- può passare argomenti ai blocchi successori

A region with a single block may opt out of this requirement by attaching the NoTerminator on the enclosing op.

- The top-level ModuleOp is an example of such an operation which defines this trait and whose block body does not have a terminator.

## Hierarchical IR

Because regions belong to operations, and inside regions we have other operations, IR can nest arbitrarily.

Example:

module
  func.func
    scf.for
      scf.if
        arith.addi

This is one of MLIR's key design goals: hierarchical IR.

Why is this important? (non è che abbia capito bene)

```
They solve several problems that traditional flat IRs (like LLVM IR) struggle with.

A good way to think about it: MLIR represents programs as nested structures instead of a single flat CFG.

This enables better abstraction, optimization, and composability.


1. Preserving high-level program structure

Traditional IRs flatten everything into basic blocks.

Example high-level code:

for (int i = 0; i < N; i++) {
    if (A[i] > 0)
        B[i] = A[i];
}

In something like LLVM IR, this becomes a flat CFG:

    entry
    ↓
    loop_header
    ↓
    loop_body
    ↓
    if_then
    ↓
    loop_latch

THE COMPILER LOSES THE EXPLICIT CONCEPT OF "LOOP" AND "IF".

MLIR keeps it:

    scf.for %i = %lb to %ub step %step {
        scf.if %cond {
            ...
        }
    }

THE COMPILER NOW KNOWS THIS IS A LOOP CONTAINING AN IF.

2. Better optimization boundaries

Nested regions create natural scopes for transformations.

Example:

    func
        scf.for
            linalg.matmul

A pass can:

- optimize only inside the loop
- optimize only the matmul
- rewrite the entire loop

This localized reasoning is much harder in a flat CFG.
```

# Operations

the set of operations in MLIR is extensible.

Operations are modeled using a small set of concepts, enabling operations to be reasoned about and manipulated generically. These concepts are:

- A name for the operation.
- A list of SSA operand values.
- A list of attributes.
- A list of types for result values.
- A source location for debugging purposes.
- A list of successors blocks (for branches, mostly).
- A list of regions (for structural operations like functions).

# Opaque API

MLIR può manipolare IR anche senza conoscere il significato delle operazioni scritte al suo interno

- Opaque = il compilatore non conosce il significato dell'operazione

MLIR distingue due livelli:

**Operazioni registrate**: quando registri un dialect e le sue operazioni

MLIR conosce:

- quanti operandi
- quanti risultati
- quali tipi sono validi
- se è terminator
- eventuali verifiche

E può fare:

- verifiche semantiche
- trasformazioni
- analisi

**Operazioni non registrate (opaque)**

Se il dialect non è registrato, MLIR vede solo la forma sintattica, ad esempio:

"toy.transpose"

Per MLIR è semplicemente:

operation name = "toy.transpose"
operands = (...)
results = (...)
attributes = {...}

This is because **IR elements can always be reduced to the above fundamental concepts**

Non conosce:

- cosa fa
- se è valida
- quali tipi dovrebbe avere
- quanti operandi dovrebbe avere

È una scatola nera.

Quindi "Opaque API" significa: **MLIR può rappresentare e manipolare operazioni senza sapere cosa fanno.**

# Why is progressive lowering useful?

The core idea is that each level of abstraction lets you do different optimizations that would be impossible or much harder at other levels.

**Concrete Example: Matrix Multiplication on a GPU**

Let's trace C = A * B (two 1024×1024 matrices) through progressive lowering.

- Level 1 — High-level ML IR (e.g., linalg dialect)
  - `%C = linalg.matmul ins(%A, %B) outs(%C)`
  - At this level, MLIR knows this is a matrix multiply semantically. It can:
    - Tile the operation into cache-friendly chunks (e.g., 64×64 blocks)
    - Fuse it with a following ReLU into a single loop (avoiding a second pass over memory)
    - Determine parallelism — rows are independent, so this can be distributed
  - **None of this would be visible once you've flattened it to loops.**

- Level 2 — Loop IR (e.g., affine/scf dialects)
    `
  for i in 0..1024:
    for j in 0..1024:
        for k in 0..1024:
        C[i,j] += A[i,k] * B[k,j]
`
  - Now you can apply loop-level transforms:
    - Reorder loops for better cache locality (i, k, j order)
    - Unroll the inner loop by 4×
    - Insert prefetch hints
  - You couldn't do loop reordering at Level 1 (no loops existed), and you couldn't do the semantic fusion if you started here (no knowledge that this is matmul).

- Level 3 — Vector IR (e.g., vector dialect)
  - now we can use SIMD instructions
  - This requires knowing the target's register width — information that wasn't relevant at Level 1 or 2.

- Level 4 — Low-level IR (LLVM IR or PTX for GPU)
  - Finally, register allocation, instruction scheduling, and memory addressing happen.
  - The compiler has no idea this was a matrix multiply anymore — it just sees arithmetic and loads/stores.

If you skipped to:

- Raw loops immediately
  - You'd lose Semantic fusion (matmul + ReLU in one pass)
- LLVM IR immediately
  - You'd lose Loop tiling, vectorization strategy

**Each level is the right abstraction for a different class of optimization.**

MLIR's insight is to make it easy to mix and progressively lower these dialects in a single framework, rather than having entirely separate compilers for each level

- eg: compiler from rust to rust-ir, and from rust-ir to llvm-ir

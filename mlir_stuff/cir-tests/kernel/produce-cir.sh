IN_NAME=dotp.c
OUT_NAME=dotp.cir


clang -O3 $IN_NAME -emit-cir -o $OUT_NAME
cir-opt --cir-canonicalize \
        --cir-simplify \
        --mem2reg \
        --sroa \
        --cse \
        --loop-invariant-code-motion \
        --sccp \
        --canonicalize \
        $OUT_NAME -o $OUT_NAME

# mem2reg, cse, sccp hanno fatto qualcosa, il resto mica tanto

# canonicalize sembra applicare un po' di roba tutta insieme

# ricordati di:
#   --mlir-print-ir-after-all
#   --dump-pass-pipeline

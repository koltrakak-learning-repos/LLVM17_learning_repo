
cir-opt --dump-pass-pipeline \
        --cir-canonicalize \
        --cir-simplify \
        --mem2reg \
        --sroa \
        --cse \
        --loop-invariant-code-motion \
        --sccp \
        --canonicalize \
        matmul.cir -o matmul.opt.cir

# mem2reg, cse, sccp hanno fatto qualcosa, il resto mica tanto

# canonicalize sembra applicare un po' di roba tutta insieme

# ricordati di:
#   --mlir-print-ir-after-all
#   --dump-pass-pipeline

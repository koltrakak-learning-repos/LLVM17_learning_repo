# macro aggiunta da AddMLIR:
# - cerca un file chiamato StandaloneOps.td nella cartella corrente.
# - genera sia gli header (.h.inc) che i sorgenti (.cpp.inc) per le operazioni e
#   le definizioni del dialetto
#   - NB: abbiamo che ops.td include types.td che include dialect.td
#   - riusciamo a generare tutto specificando solo ops.td per questo motivo
# - crea un target CMake chiamato "MLIRStandaloneOpsIncGen".
#   - Se vuoi rigenerare solo le operazioni, puoi lanciare ninja MLIRStandaloneOpsIncGen.
#
# (standalone dovrebbe essere il nome del namespace cpp che si utilizza nei file
# cpp generati (::mlir::standalone))
add_mlir_dialect(StandaloneOps standalone)
# add_mlir_doc: Prende le summary e description che hai scritto per le tue operazioni
# e genera dei file Markdown (.md).
# - argomento 1: prefisso nome target cmake
# - argomento 2: nome file md che verrà creato
# - argomento 3: dove posizionare i file md dentro alla cartella di doc
# - argomento 4: cosa passare a mlir-tblgen per generare i file di doc
add_mlir_doc(StandaloneDialect StandaloneDialect Standalone/ -gen-dialect-doc)
add_mlir_doc(StandaloneOps StandaloneOps Standalone/ -gen-op-doc)

# NB: add_mlir_dialect NON basta se il dialetto definisce attributi (non chiama
# mlir_tablegen per attributi). Se bisogna anche generare attributi decommentare
# questo:
# set(LLVM_TARGET_DEFINITIONS StandaloneAttributes.td)
# mlir_tablegen(StandaloneAttributes.h.inc -gen-attrdef-decls)
# mlir_tablegen(StandaloneAttributes.cpp.inc -gen-attrdef-defs)
# add_public_tablegen_target(MLIRStandaloneAttributesIncGen)

# i passi richiedono una configurazione più manuale
# - LLVM_TARGET_DEFINITIONS dice a tablegen il file da processare
# - mlir_tablegen() è equivalente a invocare il comando da terminale
#   - siccome siamo dentro a include generiamo solo le decls (header)
# - infine, produciamo un target CMake come per le ops.
#   - NB: aggiungere questo target è fondamentale dato che lo usiamo
#     quando dichiariamo le dipendenze delle implementazioni (vedi dentro lib/)
set(LLVM_TARGET_DEFINITIONS StandalonePasses.td)
mlir_tablegen(StandalonePasses.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIRStandalonePassesIncGen)

# User, Use, Value

gli operandi di un'istruzione sono **usi** (references)

Una Instruction è un User, ma anche un Value

- gerarchia di classi
- le instruction sono sia **users che usee**
    - user perchè ha degli operandi
    - usee perchè il suo risultato (value) viene utilizzato da altre istruzioni (usee == value)
        - questo è il risultato della forma SSA


abbiamo poi anche il concetto di use

- gli user di un'istruzione sono altre istruzioni
- gli use sono gli argomenti specifici
    - se ad esempio avessimo un %2 = add %1, %1
    - avremmo che l'istruzione add (%2) è uno user
    - l'istruzione però ha due usi di %1 (che è un'altra istruzione)

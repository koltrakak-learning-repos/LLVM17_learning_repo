Abbiamo visto che il middle-end è organizzato come una sequenza di passi. In particolare, abbiamo:
- Passi di analisi
    - Consumano la IR
- Passi di trasformazione
    - Producono nuova IR

Per analizzare il codice e trasformarlo in una versione ottimizzata occorre una IR espressiva che mantenga tutte le informazioni importanti da trasmettere da un passo all’altro

Proprietà di una IR:
- Facilità di generazione
- Facilità e costo di manipolazione
    - dipende dalla struttura dati
- Livello di astrazione

**NB**: Sottili decisioni di progetto di una IR possono avere **effetti molto intricati sulla velocità ed efficienza di un compilatore**


In un compilatore possono esserci tanti tipi di IR. Le principali categorie sono:
- Grafiche (o strutturali)
    - Orientate ai grafi
    - Molto usate nella source-to-source translation
    - Tendono a essere voluminose
    - Esempi:
        - AST, DAG
- Lineari
    - sintaticcamente vicine all'assembly
    - Pseudocodice per macchine astratte
    - Il livello di astrazione varia
    - Strutture dati semplici e compatte
    - Facile da riarrangiare
    - Esempi:
        - 3-address code, SSA
- Ibride
    - Sfruttano una combinazione di forme grafiche e lineari
    - Esempi:
        - Control flow graph









## Esempi di IR

### AST
- Un albero i cui nodi rappresentano diverse parti del programma
- Il nodo radice è la nozione del programma che contiene un blocco di istruzioni, da cui discendono tanti nodi figli (di tipo diverso) quante sono le istruzioni

PRO
- Molto comodo per scrivere un interprete
- Basta usare una funzione ricorsiva per processare l’albero

CONTRO
- Tutti i diversi tipi di nodo nell’albero hanno un comportamento diverso.
- Scrivere un passo di analisi che sfrutti questa IR richiede di ragionare costantemente sulla differenza nella semantica dei vari tipi di nodo


### DAG
- Un DAG è una contrazione di un AST che evita la duplicazione delle espressioni

Come leggere il DAG: 
- Foglie == valori iniziali:
- Nodi interni == operazioni:

PRO
- Rappresentazione più compatta dell’AST
- **Per espressioni prive di assegnamento** si può generare codice che evita duplicazioni (Common Subexpression Elimination)

CONTRO
- **Il riuso di un’espressione è possibile solo se si dimostra che il suo valore non è cambiato**
    - Assegnamenti e chiamate sono frequentissimi e possono alterare il valore delle espressioni
    - **Il DAG non ha una nozione di come le espressioni cambiano valore nel tempo**
    - Ne osserva solo la rappresentazione testuale

Serve una IR che renda l’analisi (e la trasformazione) del codice più regolare e predicibile


### 3-Address Code (3AC)
Le istruzioni hanno la forma: x = y op z
- Un unico operatore e massimo tre operandi (3AC)

PRO
- Espressioni complesse vengono spezzate
- Forma compatta e molto simile all’assembly
- Vengono introdotti dei temporanei per i risultati intermedi
    - **Registri virtuali** (illimitati)


Con la IR 3AC dunque non è immediatamente possibile applicare le ottimizzazioni viste senza aver prima analizzato **l’evoluzione temporale delle espressioni**
- Vedremo in seguito come farlo
- Esiste un’evoluzione della 3AC che semplifica questo tipo di ottimizzazioni (Static Single Assignment (SSA) form)

**Es: Constant Propagation (CP)**
- Per le variabili con valore costante (es., b = 3)
- Sostituisce gli usi futuri di b con la costante
- **Se b non è cambiato nel frattempo**

```
b = 3                                   b = 3
c = 1 + b                               c = 1 + 3
b = 4                                   b = 4
d = b + c    -> b modificato! ->        d = ? + c  // non posso sostituire ciecamente su tutto il programma b con 3
```

### Static Single Assignment (SSA)
Ogni variabile è definita (assegnata) solo una volta
- Definizioni multiple della stessa variabile originale sono tradotti in **multiple versioni della variabile**

PRO
- Ogni definizione ha associata una lista di tutti i suoi usi
    - Ogni variabile operando di un’istruzione (espressione) è un uso di una qualche definizione, ed è ad essa direttamente collegata
- Semplifica analisi e trasformazione del codice (quasi sempre)

Con la forma SSA diventa immediato propagare il valore costante di una definizione ai suoi usi
- gli stessi vantaggi si hanno anche con altre ottimizzazioni (copy propagation, dead code elimination, ...)


### Control Flow Graph (CFG)
Fin qui abbiamo visto solo esempi di sequenze lineari di codice. Cosa succede se consideriamo istruzioni di salto? **Occorre una IR che sia in grado di rappresentare il flusso di controllo (control flow) del programma**

Un CFG modella il trasferimento (flusso) del controllo in un programma tra **blocchi di istruzioni**
- I suoi **nodi** sono dei basic blocks, che contengono una sequenza lineare di istruzioni, **terminata da un’istruzione di trasferimento del controllo**
- I suoi **archi** rappresentano il flusso di controllo (loop, if/else, goto, ecc.)

**Un basic block Bi è una sequenza di istruzioni in forma 3AC**
- Solo la prima istruzione può essere raggiunta dall’esterno 
    - gli archi non possono puntare a un’istruzione nel mezzo del blocco
    - Singolo entry point
- Tutte le istruzioni nel blocco sono eseguite se viene eseguita la prima 
    - non è possibile eseguire un’istruzione di salto se non come ultima istruzione del blocco
    - Singolo exit point

**Un arco connette due nodi Bi → Bj se e solo se Bj può eseguire dopo Bi in qualche percorso del flusso di controllo del programma**
- La prima istruzione di Bj è il target dell’istruzione di salto al termine di Bi
- oppure, Bj è l’unico successore di Bi, che non ha un’istruzione di salto come ultima istruzione (fallthrough)

### Dependency Graph
- I nodi di un DG sono istruzioni, che **usano dei valori e ne definiscono un altro**
- Gli archi di un DG connettono due nodi, uno dei quali **usa i risultati definiti dall’altro**

**Indispensabili per instruction scheduling**

### Call Graph
Mostra l’insieme delle (potenziali) chiamate tra le funzioni, gerarchicamente




### Scelta della IR
- Occorre scegliere la IR col giusto livello di dettaglio per il compito da svolgere
- Occorre tenere a mente i costi legati alla manipolazione dei vari formati
- **Non esiste un’unica IR perfetta per tutti gli scopi**
    - c'è un tradeoff considerando le proprietà dell'IR (vedi sopra)
- Tipicamente ne serve più d’una
    - Completamente separate, per funzioni diverse
    - Forme ibride, che combinano grafi e forme lineari
    - Es., Control Flow Graph con 3-Address Code
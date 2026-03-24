Un passo di analisi non modificano mai la IR

Un passo di trasformazione invece si (mantenendo la semantica del programma)

# Categorie di IR

## Grafiche

comode nel frontend dato che ...

- alberi e grafi hanno una struttura navigabile esplicita

``` Come mai è necessario passare dall'AST (forma grafica) e il mio frontend non può produrre direttametne una forma lineare
Si può fare ma è scomodo.

Il parsing è context-free, la semantica no.

Durante il parsing non sai ancora se x è una variabile locale, un campo di una struct, una funzione.

Il type checking, la risoluzione dei nomi, l'analisi semantica sono molto più facili da fare su un albero che puoi visitare più volte, annotare, modificare, che non su un flusso lineare TAC che stai ancora generando.


Sinceramente, io penso che sia naturale esprimere una grammatica ricorsiva di un linguaggio con un albero
```

tendono ad essere voluminose

non adatte ad ottimizzazione

## Lineari

pseudo-assembly per macchine astratte

registri virtuali infiniti

adatte ad ottimizzazione... come mai?

**NB**: manipolare un AST è difficile dato che richiede costantemente di ragionare sul tipo del nodo corrente

- nodi diversi hanno semantiche diverse e quindi non posso applicare in maniera cieca una trasformazione
- inoltre la struttura ad albero rende difficile navigare il programma per applicare le trasformazioni
  - dovrei navigare in orizzontale ad un stesso livello di profondità, scomodo
    - pensa a CSE, posso riutilizzare un'espressione solo se uno dei suoi argomenti non è cambiato
    - come faccio a capire se nel frattempo un argomento è cambiato con un albero?

**NB**: una forma lineare è in sostanza una lunga lista di istruzioni

- le istruzioni sono molto regolari: result + opcode + operandi
- in questa forma diventa facile navigare il programma, e processarlo

# Listone

## AST

ottimo per comprendere la sintassi

pessimo per ottimizzazione

- in ottimizzazione devo fare operazioni come: istruzione 5 dipende da istruzione 1, quindi ...

## DAG

è possibile però fare ottimizzazioni anche con al livello dell'AST; il DAG è un esempio (Faccio Common Subexpression Elimination)

Il DAG però lavora solamente ad un livello sintattico e non semantico:

- es:
  - il riuso di un'espressione è possibile solo se si dimostra che l'uso dell'espressione non è cambiato nel frattempo
  - se un operando è cambiato nel frattempo, il riuso è sbagliato

Con un DAG è difficile fare analisi semantiche come "il valore di questa variabile è cambiato?"

## TAC

l'obiettivo è avvicinarsi all'assembly

- spezziamo AST profondi in istruzioni che hanno al massimo due operandi
- introduciamo tanti temporanei e assumiamo registri virtuali illimitati

Rappresentazione a quadrupla più comoda in quanto più facilmente riordinabile

- il vantaggio della tripla è occupare meno spazio

## SSA

esempio Constant Propagation per notare il live range

- ragioniamo in termini di definizioni e usi

SSA è una evoluzione della 3AC in cui le variabili possono essere definite una sola volta

- se una stessa variabile viene riassegnata, faccio **versioning**

Una IR SSA, diversamente da una TAC, **non si concentra sul rappresentare variabili, bensì, rappresenta valori**

**La caratteristica più potente di SSA è che mi consente di ragionare in termini di catene definizioni-uso / uso-definizione**

- non navigo più le istruzioni linearmente
- navigo usi e definizioni
  - sono su una definizione e itero tutti i suoi usi
- **prima era una ricerca sintattica** (cerco la variabile b)
- con SSA posso considerare informazioni semantiche
  - liveness, definizioni e usi

```
NB: un frontend deve produrre solamente AST e una forma TAC/SSA da cui deriva altre strutture dati utili (se ne ha bisogno)

- CFG: è una struttura dati che organizzi sopra la TAC/SSA.
  - I basic block sono nodi, i jump sono archi.
- DAG: usato tipicamente dentro un basic block per ottimizzare espressioni (common subexpression elimination).
  - È locale, non per l'intero programma
Call graph: rappresenta le relazioni tra funzioni, utile per analisi interprocedurali.
  - Anch'esso una struttura dati ausiliaria, non una IR standalone
```

## CFG

I basic block hanno un unica istruzione di entry e di exit

- solamente l'ultima istruzione può essere un jump/branch (non necessariamente però, posso avere anche un fallthrough)
- come conseguenza, un basic block o lo eseguo tutto o non lo eseguo

**NB**: il CFG è associato ad una singola funzione, il programma è una collezione di funzioni e quindi di CFG

- tipicamente si ottimizza una singola funzione alla volta e quindi si trasforma il CFG

### Algoritmo di costruzione di un CFG

ricorda che:

- posso fondere due blocchi che sono collegati solamente da un arco di fall-through
- è facile costruire un CFG ricordando la regola SESE (single entry single exit)

la prima parte dell'algoritmo definisce i blocchi

la seconda li collega con archi

**Oss**: Interessante notare come il CFG viene costruito sopra alla rappresentazione lineare

## Call graph

IR che collega tutti i moduli

- Un modelo è un file (o più propriamente una translation unit)
- i file contengono varie funzioni (CFG) che possono chiamarsi tra di loro

Se voglio fare intraprocedural optimization questa è l'ir da usare dato che qui ho visibilità di tutto il programma

# IR particolari

Le IR sopra sono fondamentali e utilizzate da tutti i compilatori

## Dependency graph

questa IR evidenzia le dipendenze tra le istruzioni

fondamentale per instruction scheduling nel backend

## Data dependency graph

quella sopra non è l'unica forma di dipendenza che ci interessa (dipendenze tra istruzioni)

abbiamo anche dipendenze tra dati (intesi come memoria e non registri)

rappresentare le dipendenze di dato è fondamentale per parallelizzare il codice

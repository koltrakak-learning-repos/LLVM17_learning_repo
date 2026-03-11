Un passo di analisi non modificano mai la IR

Un passo di trasformazione invece si (mantenendo la semantica del programma)

# Categorie di IR

## Grafiche

comode nel frontend dato che ...

- alberi e grafi hanno una struttura navigabile esplicita

tendono ad essere voluminose

non adatte ad ottimizzazione

## Lineari

pseudo-assembly per macchine astratte

registri virtuali infiniti

adatte ad ottimizzazione

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

**La caratteristica più potente di SSA è che mi consente di ragionare in termini di catene definizioni-uso / uso-definizione**

- non navigo più le istruzioni linearmente
- navigo usi e definizioni
  - sono su una definizione e itero tutti i suoi usi
- **prima era una ricerca sintattica** (cerco la variabile b)
- con SSA posso considerare informazioni semantiche
  - liveness, definizioni e usi

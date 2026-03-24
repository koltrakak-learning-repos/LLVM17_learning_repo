# Scrivere un passo

```
la documentazione di LLVM, è un punto di partenza... purtroppo, non è completamente esaustiva

per capire come funzionano nel dettaglio le cose, l'unica strada è studiare la codebase
```

```
llvm, ridefinisce le proprie strutture dati, helper function, etc. al posto di usare quelle della stdlib.

questo in quanto le attività da fare in llvm sono più o meno sempre quelle e quindi non si ha bisogno della generalità della stdlib, piuttosto llvm utilizza le proprie implementazione ottimizzate per le proprie attività.
```

iterazione e downcasting

...

## Interfacce dei passi LLVM

``` parentesi sulle regioni

i blocchi non sono l'unica struttura a poter avere la proprietà di SESE

le regioni sono collezioni si BB per cui vale ancora SESE

il vecchio pass manager permetteva di iterare anche sulle regioni dato la proprietà SESE è una proprietà interessante
```

vecchio pass manager rigido

Il nuovo pass manager lavora parametricamente con template

`CGSCC = Call Graph Strongly Connected Component`

Il pass manager è un componente del middle-end che gestisce l'esecuzione dei passi

- ce ne sono 4 uno per: module, CGSCC, function, loop

Separazione tra passi di analisi e passi di ottimizzazione

- trasformare la IR invalida la conoscenza pregressa del mio programma (conoscenza ottenuta dai passi di analisi)
- c'è bisogno di tenere allineate le informazioni
- c'è una nozioni di passi di analisi che sono un requisito per i passi di trasformazione
- c'è anche una nozione di che cosa invalida un passo di trasformazione

A questo scopo abbiamo anche un **analysis manager** che si occupa di queste cose

- lancia le analisi in maniera lazy
    - l'analisys manager rimane in ascolto per capire quando c'è bisogno di lanciare un passo
    - se lanciassi i passi di analisi in maniera eager
- e cacha i risultati
    - la cache viene invalidata dai passi di trasformazione
    - di nuovo in maniera lazy, solo se qualcuno richiederà di nuovo l'informazione invalidata, l'analysis manager si occuperà di rilanciare il passo di analisi

Analisys manager e pass manager (transform pass manager really) sono stati separati in modo da gestire in maniera efficiente le informazioni di analisi

- se avessi un unico concetto generico di passo, ricalcolerei tante volte le analisi intasando il mio compile time di analisi ridondanti

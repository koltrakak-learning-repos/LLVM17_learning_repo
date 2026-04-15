cominciamo a considerare il control flow e a fare analisi più globali (dataflow analysis)

la dataflow analysis è lo strumento che ci da informazioni su come le proprietà della singola funzione si estendono sull'intero CFG

attenzione:

- rappresentazione statica vs rappresentazione dinamica di un programma
- dataflow analysis considera tutte le possibili informazioni
  - cio di cui non siamo sicuri sono le informazioni riguardanti ad una specifica esecuzione dinamica
  - in dataflow analysis stiamo larghi e quindi consideriamo anche le possibili informazioni dinamiche di cui non siamo sicuri

## un po' di nomenclatura

ogni istruzione ha questi effetti:

...

uso localmente esposto = uso di una variabile che non ha definito il mio BB

definizione localmente disponibile/esposta = ultima definizione di una qualsiasi variabile in un BB

# Reaching definitions

prima di SSA capire quale fosse lo statement che definisce un uso non era facile

il problema di reaching definitions: ``

---

...

in un cfg con predecessori multipli è detto nodo di join

...

quando ci sono dei cicli, devo fare delle assunzioni sui valori iniziali (boundary conditions) e iterare fino a convergenza

- al primo giro, aggiornerò le mie boundary conditions
- questo cambierà l'input di qualche blocco e quindi dovrò fare un altro giro
- ma questo potrebbe cambiare l'input di altri blocchi ancora, ... da cui la necessità di iterare fino a convergenza

boundary conditions sono spesso pari all'insieme vuoto

- out(entry_block)
- out dei back-edges

# Liveness analysis

Problema che cerca di capire quali variabili sono vive in un blocco

- molto utilizzata nel backend in cui si fanno cose come register allocation
- ci permette di riutilizzare lo stesso registro se la variabile che conteneva precedentemente è morta

Una variabile è viva in punto del programma p se da li in poi avrà un altro uso

di nuovo abbiamo bisogno di **almeno un** percorso ...

Dominio:

- in reaching definitions avevamo come nel bit vector tutte le definizioni del programma
- in questo caso abbiamo tutte le variabili del programma
    - ridefinizioni della stessa variabile (punti di kill) non sono entry distinte nel bit vector

siccome questo è un backward problem, la funzione di trasferimento ha input e output del blocco invertiti

interessante come l'esempio in slide 57 ci fa notare che **ci interessano solamente gli usi localmente esposti**

- d non conta come uso


# Framework DFA

...

interessante notare che nella formulazione con bit vector che abbiamo studiato, un proprietà o c'è o non c'è (bit), questo è un **limite del framework**

Se per rispondere alla domanda che ci stiamo ponendo

- dobbiamo partire dal entrypoint ed arriviamo al punto P di interessa, allora abbiamo un analisi forward
  - reaching definitions: una definizione raggiunge il punto p se esiste almeno una definizione precedente non killata
- dobbiamo partire dal punto P di interesse ed arriviamo all'exit point, allora abbiamo un analisi backward
  - liveness analysis: una variabile in punto P è viva solo da quel punto in poi esiste almeno un path in cui esiste un uso

la funzione di trasferimento è la stessa per qualunque problema di DFA, basta capire come pensare ai:

- gen-set: insieme di nuova informazione generata dal basic block
- kill-set: insieme di informazione distrutta dal basic block
- eg: liveness analysis
    - gen == usi
    - kill == definizioni

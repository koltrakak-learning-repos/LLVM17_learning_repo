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

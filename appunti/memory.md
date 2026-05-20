...

slide 35:

- la performance del memory subsytem non è direttamente correlata con il numero di istruzioni
- il radix sort, per un k sufficientemente alto, sembra operare meglio rispetto ad un quick sort in termini di instruction/item
- questo potrebbe far pensare che il radix sort opera meglio di quick sort
- tuttavia, questo non è vero se si considerano gli effetti della memoria
- infatti, radix-sort ha più cache miss e di conseguenza più cycles/item

**Perchè questo è importante?**

- nei compilatori la maniera più diretta per fare una stima del beneficio di una trasformazione è contare il numero di istruzioni (informazione statica)
- come detto sopra però, bisogna tenere conto in realtà delle cache miss e del sottosistema di memoria
- ci si riesce?
  - In pratica no, le cache miss sono una informazione dinamica non disponibile al compilatore (se non con profile information)

# Data-reuse e data locality

data reuse:

- un programma ha data-reuse quando chiede più volte gli stessi indirizzi
  - STESSI indirizzi, indirizzi vicini non contano
- data reuse è legato alla località temporale

data locality:

- un programma ha data-locality quando richiede indirizzi vicini
- legato alla località spaziale

...

Gli algoritmi hanno località

- accedono allo stesso indirizzo o a indirizzi vicini

Le cache non perforza riescono a rendere efficiente questo riuso

- le cache possono fare eviction di un dato prima del prossimo riuso
  - e.g ho avuto tanti altri accessi che hanno riempito la mia cache e hanno causato l'eviction del mio dato
- layout infelice
  - e.g. matrice row-major e accesso per colonne

...

Il data reuse (località temporale) è difficile crearlo

- dato un algoritmo, o riusa i dati o no
- creare data reuse significa cambiare l'algoritmo

---

iteration space != data-space

---

come faccio a sfruttare meglio la località spaziale e temporale dei miei algoritmi?

- loop interchange
  - riordinare l'ordine di iterazione nel mio data-space in maniera cache friendly
  - se riuso un dato dopo un po', voglio che non sia stato evictato dalla cache
  - se accedo a dati vicini voglio trovarli in cache
- blocking/tiling
  - spezziamo un struttura dati enorme in blocchi che stanno in cache
  - permette di tenere in cache i dati riutilizzati
  - esempio matmul:
    - matrice C non ha data reuse, A e B invece si
    - una riga intera di A è difficile che ci stia in cache (però il riuso c'è per tutte le iterazioni di j -> non riesco a sfruttarlo :( )
    - accedere a colonne di B non è cache friendly

**Blocking reorders loop iterations (piega l'iteration space)** to bring iterations that reuse data closer in time (and thus more likely to be in cache)

Nota come dopo il blocking il loop esterno rimane immodificato

- scorro come prima tutte le righe
- scorrere la singola riga però adesso viene spezzata nella dimensione del blocco

Il Tiling è fondamentale non solo nelle cache ma tutte le volte che voglio gestire una memoria veloce (più vicina al compute) ma di dimensione limitata

- scratchpad memories
- shared memory in cuda
- ...

**NB**: loop interchange e blocking riordinano l'ordine in cui si attraversa l'iteration space, bisogna verificare che questo sia safe analizzando le dipendenze di dato

### Loop fusion (prossimo lab)

c'è riuso, A e C sono stati scorsi per intero nel primo loop e vengono riscorsi nel secondo

se A e C non ci stanno nella cache, il secondo loop non sfrutterebbe il riuso

quali sono i prerequisiti per la loop fusion:

- stesso iteration space (liv hanno lo stesso range)
- l'ordine di esecuzione deve essere irrilevante
  - di nuovo discorso sulle dipendenze di dato (in particolare forward)
  - ad esempio, se il secondo loop leggesse i+1, dopo la fusione le dipendenze di dato non sarebbero rispettate

# Software prefetching

schedulo in anticipo l'accesso ai dati (non vere load ma prefetch instructions) che mi serviranno in futuro

overlap memory accesses with computation and other accesses

che problemi ha questo approccio?

- non è sempre detto che il dato prefetchato sia presente quando lo vado ad usare
  - ho bisogno di stimare il tempo che impiegano le istruzioni che precedono la load
  - ad esempio in un sistema multiprogrammato (neanche multicore) un cambio di contesto verso un altro processo potrebbe fare eviction del dato prefetchato
  - il tempo è legato alla probabilità che il dato prefetchato nella cache non ci sia più
- da un lato io voglio una prefetch distance lunga in modo da nascondere più latenza possibile, dall'altro voglio che sia corta in modo da avere più certezza che il dato non sia stato evictato
- una branch prediction sbagliata mi fa fare dei prefetch inutili

Per questi motivi, tecniche di prefetching si prestano bene quando applicate ai loop che hanno un control flow regolare e una durata dell'iterazione regolare (shortest path se ci sono degli if)

## Software pipelining

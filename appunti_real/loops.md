# Come riconosciamo i loop nel CFG?

- Un analisi che ci dice dove sono i loop nel CFG utilizza come strumento base l'analisi di dominanza
- l'informazione di dominanza può essere comodamente acceduta salvondola in un **dominance tree**

prima di questo, **cos'è un loop?**

- in assembly
    - è un branch iniziale che salta ad exit oppure procede con un'iterazione
    - un salto incondizionato alla fine
- nel CFG
    - è sicuramente un ciclo
    - non tutti i cicli ci interessano però dal punto di vista dell'ottimizzazione
    - **vogliamo un singolo entry point**
        - ci da la nozione di iterazione
        - considera l'esempio nella slide con più entrypoint: l'iterazione può avere come blocchi c-d ma anche a-c-d
        - **non ho una serie di blocchi univoca che mi da la mia iterazione**
    - questa nozione di control flow che "si ripete sempre uguale, o non si ripete affatta" si traduce in un singolo entry point

## Loop naturali

header del loop == singolo entry point del loop

la nozione di dominanza ci aiuta nel identificare il singolo entry point del loop

- ...
- ...

trovare i backedge consiste in:

- fare una visita post-order del dominator tree
- per ogni coppia figlio padre, controllare se nel CFG c'è un arco che collega il figlio al padre

trovare i loop naturali una volta trovato il backedge è facile:

- ... guarda le slide

# Catene DU-UD

...

reaching definitions stabilisce la catena UD-DU

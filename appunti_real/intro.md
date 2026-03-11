Interessante la differenza di ottimizzazioni statiche e dinamiche

- ottimizzazioni a runtime sono del tipo:
  - compilatore inserisce un controllo per verificare la condizione dinamica
  - se la condizione è verificata eseguo il percorso ottimizzato
  - es: complicated_expression() || dynamic_true

Ottimizzazioni che possono sembrare banali in realtà sono importanti dato che bisogna considerare che le ottimizzazioni vengono applicate in cascata

Interessante considerare anche metriche diverse nell'ottimizzazione

- se io voglio ottimizzare il footprint di memoria, sostituire un'istruzione da 4 cicli con 2 da 1 ciclo è peggio nonostante io stia andando più veloce

I passi di ottimizzazione spesso hanno un preambolo che valuta (anche a spanne) il beneficio che si potrebbe guadagnare applicando l'ottimizzazione

- se la metrica scelta non viene migliorata, il passo viene saltato

Differenziamo ottimizzazioni applicabili nel middle-end e back-end:

- middle-end applica ottimizzazioni agnostiche all'architettura target
- back-end il contrario. Ad esempio, dipendentemente dall'architettura target posso vedere se fare strength reduction per sostituire un mul da 4 cicli (nell'architettura target) con delle shift e somme da 3 cicli totali (nell'architettura target)

Possiamo applicare Dead Code Elimination nella fase di linking:

- nella singola translation-unit non ho visibilità globale di tutti i call site di funzioni
- nel singolo file una funzione può sembrare inutilizzata, ma magari viene utilizzata da altri file
- a link time ho visibilità globale e quindi so effettivamente quali funzioni vengono utilizzate e quali no
- quelle inutilizzate posso eliminarle in tranquillità

DCE è una delle ottimizzazioni più importanti

- altri passi possono trasformare il codice in maniera tale da lasciare molte istruzioni inutili
- per questo motivo, ogni tanto si applica una DCE per pulire

```
Ripassare calling convention e meccanismo di chiamata di funzione potrebbe essere una buona idea
- registri argomento
    - a0 è sia valore di ritorno che il primo argomento
- registri saved
- jump and link
```

Espressioni loop-invariant non cambiano da iterazione a iterazione di un loop

- non è necessario calcolarle ogni volta
- posso portarle fuori facendo hoisting

# Come è cambiato il ruolo dei compilatori?

...

Un compilatore può fare la magia del autoparallelizzare un programma?

- la risposta breve è no
- i compilatori aiutano dove ci sono loop e grandi strutture dati regolari (big tensors)
- purtroppo questo copre solo una parte dei workload dei programmi

Questo è la parte di autoparallelizzazione, ma i compilatori servono anche a definire dei **parallel programming models**

- impossibile evitare di richiedere ai programmatori di specificare dove è richiesto parallelismo

Tuttavia, i parallel programming models cercano di astrarre i dettagli di basso livello, offrono invece dei costrutti per esprimere il parallelismo di un programma

- simile a come il C è comunque un linguaggio astratto che nasconde i dettagli dell'assembly
- i parallel programming models implementati dai compilatori semplificano la vita dato che permettono di concentrarsi solo sulla parallelizzazione degli algoritmi

...

interessante **l'outlining fatto da OpenMP**

- prende un pezzo di codice e lo mette in una funzione
- opposto di inlining che espande una funzione
- se devo parallelizzare un blocco, la maniera più facile è metterlo in una funzione e lanciare multipli thread su questa funzione

**TAKEAWAY: supportare un programming model richiede sempre un compilatore**

- devo tradurre le keyword e i concetti del programming model

...

Oltre a multicore, al giorno d'oggi il programming model deve gestire anche gli acceleratori

- identifica pezzi del programma da mandare all'acceleratore
- gestisce trasferimenti di memoria e indirizzi virtuali
- ... casino che senza i compilatori, apriti cielo

# Anatomia di un compilatore

se scelgo una IR buona, con buona che significa il poter essere utilizzata per qualsiasi ottimizzazione, allora posso separare

```
chiedi meglio cosa significa IR buona
```

opt è il middle-end llvm

llc è il back-end llvm

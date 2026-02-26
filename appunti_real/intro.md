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

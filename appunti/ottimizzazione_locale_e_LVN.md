ottimizzazioni locali hanno come scope un unico bb

ottimizzazione globale ha come scope l'intero CFG di una funzione

infine ottimizzazione interprocedurale (scope è l'intero call graph)

```
criterio per DCE = eliminiamo tutte le istruzioni (senza side effect) che non sono mai usate (no users)
```

...

# LVN

nelle versioni base della DCE che abbiamo visto non c'è una nozione di ordine delle istruzioni

nel caso delle Dead Store, abbiamo dovuto definire una struttura dati che tenesse conto dell'ordine delle istruzioni

- ci interessa sapere se tra due definizioni è presente un uso


...

c'è un problema comune di discrepenza tra valori e variabili

notiamo quindi possibilità di ottimizzazione che cercano di ricondursi ad una forma del codice in cui non c'è più ridondanza (ogni valore ha una sola variabile)

il workflow è:

- ir -> tabella
- tabella -> ir senza ridondanze

value numbering permette di spostare il focus dalle variabili ai valori (filosofia SSA)

- le ridondanze sono eliminate by design
- otteniamo DS, CP e CSE automaticamente

OSS: questa non ha tutte le proprietà di SSA, ma è stato un primo passo. Separiamo i nomi dai valori.


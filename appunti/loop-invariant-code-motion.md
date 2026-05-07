Come possiamo identificare un'espressione loop-invariant?

- gli operandi non devono essere ridefiniti nel loop
- ovvero, gli operandi, dentro al loop, sono utilizzati solamente come usi
- ovvero, le definizioni sono fuori dal loop
- **le reaching definitions di espressioni loop invariant provengono da fuori dal loop**
- tuttavia, espressioni con definizioni interne al loop possono comunque essere loop-invariant se i loro usi sono già marcati come loop-invariant (o costanti)
    - attenzione però, le reaching definitions relative agli usi loop-invariant devono essere tutte interne al loop per far si che l'espressione user sia anch'essa loop-invariant
    - altrimenti, con definizioni sia interne che esterne, non sarebbe più vero che il valore dell'espressione non cambia durante l'esecuzione del loop
        - (siccome arrivano più reaching definitions, esiste un percorso nel cfg del loop per cui verrà utilizzata la definizione esterna e uno in cui verrà utilizzata quella interna -> poco loop-invariant)


```
le reaching definitions in generale ci servono a costruire le catene UD; dato un uso, le reaching definitions mi dicono qual'è la definizione corrispondente all'uso (tra tutte le definizioni omonime se non sono in SSA)

Se sono in SSA le reaching definitions sono già pronte
```

**condizioni per la code motion**:

- l'istruzione (già marcata come loop-invariant) si trova in un blocco che domina tutte le uscite
    - l'avrei eseguita comunque anche senza spostarla fuori dal loop
- non ci sono altre definizioni della variabile nel loop
    - dopo l'hoisting una definizione intera potrebbe sovrascrivere la definizione hoistata
- l'istruzione (la definizione) domina tutti gli usi o non ci sono altre reaching definitions
    - non cambio il dataflow con l'hoisting
    - se la definizione dominava già tutti gli usi, spostarla nel preheader preserva la dominanza
    - non avere altre reaching definitions non è la stessa cosa?

**NB**: le ultime due condizioni in SSA sono già soddisfatte dato che ho una sola definizione per ogni variabile


```es slide 23
a=b+c non è hoistable dato che il suo blocco non domina tutte le uscite
```

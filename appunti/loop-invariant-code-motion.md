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

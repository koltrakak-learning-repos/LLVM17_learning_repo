Come possiamo identificare un'espressione loop-invariant?

- gli operandi non devono essere ridefiniti nel loop
- ovvero, gli operandi sono utilizzati solamente come usi dentro al loop
- ovvero, le definizioni sono fuori dal loop
- **le reaching definitions di espressioni loop invariant provengono da fuori dal loop**
  - se provengono da dentro il loop, magari sono definizioni già marcate come loop-invariant

```
le reaching definitions in generale ci servono a costruire le catene UD; dato un uso, le reaching definitions mi dicono qual'è la definizione corrispondente all'uso (tra tutte le definizioni omonime se non sono in SSA)

Se sono in SSA le reaching definitions sono già pronte
```

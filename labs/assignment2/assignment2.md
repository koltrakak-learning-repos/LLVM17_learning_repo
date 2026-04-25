# Very busy expressions

|                    |                              |
|--------------------|------------------------------|
| Domain             | espressioni                  |
| Direction          | backward                     |
| Transfer Function  | gen(B) U (out(B) - kill(B))  |
| Meet Operation     | intersezione                 |
| Boundary Condition | in(exit) = 0                 |
| Initial Conditions | in(B) = tutte le espressioni |

ho 4 espressioni nel mio dominio:

1. a!=b
2. b-a
3. a-b

### iterazione 1

|             |     in      |     out     |
|-------------|:-----------:|:-----------:|
| BB1 (entry) |      -      | {a!=b, b-a} |
| BB2         | {a!=b, b-a} |    {b-a}    |
| BB3         | {b-a, a-b}  |    {a-b}    |
| BB4         |    {a-b}    |      0      |
| BB5         |    {b-a}    |      0      |
| BB6         |      0      |    {a-b}    |
| BB7         |    {a-b}    |      0      |
| BB8 (exit)  |      0      |      -      |

La prima iterazione aggiorna tutti gli out, non c'è però bisogno di fare un'altra iterazione dato che non abbiamo cicli

# Dominator analysis

|                    |                                                              |
|--------------------|--------------------------------------------------------------|
| Domain             | basic block                                                  |
| Direction          | forward                                                      |
| Transfer Function  | assente, dato che non ho degli insiemi in e out da correlare |
| Meet Operation     | intersezione                                                 |
| Boundary Condition | dom(entry) = 0                                               |
| Initial Conditions | dom(B) = tutti i blocchi                                     |

NB: per l'analisi di dominanza non ha senso di parlare di insiemi in e out. Utilizziamo solamente un unico insieme dom.

### iterazione 1

|           |   dom   |
|-----------|:-------:|
| A (entry) |    -    |
| B         |  {A,B}  |
| C         |  {A,C}  |
| D         | {A,C,D} |
| E         | {A,C,E} |
| F         | {A,C,F} |
| G (exit)  |   {A}   |

anche qui non ho cicli e quindi non devo iterare

# Constant propagation

|                    |                                                                                                                       |
|--------------------|-----------------------------------------------------------------------------------------------------------------------|
| Domain             | coppie {var: costante}                                                                                                |
| Direction          | forward                                                                                                               |
| Transfer Function  | gen(B) U (out(B) - kill(B))                                                                                           |
| Meet Operation     | intersezione                                                                                                          |
| Boundary Condition | out(entry) = 0                                                                                                        |
| Initial Conditions | out(B) = l'insieme di tutte le combinazioni {var: costante} considerando le variabili e le costanti nel mio programma |

**NB**: l'insieme delle condizioni iniziali scala come: num_var*num_const; per funzioni grandi potrebbe diventare molto grande... però come faccio a gestire l'effetto dell'intersezione altrimenti?

**NB2**: ricorda che l'analisi fa anche un constant folding, **ma allora ci possono essere anche delle costanti nascoste**

- posso far ripartire il mio algoritmo da capo ogni volta che scopro una nuova costante, aggiungendo agli insiemi delle condizioni iniziali tutte le combinazioni relative alla nuova costante
    - soluzione orribile

Forse mi conviene **eliminare il bisogno delle condizioni iniziali**:

- posso imporre di partire sempre da entry
- e posso disabilitare gli input provenienti da backedge alla prima iterazione
- in questo
    - gli insiemi out appartenti a blocchi provenienti da rami divergenti del CFG vengono calcolati prima di doverli usare
    - gli insiemi out appartenti a blocchi provenienti da backedge vengono calcolati alla prima iterazione e usati nelle successive
- prerequisito di questa DFA è quindi **riconoscere i backedge nel CFG**

### iterazione 1

|             |                 in                  |                   out                   |
|-------------|:-----------------------------------:|:---------------------------------------:|
| BB1 (entry) |                  -                  |                    0                    |
| BB2         |                  0                  |                 {{k:2}}                 |
| BB3         |               {{k:2}}               |                 {{k:2}}                 |
| BB4         |               {{k:2}}               |             {{a:4}, {k:2}}              |
| BB5         |           {{a:4}, {k:2}}            |          {{x:5}, {a:4}, {k:2}}          |
| BB6         |               {{k:2}}               |             {{a:4}, {k:2}}              |
| BB7         |           {{a:4}, {k:2}}            |          {{x:8}, {a:4}, {k:2}}          |
| BB8         |           {{a:4}, {k:2}}            |           {**{k:4}**, {a:4}}            |
| BB9         |           {{k:4}, {a:4}}            |             {{k:4}, {a:4}}              |
| BB10        |           {{k:4}, {a:4}}            |          {{b:2}, {k:4}, {a:4}}          |
| BB11        |        {{b:2}, {k:4}, {a:4}}        |      {{x: 8}, {b:2}, {k:4}, {a:4}}      |
| BB12        |    {{x: 8}, {b:2}, {k:4}, {a:4}}    |   {{y:8}, {x: 8},{b:2}, {k:4}, {a:4}}   |
| BB13        | {{y:8}, {x: 8},{b:2}, {k:4}, {a:4}} | {**{k:5}**, {y:8}, {x: 8},{b:2}, {a:4}} |
| BB14        |           {{k:4}, {a:4}}            |             {{k:4}, {a:4}}              |
| BB15 (exit) |           {{k:4}, {a:4}}            |                    -                    |

Questa prima iterazione ha aggiornato l'insieme out associato al backedge, e quindi dobbiamo sicuramente fare almeno un'altra iterazione

### iterazione 2

|             |                         in                          |                             out                             |
|-------------|:---------------------------------------------------:|:-----------------------------------------------------------:|
| BB1 (entry) |                          -                          |                              0                              |
| BB2         |                          0                          |                           {{k:2}}                           |
| BB3         |                       {{k:2}}                       |                           {{k:2}}                           |
| BB4         |                       {{k:2}}                       |                       {{a:4}, {k:2}}                        |
| BB5         |                   {{a:4}, {k:2}}                    |                    {{x:5}, {a:4}, {k:2}}                    |
| BB6         |                       {{k:2}}                       |                       {{a:4}, {k:2}}                        |
| BB7         |                   {{a:4}, {k:2}}                    |                    {{x:8}, {a:4}, {k:2}}                    |
| BB8         |                   {{a:4}, {k:2}}                    |                       {{k:4}, {a:4}}                        |
| BB9         | **{{a:4}}** (intersezione con out(13) ha tolto k:4) |                           {{a:4}}                           |
| BB10        |                       {{a:4}}                       |                       {{b:2}, {a:4}}                        |
| BB11        |                   {{b:2}, {a:4}}                    | **{{b:2}, {a:4}}** (non avendo più k, non ho neanche più x) |
| BB12        |                   {{b:2}, {a:4}}                    |                    {{y:8}, {b:2}, {a:4}}                    |
| BB13        |                {{y:8}, {b:2}, {a:4}}                |                    {{y:8}, {b:2}, {a:4}}                    |
| BB14        |                       {{a:4}}                       |                           {{a:4}}                           |
| BB15 (exit) |                       {{a:4}}                       |                              -                              |

Il backedge continua ad avere {a:4} e quindi abbiamo raggiunto la convergenza

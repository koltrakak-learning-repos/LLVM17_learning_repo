l'utilità principale di fondere due loop è avvicinare load/store che generano miss con load/store che generano hit (se abbastanza vicine) -> data-reuse

# requisiti per la fusion

1. L1 e L2 devono essere adiacenti (no istruzioni in mezzo)

2. iterano lo stesso numero di volte

3. dobbiamo avere la garanzia che non esista un percorso di esecuzione che porti all'esecuzione di un loop e non di un altro -> control flow equivalent
    - se esegue L1, esegue anche L2 e viceversa
    - problema di dominanza e postdominanza

4. non ci devono essere dipendenze di dato a distanza negativa (del tipo i+1)

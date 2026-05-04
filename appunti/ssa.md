3. Perché è una "Filosofia"?
Questa visione trasforma il compilatore da un "esecutore di passi" a un "analizzatore di relazioni":

Ottimizzazione: Se un'istruzione non ha "archi" che escono da essa (nessuno usa il suo risultato), il compilatore sa istantaneamente che può cancellarla (Dead Code Elimination), perché quel valore non fluisce da nessuna parte.

Spostamento del codice: Finché rispetti gli archi (le dipendenze), puoi spostare le istruzioni dove vuoi. Non devi preoccuparti se qualcuno "sovrascrive" una variabile nel mezzo, perché in SSA le variabili sono immutabili.

In breve: Gli argomenti definiscono la topologia del grafo. Un programma SSA non è una lista di comandi, ma una rete stradale dove i dati viaggiano lungo i bordi (edges) tra un incrocio (istruzione) e l'altro.

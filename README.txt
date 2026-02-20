Il seguente progetto implementa un simulatore di circuiti quantistici che opera su N qubits basato su moltiplicazioni matrice × vettore di numeri complessi.

Lo stato del sistema è rappresentato da un vettore di dimensione 2^N, e ogni operatore del circuito è una porta quantistica a N qubits rappresentata come una matrice quadrata 2^N x 2^N. L'esecuzione del circuito applica in sequenza gli operatori allo stato iniziale.

L'applicativo è implementato in una versione multi-thread basata su pthread, che riusa una squadra di thread per eseguire più moltiplicazioni consecutivamente.

STRUTTURA DEI FILE

main.c
Il modulo si occupa dell’analisi degli argomenti da linea di comando, del caricamento dei file di input e dell’inizializzazione della squadra di thread. Successivamente esegue il circuito, stampa lo stato finale e conclude liberando le risorse, distruggendo la squadra di thread e deallocando la memoria riservata ai dati di input.

complesso.c/ complesso.h
Definisce il tipo complesso_t con operazioni base come somma, prodotto, modulo e stampa.

matrice.c/ matrice.h
Definisce il tipo matrice_t contenente numeri complessi di tipo complesso_t e implementa funzioni di utilità per la creazione, moltiplicazione, stampa e distruzione di matrici.

lettore_input.c/ lettore_input.h
Definisce i tipi: 
operatore_quantiscito_t per la rappresentazione di un singolo operatore del circuito;
istruzione_circuito_t per la rappresentazione di una singola istruzione del circuito;
dati_input_t per la raccolta delle informazioni date in input necessarie per la definizione del circuito quantistico.
Definisce le funzionalità per la lettura e analisi dei file di input (#qubits, #init, #define, #circ) tramite funzioni dedicate, e fornisce inoltre una funzione di pulizia incaricata di deallocare la memoria utilizzata per i dati di input.

thread_matrice.c/ thread_matrice.h
Definisce le funzioni per la creazione e la distruzione della squadra di thread, nonché la funzione principale eseguita da ciascun thread per lo svolgimento delle attività assegnate.

Makefile
Permette di compilare il progetto eseguendo semplicemente make nella directory. 


MANUALE UTENTE 

Compilazione: 

Aprire la shell dei comandi e posizionarsi nella directory del progetto. A questo punto sarà sufficiente eseguire il comando "make" per compilare il programma.

Esecuzione: 

Comporre sulla riga di comando: ./progetto_qsim -t <numero_thread> -i <file_iniziale> -c <file_circuito>

Parametri:

-t <numero_thread>: numero di thread che si desidera usare per lo svolgimento del circuito quantistico.

-i <file_iniziale>: percorso del file testuale contenente #qubits e #init.

-c <file_circuito>: percorso del file testuale contenente #define e #circ.

Note: Il programma si aspetta che i file di input rispettino il formato con direttive (#qubits, #init per il file dato in input con -i e #define, #circ per il file dato in input con -c) e che siano unici per ogni parametro. Non è rilevante l'ordine di inserimento degli input.


Esempio 1:

./progetto_qsim -t 4 -i file_init.txt -c file_circ.txt

Esempio 2:

./progetto_qsim -c file_circ.txt -i file_init.txt -t 2


Risultato: Il programma stampa lo stato finale (vettore complesso) su stdout.


Rimozione dei file generati: 

Il comando "make clean" consente di eseguire la pulizia della directory di lavoro, rimuovendo i file oggetto e l’eseguibile generati durante la compilazione.
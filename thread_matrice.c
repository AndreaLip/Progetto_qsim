#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "thread_matrice.h"
#include "complesso.h"
         

/*
 * Nuovo tipo utilizzato per raccogliere i dati assegnati a ciascun thread della squadra.
 * Ogni thread calcola sempre lo stesso intervallo di righe (riga_inizio, riga_fine).
 */
typedef struct {
    int riga_inizio;                    // Riga inizio calcolo
    int riga_fine;                      // Riga fine calcolo
    unsigned long last_job_visto;       // Id dell'ultimo job eseguito
} dati_thread_squadra_t;


/* Stato della squadra di thread (variabili statiche del modulo) */

static pthread_t* g_thread = NULL;                 // Array dei thread della squadra
static dati_thread_squadra_t* g_dati = NULL;       // Array contenente i dati di ogni thread
static int g_numero_thread = 0;                    // Numero thread creati
static int g_dimensione = 0;                       // Dimensione N (2^qubit)

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;     // Protegge lo stato condiviso
static pthread_cond_t g_cond_inizio = PTHREAD_COND_INITIALIZER; // Segnale: lavoro disponibile
static pthread_cond_t g_cond_fine = PTHREAD_COND_INITIALIZER;   // Segnale: lavoro completato

static int g_lavoro_disponibile = 0;               // 1 se c'è un job da eseguire
static int g_termina = 0;                          // 1 per dire ai thread di terminare
static int g_thread_finiti = 0;                    // Quanti thread hanno finito il job corrente
static unsigned long g_job_id = 0;                 // Identificatore del job corrente (incrementa ad ogni moltiplicazione)

/* Puntatori al job corrente (validi solo durante l’esecuzione di una moltiplicazione) */
static matrice_t* g_matrice = NULL;
static complesso_t* g_vettore = NULL;
static complesso_t* g_risultato = NULL;


/*
 * Funzione eseguita da ciascun thread della squadra.
 * Il thread resta vivo: attende lavoro, calcola, segnala fine, torna in attesa.
 */
static void* funzione_thread_squadra(void* arg) {
    dati_thread_squadra_t* dati = (dati_thread_squadra_t*)arg;  // cast del tipo di struttura necessario perché la funzione prende void *arg

    while (1) {    // ciclo infinito perché il thread è persistente, non termina dopo il lavoro eseguito, ne attende altri
        pthread_mutex_lock(&g_mutex);   // Effettua un lock per entrare nella sezione critica 

        // Attende che ci sia lavoro e sia un job nuovo (non già visto da questo thread)
        // oppure che venga richiesto di terminare          
        while (((!g_lavoro_disponibile) || (dati->last_job_visto == g_job_id)) && !g_termina) {
            pthread_cond_wait(&g_cond_inizio, &g_mutex);    // Rilascia g_mutex (unlock) e va in attesa sulla condition g_cond_inizio
        }                                                   // Il lock è nuovamente acquisito dal thread quando viene risvegliato

        /* Se richiesto, termina il thread */
        if (g_termina) {
            pthread_mutex_unlock(&g_mutex);     // Effettua un unlock
            return NULL;
        }

        /* Segna che questo thread sta per processare il job corrente */
        dati->last_job_visto = g_job_id;

        /* Copia locale dei parametri del job, poi rilascia il mutex e calcola */
        matrice_t* m = g_matrice;           // Matrice da moltiplicare con il vettore
        complesso_t* v = g_vettore;         // Vettore da moltiplicare con la matrice 
        complesso_t* out = g_risultato;     // Vettore che conterrà il risultato parzialmente calcolato
        int n = g_dimensione;               // Dimensione matrice 

        int r0 = dati->riga_inizio;         // Riga da cui inizia il calcolo del thread
        int r1 = dati->riga_fine;           // Riga che delimita la fine, non verra calcolata dal thread

        pthread_mutex_unlock(&g_mutex);     // Effettua un unlock

        /* Stampa di debug */
        //fprintf(stderr, "Thread righe (%d,%d) parte: lavoro=%d finiti=%d job=%lu out=%p v=%p\n", r0, r1, g_lavoro_disponibile, g_thread_finiti, g_job_id, (void*)out, (void*)v);

        /* Calcola le righe assegnate */
        for (int i = r0; i < r1; i++) {
            complesso_t somma = (complesso_t){0.0, 0.0, '\0'};

            for (int j = 0; j < n; j++) {
                complesso_t prodotto = moltiplica_complessi(m->dati[i][j], v[j]);
                somma = somma_complessi(somma, prodotto);
            }

            out[i] = somma;
        }

        /* Segnala completamento */
        pthread_mutex_lock(&g_mutex);   // Rientra in sezione critica per aggiornare il contatore 
        g_thread_finiti++;              // Dei thread che hanno finito il lavoro

        /* L’ultimo thread che finisce sveglia chi sta aspettando */
        if (g_thread_finiti == g_numero_thread) {   // g_numero_thread sono i lavori totali per ottenere il risultato finale
            g_lavoro_disponibile = 0;               // Se è vero non c'è più nulla da fare
            pthread_cond_signal(&g_cond_fine);      // Sveglia il thread main 
        }

        pthread_mutex_unlock(&g_mutex);    // Effettua un unlock ed esce dalla sezione critica 
    }
}


/*
 * Inizializza una squadra di thread riutilizzabili per le moltiplicazioni matrice × vettore.
 * Parametri:
 * numero_thread → numero di thread da creare
 * dimensione → dimensione della matrice e del vettore
 * Ritorna: 0 se tutto ok, -1 in caso di errore
 */
int inizializza_squadra_thread(int numero_thread, int dimensione) {
    if (numero_thread <= 0 || dimensione <= 0) return -1;
    if (g_thread != NULL) return -1;    // Evita doppia inizializzazione 

    g_numero_thread = numero_thread;    // Numero di thread che comporra la squadra
    g_dimensione = dimensione;          // Dimensione N

    g_thread = (pthread_t*)malloc(numero_thread * sizeof(pthread_t));   // Alloca memoria per array di thread della squadra
    g_dati = (dati_thread_squadra_t*)malloc(numero_thread * sizeof(dati_thread_squadra_t));     // Alloca memoria per array di dati dei thread
    if (!g_thread || !g_dati) {     // Se almeno un'allocazione è fallita
        free(g_thread);             // Libera la memoria e ripristina le variabili
        free(g_dati);               
        g_thread = NULL;
        g_dati = NULL;
        g_numero_thread = 0;        // Resetta tutte le variabili 
        g_dimensione = 0;
        return -1;
    }

    /* Suddivide le righe tra i thread */
    int righe_per_thread = dimensione / numero_thread;  
    int riga_corrente = 0;

    for (int t = 0; t < numero_thread; t++) {   // Assegna ad ogni t-esimo thread il suo intervallo di righe
        g_dati[t].riga_inizio = riga_corrente;

        if (t == numero_thread - 1) {
            g_dati[t].riga_fine = dimensione;  // ultimo thread prende eventuali righe rimanenti 
        } else {
            g_dati[t].riga_fine = riga_corrente + righe_per_thread;
        }

        g_dati[t].last_job_visto = 0;   // Inizializza lo stato interno del thread

        riga_corrente = g_dati[t].riga_fine;    // Aggiorna la riga corrente prima di passare alla prossima iterazione

        /*
         * &g_thread[t]: Destinazione dell'id del thread appena creato;
         * funzione_thread_squadra: Funzione che verrà eseguita dal thread;
         * &g_dati[t]: Argomento passato al thread per la funzione, ogni thread riceve l’indirizzo della sua struct diversa per ogni t
         */
        if (pthread_create(&g_thread[t], NULL, funzione_thread_squadra, &g_dati[t]) != 0) {
            /* In caso di errore, chiede terminazione ai thread già creati e fa join */
            pthread_mutex_lock(&g_mutex);            // Effettua un lock
            g_termina = 1;                           // Imposta g_termina a 1
            pthread_cond_broadcast(&g_cond_inizio);  // Sveglia tutti i thread che potrebbero essere bloccati in g_cond_inizio
            pthread_mutex_unlock(&g_mutex);          // Rilascia il lock così gli altri thread posso vedere g_termina ad 1

            for (int k = 0; k < t; k++) {
                pthread_join(g_thread[k], NULL);     // Attende che tutti i thread siano terminati prima di procedere
            }                                        

            free(g_thread);         // Libera la memoria allocata
            free(g_dati);

            g_thread = NULL;   
            g_dati = NULL;
            g_numero_thread = 0;    // Resetta tutte le variabili
            g_dimensione = 0;
            g_termina = 0;

            return -1;
        }
    }

    return 0;
}



/*
 * Funzione per la moltiplicazione matrice × vettore che utilizza una squadra di thread già inizializzata.
 * Parametri:
 * m → matrice quadrata N × N
 * v → vettore di dimensione N
 * Valore di ritorno: puntatore a un nuovo vettore contenente il risultato in caso di successo,
 * oppure NULL in caso di errore
 */
complesso_t* moltiplica_matrice_vettore_mt_riuso(matrice_t* m, complesso_t* v) {

    /* Controllo parametri */
    if (m == NULL || v == NULL) return NULL;

    /* Verifica che la squadra esista e che la dimensione sia coerente */
    if (g_thread == NULL || g_dimensione != m->dimensione) return NULL;

    int dimensione = m->dimensione;

    /* Alloca il vettore risultato */
    complesso_t* risultato = (complesso_t*)malloc(dimensione * sizeof(complesso_t));
    if (!risultato) return NULL;

    pthread_mutex_lock(&g_mutex);   // Effettua un lock prima di entrare nella sezione critica 

    /* Imposta il job corrente */
    g_matrice = m;                  // Matrice m da moltiplicare al vettore v
    g_vettore = v;                  // Vettore v da moltiplicare a matrice m
    g_risultato = risultato;        // Risultato della moltiplicazione r = m * v

    /* Nuovo job: incrementa id e reset contatori */
    g_job_id++;                     // Incrementa contatore dei lavori
    g_thread_finiti = 0;            // Setta la variabile dei thread che hanno gia finito a 0
    g_lavoro_disponibile = 1;       // Setta la variabile del lavoro disponibile a 1

    /* Sveglia tutti i thread per iniziare il lavoro */
    pthread_cond_broadcast(&g_cond_inizio); 

    /* Attende che l’ultimo thread segnali la fine */
    while (g_lavoro_disponibile) {      // Finché tutte le operazioni non sono terminare attende
        pthread_cond_wait(&g_cond_fine, &g_mutex);
    }

    pthread_mutex_unlock(&g_mutex);     // Effettua un unlock ed esce dalla sezione critica

    return risultato;   // Ritorna il risultato
}


/*
 * Distrugge la squadra di thread: segnala terminazione, attende (join) e libera la memoria.
 * Ritorna: 0 se tutto ok, -1 se la squadra non era inizializzata
 */
int distruggi_squadra_thread(void) {
    if (g_thread == NULL) return -1;    

    pthread_mutex_lock(&g_mutex);               // Effettua un lock per entrare nella sezione critica
    g_termina = 1;                              // Setta la variabile di termine a 1
    pthread_cond_broadcast(&g_cond_inizio);     // Sveglia tutti i thread che potrebbero essere addormentati in g_cond_inizio
    pthread_mutex_unlock(&g_mutex);             // Effettua un unlock ed esce dalla sezione critica 

    for (int t = 0; t < g_numero_thread; t++) { // Attende tutti i thread della squadra 
        pthread_join(g_thread[t], NULL);        // NULL indica che non importa il valore di ritorno della funzione pthread_join
    }

    free(g_thread);         // Libera la memoria occupata dall'allocazione degli array 
    free(g_dati);           // g_thread e g_dati

    /* Reset delle variabili statiche del modulo */
    g_thread = NULL;
    g_dati = NULL;          
    g_numero_thread = 0;
    g_dimensione = 0;

    g_matrice = NULL;
    g_vettore = NULL;
    g_risultato = NULL;

    g_lavoro_disponibile = 0;
    g_thread_finiti = 0;
    g_termina = 0;
    g_job_id = 0;

    return 0;
}

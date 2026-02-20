#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lettore_input.h"
#include "thread_matrice.h"
#include "matrice.h"


/* Struttura che raccoglie le opzioni della riga di comando */
typedef struct {
    int numero_thread;
    const char* file_iniziale;
    const char* file_circuito;
} opzioni_t;


/* Funzione che stampa un messaggio in caso di errore che spiega come passare correttamente gli input all'eseguibile */
static void stampa_uso(const char* nome_programma) {
    fprintf(stderr, "Utilizzo corretto del programma:\n%s -t <numero_thread> -i <file_iniziale> -c <file_circuito>\n", nome_programma);
}

/* Analisi della riga di comando con getopt. Ritorna 0 se ok, -1 se errore */
static int analisi_argomenti(int argc, char* argv[], opzioni_t* opt) {
    if (!opt) return -1;

    opt->numero_thread = -1;    // Variabile che conterrà il numero di thread da utilizzare
    opt->file_iniziale = NULL;  // Puntatore che punterà il file che contiene lo stato iniziale
    opt->file_circuito = NULL;  // Puntatole che punterà il file che contiene il circuito
    int c;                      // Variabile che conterrà il valore del carattere 
    
    int visto_i = 0, visto_c = 0, visto_t = 0;      // Variabili per verifica di un parametro doppione nel while

    /* Guarda dentro argv[] e trova la prossima opzione (tipo -t, -i, -c). Se l’opzione richiede un argomento 
       (dopo la lettera c’è : nella stringa "t:i:c:"), getopt mette il relativo valore in optarg */
    while ((c = getopt(argc, argv, "t:i:c:")) != -1) {
        
        switch (c) {
            case 't': 
                if (visto_t) return -1;
                visto_t = 1;
                opt->numero_thread = atoi(optarg); 
                break;

            case 'i': 
                if (visto_i) return -1;
                visto_i = 1;
                opt->file_iniziale = optarg; 
                break;

            case 'c': 
                if (visto_c) return -1;
                visto_c = 1;
                opt->file_circuito = optarg; 
                break;

            default: return -1;
        }
    }

    /* Niente argomenti extra: se l’utente ha scritto argomenti “in più” non riconosciuti,
       saranno in argv[optind]. (optind è l'indice del prossimo elemento in argv). */
    if (optind < argc) return -1;

    /* Presenza e validità minima */
    if (opt->numero_thread <= 0) return -1;
    if (!opt->file_iniziale || !opt->file_circuito) return -1;

    return 0;
}

/* Carica e valida i due file nella struttura "dati". Ritorna 0 se ok, -1 se errore. */
static int carica_input(const opzioni_t* opt, dati_input_t* dati, int* dimensione) {
    if (!opt || !dati || !dimensione) return -1;

    /* File iniziale: #qubits e #init */
    if (leggi_input(opt->file_iniziale, dati) != 0) return -1;
    if (!(dati->numero_qubit > 0 && dati->stato_iniziale != NULL)) return -1;

    /* Dimensione = 2^numero_qubit */
    *dimensione = 1 << dati->numero_qubit;  // shift a sinistra di numero_qubit posizioni

    /* Verifica compatibilità dimensioni */
    int dim_operatori = dimensione_operatori(opt->file_circuito);   // Dimensione operatori

    /* Se le due dimensioni non coincidono il circuito non può essere applicato allo stato iniziale */
    if (*dimensione != dim_operatori) return -1;     
                                                     
    /* File circuito: #define e #circ */
    if (leggi_input(opt->file_circuito, dati) != 0) return -1;
    if (!(dati->numero_operatori > 0 && dati->circuito != NULL)) return -1;

    return 0;
}

/* Esegue il circuito: per ogni istruzione fa stato = M * stato. Ritorna 0 se ok, -1 se errore. */
static int esegui_circuito(const dati_input_t* dati, int dimensione, complesso_t** stato_finale) {
    if (!dati || dimensione <= 0 || !stato_finale) return -1;

    complesso_t* stato = dati->stato_iniziale;   // Stato iniziale preso da #init
    if (!stato) return -1;

    for (int i = 0; i < dati->numero_istruzioni; i++) {     // Per ogni istruzione presa da #circ
        const char* nome_op = dati->circuito[i].nome_operatore;     // Prende il nome dell'operatore
        operatore_quantistico_t* op = trova_operatore((dati_input_t*)dati, nome_op);    // Lo cerca nell'array che li contiene 

        if (!op || !op->matrice) {                            // Se l'operatore non è stato trovato o la sua matrice non è valida
            if (stato != dati->stato_iniziale) free(stato);   // Libera eventualmente la memoria e lancia errore
            return -1;
        }

        complesso_t* nuovo_stato = moltiplica_matrice_vettore_mt_riuso(op->matrice, stato);     // Puntatore al vettore che conterrà lo stato aggiornato
        if (!nuovo_stato) {
            if (stato != dati->stato_iniziale) free(stato);
            return -1;
        }

        if (stato != dati->stato_iniziale) free(stato);     // Se lo stato non è quello iniziale, liberiamo la memoria perché non servirà più
        stato = nuovo_stato;                                // Aggiorniamo lo stato con il nuovo stato
        
        //printf("\nStato aggiornato: ");             
        //stampa_vettore(nuovo_stato, dimensione);    // Stampa di debug
        //printf("\n");
    }

    *stato_finale = stato;      // Aggiorniamo lo stato finale con stato calcolato
    return 0;
}


/* Funzione principale per il calcolo del circuito quantistico */
int main(int argc, char* argv[]) {
    int ret = 1;                              // Variabile che conterrà il valore di ritorno

    opzioni_t opt;                            // Struttura che raccoglierà le opzioni della riga di comando 
    dati_input_t dati = (dati_input_t){0};    // Struttura che raccoglierà i dati in input

    int dimensione = 0;                       // Variabile che conterrà la dimensione della matrice
    int thread_inizializzati = 0;             // Variabile che conterrà lo stato della squadra dei thread (1 se inizializzata, 0 altrimenti)
    complesso_t* stato_finale = NULL;         // Puntatore al vettore dello stato finale

    /* Analisi degli argomenti */
    if (analisi_argomenti(argc, argv, &opt) != 0) {
        fprintf(stderr, "Errore: argomenti non validi\n");
        stampa_uso(argv[0]);
        goto cleanup;
    }

    /* Caricamento input */
    if (carica_input(&opt, &dati, &dimensione) != 0) {
        fprintf(stderr, "Errore: file non leggibili o input non valido, verificare compatibilita' tra file\n");
        stampa_uso(argv[0]);
        goto cleanup;
    }

    /* Limita numero_thread per evitare thread idle:
     * Se il numero di thread è maggiore alla dimensione della matrice, avremo un overhaed di creazioni (di thread) e thread idle (senza lavoro)
     * Limitiamo quindi il numero di thread alla dimensione così da dare almeno un lavoro ad ogni thread e limitare il costo. */
    if (opt.numero_thread > dimensione) opt.numero_thread = dimensione;

    /* Stampa di verifica */
    //stampa_dati(dati, dimensione);
   
    /* Inizializza la squadra di thread */
    if (inizializza_squadra_thread(opt.numero_thread, dimensione) != 0) {
        fprintf(stderr, "Errore: impossibile inizializzare la squadra di thread\n");
        goto cleanup;
    }
    thread_inizializzati = 1;   // Aggiorniamo lo stato della squadra

    /* Esecuzione circuito */
    if (esegui_circuito(&dati, dimensione, &stato_finale) != 0) {
        fprintf(stderr, "Errore: esecuzione circuito fallita\n");
        goto cleanup;
    }

    /* Stampa lo stato finale */
    printf("\nStato finale:\n");
    stampa_vettore(stato_finale, dimensione);
    printf("\n");

    ret = 0;

cleanup:
    /* Verifica se la squadra già esiste ed eventualmente la distrugge */
    if (thread_inizializzati) {
        distruggi_squadra_thread();
    }

    /* Se lo stato finale NON coincide con lo stato iniziale, è memoria allocata durante il circuito e va liberata */
    if (stato_finale && stato_finale != dati.stato_iniziale) {
        free(stato_finale);
        stato_finale = NULL;
    }

    /* Liberiamo tutta la memoria allocata per la struttura dei dati */
    libera_dati_input(&dati);

    return ret;    
}

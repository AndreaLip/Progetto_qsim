#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lettore_input.h"


/*
 * Funzione di supporto che dato un file di testo salta whitespace fino al primo carattere
 * Paramentri: file → file su cui bisogna saltare i whitespace
 */
static void salta_spazi(FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {       // Legge caratteri finché non arriva a EOF
        if (!isspace((unsigned char)c)) {    // Se NON è whitespace (spazio, \n, \t, ...)
            ungetc(c, file);                 // Rimette indietro il carattere “utile” per farlo rileggere
            return;                          // Esce lasciando lo stream posizionato sul primo char non-whitespace
        }
    }
}

/*
 * Funzione di supporto che salta caratteri separatori [ ] ( ) , e whitespace 
 * Paramentri: file → file su cui bisogna saltare i separatori e whitespace
 * Risultato: intero che rappresenta il valore del carattere 
 */
static int salta_separatore(FILE* file) {
    int c;
    do {
        c = fgetc(file);                           // Legge 1 carattere dal file
    } while (c != EOF &&
            (isspace((unsigned char)c) ||          // Salta whitespace
             c == '[' || c == ']' ||               // Salta parentesi quadre
             c == '(' || c == ')' || c == ','));   // Salta parentesi tonde e virgole
    return c;                                      // Ritorna il primo carattere “non separatore” oppure EOF
}

/*
 * Funzione di supporto che legge un numero complesso dato un file
 * Paramentri: 
 * file → file su cui bisogna leggere il numero complesso
 * z → complesso che verrà eventualmente aggiornato 
 * Ritorna 0 se ok, 1 se fallisce.
 * 
 * 0, i) (i, 0)]
 */
static int leggi_complesso(FILE* file, complesso_t* z) {
    double re = 0.0, im = 0.0;      // Variabili locali per costruire il complesso
    char segno = '\0';              // Variabile carattere locale per il segno     

    /* Posizionati sul prossimo carattere non speratore (salta [ ] ( ) , e spazi) */
    int c = salta_separatore(file);                    // Cerca l’inizio del prossimo numero/segno utile
    if (c == EOF) return -1;                           // Se finisce il file non si può leggere altro
   
    /* Verifica se c'è direttamente la parte immaginaria */
    if (c == 'i') {                                          // Se il prossimo carattere è i
        segno = '+';                                         // Aggiorna il segno
        if (fscanf(file, " %lf", &im) != 1) {                // Prova a leggere eventuali coefficienti di i 
            im = 1.0;                                        // Se non trova nulla assumiamo 1 come coefficiente
        }
        goto termina;                                        // C'è un coefficiente, può terminare
    } else if (c == '+' || c == '-') {                       // Se invece trova un segno (potrebbe essere della parte reale)
        segno = c;                                           // Conserva il segno
        c = fgetc(file);                                     // Prossimo carattere potrebbe essere i
        if (c == 'i') {                                      // Se è proprio i
                if (fscanf(file, " %lf", &im) != 1) {        // Prova a leggere eventuali coefficienti di i 
                    im = 1.0;                                // Se non trova nulla assumiamo 1 come coefficiente
                 } 
                 goto termina; 
        } else {                                             // Altrimenti è la parte reale 
            ungetc(c, file);                                 // Torna indietro per leggere il valore
            if (fscanf(file, " %lf", &re) != 1) return -1;   // Se fallisce vuol dire che c'è un input errato 
            if (segno == '-') re = -re;                      // Se il segno iniziale era negativo corregge il valore reale
            segno = '\0';                                    // Ripristina il segno
            goto Parte_immaginaria;                          // Procede per la parte immaginaria
        }
    }  

    /* Leggi parte reale */
    ungetc(c, file);                                         // indietro dell’ultima lettura per non perdere il char trovato
    if (fscanf(file, " %lf", &re) != 1) return -1;           // Valorizza il reale, se fallisce vuol dire che c'è un input errato

    Parte_immaginaria:

    /* Prova a leggere eventuale parte immaginaria del tipo ± i <numero> */
    salta_spazi(file);                                 // Salta eventuali spazi tra reale e segno
    c = fgetc(file);                                   // Legge il carattere successivo (potrebbe essere +, -, oppure altro)

    if (c == '+' || c == '-') {                        // Se c’è + o - allora POTREBBE esserci la parte immaginaria
        segno = c;                                     // Salviamo il segno per applicarlo alla parte immaginaria

        salta_spazi(file);                             // Salta spazi prima della 'i' (se presenti)
        c = fgetc(file);                               // Legge il carattere che dovrebbe essere 'i'

        if (c == 'i') {                                // Se trova 'i', allora è proprio un complesso nel formato a±i b
            if (fscanf(file, " %lf", &im) != 1) {      // Legge il valore della parte immaginaria
                    im = 1.0;                          // Se non trova nulla assumiamo 1 come coefficiente
                }                                          
                if (segno == '-') im = -im;            // Applica il segno: a - i b => immaginaria negativa
                goto termina;
        } else {
            /* Non era un complesso nel formato previsto: ripristina */
            ungetc(c, file);                           // Rimette indietro il carattere letto “per errore”
            ungetc(segno, file);                       // Rimette indietro anche il segno, così il flusso resta coerente
        }
    } else if (c != EOF) {
        ungetc(c, file);                               // Se non era + o -, rimettilo indietro: dopo sarà gestito dal chiamante
    }

    termina: 
    z->segno = segno;                                  // Scrive il segno nel risultato
    z->parte_reale = re;                               // Scrive parte reale nel risultato
    z->parte_immaginaria = im;                         // Scrive parte immaginaria nel risultato

    if ( z->segno == '\0') {                           // Se la parte immaginaria non esiste la rappresentazione in stringhe sarà positiva
        z->segno = '+';}

    return 0;                                          
}


/*
 * Funzione di supporto che legge da un file lo stato iniziale di un vettore
 * Paramentri: 
 * file → file su cui bisogna leggere lo stato iniziale
 * dati → struttura che verrà valorizzata con i dati letti 
 * Ritorna 0 se ok, 1 se fallisce.
 */
static int leggi_init(FILE* file, dati_input_t* dati) {
    int dimensione = 1 << dati->numero_qubit;          // Dimensione del vettore di stato: 2^numero_qubit

    dati->stato_iniziale = calloc(dimensione, sizeof(complesso_t)); // Alloca e azzera il vettore
    if (!dati->stato_iniziale) return -1;              // Fallimento allocazione

    /* Trova '[' senza rischiare loop infinito su EOF */
    int c;                                             // variabile temporanea per il posizionamento
    while ((c = fgetc(file)) != EOF && c != '[') {}    // Consuma input fino alla '[' che apre la lista
    if (c == EOF) return -1;                           // Se non trovi '[', input errato

    /* File aperto con puntatore posizionato dopo il carattere [*/
    for (int i = 0; i < dimensione; i++) {             // Legge esattamente 2^n valori (reali o complessi)
        if (leggi_complesso(file, &dati->stato_iniziale[i]) != 0) return -1; // Riempie ogni posizione, torna -1 in caso di errore
    }
    return 0;                                          
}


/*
 * Funzione di supporto che legge dal file la descrizione di un operatore quantistico 
 * Paramentri: 
 * file → file su cui bisogna leggere la descrizione di un operatore quantistico
 * dati → struttura che verrà valorizzata con i dati letti 
 * Ritorna 0 se ok, 1 se fallisce.
 */
static int leggi_operatore(FILE* file, dati_input_t* dati) {
    operatore_quantistico_t* tmp = realloc(            // Rialloca l’array per aggiungere 1 operatore
        dati->operatori,
        (dati->numero_operatori + 1) * sizeof(operatore_quantistico_t)
    );
    if (!tmp) return -1;                               // Se realloc fallisce, non toccare il puntatore originale e ritorna errore
    dati->operatori = tmp;                             // Aggiorna il puntatore all’array (ora più grande)

    operatore_quantistico_t* op = &dati->operatori[dati->numero_operatori]; // Puntatore al nuovo slot appena aggiunto (l’elemento in coda)

    if (fscanf(file, " %31s", op->nome) != 1) return -1; // Legge il nome dell’operatore (es. H, I, ...) max 31 char

    int dimensione = 1 << dati->numero_qubit;          // Matrice dell’operatore: 2^numero_qubit x 2^numero_qubit
    op->matrice = crea_matrice(dimensione);            // Alloca la matrice
    if (!op->matrice) return -1;                       // Se fallisce ritorna -1

    /* Trova '[' */
    int c;
    while ((c = fgetc(file)) != EOF && c != '[') {}    // Consuma fino alla '[' che apre la matrice
    if (c == EOF) return -1;                           // Input errato: manca '['

    for (int i = 0; i < dimensione; i++) {
        /* Ogni vettore è racchiuso da '(' ... ')' */
        while ((c = fgetc(file)) != EOF && c != '(') {}  // Cerca l’inizio del vettore '('
        if (c == EOF) return -1;                         // Ritorna -1 se l'input è errato

        for (int j = 0; j < dimensione; j++) {         // Legge esattamente dimensione (2^numero_qubit) complessi per ogni vettore
            if (leggi_complesso(file, &op->matrice->dati[i][j]) != 0) return -1; // Salva cella (i,j) elemento j-esimo del i-esimo vettore
        }
    }

    dati->numero_operatori++;                          // Ora c’è un operatore in più
    return 0;
}

/*
 * Funzione di supporto che legge dal file il circuito da simulare come sequenza di nomi di operatori
 * Paramentri: 
 * file → file su cui bisogna leggere il circuito da simulare
 * dati → struttura che verrà valorizzata con i dati letti 
 * Ritorna 0 se ok, -1 in caso di errore di allocazione.
 */
static int leggi_circuito(FILE* file, dati_input_t* dati) {
    char nome[32];                                     // Il nome può essere lungo al massimo 32 caratteri

    while (fscanf(file, " %31s", nome) == 1) {         // Legge nomi uno per volta
        if (nome[0] == '#') {                          // Se inizia una nuova direttiva, fermati
            ungetc('#', file);                         // Rimetti '#' nello stream per permettere al loop esterno di rileggerlo
            return 0;
        }

        istruzione_circuito_t* tmp = realloc(          // Aggiunge una nuova istruzione al vettore dinamico
            dati->circuito,
            (dati->numero_istruzioni + 1) * sizeof(istruzione_circuito_t)
        );
        if (!tmp) return -1;
        dati->circuito = tmp;

        strcpy(dati->circuito[dati->numero_istruzioni].nome_operatore, nome); // Copia nome dell'operatore nell’istruzione
        dati->numero_istruzioni++;                     // Incrementa contatore istruzioni
    }

    return 0;                                         
}

/*
 * Legge e interpreta un file di input testuale aprendo il file in modalità lettura e scansionando direttive.
 * Gestisce le sezioni: #qubits (numero di qubit), #init (stato iniziale), #define (operatori), #circ (circuito),
 * delegando l'analisi sintattica alle funzioni di supporto leggi_init/leggi_operatore/leggi_circuito.
 * Paramentri: 
 * file → file su cui bisogna leggere gli input
 * dati → struttura che verrà valorizzata con i dati letti 
 * Ritorna 0 se tutto ok, -1 in caso di errori (apertura file, formato non valido o fallimenti nelle letture).
 */
int leggi_input(const char* nome_file, dati_input_t* dati) {
    FILE* file = fopen(nome_file, "r");                // Apre file in lettura
    if (!file) {
        perror(nome_file);                             // Stampa errore di sistema 
        return -1;
    }

    char parola[32];

    while (fscanf(file, " %31s", parola) == 1) {       // Legge la prossima “parola” 
        if (strcmp(parola, "#qubits") == 0) {          // Se #qubits: numero di qubit
            if (fscanf(file, " %d", &dati->numero_qubit) != 1) { // Legge intero n
                fclose(file);                          // Chiude il file
                return -1;
            }
        }
        else if (strcmp(parola, "#init") == 0) {       // Se #init: stato iniziale
            if (leggi_init(file, dati) != 0) {         // Legge lo stato iniziale
                fclose(file);                          // Chiude il file
                return -1;
            }
        }
        else if (strcmp(parola, "#define") == 0) {     // Se #define: definizione operatore
            if (leggi_operatore(file, dati) != 0) {    // Legge l'operatore
                fclose(file);                          // Chiude il file
                return -1;
            }
        }
        else if (strcmp(parola, "#circ") == 0) {       // Se #circ: circuito (sequenza di nomi)
            if (leggi_circuito(file, dati) != 0) {     // Legge il circuito
                fclose(file);                          // Chiude il file
                return -1;
            }
        }
        
    }

    fclose(file);   // Chiude il file                    
    return 0;                                          
} 

/*
 * Cerca un operatore per nome nell’array degli operatori.
 * Parametri: 
 * dati → struttura da cui prendere l'array
 * nome → nome dell'operatore da cercare
 * Ritorna puntatore all’operatore se trovato, NULL altrimenti.
 */
operatore_quantistico_t* trova_operatore(dati_input_t* dati, const char* nome) {

    for (int i = 0; i < dati->numero_operatori; i++) {
        if (strcmp(dati->operatori[i].nome, nome) == 0) {
            return &dati->operatori[i];
        }
    }

    return NULL;
}

/*
 * Funzione che permette di calcolare la dimensione della matrice utilizzata dagli operatori 
 * quantistici definiti in un file testuale.
 * Parametri:
 * file → file testuale su cui bisogna leggere 
 * ritorna → intero che rappresenta la dimensione della matrice in termini di qubits
 */
int dimensione_operatori(const char* nome_file) {

    FILE* file = fopen(nome_file, "r");                // Apre file in lettura
    if (!file) {
        perror(nome_file);                             // Stampa errore di sistema 
        return -1;
    }

    /* Trova l'inizio di una matrice */
    int c;
    while ((c = fgetc(file)) != EOF && c != '[') {}    // Consuma fino alla '[' che apre la matrice di un operatore
    if (c == EOF) return -1;                           // Input errato: manca '['

    /* Calcola la dimensione della matrice */
    int dimensione = 0;
    while ((c = fgetc(file)) != EOF && c != ']') {   // Consuma fino alla ']' che chiude la matrice dell'operatore
        if (c == '(') dimensione += 1;
    }    
    if (c == EOF) return -1;        // File incompleto, ritorna errore

    fclose(file);            // Chiude il file  
    return dimensione;       // Ritorna la dimensione della matrice 
}

/*
 * Libera tutta la memoria allocata dentro in una struttura di tipo dati_input_t.
 * Parametri: dati → struttura da liberare
 */
void libera_dati_input(dati_input_t* dati) {
    
    free(dati->stato_iniziale);     // Libera il vettore dello stato iniziale
    dati->stato_iniziale = NULL;    // Imposta il puntatore a NULL

    if (dati->operatori) {          // Controlla che l'array di operatori esista
        for (int i = 0; i < dati->numero_operatori; i++) {
            distruggi_matrice(dati->operatori[i].matrice);  // Libera la matrice dell'operatore
            dati->operatori[i].matrice = NULL;              // Imposta il puntatore a NULL 
        }
        free(dati->operatori);      // Libera l'array degli operatori
        dati->operatori = NULL;     // Imposta il puntatore a NULL
    }
    dati->numero_operatori = 0;     // Azzeramento del contatore degli operatori

    free(dati->circuito);           // Libera l'array del circuito (istruzioni)
    dati->circuito = NULL;           // Imposta il puntatore a NULL
    dati->numero_istruzioni = 0;     // Azzeramento del numero di istruzioni

    dati->numero_qubit = 0;          // Azzeramento del numero di qubit nella struttura
}

/* Funzione di debug utilizzata nel main per stampe di dati */
void stampa_dati(dati_input_t dati, int dimensione) {

    printf("Qubit: %d\n", dati.numero_qubit);

    printf("\nStato iniziale:\n");
    stampa_vettore(dati.stato_iniziale, dimensione);

    printf("\nOperatori definiti: %d\n", dati.numero_operatori);
    for (int i = 0; i < dati.numero_operatori; i++) {
        printf("\n#define %s \n", dati.operatori[i].nome);
        if (dati.operatori[i].matrice) {
            stampa_matrice(dati.operatori[i].matrice);
        } else {
            printf("(matrice NULL)\n");
        }
        printf("\n");
    }

    printf("\nCircuito:\n");
    printf("#circ ");
    for (int i = 0; i < dati.numero_istruzioni; i++) {
        printf("%s", dati.circuito[i].nome_operatore);
        if (i < dati.numero_istruzioni - 1) printf(" ");
    }
    printf("\n\n");

}

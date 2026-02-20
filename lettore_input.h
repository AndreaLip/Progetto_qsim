#ifndef LETTORE_INPUT_H
#define LETTORE_INPUT_H

#include "matrice.h"

/* Nuovo tipo che rappresenta un operatore quantistico */
typedef struct {
    char nome[32];            // Nome simbolico dell’operatore
    matrice_t* matrice;       // Puntatore alla matrice
} operatore_quantistico_t;

/* Nuovo tipo che rappresenta un'istruzione del circuito */
typedef struct {
    char nome_operatore[32];  // Nome dell’operatore da applicare
} istruzione_circuito_t;

/* Nuvo tipo che conterrà tutti i dati di input */
typedef struct {
    int numero_qubit;                    // Qubits utilizzati dal circuito quantistico (#qubits)
    complesso_t* stato_iniziale;         // Vettore di dimensione 2^numero_qubit (#init)

    operatore_quantistico_t* operatori;  // Array dinamico di operatori (#define) 
    int numero_operatori;                // Dimensione array operatori

    istruzione_circuito_t* circuito;     // Array dinamico di istruzioni del circuito (#circ)
    int numero_istruzioni;               // Dimensione array circuito
} dati_input_t;

/*
 * Legge e interpreta un file di input testuale aprendo il file in modalità lettura e scansionando direttive.
 * Gestisce le sezioni: #qubits (numero di qubit), #init (stato iniziale), #define (operatori), #circ (circuito),
 * delegando l'analisi sintattica alle funzioni di supporto leggi_init/leggi_operatore/leggi_circuito.
 * Paramentri: 
 * file → file su cui bisogna leggere gli input
 * dati → struttura che verrà valorizzata con i dati letti 
 * Ritorna 0 se tutto ok, -1 in caso di errori (apertura file, formato non valido o fallimenti nelle letture).
 */
int leggi_input(const char* nome_file, dati_input_t* dati);

/*
 * Funzione che permette di calcolare la dimensione della matrice utilizzata dagli operatori 
 * quantistici definiti in un file testuale.
 * Parametri:
 * file → file testuale su cui bisogna leggere 
 * ritorna → intero che rappresenta la dimensione della matrice in termini di qubits
 */
int dimensione_operatori(const char* nome_file);

/*
 * Cerca un operatore per nome nell’array degli operatori.
 * Parametri: 
 * dati → struttura da cui prendere l'array
 * nome → nome dell'operatore da cercare
 * Ritorna puntatore all’operatore se trovato, NULL altrimenti.
 */
operatore_quantistico_t* trova_operatore(dati_input_t* dati, const char* nome);

/*
 * Libera tutta la memoria allocata dentro in una struttura di tipo dati_input_t.
 * Parametri: dati → struttura da liberare
 */
void libera_dati_input(dati_input_t* dati);

/* Funzione di debug utilizzata nel main per stampe di dati */
void stampa_dati(dati_input_t dati, int dimensione);

#endif

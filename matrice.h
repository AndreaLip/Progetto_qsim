#ifndef MATRICE_H
#define MATRICE_H
#include "complesso.h"

/*
 * Nuovo tipo che rappresenta una matrice quadrata
 * di numeri complessi di dimensione: dimensione x dimensione
 */
typedef struct {
    int dimensione;          // Numero di righe e colonne
    complesso_t **dati;      // Elementi della matrice
} matrice_t;

/*
 * Alloca dinamicamente una matrice quadrata di dimensione N x N
 * Parametri: dimensione → numero di righe/colonne
 * Ritorna: puntatore alla matrice allocata
 */
matrice_t* crea_matrice(int dimensione);

/*
 * Dealloca tutta la memoria associata a una matrice
 * Parametri: m → matrice da deallocare
 */
void distruggi_matrice(matrice_t* m);

/*
 * Moltiplicazione tra due matrici quadrate
 * Parametri: a, b → matrici da moltiplicare (stessa dimensione)
 * Ritorna: nuova matrice risultato (a · b)
 */
matrice_t* moltiplica_matrici(matrice_t* a, matrice_t* b);

/*
 * Moltiplicazione matrice × vettore
 * Parametri: m → matrice quadrata, v → vettore di numeri complessi
 * Ritorna: nuovo vettore risultato
 */
complesso_t* moltiplica_matrice_vettore(matrice_t* m, complesso_t* v);

/*
 * Stampa una matrice su stdout
 * Parametri: m → matrice quadrata da stampare
 */
void stampa_matrice(matrice_t* m);

/*
 * Stampa un vettore su stdout
 * Parametri: v → vettore di numeri complessi da stampare, n → intero che ne indica la lunghezza
 */
void stampa_vettore(complesso_t* v, int n);

#endif

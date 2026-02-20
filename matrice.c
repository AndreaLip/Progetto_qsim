#include <stdio.h>
#include <stdlib.h>
#include "matrice.h"

/*
 * Alloca dinamicamente una matrice quadrata di dimensione N x N
 * Parametri: dimensione → numero di righe/colonne
 * Ritorna: puntatore alla matrice allocata
 */
matrice_t* crea_matrice(int dimensione) {
    int i; 
    /* Alloca dinamicamente memoria per una variabile di tipo matrice_t 
       e assegna a m l’indirizzo di quella memoria */
    matrice_t* m = (matrice_t*) malloc(sizeof(matrice_t)); 
    if (m == NULL) {        
        return NULL;
    }

    m->dimensione = dimensione;

    /* Allocazione delle righe */
    m->dati = (complesso_t**) malloc(dimensione * sizeof(complesso_t*));
    if (m->dati == NULL) {
        free(m); // Libera le righe allocate con successo prima di i
        return NULL;
    }

    /* Allocazione delle colonne */
    for (i = 0; i < dimensione; i++) {
        m->dati[i] = (complesso_t*) malloc(dimensione * sizeof(complesso_t));
        if (m->dati[i] == NULL) {
            /* Deallocazione parziale in caso di errore */
            for (int j = 0; j < i; j++) {
                free(m->dati[j]);
            }
            free(m->dati); // Libera l'array dei puntatori alle righe
            free(m);       // Libera la struttura m
            return NULL;
        }
    }

    return m;
}

/*
 * Dealloca tutta la memoria associata a una matrice
 * Parametri: m → matrice da deallocare
 */
void distruggi_matrice(matrice_t* m) {
    if (m == NULL) return;

    for (int i = 0; i < m->dimensione; i++) {
        free(m->dati[i]);   // Dealloca ogni i-esima riga
    }

    free(m->dati);  // Libera l'array dei puntatori alle righe
    free(m);        // Libera la struttura m
}

/*
 * Moltiplicazione tra due matrici quadrate (implementazione sequenziale)
 * Parametri: a, b → matrici da moltiplicare (stessa dimensione)
 * Ritorna: nuova matrice risultato (a · b) 
 * Ogni elemento: risultato[i][j] = somma_{k=0..n-1} a[i][k] * b[k][j]
 */
matrice_t* moltiplica_matrici(matrice_t* a, matrice_t* b) {
    if (a == NULL || b == NULL) return NULL;
    if (a->dimensione != b->dimensione) return NULL;

    int n = a->dimensione;
    matrice_t* risultato = crea_matrice(n); // Crea la matrice risultato (n x n) già allocata in memoria
    if (risultato == NULL) return NULL;

    for (int i = 0; i < n; i++) {   // Scorre tutte le celle (i,j) della matrice risultato
        for (int j = 0; j < n; j++) {  
            complesso_t somma = {0.0, 0.0, '\0'};  // Accumulatore della somma per la cella (i,j), inizializzato a 0 + 0i

            for (int k = 0; k < n; k++) {  // Calcola il prodotto scalare tra riga i di 'a' e colonna j di 'b'
                complesso_t prodotto = moltiplica_complessi(a->dati[i][k], b->dati[k][j]);
                somma = somma_complessi(somma, prodotto);  // somma = somma + prodotto
            }

            risultato->dati[i][j] = somma;  // Scrive il valore finale calcolato nella cella (i,j) del risultato
        }
    }

    return risultato;
}


/*
 * Moltiplicazione matrice × vettore
 * Parametri: m → matrice quadrata, v → vettore di numeri complessi
 * Ritorna: nuovo vettore risultato
 * Ogni elemento: risultato[i] = somma_{j=0..n-1} m[i][j] * v[j]
 */
complesso_t* moltiplica_matrice_vettore(matrice_t* m, complesso_t* v) {
    if (m == NULL || v == NULL) return NULL;

    int n = m->dimensione;
    /* Alloca dinamicamente memoria per un array di n elementi di tipo 
       complesso_t e restituisce un puntatore al primo elemento */
    complesso_t* risultato = (complesso_t*) malloc(n * sizeof(complesso_t));  
                                                                            
    for (int i = 0; i < n; i++) {
        complesso_t somma = {0.0, 0.0, '\0'};  // Accumulatore della somma (parte reale, parte immaginaria)

        for (int j = 0; j < n; j++) {
            complesso_t prodotto = moltiplica_complessi(m->dati[i][j], v[j]);  // prodotto = m[i][j] * v[j] (moltiplicazione tra complessi)
            somma = somma_complessi(somma, prodotto);  // somma += prodotto (accumula i contributi del prodotto scalare)
        }

        risultato[i] = somma;  // Scrive l'elemento i-esimo del vettore risultato
    }

    return risultato; 
}

/*
 * Stampa una matrice su stdout
 * Parametri: m → matrice quadrata da stampare
 */
void stampa_matrice(matrice_t* m) {
    if (m == NULL) return;

    printf("[ ");
    for (int i = 0; i < m->dimensione; i++) {   // Per ogni riga della matrice
        printf("(");
        for (int j = 0; j < m->dimensione; j++) {   // per ogni colonna della matrice
            stampa_complesso(m->dati[i][j]);
            if (j + 1 < m->dimensione) printf(",");
            printf(" ");    
        }
        printf(")");
        if (i + 1 < m->dimensione) printf("\n");
    }
    printf(" ]");
}

/*
 * Stampa un vettore su stdout
 * Parametri: 
 * v → vettore di numeri complessi da stampare
 * n → intero che ne indica la lunghezza
 */
void stampa_vettore(complesso_t *v, int dimensione) {
    if (v == NULL) return;

    printf("[ (");

    for (int i = 0; i < dimensione; i++) {  // Per ogni elemento del vettore
        stampa_complesso(v[i]);
        if (i < dimensione - 1) {
            printf(", ");
        }
    }

    printf(") ]\n");
}

#ifndef THREAD_MATRICE_H
#define THREAD_MATRICE_H
#include "matrice.h"

/*
 * Inizializza una squadra di thread riutilizzabili per le moltiplicazioni matrice × vettore.
 * Parametri:
 * numero_thread → numero di thread da creare
 * dimensione → dimensione della matrice e del vettore
 * Ritorna: 0 se tutto ok, -1 in caso di errore
 */
int inizializza_squadra_thread(int numero_thread, int dimensione);

/*
 * Funzione per la moltiplicazione matrice × vettore che utilizza una squadra di thread già inizializzata.
 * Parametri:
 * m → matrice quadrata N × N
 * v → vettore di dimensione N
 * Valore di ritorno: puntatore a un nuovo vettore contenente il risultato in caso di successo,
 * oppure NULL in caso di errore
 */
complesso_t* moltiplica_matrice_vettore_mt_riuso(matrice_t* m, complesso_t* v);

/*
 * Distrugge la squadra di thread: segnala terminazione, attende (join) e libera la memoria.
 * Ritorna: 0 se tutto ok, -1 se la squadra non era inizializzata
 */
int distruggi_squadra_thread(void);

#endif

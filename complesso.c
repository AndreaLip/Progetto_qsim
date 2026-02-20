#include <stdio.h>
#include <math.h> 
#include "complesso.h"

/*
 * Implementazione della somma di due numeri complessi
 * (a + ib) + (c + id) = (a + c) + i(b + d)
 * Parametri: a, b → numeri complessi da sommare
 * Ritorna: risultato della somma
 */
complesso_t somma_complessi(complesso_t a, complesso_t b) {
    complesso_t risultato;

    risultato.parte_reale = a.parte_reale + b.parte_reale;
    risultato.parte_immaginaria = a.parte_immaginaria + b.parte_immaginaria;

    if (risultato.parte_immaginaria < 0) {
        risultato.segno = '-';
    } else {
        risultato.segno = '+';
    }

    return risultato;
}

/*
 * Implementazione della moltiplicazione tra due numeri complessi
 * (a + ib)(c + id) = (ac − bd) + i(ad + bc)
 * Parametri: a, b → numeri complessi da moltiplicare
 * Ritorna: risultato del prodotto
 */
complesso_t moltiplica_complessi(complesso_t a, complesso_t b) {
    complesso_t risultato;

    risultato.parte_reale =
            a.parte_reale * b.parte_reale -
            a.parte_immaginaria * b.parte_immaginaria;

    risultato.parte_immaginaria =
            a.parte_reale * b.parte_immaginaria +
            a.parte_immaginaria * b.parte_reale;

    if (risultato.parte_immaginaria < 0) {
        risultato.segno = '-';
    } else {
        risultato.segno = '+';
    }  

    return risultato;
}

/*
 * Calcola il modulo di un numero complesso
 * |z| = √(a² + b²)
 * Parametro: z → numero complesso a cui va eseguito il modulo 
 * Ritorna: risultato del modulo
 */
double modulo_complesso(complesso_t z) {
    return sqrt(z.parte_reale * z.parte_reale +
                z.parte_immaginaria * z.parte_immaginaria);
}

/*
 * Stampa un numero complesso su stdout
 * Formato: a+i b  oppure  a-i b
 */
void stampa_complesso(complesso_t z) {
    if (z.segno == '+') {
        printf("%.5f+i%.5f", z.parte_reale, z.parte_immaginaria);
    } else {
        printf("%.5f-i%.5f", z.parte_reale, fabs(z.parte_immaginaria));  // fabs restituisce il valore assoluto della parte immaginaria
    }
}


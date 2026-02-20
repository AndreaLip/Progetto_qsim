#ifndef COMPLESSO_H 
#define COMPLESSO_H    

/*
 * Nuovo tipo che rappresenta un numero complesso
 * z = parte_reale +/- i * parte_immaginaria
 */
typedef struct {
    double parte_reale;
    double parte_immaginaria;
    char segno;
} complesso_t;

/*
 * Somma di due numeri complessi
 * Parametri: a, b → numeri complessi da sommare
 * Ritorna: risultato della somma
 */
complesso_t somma_complessi(complesso_t a, complesso_t b);

/*
 * Moltiplicazione di due numeri complessi
 * Parametri: a, b → numeri complessi da moltiplicare
 * Ritorna: risultato del prodotto
 */
complesso_t moltiplica_complessi(complesso_t a, complesso_t b);

/*
 * Modulo di un numero complesso 
 * Parametro: z → numero complesso a cui va eseguito il modulo 
 * Ritorna: risultato del modulo
 */
double modulo_complesso(complesso_t z);

/*
 * Stampa un numero complesso su stdout
 * Formato: a+i b  oppure  a-i b
 */
void stampa_complesso(complesso_t z);

#endif 
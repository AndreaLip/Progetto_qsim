# Makefile per progetto C - Compilazione con "make" - Pulizia con "make clean"

# Nome dell'eseguibile finale 
TARGET := progetto_qsim

# Compilatore C
CC := gcc

# Flag di compilazione:
# -Wall -Wextra : attiva warning utili
# -O2           : ottimizzazione
CFLAGS := -Wall -Wextra -O2

# Librerie da linkare (qui serve pthread)
LDLIBS := -pthread 

# Lista dei sorgenti: prende automaticamente tutti i .c nella cartella
SRCS := $(wildcard *.c)

# Lista degli oggetti .o corrispondenti (main.c -> main.o, ecc.)
OBJS := $(SRCS:.c=.o)

# Target di default (quello eseguito con "make")
all: $(TARGET)


# Link finale: crea l'eseguibile a partire dagli oggetti
# $@ = nome del target (qui: progetto_qsim)
# $^ = tutte le dipendenze (qui: tutti i .o) 
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)


# Regola generica di compilazione: come ottenere X.o da X.c
# $< = prima dipendenza (qui: il file .c)
# -c = compila senza linkare
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Pulizia: cancella eseguibile e .o
clean:
	rm -f $(OBJS) $(TARGET)

# Dice a make che "all" e "clean" non sono file veri, ma comandi.
.PHONY: all clean

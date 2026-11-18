CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lraylib -lm

FONTES = main.c perguntas.c
SAIDA = ../compilado/jogo

all:
	mkdir -p ../compilado
	$(CC) $(CFLAGS) $(FONTES) -o $(SAIDA) $(LIBS)

run: all
	$(SAIDA)

clean:
	rm -f $(SAIDA)
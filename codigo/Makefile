CC = gcc
CFLAGS = -Wall -Wextra

# Detecta o sistema: no Windows a raylib precisa de mais bibliotecas
ifeq ($(OS),Windows_NT)
    EXT = .exe
    CFLAGS += $(shell pkg-config --cflags raylib)
    LIBS = $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm
else
    EXT =
    LIBS = -lraylib -lm
endif

PASTA = ../compilado

# Jogo principal (menu de perguntas)
# Quando criar um .c novo do jogo, acrescente o nome dele aqui
FONTES = main.c perguntas.c jogo.c premio.c
JOGO = $(PASTA)/jogo$(EXT)

# Minigame Guitar Hero (tem main proprio, por isso compila separado)
GUITARHERO_FONTES = minigames/guitarhero/guitarhero.c
GUITARHERO = $(PASTA)/guitarhero$(EXT)

.PHONY: all run guitarhero run-guitarhero clean

all: $(JOGO)

$(JOGO): $(FONTES) $(wildcard *.h)
	mkdir -p $(PASTA)
	$(CC) $(CFLAGS) $(FONTES) -o $@ $(LIBS)

run: $(JOGO)
	$(JOGO)

guitarhero: $(GUITARHERO)

$(GUITARHERO): $(GUITARHERO_FONTES)
	mkdir -p $(PASTA)
	$(CC) -std=c99 $(CFLAGS) $(GUITARHERO_FONTES) -o $@ $(LIBS)

run-guitarhero: $(GUITARHERO)
	$(GUITARHERO)

clean:
	rm -f $(JOGO) $(GUITARHERO)

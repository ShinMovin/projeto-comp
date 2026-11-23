CC = gcc
CFLAGS ?= -Wall -Wextra
PKG_CONFIG ?= pkg-config

# Bibliotecas do sistema usadas pela raylib para desktop.
ifeq ($(OS),Windows_NT)
    EXT = .exe
    LIBS_SISTEMA = -lopengl32 -lgdi32 -lwinmm -lm
else
    EXT =
    LIBS_SISTEMA = -lGL -lm -lpthread -ldl -lrt -lX11
endif

PASTA = ../compilado
RAYLIB_SRC ?= ../compilado/raylib/src

# Usa a copia local se o codigo-fonte da raylib estiver disponivel.
ifneq ($(wildcard $(RAYLIB_SRC)/Makefile),)
    RAYLIB_ARQUIVO = $(RAYLIB_SRC)/libraylib.a
    RAYLIB_CFLAGS = -I"$(RAYLIB_SRC)"
    LIBS = "$(RAYLIB_ARQUIVO)" $(LIBS_SISTEMA)
else ifneq ($(shell $(PKG_CONFIG) --exists raylib 2>/dev/null && echo yes),)
    RAYLIB_CFLAGS = $(shell $(PKG_CONFIG) --cflags raylib)
    LIBS = $(shell $(PKG_CONFIG) --static --libs raylib 2>/dev/null || $(PKG_CONFIG) --libs raylib) -lm
else
    LIBS = -lraylib $(LIBS_SISTEMA)
endif

# Jogo principal (menu de perguntas)
# Quando criar um .c novo do jogo, acrescente o nome dele aqui
FONTES = main.c perguntas.c jogo.c premio.c interface.c
JOGO = $(PASTA)/jogo$(EXT)

# Minigame Guitar Hero (tem main proprio, por isso compila separado)
GUITARHERO_FONTES = minigames/guitarhero/guitarhero.c
GUITARHERO = $(PASTA)/guitarhero$(EXT)

.PHONY: all run guitarhero run-guitarhero clean

all: $(JOGO)

ifneq ($(RAYLIB_ARQUIVO),)
$(RAYLIB_ARQUIVO): private MAKEOVERRIDES =
$(RAYLIB_ARQUIVO): $(RAYLIB_SRC)/Makefile
	$(MAKE) -C "$(RAYLIB_SRC)" PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC CC="$(CC)" AR="$(AR)"
endif

$(JOGO): $(FONTES) $(wildcard *.h) Makefile $(RAYLIB_ARQUIVO)
	mkdir -p "$(PASTA)"
	$(CC) $(CPPFLAGS) $(RAYLIB_CFLAGS) $(CFLAGS) $(FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run: $(JOGO)
	$(JOGO)

guitarhero: $(GUITARHERO)

$(GUITARHERO): $(GUITARHERO_FONTES) Makefile $(RAYLIB_ARQUIVO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 $(CPPFLAGS) $(RAYLIB_CFLAGS) $(CFLAGS) $(GUITARHERO_FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run-guitarhero: $(GUITARHERO)
	$(GUITARHERO)

clean:
	rm -f $(JOGO) $(GUITARHERO)

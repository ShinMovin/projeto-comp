CC = gcc
CFLAGS ?= -Wall -Wextra
PKG_CONFIG ?= pkg-config

# No WSL, uname identifica o alvo Linux mesmo quando o ambiente vem do Windows.
ifeq ($(shell uname -s 2>/dev/null),Linux)
    PLATAFORMA = linux
    EXT =
    LIBS_SISTEMA = -lGL -lX11 -lm -lpthread -ldl -lrt
else ifeq ($(OS),Windows_NT)
    PLATAFORMA = windows
    EXT = .exe
    LIBS_SISTEMA = -lopengl32 -lgdi32 -lwinmm -lm
else
    $(error Sistema de compilação não reconhecido)
endif

PASTA = ../compilado
RAYLIB_SRC ?= ../compilado/raylib-$(PLATAFORMA)/src
RAYLIB_PREFIX ?=

# Uma cópia pré-compilada é opcional e deve ser da mesma plataforma do GCC.
ifneq ($(strip $(RAYLIB_PREFIX)),)
    RAYLIB_ARQUIVO = $(RAYLIB_PREFIX)/lib/libraylib.a
    RAYLIB_CFLAGS = -I"$(RAYLIB_PREFIX)/include"
    LIBS = "$(RAYLIB_ARQUIVO)" $(LIBS_SISTEMA)
else ifneq ($(wildcard $(RAYLIB_SRC)/Makefile),)
    RAYLIB_ARQUIVO = $(RAYLIB_SRC)/libraylib.a
    RAYLIB_CFLAGS = -I"$(RAYLIB_SRC)"
    LIBS = "$(RAYLIB_ARQUIVO)" $(LIBS_SISTEMA)
    COMPILAR_RAYLIB = sim
else ifneq ($(shell $(PKG_CONFIG) --exists raylib 2>/dev/null && echo yes),)
    RAYLIB_CFLAGS = $(shell $(PKG_CONFIG) --cflags raylib)
    LIBS = $(shell $(PKG_CONFIG) --static --libs raylib 2>/dev/null || $(PKG_CONFIG) --libs raylib) -lm
else
    LIBS = -lraylib $(LIBS_SISTEMA)
endif

# Jogo principal (menu de perguntas)
# Quando criar um .c novo do jogo, acrescente o nome dele aqui
FONTES = main.c perguntas.c jogo.c premio.c desenho.c interface.c roleta.c banco_arquivos.c interface_banco.c \
    interface_ajudas.c minigames/minigame.c $(GUITARHERO_FONTES) $(TIROALVO_FONTES)
CABECALHOS = $(wildcard *.h minigames/*.h minigames/*/*.h)
JOGO = $(PASTA)/jogo$(EXT)

# Os alvos independentes reaproveitam os mesmos módulos do jogo principal.
GUITARHERO_FONTES = minigames/guitarhero/guitarhero.c
GUITARHERO = $(PASTA)/guitarhero$(EXT)
GUITARHERO_AUDIO = $(addprefix $(PASTA)/guitarhero-musicas/, \
    harry-potter.wav blue-bird.wav sweden.wav)

# Minigame Tiro ao Alvo
TIROALVO_FONTES = minigames/tiroalvo/tiroalvo.c
TIROALVO = $(PASTA)/tiroalvo$(EXT)
TIROALVO_AUDIO = $(addprefix $(PASTA)/audio-tiroalvo/, \
    ambiente.wav acerto.wav master.mp3 eu-voce-o-mar-e-ela.mp3)
ifneq ($(wildcard minigames/tiroalvo/audio/musica-local.mp3),)
    TIROALVO_AUDIO += $(PASTA)/audio-tiroalvo/musica-local.mp3
endif

.PHONY: all run guitarhero run-guitarhero tiroalvo run-tiroalvo clean

all: $(JOGO)

ifeq ($(COMPILAR_RAYLIB),sim)
$(RAYLIB_ARQUIVO): private MAKEOVERRIDES =
$(RAYLIB_ARQUIVO): $(RAYLIB_SRC)/Makefile
	$(MAKE) -C "$(RAYLIB_SRC)" PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC CC="$(CC)" AR="$(AR)"
endif

$(JOGO): $(FONTES) $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(GUITARHERO_AUDIO) $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) $(CPPFLAGS) $(RAYLIB_CFLAGS) $(CFLAGS) $(FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run: $(JOGO)
	$(JOGO)

guitarhero: $(GUITARHERO)

$(GUITARHERO): $(GUITARHERO_FONTES) desenho.c $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(GUITARHERO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DGUITARHERO_INDEPENDENTE $(CPPFLAGS) $(RAYLIB_CFLAGS) $(CFLAGS) $(GUITARHERO_FONTES) desenho.c $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/guitarhero-musicas/%: minigames/guitarhero/guitarhero-musicas/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-guitarhero: $(GUITARHERO)
	$(GUITARHERO)

tiroalvo: $(TIROALVO)

$(TIROALVO): $(TIROALVO_FONTES) desenho.c $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DTIROALVO_INDEPENDENTE $(CPPFLAGS) $(RAYLIB_CFLAGS) $(CFLAGS) $(TIROALVO_FONTES) desenho.c $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/audio-tiroalvo/%: minigames/tiroalvo/audio/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-tiroalvo: $(TIROALVO)
	$(TIROALVO)

clean:
	rm -f $(JOGO) $(GUITARHERO) $(GUITARHERO_AUDIO) $(TIROALVO) $(TIROALVO_AUDIO)

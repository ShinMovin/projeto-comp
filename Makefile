# Jogo do Milhão — compila e roda no Linux.
#   make      compila o jogo em compilado/jogo
#   make run  compila (se precisar) e abre o jogo
CC = gcc
CFLAGS ?= -Wall -Wextra
PASTA = compilado

# A raylib é encontrada pelo pkg-config. Sem ele, usa a instalação padrão (/usr/local).
ifneq ($(shell pkg-config --exists raylib 2>/dev/null && echo sim),)
    RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
    LIBS = $(shell pkg-config --libs raylib) -lm
else
    LIBS = -lraylib -lGL -lX11 -lm -lpthread -ldl -lrt
endif

# Caminhos a partir da raiz.
INCLUDES_PROJETO = -Icodigo -Icodigo/interface -Icodigo/perguntas -Icodigo/jogo
OPCOES_C = $(CPPFLAGS) $(INCLUDES_PROJETO) $(RAYLIB_CFLAGS) $(CFLAGS)
DESENHO = codigo/interface/desenho.c
FONTES = codigo/main.c $(DESENHO) \
    codigo/interface/interface.c codigo/interface/interface_ajudas.c \
    codigo/perguntas/perguntas.c \
    codigo/jogo/jogo.c codigo/jogo/premio.c codigo/jogo/roleta.c \
    codigo/minigames/minigame.c $(GUITARHERO_FONTES) $(TIROALVO_FONTES) $(TRAVESSIA_FONTES)
CABECALHOS = $(wildcard codigo/*.h codigo/interface/*.h codigo/perguntas/*.h codigo/jogo/*.h codigo/minigames/*.h codigo/minigames/*/*.h)
JOGO = $(PASTA)/jogo

# Minigame Guitar Hero
GUITARHERO_FONTES = codigo/minigames/guitarhero/guitarhero.c
GUITARHERO_AUDIO = $(addprefix $(PASTA)/guitarhero-musicas/, \
    harry-potter.wav blue-bird.wav sweden.wav)

# Minigame Tiro ao Alvo
TIROALVO_FONTES = codigo/minigames/tiroalvo/tiroalvo.c
TIROALVO_AUDIO = $(addprefix $(PASTA)/audio-tiroalvo/, \
    ambiente.wav acerto.wav master.mp3 eu-voce-o-mar-e-ela.mp3)
ifneq ($(wildcard codigo/minigames/tiroalvo/audio/musica-local.mp3),)
    TIROALVO_AUDIO += $(PASTA)/audio-tiroalvo/musica-local.mp3
endif

# Minigame Travessia
TRAVESSIA_FONTES = codigo/minigames/travessia/travessia.c

.PHONY: all run clean

all: $(JOGO)

$(JOGO): $(FONTES) $(CABECALHOS) Makefile | $(GUITARHERO_AUDIO) $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) $(OPCOES_C) $(FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run: $(JOGO)
	"./$(JOGO)"

$(PASTA)/guitarhero-musicas/%: codigo/minigames/guitarhero/guitarhero-musicas/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

$(PASTA)/audio-tiroalvo/%: codigo/minigames/tiroalvo/audio/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

clean:
	rm -f $(JOGO) $(GUITARHERO_AUDIO) $(TIROALVO_AUDIO)

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

# Caminhos a partir da raiz, também usados pelos alvos independentes.
INCLUDES_PROJETO = -Icodigo -Icodigo/interface -Icodigo/perguntas -Icodigo/jogo
OPCOES_C = $(CPPFLAGS) $(INCLUDES_PROJETO) $(RAYLIB_CFLAGS) $(CFLAGS)
DESENHO = codigo/interface/desenho.c
FONTES = codigo/main.c $(DESENHO) \
    codigo/interface/interface.c codigo/interface/interface_ajudas.c codigo/interface/interface_banco.c \
    codigo/perguntas/perguntas.c codigo/perguntas/banco_arquivos.c \
    codigo/jogo/jogo.c codigo/jogo/premio.c codigo/jogo/roleta.c \
    codigo/minigames/minigame.c $(GUITARHERO_FONTES) $(TIROALVO_FONTES) $(TRAVESSIA_FONTES)
CABECALHOS = $(wildcard codigo/*.h codigo/interface/*.h codigo/perguntas/*.h codigo/jogo/*.h codigo/minigames/*.h codigo/minigames/*/*.h)
JOGO = $(PASTA)/jogo

# Os alvos independentes reaproveitam os mesmos módulos do jogo principal.
GUITARHERO_FONTES = codigo/minigames/guitarhero/guitarhero.c
GUITARHERO = $(PASTA)/guitarhero
GUITARHERO_AUDIO = $(addprefix $(PASTA)/guitarhero-musicas/, \
    harry-potter.wav blue-bird.wav sweden.wav)

# Minigame Tiro ao Alvo
TIROALVO_FONTES = codigo/minigames/tiroalvo/tiroalvo.c
TIROALVO = $(PASTA)/tiroalvo
TIROALVO_AUDIO = $(addprefix $(PASTA)/audio-tiroalvo/, \
    ambiente.wav acerto.wav master.mp3 eu-voce-o-mar-e-ela.mp3)
ifneq ($(wildcard codigo/minigames/tiroalvo/audio/musica-local.mp3),)
    TIROALVO_AUDIO += $(PASTA)/audio-tiroalvo/musica-local.mp3
endif

# Minigame Travessia
TRAVESSIA_FONTES = codigo/minigames/travessia/travessia.c
TRAVESSIA = $(PASTA)/travessia

.PHONY: all run guitarhero run-guitarhero tiroalvo run-tiroalvo travessia run-travessia clean

all: $(JOGO)

$(JOGO): $(FONTES) $(CABECALHOS) Makefile | $(GUITARHERO_AUDIO) $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) $(OPCOES_C) $(FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run: $(JOGO)
	"./$(JOGO)"

guitarhero: $(GUITARHERO)

$(GUITARHERO): $(GUITARHERO_FONTES) $(DESENHO) $(CABECALHOS) Makefile | $(GUITARHERO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DGUITARHERO_INDEPENDENTE $(OPCOES_C) $(GUITARHERO_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/guitarhero-musicas/%: codigo/minigames/guitarhero/guitarhero-musicas/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-guitarhero: $(GUITARHERO)
	"./$(GUITARHERO)"

tiroalvo: $(TIROALVO)

$(TIROALVO): $(TIROALVO_FONTES) $(DESENHO) $(CABECALHOS) Makefile | $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DTIROALVO_INDEPENDENTE $(OPCOES_C) $(TIROALVO_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/audio-tiroalvo/%: codigo/minigames/tiroalvo/audio/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-tiroalvo: $(TIROALVO)
	"./$(TIROALVO)"

travessia: $(TRAVESSIA)

$(TRAVESSIA): $(TRAVESSIA_FONTES) $(DESENHO) $(CABECALHOS) Makefile
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DTRAVESSIA_INDEPENDENTE $(OPCOES_C) $(TRAVESSIA_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run-travessia: $(TRAVESSIA)
	"./$(TRAVESSIA)"

clean:
	rm -f $(JOGO) $(GUITARHERO) $(GUITARHERO_AUDIO) $(TIROALVO) $(TIROALVO_AUDIO) $(TRAVESSIA)

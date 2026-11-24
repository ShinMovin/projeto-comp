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

PASTA = compilado
RAYLIB_SRC ?= $(PASTA)/raylib-$(PLATAFORMA)/src
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
JOGO = $(PASTA)/jogo$(EXT)

# Os alvos independentes reaproveitam os mesmos módulos do jogo principal.
GUITARHERO_FONTES = codigo/minigames/guitarhero/guitarhero.c
GUITARHERO = $(PASTA)/guitarhero$(EXT)
GUITARHERO_AUDIO = $(addprefix $(PASTA)/guitarhero-musicas/, \
    harry-potter.wav blue-bird.wav sweden.wav)

# Minigame Tiro ao Alvo
TIROALVO_FONTES = codigo/minigames/tiroalvo/tiroalvo.c
TIROALVO = $(PASTA)/tiroalvo$(EXT)
TIROALVO_AUDIO = $(addprefix $(PASTA)/audio-tiroalvo/, \
    ambiente.wav acerto.wav master.mp3 eu-voce-o-mar-e-ela.mp3)
ifneq ($(wildcard codigo/minigames/tiroalvo/audio/musica-local.mp3),)
    TIROALVO_AUDIO += $(PASTA)/audio-tiroalvo/musica-local.mp3
endif

# Minigame Travessia
TRAVESSIA_FONTES = codigo/minigames/travessia/travessia.c
TRAVESSIA = $(PASTA)/travessia$(EXT)

.PHONY: all run guitarhero run-guitarhero tiroalvo run-tiroalvo travessia run-travessia clean

all: $(JOGO)

ifeq ($(COMPILAR_RAYLIB),sim)
$(RAYLIB_ARQUIVO): private MAKEOVERRIDES =
$(RAYLIB_ARQUIVO): $(RAYLIB_SRC)/Makefile
	$(MAKE) -C "$(RAYLIB_SRC)" PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC CC="$(CC)" AR="$(AR)"
endif

$(JOGO): $(FONTES) $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(GUITARHERO_AUDIO) $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) $(OPCOES_C) $(FONTES) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run: $(JOGO)
	"./$(JOGO)"

guitarhero: $(GUITARHERO)

$(GUITARHERO): $(GUITARHERO_FONTES) $(DESENHO) $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(GUITARHERO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DGUITARHERO_INDEPENDENTE $(OPCOES_C) $(GUITARHERO_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/guitarhero-musicas/%: codigo/minigames/guitarhero/guitarhero-musicas/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-guitarhero: $(GUITARHERO)
	"./$(GUITARHERO)"

tiroalvo: $(TIROALVO)

$(TIROALVO): $(TIROALVO_FONTES) $(DESENHO) $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO) | $(TIROALVO_AUDIO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DTIROALVO_INDEPENDENTE $(OPCOES_C) $(TIROALVO_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

$(PASTA)/audio-tiroalvo/%: codigo/minigames/tiroalvo/audio/%
	mkdir -p "$(@D)"
	cp "$<" "$@"

run-tiroalvo: $(TIROALVO)
	"./$(TIROALVO)"

travessia: $(TRAVESSIA)

$(TRAVESSIA): $(TRAVESSIA_FONTES) $(DESENHO) $(CABECALHOS) Makefile $(RAYLIB_ARQUIVO)
	mkdir -p "$(PASTA)"
	$(CC) -std=c99 -DTRAVESSIA_INDEPENDENTE $(OPCOES_C) $(TRAVESSIA_FONTES) $(DESENHO) $(LDFLAGS) -o "$@" $(LIBS) $(LDLIBS)

run-travessia: $(TRAVESSIA)
	"./$(TRAVESSIA)"

clean:
	rm -f $(JOGO) $(GUITARHERO) $(GUITARHERO_AUDIO) $(TIROALVO) $(TIROALVO_AUDIO) $(TRAVESSIA)

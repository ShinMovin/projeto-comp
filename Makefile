
# Compilador
CC = gcc

# Arquivo do Guitar Hero
SRC = codigo/minigames/guitarhero/guitarhero.c

# Nome do executavel
OUT = guitarhero.exe

# Configuracoes de compilacao
CFLAGS = -std=c99 -Wall -Wextra $(shell pkg-config --cflags raylib)

# Bibliotecas necessarias
LIBS = $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm

# Compila o jogo
all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

# Executa o jogo
run: all
	./$(OUT)

# Remove o executavel
clean:
	rm -f $(OUT)

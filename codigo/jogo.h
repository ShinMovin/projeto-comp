#ifndef JOGO_H
#define JOGO_H
#include "perguntas.h"

typedef enum { SORTEA, PERGUNTA, ACERTOU, ERROU, FIM} Estado;

typedef struct
{
    Estado estado;
    int rodada;
    int dificuldade;
    int categoria;
    int indicePergunta;
    int premio;
    char resposta;
}Jogo;

void jogar(BancoPerguntas *banco);

#endif
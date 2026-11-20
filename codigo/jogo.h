/**
 * @file jogo.h
 * @author VGA
 * @brief Estados e struct da partida.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
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
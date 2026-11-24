#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdbool.h>

typedef enum { SEM_MINIGAME, GUITAR_HERO, TRAVESSIA, TIRO_AO_ALVO, QUANTIDADE_MINIGAMES } TipoMinigame;
typedef enum { MINIGAME_EM_CURSO, MINIGAME_VITORIA, MINIGAME_DERROTA, MINIGAME_ABANDONADO } ResultadoMinigame;

typedef struct
{
    int pontos;
    int acertos;
    int notas;
} PlacarMinigame;

// A janela, o loop e o dispositivo de áudio pertencem ao aplicativo.
typedef struct
{
    const char *(*nomeOpcao)(bool treino, int opcao);
    void *(*iniciar)(bool treino, int opcao);
    ResultadoMinigame (*atualizar)(void *sessao, float tempoQuadro);
    void (*desenhar)(const void *sessao);
    PlacarMinigame (*placar)(const void *sessao);
    void (*encerrar)(void *sessao);
} Minigame;

const Minigame *obterMinigame(TipoMinigame tipo);

#endif

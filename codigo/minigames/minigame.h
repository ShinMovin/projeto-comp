#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdbool.h>

typedef enum { SEM_MINIGAME, GUITAR_HERO, TRAVESSIA, TIRO_AO_ALVO, QUANTIDADE_MINIGAMES } TipoMinigame;
typedef enum { MINIGAME_EM_CURSO, MINIGAME_VITORIA, MINIGAME_DERROTA, MINIGAME_ABANDONADO } ResultadoMinigame;

// A tela de resultado usa pontos em qualquer jogo e também notas e acertos no Guitar Hero.
typedef struct
{
    int pontos;
    int acertos;
    int notas;
} PlacarMinigame;

bool minigameDisponivel(TipoMinigame tipo);
/** @brief Nome da faixa para o modo e índice escolhidos. NULL indica ausência de opção. */
const char *nomeOpcaoMinigame(TipoMinigame tipo, bool treino, int opcao);
/** @brief Cria a sessão do tipo escolhido. Retorna NULL se não conseguiu iniciar. */
void *iniciarMinigame(TipoMinigame tipo, bool treino, int opcao);
/** @brief Passa o tempo em segundos à sessão e devolve o resultado de seu último quadro. */
ResultadoMinigame atualizarMinigame(TipoMinigame tipo, void *sessao, float segundos);
void desenharMinigame(TipoMinigame tipo, const void *sessao);
PlacarMinigame placarMinigame(TipoMinigame tipo, const void *sessao);
/** @brief Libera a sessão e seus recursos. A tela deve zerar o ponteiro em seguida. */
void encerrarMinigame(TipoMinigame tipo, void *sessao);

#endif

#ifndef INTERFACE_AJUDAS_H
#define INTERFACE_AJUDAS_H

#include "jogo.h"

typedef enum { DESAFIOS_FECHADOS, DESAFIOS_CARTOES, DESAFIOS_INSTRUCOES, DESAFIOS_JOGANDO, DESAFIOS_RESULTADO } TelaDesafios;

// Guarda a tela do desafio, o tipo escolhido e sua sessão.
// O placar continua disponível na tela de resultado mesmo depois de liberar a sessão.
typedef struct
{
    TelaDesafios tela;
    TipoMinigame tipo;
    int opcao;
    void *sessao;
    ResultadoMinigame resultado;
    PlacarMinigame placar;
    bool recompensa;
    bool erroAoIniciar;
    float tempoResultado;
} Desafios;

int contarAjudasDisponiveis(const Jogo *jogo);
void abrirDesafios(Desafios *desafios);
void liberarDesafios(Desafios *desafios);
/** @brief Atualiza o desafio. "jogo" e "pergunta" são NULL no modo de treino. */
void atualizarDesafios(Desafios *desafios, Jogo *jogo, const Pergunta *pergunta, float tempoQuadro);
/** @brief Desenha a etapa do desafio e retorna true quando o jogador pede para sair. */
bool desenharDesafios(Desafios *desafios, Jogo *jogo);

#endif

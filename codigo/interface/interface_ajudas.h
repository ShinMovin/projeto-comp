#ifndef INTERFACE_AJUDAS_H
#define INTERFACE_AJUDAS_H

#include "jogo.h"

typedef enum { DESAFIOS_FECHADOS, DESAFIOS_CARTOES, DESAFIOS_INSTRUCOES, DESAFIOS_JOGANDO, DESAFIOS_RESULTADO } TelaDesafios;

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
void atualizarDesafios(Desafios *desafios, Jogo *jogo, const Pergunta *pergunta, float tempoQuadro);
bool desenharDesafios(Desafios *desafios, Jogo *jogo);

#endif

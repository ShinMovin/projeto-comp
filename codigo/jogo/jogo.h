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
#include "minigames/minigame.h"

typedef enum
{
    SORTEA, NOVO_NIVEL, PERGUNTA, CONFIRMANDO, SUSPENSE, ACERTOU, ERROU, VITORIA, DERROTA
} Estado;

typedef struct
{
    Estado estado;
    int rodada;
    Dificuldade dificuldade;
    Categoria categoria;
    int indicePergunta;
    int premio;
    int premioGarantido;
    int ultimoCheckpoint; // Número exibido da pergunta. Zero antes do primeiro checkpoint.
    int premioPerdido;
    bool segundaChance;
    char respostaBloqueada;
    bool eliminadas[4];
    bool ajudasUsadas[QUANTIDADE_MINIGAMES];
    TipoMinigame ajudaAtiva;
    char resposta;
    float tempoEstado;
} Jogo;

/** @brief Inicia uma partida e libera as perguntas usadas na partida anterior. */
void iniciarJogo(Jogo *jogo, BancoPerguntas *banco);

/** @brief Carrega uma pergunta inédita da categoria e da dificuldade da rodada. */
bool selecionarPergunta(Jogo *jogo, BancoPerguntas *banco, Categoria categoria);

void escolherResposta(Jogo *jogo, char alternativa);
bool alternativaBloqueada(const Jogo *jogo, char alternativa);
void cancelarResposta(Jogo *jogo);
void confirmarResposta(Jogo *jogo);
void atualizarJogo(Jogo *jogo, const BancoPerguntas *banco, float tempoQuadro);
bool avancarRodada(Jogo *jogo);
bool ajudaDisponivel(const Jogo *jogo, TipoMinigame tipo);
bool iniciarAjuda(Jogo *jogo, TipoMinigame tipo);
bool concluirAjuda(Jogo *jogo, const Pergunta *pergunta, ResultadoMinigame resultado);

#endif

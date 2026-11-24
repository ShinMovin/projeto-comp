/**
 * @file jogo.c
 * @author VGA
 * @brief Estado da partida, rodadas, respostas e premiação.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "jogo.h"
#include "premio.h"
#include <stdlib.h>

static const float duracaoSuspense = 1.15f;
static const float duracaoRevelacao = 1.55f;

static void conquistarRodada(Jogo *jogo)
{
    jogo->premio = valorRodada(jogo->rodada);
    if (rodadaTemCheckpoint(jogo->rodada))
    {
        jogo->premioGarantido = jogo->premio;
        jogo->ultimoCheckpoint = jogo->rodada + 1;
    }
    jogo->estado = ACERTOU;
}

void iniciarJogo(Jogo *jogo, BancoPerguntas *banco)
{
    *jogo = (Jogo){.estado = SORTEA, .dificuldade = dificuldadeRodada(0), .indicePergunta = -1};

    if (!banco)
        return;
    for (int i = 0; i < banco->quantidade; i++)
    {
        banco->usadas[i] = false;
    }
}

bool selecionarPergunta(Jogo *jogo, BancoPerguntas *banco, Categoria categoria)
{
    if (jogo->estado != SORTEA || !banco)
        return false;

    int indice = sorteaPergunta(banco, jogo->dificuldade, categoria);
    if (indice < 0)
        return false;

    jogo->categoria = categoria;
    jogo->indicePergunta = indice;
    jogo->estado = PERGUNTA;
    return true;
}

bool alternativaBloqueada(const Jogo *jogo, char alternativa)
{
    return alternativa < 'A' || alternativa > 'D' ||
           alternativa == jogo->respostaBloqueada || jogo->eliminadas[alternativa - 'A'];
}

void escolherResposta(Jogo *jogo, char alternativa)
{
    if (jogo->estado == PERGUNTA && jogo->ajudaAtiva == SEM_MINIGAME &&
        !alternativaBloqueada(jogo, alternativa))
    {
        jogo->resposta = alternativa;
        jogo->estado = CONFIRMANDO;
    }
}

void cancelarResposta(Jogo *jogo)
{
    if (jogo->estado == CONFIRMANDO)
    {
        jogo->resposta = '\0';
        jogo->estado = PERGUNTA;
    }
}

void confirmarResposta(Jogo *jogo)
{
    if (jogo->estado == CONFIRMANDO)
    {
        jogo->tempoEstado = 0.0f;
        jogo->estado = SUSPENSE;
    }
}

static void julgarResposta(Jogo *jogo, const Pergunta *pergunta)
{
    jogo->tempoEstado = 0.0f;
    if (jogo->resposta == pergunta->alternativa_correta)
        conquistarRodada(jogo);
    else if (jogo->segundaChance)
    {
        // O primeiro erro protegido bloqueia só a letra escolhida e mantém a rodada.
        jogo->segundaChance = false;
        jogo->respostaBloqueada = jogo->resposta;
        jogo->resposta = '\0';
        jogo->estado = PERGUNTA;
    }
    else
    {
        jogo->premioPerdido = jogo->premio - jogo->premioGarantido;
        jogo->premio = jogo->premioGarantido;
        jogo->estado = ERROU;
    }
}

void atualizarJogo(Jogo *jogo, const BancoPerguntas *banco, float tempoQuadro)
{
    if (jogo->estado == SUSPENSE)
    {
        jogo->tempoEstado += tempoQuadro;
        if (jogo->tempoEstado >= duracaoSuspense)
            julgarResposta(jogo, &banco->perguntas[jogo->indicePergunta]);
    }
    else if (jogo->estado == ERROU || (jogo->estado == ACERTOU && jogo->rodada == TOTAL_RODADAS - 1))
    {
        jogo->tempoEstado += tempoQuadro;
        if (jogo->tempoEstado >= duracaoRevelacao)
        {
            jogo->tempoEstado = 0.0f;
            jogo->estado = jogo->estado == ACERTOU ? VITORIA : DERROTA;
        }
    }
    else if (jogo->estado == DERROTA || jogo->estado == NOVO_NIVEL)
        jogo->tempoEstado += tempoQuadro;
}

bool avancarRodada(Jogo *jogo)
{
    if (jogo->estado != ACERTOU || jogo->rodada == TOTAL_RODADAS - 1)
        return false;

    jogo->rodada++;
    jogo->dificuldade = dificuldadeRodada(jogo->rodada);
    jogo->indicePergunta = -1;
    jogo->categoria = 0;
    jogo->resposta = '\0';
    jogo->respostaBloqueada = '\0';
    jogo->segundaChance = false;
    for (int i = 0; i < 4; i++)
        jogo->eliminadas[i] = false;
    jogo->tempoEstado = 0.0f;
    // A roleta do próximo nível só abre depois que o jogador confirma o checkpoint.
    jogo->estado = rodadaTemCheckpoint(jogo->rodada - 1) ? NOVO_NIVEL : SORTEA;
    return true;
}

bool ajudaDisponivel(const Jogo *jogo, TipoMinigame tipo)
{
    return tipo > SEM_MINIGAME && tipo < QUANTIDADE_MINIGAMES && jogo->estado == PERGUNTA &&
           jogo->ajudaAtiva == SEM_MINIGAME && !jogo->ajudasUsadas[tipo] &&
           (tipo != GUITAR_HERO || jogo->rodada < TOTAL_RODADAS - 1);
}

bool iniciarAjuda(Jogo *jogo, TipoMinigame tipo)
{
    if (!ajudaDisponivel(jogo, tipo))
        return false;
    jogo->ajudasUsadas[tipo] = true;
    jogo->ajudaAtiva = tipo;
    return true;
}

static bool eliminarIncorretas(Jogo *jogo, const Pergunta *pergunta)
{
    int candidatas[4], total = 0;
    for (int i = 0; i < 4; i++)
    {
        char alternativa = (char)('A' + i);
        if (alternativa != pergunta->alternativa_correta && !alternativaBloqueada(jogo, alternativa))
            candidatas[total++] = i;
    }
    if (total < 2)
        return false;
    // Retira cada índice sorteado da lista, evitando eliminar a mesma letra duas vezes.
    for (int i = 0; i < 2; i++)
    {
        int sorteada = rand() % total;
        jogo->eliminadas[candidatas[sorteada]] = true;
        candidatas[sorteada] = candidatas[--total];
    }
    return true;
}

bool concluirAjuda(Jogo *jogo, const Pergunta *pergunta, ResultadoMinigame resultado)
{
    if (jogo->ajudaAtiva == SEM_MINIGAME || resultado == MINIGAME_EM_CURSO)
        return false;
    TipoMinigame tipo = jogo->ajudaAtiva;
    jogo->ajudaAtiva = SEM_MINIGAME;
    if (resultado != MINIGAME_VITORIA || jogo->estado != PERGUNTA)
        return false;

    switch (tipo)
    {
    case GUITAR_HERO:
        if (jogo->rodada >= TOTAL_RODADAS - 1)
            return false;
        conquistarRodada(jogo);
        return true;
    case CROSSY_ROAD:
        jogo->segundaChance = true;
        return true;
    case TIRO_AO_ALVO:
        return pergunta && eliminarIncorretas(jogo, pergunta);
    default:
        return false;
    }
}

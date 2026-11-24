/**
 * @file premio.h
 * @author VGA
 * @brief Funcoes de dificuldade por rodada e premio.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef PREMIO_H
#define PREMIO_H
#include "perguntas.h"

enum { RODADAS_POR_NIVEL = 3, TOTAL_RODADAS = RODADAS_POR_NIVEL * IMPOSSIVEL };

/**
 * @brief Dificuldade base de uma rodada.
 * @param rodada rodada de 0 a 14
 * @return dificuldade de 1 a 5
 */
Dificuldade dificuldadeRodada(int rodada);

/** @brief Valor da pergunta na rodada indicada, numerada de 0 a 14. */
int valorRodada(int rodada);

/**
 * @brief Calcula o prêmio da rodada (índice de 0 a 14).
 * @param bonusAcumulado soma dos bônus concedidos pelos acertos da partida
 * @return valor da rodada mais o bônus, em reais, limitado a R$ 1 milhão
 */
int premioComBonus(int rodada, int bonusAcumulado);

/** @brief Indica se concluir esta rodada (índice de 0 a 14) garante um prêmio. */
bool rodadaTemCheckpoint(int rodada);

/** @brief Resultados possíveis da roleta de dificuldade. */
typedef enum { DIF_DIMINUIR, DIF_MANTER, DIF_AUMENTAR, QUANTIDADE_OPCOES_DIF } OpcaoDificuldade;

/**
 * @brief Chances, em porcentagem, de cada opção da roleta de dificuldade.
 * Diminuir começa em 10% e cai até 3%; aumentar sobe de 15% até 50%.
 * @param rodada rodada de 0 a 14
 * @param chances vetor preenchido com DIF_DIMINUIR, DIF_MANTER e DIF_AUMENTAR (soma 100)
 */
void chancesDificuldade(int rodada, int chances[QUANTIDADE_OPCOES_DIF]);

/**
 * @brief Bônus por acertar com a dificuldade aumentada: metade do valor da rodada por nível acima.
 * @return zero se a dificuldade não foi aumentada
 */
int bonusDificuldade(int rodada, Dificuldade base, Dificuldade final);

#endif

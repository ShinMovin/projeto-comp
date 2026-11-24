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

/** @brief Indica se concluir esta rodada (índice de 0 a 14) garante um prêmio. */
bool rodadaTemCheckpoint(int rodada);

#endif

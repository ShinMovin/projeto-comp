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

/**
 * @brief Valor de uma pergunta, usando a maior entre as duas dificuldades.
 * @param difBase dificuldade da rodada
 * @param difFinal dificuldade depois da roleta
 * @return valor em reais
 */
int valorPergunta(int difBase, int difFinal);

/**
 * @brief Dificuldade base de uma rodada.
 * @param rodada rodada de 0 a 14
 * @return dificuldade de 1 a 5
 */
int dificuldadeRodada(int rodada);

#endif
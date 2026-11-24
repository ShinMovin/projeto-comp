/**
 * @file premio.c
 * @author VGA
 * @brief Tabelas de dificuldade por rodada e valor do premio.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "premio.h"

static const int valoresRodadas[TOTAL_RODADAS] = {
    1000, 2000, 5000,
    10000, 15000, 20000,
    30000, 40000, 50000,
    75000, 100000, 200000,
    300000, 500000, 1000000
};

Dificuldade dificuldadeRodada(int rodada)
{
    return (Dificuldade)(MUITO_FACIL + rodada / RODADAS_POR_NIVEL);
}

int valorRodada(int rodada)
{
    return rodada >= 0 && rodada < TOTAL_RODADAS ? valoresRodadas[rodada] : 0;
}

void chancesDificuldade(int rodada, int chances[QUANTIDADE_OPCOES_DIF])
{
    // Interpola da primeira rodada (10/75/15) até a última (3/47/50).
    // Diminuir é raro e fica mais raro; aumentar fica mais provável perto do milhão.
    int ultima = TOTAL_RODADAS - 1;
    chances[DIF_DIMINUIR] = 10 - 7 * rodada / ultima;
    chances[DIF_AUMENTAR] = 15 + 35 * rodada / ultima;
    chances[DIF_MANTER] = 100 - chances[DIF_DIMINUIR] - chances[DIF_AUMENTAR];
}

int bonusDificuldade(int rodada, Dificuldade base, Dificuldade final)
{
    if (final <= base)
        return 0;
    return valorRodada(rodada) / 2 * (final - base);
}

bool rodadaTemCheckpoint(int rodada)
{
    return rodada >= 0 && rodada < TOTAL_RODADAS - 1 &&
           (rodada + 1) % RODADAS_POR_NIVEL == 0;
}

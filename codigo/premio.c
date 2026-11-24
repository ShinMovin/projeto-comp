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

bool rodadaTemCheckpoint(int rodada)
{
    return rodada >= 0 && rodada < TOTAL_RODADAS - 1 &&
           (rodada + 1) % RODADAS_POR_NIVEL == 0;
}

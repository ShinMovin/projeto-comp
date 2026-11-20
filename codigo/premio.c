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

int dificuldadeBase[15] = {1,1, 2,2, 3,3,3,3,3, 4,4,4,4, 5,5};
int premioDificuldade[6] = {0, 5000, 10000, 50000, 100000, 300000};

int valorPergunta(int difBase, int difFinal)
{
    if(difBase >= difFinal)
    {
        return premioDificuldade[difBase];
    }
    else
    {
        return premioDificuldade[difFinal];
    }
}

int dificuldadeRodada(int rodada)
{
    return dificuldadeBase[rodada];
}
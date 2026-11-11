#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void liberaPerguntas(BancoPerguntas *banco)
{
    free(&banco);
}

int carregaPerguntas(BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq;
    Pergunta *perguntas_temp;
    

}
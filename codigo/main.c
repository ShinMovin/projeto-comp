#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "perguntas.h"
#include "raylib.h"
#include <time.h>
#include "jogo.h"

int main(){
    BancoPerguntas banco;
    Pergunta nova;
    int opcao;

    srand(time(NULL));//sorteio com resultados diferentes

    if(inicializaBanco(&banco) == 0) {
        return 1;
    }

    //tenta carregar o binario; se nao existir, carrega o csv.
    if(carregaBinario(&banco, "../dados/perguntas.dat") == 0)
    {
        if(carregaPerguntas(&banco, "../dados/perguntas.csv") == 0)
        {
            liberaBancoPergunta(&banco);
            return 1;
        }
        salvaBinario(&banco, "../dados/perguntas.dat");
    }
    
    do
    {
        printf("======= MENU =======\n");
        printf("1 - Listar perguntas\n2 - Cadastrar pergunta\n3 - Pesquisar pergunta\n4 - Alterar pergunta\n5 - Excluir pergunta\n6 - Exportar perguntas para CSV\n7 - Jogar\n0 - Sair\n");
        printf("====================\n");

        opcao = -1;

        printf("Opção :");
        scanf(" %d", &opcao);
        printf("\n");

        while(getchar() != '\n');

        switch (opcao)
        {
        case 1:
            listaPergunta(&banco);
            break;

        case 2:
            lerPergunta(&nova);
            if(adicionaPerguntas(&banco, &nova) == 1)
            {
                salvaBinario(&banco, "../dados/perguntas.dat");
            }
            break;

        case 3:
            pesquisaPergunta(&banco);
            break;

        case 4:
            if(alteraPergunta(&banco) == 1)
            {
                salvaBinario(&banco, "../dados/perguntas.dat");
            }
            break;

        case 5:
            if(excluiPergunta(&banco) == 1)
            {
                salvaBinario(&banco, "../dados/perguntas.dat");
            }
            break;

        case 6:
            if(salvaPergunta(&banco, "../dados/perguntas_exportadas.csv") == 1)
            {
                printf("Perguntas exportadas para dados/perguntas_exportadas.csv\n");
            }
            break;

        case 7:
            jogar(&banco);
            break;

        case 0:
            printf("Saindo do menu... !\n");
            break;
        
        default:
            printf("Opção invalida !\n");
            break;
        }
    } while (opcao != 0);
    

    liberaBancoPergunta(&banco);

    return 0;
}
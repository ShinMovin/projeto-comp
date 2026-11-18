#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "perguntas.h"
#include "raylib.h"
#include <time.h>

int main(){
    BancoPerguntas banco;
    Pergunta nova;
    int opcao;

    srand(time(NULL));//sorteio com resultados diferentes

    if(inicializaBanco(&banco) == 0) {
        return 1;
    }

    //teste pra ver se ta funcionando a função
    if(carregaPerguntas(&banco, "../dados/perguntas.csv") == 0){
        liberaBancoPergunta(&banco);
        return 1;
    }

    do
    {
        printf(" ====== MENU ======\n");
        printf("1 - Listar perguntas\n2 - Cadastrar pergunta\n3 - Pesquisar pergunta\n4 - Alterar pergunta\n5 - Excluir pergunta\n0 - Sair\n");
        
        opcao = -1;

        printf("Opção :");
        scanf(" %d", &opcao);

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
                salvaPergunta(&banco, "../dados/perguntas.csv");
            }
            break;

        case 3:
            pesquisaPergunta(&banco);
            break;

        case 4:
            if(alteraPergunta(&banco) == 1)
            {
                salvaPergunta(&banco, "../dados/perguntas.csv");
            }
            break;

        case 5:
            if(excluiPergunta(&banco) == 1)
            {
                salvaPergunta(&banco, "../dados/perguntas.csv");
            }
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
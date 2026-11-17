#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int inicializaBanco(BancoPerguntas *banco){
    (*banco).quantidade = 0;
    (*banco).capacidade = 5;
    (*banco).perguntas = calloc(5, sizeof(Pergunta));
    (*banco).usadas = calloc(5, sizeof(bool)); //bool serve para armazenar apenas 0 e 1 (falso/verdadeiro)

    //testando se deu erro de alocação
    if((*banco).perguntas == NULL || (*banco).usadas == NULL){
        //liberando memória 
        free((*banco).perguntas);
        free((*banco).usadas);
        //limpando ponteiro
        ((*banco).perguntas) = NULL;
        ((*banco).usadas) = NULL;

        return 0; //se deu erro retorna 0
    }

    return 1; //indica que deu certo
}

void liberaBancoPergunta(BancoPerguntas *banco){
    //liberando memória
    free((*banco).perguntas);
    free((*banco).usadas);
    //limpando ponteiro
    (*banco).perguntas = NULL;
    (*banco).usadas = NULL;
    (*banco).quantidade = 0;
    (*banco).capacidade = 0;
}

int carregaPerguntas(BancoPerguntas *banco, const char *nome_arq){
    FILE *arq;
    
    arq = fopen(nome_arq, "r");
    if(arq == NULL){
        perror("Não foi possível abrir arquivo.");
        return 0;
    }//if

    char linha[1200];                  //armazena temporariamente uma linha de perguntas.csv
    fgets(linha, sizeof(linha), arq);  //serve para ignorar o cabeçalho de perguntas.csv

    while(fgets(linha, sizeof(linha), arq) != NULL){
        Pergunta pergunta;
        int j = 0, i = 0;

        if((*banco).quantidade == (*banco).capacidade){   //verifica se o tamanho do vetor é suficiente para armazenar mais perguntas
            int novaCapacidade = (*banco).capacidade * 2;

            Pergunta *temp = realloc((*banco).perguntas, novaCapacidade * sizeof(Pergunta));
            if(temp == NULL){
                printf("Erro ao aumentar memoria!\n");
                fclose(arq);
                return 0;
            }//if

            (*banco).perguntas = temp;

            bool *tempUsadas = realloc((*banco).usadas,novaCapacidade * sizeof(bool));
            if(tempUsadas == NULL){
                printf("Erro ao aumentar memoria!\n");
                fclose(arq);
                return 0;
            }//if

            (*banco).usadas = tempUsadas;

            for(int i = (*banco).capacidade; i < novaCapacidade; i++){
                (*banco).usadas[i] = false;
            }//for

            (*banco).capacidade = novaCapacidade;

        }//if

        for(; linha[i] != '\0' && linha[i] != ',' && j < 200 ; i++){
            pergunta.conteudo_da_pergunta[j] = linha[i];
            j++;
        }
        pergunta.conteudo_da_pergunta[j] = '\0';

        if(j == 0 || linha[i] != ','){
            //verificando se tá tudo errado
            printf("Erro: enunciado vazio, muito longo ou incompleto!\n");
            fclose(arq);
            return 0;
        }

        for(int n = 0; n < 4; n++){
            //verificando se tem o separador
            if(linha[i] != ','){
                printf("Erro: linha do CSV incompleta!\n");
                fclose(arq);
                return 0;
            }//if

            i++;    // pula o ',' anterior
            j = 0;  // reinicia a posição do caracter

            for(; linha[i] != '\0' && linha[i] != ';' && j < 200; i++){  //já verifica se ultrapassa o tamanho que colocou pras perguntas e alternativas
                pergunta.alternativas[n][j] = linha[i];
                j++;
            }//for

            pergunta.alternativas[n][j] = '\0';

            //verificando se tá tudo errado
            if(j == 0 || linha[i] != ','){
                printf("Erro: alternativa vazia, muito longa ou incompleta!\n");
                fclose(arq);
                return 0;
            }//if
        }//for

        if(linha[i] != ','){
            printf("Erro: linha do CSV incompleta!\n");
            fclose(arq);
            return 0;
        }//if

        i++;  //pula o ';' depois da alternativa D
        //verificando se a alternativa tá no intervalo correto
        if(linha[i] < '0' || linha[i] > '3'){
            printf("Erro: alternativa correta fora do intervalo!\n");
            fclose(arq);
            return 0;
        }
        pergunta.alternativa_correta = linha[i] - '0';  //serve pra converter a string "char linha" em um int

        i ++;  //vai pro caracter ';' 
        //verificando se tem separador no lugar certo
        if(linha[i] != ','){
            printf("Erro: separador ausente!\n");
            fclose(arq);
            return 0;
        }

        i++; //pula o ';'
        //verificando se a dificuldade foi digitada certa
        if(linha[i] < '1' || linha[i] > '5'){
            printf("Erro: dificuldade invalida!\n");
            fclose(arq);
            return 0;
        }
        
        pergunta.dificuldades = (Dificuldade)(linha[i] - '0');  //serve pra converter a string "char linha" em um int

        i++;
        //verificando se tem algo depois da dificuldade
        if(linha[i] != '\n' && linha[i] != '\0'){
            printf("Erro: caracteres extras no final da linha!\n");
            fclose(arq);
            return 0;
        }//if

        //armazenando a pergunta que foi pega de "perguntas.csv"
        (*banco).perguntas[(*banco).quantidade] = pergunta;
        (*banco).quantidade++;

    }//while

    fclose(arq);
    return 1; //indica que deu certo   
}//carregaPerguntas

int sorteaPergunta(BancoPerguntas *banco, Dificuldade dificuldades)
{
    int disponiveis = 0;
    int sorteadas;

    //contar quantas perguntas podem ser utilizadas
    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        if(banco->perguntas[i].dificuldades == dificuldades && !banco->usadas[i])
        {
            disponiveis++;
        }
    }

    //nao possui perguntas disponiveis de determinada dificuldade
    if(disponiveis == 0)
    {
        return -1;
    }

    //sorteia das disponiveis
    sorteadas = rand() % disponiveis;

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        if(banco->perguntas[i].dificuldades == dificuldades && !banco->usadas[i])
        {
            if(sorteadas == 0)
            {
                banco->usadas[i] = true;
                return i;//devolve indice desejado
            }
        }
        sorteadas--;
    }

    //retorna erro caso na entre no if correto para retornar o i
    return -1;
}

int cadastraPergunta(BancoPerguntas *banco)
{
    
}
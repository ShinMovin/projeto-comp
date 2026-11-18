#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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

//le um campo do texto a partir da posicao
int lerString(const char *texto, int *posicao, char *destino)
{
    int i = *posicao;
    int j = 0;

    if(texto[i] == '"')
    {
        i++;
        while(texto[i] != '"')
        {
            if(texto[i] == '\n' || texto[i] == '\0' || texto[i] == '\r')
            {
                printf("Erro : aspas sem fechar !\n");
                return 0;
            }
            if(j == 200)
            {
                printf("Erro : texto muito longo !\n");
                return 0;
            }
            destino[j] = texto[i];
            i++;
            j++;
        }

        i++;
    }
    else
    {
        while (texto[i] != '\n' && texto[i] != '\0' && texto[i] != ',' && texto[i] != '\r' && j < 200)
        {
            destino[j] = texto[i];
            i++;
            j++;
        }
    }

    destino[j] = '\0';

    if(j== 0)
    {
        printf("Erro : campo vazio !\n");
        return 0;
    }
    if(texto[i] != ',')
    {
        printf("Erro : campo mal formado / muito longo\n");
        return 0;
    }

    *posicao = i;
    return 1;
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
        int i = 0;

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

        if(lerString(linha, &i, pergunta.conteudo_da_pergunta) == 0)
        {
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

            i++;  // pula o ',' anterior
            if(lerString(linha, &i, pergunta.alternativas[n]) == 0)
            {
                fclose(arq);
                return 0;
            }//if
        }//for

        if(linha[i] != ','){
            printf("Erro: linha do CSV incompleta!\n");
            fclose(arq);
            return 0;
        }//if

        i++;  //pula o ',' depois da alternativa D
        //verificando se a alternativa tá no intervalo correto
        if(toupper(linha[i]) < 'A' || toupper(linha[i]) > 'D'){
            printf("Erro: alternativa correta fora do intervalo!\n");
            fclose(arq);
            return 0;
        }
        pergunta.alternativa_correta = toupper(linha[i]);  //passar a letra para maiuscula

        i ++;  //vai para ',' depois da letra correta
        //verificando se tem separador no lugar certo
        if(linha[i] != ','){
            printf("Erro: separador ausente!\n");
            fclose(arq);
            return 0;
        }

        i++; //pula o ','
        //verificando se a dificuldade foi digitada certa
        if(linha[i] < '1' || linha[i] > '5'){
            printf("Erro: dificuldade invalida!\n");
            fclose(arq);
            return 0;
        }
        
        pergunta.dificuldades = (Dificuldade)(linha[i] - '0');  //converte os caracteres ('1' a '5') no numero da dificuldade

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
            sorteadas--;
        }
        
    }

    //retorna erro caso nao entre no if correto para retornar o i
    return -1;
}

int lerPergunta(Pergunta *perguntas)
{
    
}
#ifndef PERGUNTAS_H
#define PERGUNTAS_H
#include <stdbool.h>

typedef enum {MUITO_FACIL = 1, FACIL, MEDIO, DIFICIL, IMPOSSIVEL} Dificuldade;

typedef struct {
    char conteudo_da_pergunta[201];
    char alternativas[4][201];
    char alternativa_correta;
    Dificuldade dificuldades;
} Pergunta;

typedef struct {
    int quantidade;
    int capacidade;
    bool *usadas;
    Pergunta *perguntas;
} BancoPerguntas;

int inicializaBanco(BancoPerguntas *banco);//feito
int carregaPerguntas (BancoPerguntas *banco, const char *nome_arq);//feito
int sorteaPergunta (BancoPerguntas *banco, Dificuldade dificuldades);//feito
void liberaBancoPergunta (BancoPerguntas *banco);//feito
int lerPergunta(Pergunta *perguntas);//feito
int adicionaPergunta(BancoPerguntas *banco, Pergunta *perguntas);
int listaPergunta (BancoPerguntas *banco);
int pesquisaPergunta (BancoPerguntas *banco);
int alteraPergunta (BancoPerguntas *banco);
int excluiPergunta (BancoPerguntas *banco);

#endif 
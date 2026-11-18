#ifndef PERGUNTAS_H
#define PERGUNTAS_H
#include <stdbool.h>

typedef enum {GEEK = 1, ESPORTES, MATEMATICA, GEOGRAFIA, BIOLOGIA} Categoria;
typedef enum {MUITO_FACIL = 1, FACIL, MEDIO, DIFICIL, IMPOSSIVEL} Dificuldade;

typedef struct {
    char conteudo_da_pergunta[201];
    char alternativas[4][201];
    char alternativa_correta;
    Dificuldade dificuldades;
    Categoria categorias;
} Pergunta;

typedef struct {
    int quantidade;
    int capacidade;
    bool *usadas;
    Pergunta *perguntas;
} BancoPerguntas;

int inicializaBanco(BancoPerguntas *banco);//feito
int carregaPerguntas (BancoPerguntas *banco, const char *nome_arq);//feito
int sorteaPergunta (BancoPerguntas *banco, Dificuldade dificuldades, Categoria categorias);//feito
void liberaBancoPergunta (BancoPerguntas *banco);//feito
int lerPergunta(Pergunta *perguntas);//feito
int adicionaPerguntas(BancoPerguntas *banco, Pergunta *perguntas);//feito
int salvaPergunta(BancoPerguntas *banco, char *nome_arq);//feito
int listaPergunta (BancoPerguntas *banco);//feito
int pesquisaPergunta (BancoPerguntas *banco);//feito
int alteraPergunta (BancoPerguntas *banco);
int excluiPergunta (BancoPerguntas *banco);//feito

#endif 
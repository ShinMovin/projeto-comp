#ifndef PERGUNTAS_H
#define PERGUNTAS_H

typedef enum {MUITO_FACIL = 1, FACIL, MEDIO, DIFICIL, IMPOSSIVEL} Dificuldade;

typedef struct {
    char conteudo_da_pergunta[201];
    char alternativas[4][201];
    int alternativa_correta;
    Dificuldade dificuldade;
} Pergunta;

typedef struct {
    int quantidade;
    int capacidade;
    int *usadas;
    Pergunta *perguntas;
} BancoPerguntas;


int carregaPerguntas (BancoPerguntas *banco, const char *nome_arq);
int sorteaPergunta (BancoPerguntas *banco, Dificuldade dificuldade);
void liberaBancoPergunta (BancoPerguntas *banco);
int cadastraPergunta (BancoPerguntas *banco);
int listaPergunta (BancoPerguntas *banco);
int pesquisaPergunta (BancoPerguntas *banco);
int alteraPergunta (BancoPerguntas *banco);
int excluiPergunta (BancoPerguntas *banco);

#endif 
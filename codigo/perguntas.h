#ifndef PERGUNTAS_H
#define PERGUNTAS_H

typedef enum {MUITOFACIL = 1, FACIL, MEDIO, DIFICIL, IMPOSSIVEL} Dificuldade;

typedef struct {
    char conteudo_da_pergunta[201];
    char alternativas[4][201];
    int alternativa_correta;
    Dificuldade dificuldade;
} Pergunta;

#endif 
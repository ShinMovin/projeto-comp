#ifndef ROLETA_H
#define ROLETA_H

#include "perguntas.h"

enum { SETORES_ROLETA = 5 };

typedef enum { ROLETA_PRONTA, ROLETA_GIRANDO, ROLETA_RESULTADO } EstadoRoleta;
typedef enum { ROLETA_NENHUMA_ACAO, ROLETA_GIRAR, ROLETA_PROSSEGUIR, ROLETA_VOLTAR } AcaoRoleta;

typedef struct
{
    EstadoRoleta estado;
    Categoria categoria;
    bool disponiveis[SETORES_ROLETA];
    int quantidadeDisponivel;
    double tempoGiro;
    float tempoAnuncio;
    float angulo;
    float anguloFinal;
} Roleta;

void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade dificuldade);
bool girarRoleta(Roleta *roleta);
void atualizarRoleta(Roleta *roleta, float tempoQuadro);
AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso);

#endif

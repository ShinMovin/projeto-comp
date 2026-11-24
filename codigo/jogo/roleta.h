#ifndef ROLETA_H
#define ROLETA_H

#include "perguntas.h"
#include "premio.h"

enum { SETORES_ROLETA = 5 };

typedef enum { ROLETA_PRONTA, ROLETA_GIRANDO, ROLETA_RESULTADO } EstadoRoleta;
typedef enum { ROLETA_NENHUMA_ACAO, ROLETA_GIRAR, ROLETA_PROSSEGUIR, ROLETA_VOLTAR, ROLETA_DIFICULDADE } AcaoRoleta;

// A mesma roleta serve às duas escolhas. O peso zero impede que um setor saia.
typedef struct
{
    bool deDificuldade;
    Dificuldade nivelBase;
    int pesos[SETORES_ROLETA];
    int total;
    int resultado;
    EstadoRoleta estado;
    double tempoGiro;
    float tempoAnuncio;
    float angulo;
    float anguloFinal;
} Roleta;

void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade nivel, Categoria bloqueada);
void iniciarRoletaDificuldade(Roleta *roleta, const BancoPerguntas *banco, int rodada);
bool girarRoleta(Roleta *roleta);
void atualizarRoleta(Roleta *roleta, float segundos);
AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso, Dificuldade nivel, bool podeMudar);

#endif

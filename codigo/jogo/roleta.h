#ifndef ROLETA_H
#define ROLETA_H

#include "perguntas.h"
#include "premio.h"

enum { SETORES_ROLETA = 5 };

typedef enum { ROLETA_PRONTA, ROLETA_GIRANDO, ROLETA_RESULTADO } EstadoRoleta;
typedef enum { ROLETA_NENHUMA_ACAO, ROLETA_GIRAR, ROLETA_PROSSEGUIR, ROLETA_VOLTAR, ROLETA_DIFICULDADE } AcaoRoleta;

// Ambas as roletas desenham ângulos em graus e avançam pelo tempo de cada quadro.
typedef struct
{
    EstadoRoleta estado;
    double tempoGiro;
    float tempoAnuncio;
    float angulo;
    float anguloFinal;
} AnimacaoRoleta;

typedef struct
{
    AnimacaoRoleta animacao;
    Categoria categoria;
    bool disponiveis[SETORES_ROLETA];
    int quantidadeDisponivel;
} Roleta;

/** @brief Roleta opcional que diminui, mantém ou aumenta a dificuldade da rodada. */
typedef struct
{
    AnimacaoRoleta animacao;
    Dificuldade base;
    int chances[QUANTIDADE_OPCOES_DIF]; // Zero quando a opção não é possível (limite ou sem perguntas).
    OpcaoDificuldade resultado;
} RoletaDificuldade;

/** @brief Prepara a roleta de categorias. @param bloqueada categoria que não pode sair (0 = nenhuma) */
void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade dificuldade, Categoria bloqueada);
bool girarRoleta(Roleta *roleta);
/** @brief Avança o giro ou o anúncio pelo tempo do quadro em segundos. */
void atualizarRoleta(Roleta *roleta, float tempoQuadro);
/**
 * @brief Desenha a roleta de categorias.
 * @param dificuldade nível atual, mostrado no botão circular da roleta de dificuldade
 * @param podeMudarDificuldade se falso, o botão circular aparece apagado
 */
AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso, Dificuldade dificuldade,
                          bool podeMudarDificuldade);

void iniciarRoletaDificuldade(RoletaDificuldade *roleta, const BancoPerguntas *banco, int rodada);
/** @brief Sorteia uma opção por peso. Retorna false se o giro não pode começar. */
bool girarRoletaDificuldade(RoletaDificuldade *roleta);
void atualizarRoletaDificuldade(RoletaDificuldade *roleta, float tempoQuadro);
/** @brief Variação de nível sorteada: -1, 0 ou +1. */
int variacaoDificuldade(const RoletaDificuldade *roleta);
AcaoRoleta desenharRoletaDificuldade(const RoletaDificuldade *roleta);

#endif

#ifndef INTERFACE_H
#define INTERFACE_H
#include <stdbool.h>
#include "desenho.h"
#include "jogo.h"

// Paleta compartilhada pelo menu e pelas telas da partida.
#define COR_FUNDO ((Color){8, 14, 37, 255})
#define COR_AZUL ((Color){23, 40, 78, 255})
#define COR_DOURADO ((Color){207, 166, 78, 255})
#define COR_CLARO ((Color){243, 231, 187, 255})

typedef enum
{
    MENU_PRINCIPAL,
    TELA_JOGAR,
    TELA_MINIGAMES,
    TELA_INSTRUCOES,
    TELA_BANCO,
    TELA_SAIR

} Tela;

typedef enum
{
    PERGUNTA_SEM_ACAO, PERGUNTA_ESCOLHER, PERGUNTA_CANCELAR, PERGUNTA_CONFIRMAR,
    PERGUNTA_AJUDA, PERGUNTA_AVANCAR
} TipoAcaoPergunta;

typedef struct
{
    TipoAcaoPergunta tipo;
    char alternativa;
} AcaoPergunta;

typedef enum { RESULTADO_SEM_ACAO, RESULTADO_NOVA_PARTIDA, RESULTADO_MENU } AcaoResultado;

Tela desenharMenu(void);
bool desenharComoJogar(int *pagina);
void desenharFundo(void);
bool desenharBotao(const char *texto, Rectangle area);
void desenharTextoPergunta(const char texto[LIMITE_TEXTO_PERGUNTA + 1], Rectangle area,
                           int tamanho, bool centralizar, Color cor);
AcaoPergunta desenharTelaPergunta(const Jogo *jogo, const Pergunta *pergunta);
bool desenharNovoNivel(const Jogo *jogo);
AcaoResultado desenharResultado(const Jogo *jogo);

#endif

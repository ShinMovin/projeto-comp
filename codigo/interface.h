#ifndef INTERFACE_H
#define INTERFACE_H
#include <stdbool.h>

typedef enum
{
    MENU_PRINCIPAL,
    TELA_JOGAR,
    TELA_MINIGAMES,
    TELA_INSTRUCOES,
    TELA_SAIR

} Tela;

Tela desenharMenu(void);
bool desenharVoltar(void);

#endif
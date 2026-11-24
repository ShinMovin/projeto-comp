#ifndef GUITARHERO_H
#define GUITARHERO_H

#include "../minigame.h"

/* A sessão é criada no início, passada a cada quadro e liberada ao encerrar. */
typedef struct SessaoGuitarra SessaoGuitarra;
const char *NomeOpcaoGuitarra(bool treino, int opcao);
SessaoGuitarra *IniciarGuitarra(bool treino, int opcao);
ResultadoMinigame AtualizarGuitarra(SessaoGuitarra *sessao, float segundos);
void DesenharGuitarra(const SessaoGuitarra *sessao);
PlacarMinigame PlacarGuitarra(const SessaoGuitarra *sessao);
void EncerrarGuitarra(SessaoGuitarra *sessao);

#endif

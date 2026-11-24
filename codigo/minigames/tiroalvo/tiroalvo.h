#ifndef TIROALVO_H
#define TIROALVO_H

#include "../minigame.h"

/* A sessão é criada no início, passada a cada quadro e liberada ao encerrar. */
typedef struct SessaoTiro SessaoTiro;
const char *NomeOpcaoTiro(bool treino, int opcao);
SessaoTiro *IniciarTiro(bool treino, int opcao);
ResultadoMinigame AtualizarTiro(SessaoTiro *sessao, float segundos);
void DesenharTiro(const SessaoTiro *sessao);
PlacarMinigame PlacarTiro(const SessaoTiro *sessao);
void EncerrarTiro(SessaoTiro *sessao);

#endif

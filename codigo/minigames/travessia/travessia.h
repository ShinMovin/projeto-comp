/**
 * @file travessia.h
 * @author VGA
 * @brief Minigame Travessia: atravesse pistas desviando dos carros.
 */
#ifndef TRAVESSIA_H
#define TRAVESSIA_H

#include "../minigame.h"

/* A sessão é criada no início, passada a cada quadro e liberada ao encerrar. */
typedef struct SessaoTravessia SessaoTravessia;
SessaoTravessia *IniciarTravessia(bool treino, int opcao);
ResultadoMinigame AtualizarTravessia(SessaoTravessia *sessao, float segundos);
void DesenharTravessia(const SessaoTravessia *sessao);
PlacarMinigame PlacarTravessia(const SessaoTravessia *sessao);
void EncerrarTravessia(SessaoTravessia *sessao);

#endif

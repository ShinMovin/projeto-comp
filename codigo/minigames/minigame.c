#include "minigame.h"
#include "guitarhero/guitarhero.h"
#include "tiroalvo/tiroalvo.h"
#include "travessia/travessia.h"
#include <stddef.h>

bool minigameDisponivel(TipoMinigame tipo)
{
    return tipo == GUITAR_HERO || tipo == TIRO_AO_ALVO || tipo == TRAVESSIA;
}

const char *nomeOpcaoMinigame(TipoMinigame tipo, bool treino, int opcao)
{
    switch (tipo)
    {
    case GUITAR_HERO: return NomeOpcaoGuitarra(treino, opcao);
    case TIRO_AO_ALVO: return NomeOpcaoTiro(treino, opcao);
    default: return NULL; // A Travessia tem uma única opção, sem seleção de faixa.
    }
}

// Cada jogo tem uma struct de sessão diferente. O tipo identifica qual ponteiro foi guardado.
void *iniciarMinigame(TipoMinigame tipo, bool treino, int opcao)
{
    switch (tipo)
    {
    case GUITAR_HERO: return IniciarGuitarra(treino, opcao);
    case TIRO_AO_ALVO: return IniciarTiro(treino, opcao);
    case TRAVESSIA: return IniciarTravessia(treino, opcao);
    default: return NULL;
    }
}

ResultadoMinigame atualizarMinigame(TipoMinigame tipo, void *sessao, float segundos)
{
    switch (tipo)
    {
    case GUITAR_HERO: return AtualizarGuitarra(sessao, segundos);
    case TIRO_AO_ALVO: return AtualizarTiro(sessao, segundos);
    case TRAVESSIA: return AtualizarTravessia(sessao, segundos);
    default: return MINIGAME_ABANDONADO;
    }
}

void desenharMinigame(TipoMinigame tipo, const void *sessao)
{
    switch (tipo)
    {
    case GUITAR_HERO: DesenharGuitarra(sessao); break;
    case TIRO_AO_ALVO: DesenharTiro(sessao); break;
    case TRAVESSIA: DesenharTravessia(sessao); break;
    default: break;
    }
}

PlacarMinigame placarMinigame(TipoMinigame tipo, const void *sessao)
{
    switch (tipo)
    {
    case GUITAR_HERO: return PlacarGuitarra(sessao);
    case TIRO_AO_ALVO: return PlacarTiro(sessao);
    case TRAVESSIA: return PlacarTravessia(sessao);
    default: return (PlacarMinigame){0};
    }
}

void encerrarMinigame(TipoMinigame tipo, void *sessao)
{
    switch (tipo)
    {
    case GUITAR_HERO: EncerrarGuitarra(sessao); break;
    case TIRO_AO_ALVO: EncerrarTiro(sessao); break;
    case TRAVESSIA: EncerrarTravessia(sessao); break;
    default: break;
    }
}

#include "minigame.h"
#include "guitarhero/guitarhero.h"
#include "tiroalvo/tiroalvo.h"
#include "travessia/travessia.h"
#include <stddef.h>

const Minigame *obterMinigame(TipoMinigame tipo)
{
    switch (tipo)
    {
    case GUITAR_HERO:
        return &guitarHero;
    case TIRO_AO_ALVO:
        return &tiroAoAlvo;
    case TRAVESSIA:
        return &travessia;
    default:
        return NULL;
    }
}

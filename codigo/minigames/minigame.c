#include "minigame.h"
#include "guitarhero/guitarhero.h"
#include "tiroalvo/tiroalvo.h"
#include <stddef.h>

const Minigame *obterMinigame(TipoMinigame tipo)
{
    switch (tipo)
    {
    case GUITAR_HERO:
        return &guitarHero;
    case TIRO_AO_ALVO:
        return &tiroAoAlvo;
    default:
        // Crossy Road ainda não tem fontes disponíveis neste repositório.
        return NULL;
    }
}

#include "desenho.h"
#include <math.h>

void textoCentradoEm(const char *texto, int centroX, int y, int tamanho, Color cor)
{
    DrawText(texto, centroX - MeasureText(texto, tamanho) / 2, y, tamanho, cor);
}

void textoCentralizado(const char *texto, int y, int tamanho, Color cor)
{
    textoCentradoEm(texto, GetScreenWidth() / 2, y, tamanho, cor);
}

void textoNoRetangulo(const char *texto, Rectangle area, int tamanho, Color cor)
{
    int x = (int)(area.x + (area.width - MeasureText(texto, tamanho)) / 2);
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(texto, x, y, tamanho, cor);
}

float animarEntrada(float tempo, float duracao)
{
    // A interpolação desacelera até parar na posição final.
    float restante = 1.0f - fminf(tempo / duracao, 1.0f);
    return 1.0f - restante * restante;
}

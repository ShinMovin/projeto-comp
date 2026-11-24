#ifndef DESENHO_H
#define DESENHO_H

#include "raylib.h"

void textoCentradoEm(const char *texto, int centroX, int y, int tamanho, Color cor);
void textoCentralizado(const char *texto, int y, int tamanho, Color cor);
void textoNoRetangulo(const char *texto, Rectangle area, int tamanho, Color cor);
float animarEntrada(float tempo, float duracao);

#endif

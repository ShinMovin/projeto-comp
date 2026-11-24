#ifndef DESENHO_H
#define DESENHO_H

#include "raylib.h"

void textoCentradoEm(const char *texto, int centroX, int y, int tamanho, Color cor);
void textoCentralizado(const char *texto, int y, int tamanho, Color cor);
void textoNoRetangulo(const char *texto, Rectangle area, int tamanho, Color cor);
/** @brief Transforma tempo em progresso de 0 a 1, desacelerando no final da animação. */
float animarEntrada(float tempo, float duracao);

#endif

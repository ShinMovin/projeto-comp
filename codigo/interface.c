#include "interface.h"
#include "raylib.h"

// Paleta de cores
#define FUNDO (Color){8, 14, 37, 255}
#define AZUL (Color){23, 40, 78, 255}
#define DOURADO (Color){207, 166, 78, 255}
#define CLARO (Color){243, 231, 187, 255}

// Centraliza um texto horizontalmente
static void textoCentralizado(const char *texto,
                              int y, int tamanho, Color cor)
{
    int largura = MeasureText(texto, tamanho);

    DrawText(texto, (GetScreenWidth() - largura) / 2,
             y, tamanho, cor);
}

// Desenha um botão
static void desenharBotao(const char *texto, Rectangle area)
{
    Vector2 mouse = GetMousePosition();

    bool selecionado = CheckCollisionPointRec(mouse, area);

    Color cor = selecionado ? DOURADO : AZUL;
    Color corTexto = selecionado ? FUNDO : CLARO;

    DrawRectangleRec(area, cor);
    DrawRectangleLinesEx(area, 2, DOURADO);

    int largura = MeasureText(texto, 22);

    DrawText(texto,
             area.x + (area.width - largura) / 2,
             area.y + (area.height - 22) / 2,
             22, corTexto);
}

// Interface principal
void desenharMenu(void)
{
    ClearBackground(FUNDO);

    // Linhas decorativas
    for (int i = 0; i < 900; i += 100)
    {
        DrawLine(i, 0, 400, 300, AZUL);
        DrawLine(i, 600, 400, 300, AZUL);
    }

    // Moldura externa
    DrawRectangleLinesEx(
        (Rectangle){20, 20, 760, 560},
        2, DOURADO
    );

    // Logotipo
    DrawCircle(400, 172, 133, AZUL);
    DrawCircleLines(400, 172, 133, DOURADO);
    DrawCircleLines(400, 172, 122, DOURADO);
    DrawCircle(400, 172, 108, FUNDO);

    DrawLine(290, 142, 510, 142, DOURADO);
    DrawLine(290, 210, 510, 210, DOURADO);

    textoCentralizado("JOGO DO", 105, 26, CLARO);
    textoCentralizado("MILHAO", 151, 48, DOURADO);
    textoCentralizado("DESAFIE SEU SABER", 222, 15, CLARO);

    // Botões
    desenharBotao("JOGAR",
        (Rectangle){250, 330, 300, 40});

    desenharBotao("MINIGAMES",
        (Rectangle){250, 384, 300, 40});

    desenharBotao("COMO JOGAR",
        (Rectangle){250, 438, 300, 40});

    desenharBotao("SAIR",
        (Rectangle){250, 492, 300, 40});

    // Rodapé
    textoCentralizado("ENGENHARIA DE COMPUTACAO - UTFPR",
                      557, 12, GRAY);
}

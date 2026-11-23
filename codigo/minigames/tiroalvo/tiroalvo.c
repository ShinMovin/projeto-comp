#include "raylib.h"

#define LARGURA 800
#define ALTURA 600

#define TELA_MENU 0
#define TELA_JOGO 1

// Desenha o botão e retorna true quando ele é clicado.
bool Botao(Rectangle area, const char *texto)
{
    int tamanho = 24;
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = {30, 30, 30, 255};

    // Muda a cor quando o cursor está sobre o botão.
    if (sobreBotao)
        cor = DARKGRAY;

    DrawRectangleRec(area, cor);

    // Centraliza o texto dentro do retângulo.
    int x = (int)(area.x + (area.width - MeasureText(texto, tamanho)) / 2);
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(texto, x, y, tamanho, WHITE);

    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Os contornos e as trilhas dão profundidade à cabine.
void DesenharCabine(const Rectangle janelas[])
{
    DrawRectangleLines(54, 106, 692, 404, GRAY);
    DrawRectangleLines(250, 154, 300, 135, DARKGRAY);
    DrawLine(54, 106, 250, 154, DARKGRAY);
    DrawLine(746, 106, 550, 154, DARKGRAY);
    DrawLine(54, 510, 250, 289, DARKGRAY);
    DrawLine(746, 510, 550, 289, DARKGRAY);

    // As molduras marcam os dois lugares onde o alvo roxo pode aparecer.
    for (int i = 0; i < 2; i++)
    {
        Rectangle janela = janelas[i];
        DrawRectangleRec(janela, BLACK);
        DrawRectangleLinesEx(janela, 2, GRAY);
    }

    // Trilhas do fundo, do meio e da frente, nesta ordem.
    DrawRectangle(260, 248, 280, 3, GRAY);
    DrawRectangle(178, 363, 444, 3, GRAY);
    DrawRectangle(100, 464, 600, 3, GRAY);

    // O balcão fecha a cabine na parte mais próxima do jogador.
    DrawRectangle(32, 510, 736, 20, DARKGRAY);
}

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Tiro ao Alvo");
    SetTargetFPS(60);

    // Começa pelo menu e posiciona os botões no centro da janela.
    int tela = TELA_MENU;
    const Rectangle areaBotaoJogar = {(LARGURA - 220) / 2, 260, 220, 52};
    const Rectangle areaBotaoSair = {(LARGURA - 220) / 2, 330, 220, 52};

    const Rectangle janelas[] = {{86, 190, 130, 88}, {584, 190, 130, 88}};

    // Loop principal
    while (!WindowShouldClose())
    {
        // Limpa a janela e redesenha a tela atual a cada quadro.
        BeginDrawing();
        ClearBackground(BLACK);

        bool sair = false;
        if (tela == TELA_MENU)
        {
            int xTitulo = (LARGURA - MeasureText("Tiro ao Alvo", 48)) / 2;
            DrawText("Tiro ao Alvo", xTitulo, 165, 48, WHITE);

            // Troca de tela no próximo quadro quando o jogador clica em Jogar.
            if (Botao(areaBotaoJogar, "Jogar"))
            {
                tela = TELA_JOGO;
            }
            sair = Botao(areaBotaoSair, "Sair");
        }
        else
        {
            DesenharCabine(janelas);
            int xTitulo = LARGURA - 32 - MeasureText("Tiro ao Alvo", 20);
            DrawText("Tiro ao Alvo", xTitulo, 35, 20, RAYWHITE);
        }

        EndDrawing();
        if (sair)
            break;
    }

    // Fecha a janela e libera os recursos da raylib.
    CloseWindow();

    return 0;
}

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
    Color cor = {30, 30, 45, 255};

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

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Tiro ao Alvo");
    SetTargetFPS(60);

    // Começa pelo menu e posiciona os botões no centro da janela.
    int tela = TELA_MENU;
    const Rectangle areaBotaoJogar = {(LARGURA - 220) / 2, 260, 220, 52};
    const Rectangle areaBotaoSair = {(LARGURA - 220) / 2, 330, 220, 52};

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
            // A lógica e o desenho da partida entram aqui na próxima etapa.
        }

        EndDrawing();
        if (sair)
            break;
    }

    // Fecha a janela e libera os recursos da raylib.
    CloseWindow();

    return 0;
}

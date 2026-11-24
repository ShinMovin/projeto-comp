
#include "raylib.h"
#include "interface.h"

int main(void)
{
    InitWindow(800, 600, "Jogo do Milhao");

    SetTargetFPS(60);

    Tela telaAtual = MENU_PRINCIPAL;

    while (!WindowShouldClose() &&
           telaAtual != TELA_SAIR)
    {
        BeginDrawing();

        ClearBackground(BLACK);

        switch (telaAtual)
        {
            case MENU_PRINCIPAL:
                telaAtual = desenharMenu();
                break;

            case TELA_JOGAR:
                DrawText("TELA DE PERGUNTAS",
                         200, 250, 30, WHITE);
                break;

            case TELA_MINIGAMES:
                DrawText("TELA DE MINIGAMES",
                         200, 250, 30, WHITE);
                break;

            case TELA_INSTRUCOES:
                DrawText("COMO JOGAR",
                         280, 250, 30, WHITE);
                break;

            case TELA_SAIR:
                break;
        }

        if (telaAtual != MENU_PRINCIPAL && telaAtual != TELA_SAIR)
    {
        if (desenharVoltar() || IsKeyPressed(KEY_BACKSPACE))
        {
            telaAtual = MENU_PRINCIPAL;
        }
    }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

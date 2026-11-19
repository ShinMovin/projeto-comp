#include "raylib.h"
#include "jogo.h"

void jogar(BancoPerguntas *banco)
{
    InitWindow(800, 600, "Show do Milhao");
    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawText("Jogo", 350, 280, 40, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
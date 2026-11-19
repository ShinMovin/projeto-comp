#include "raylib.h"
#include "jogo.h"

void jogar(BancoPerguntas *banco)
{
    InitWindow(1280, 720, "Show do Milhao");
    SetTargetFPS(60);

    Jogo jogo;

    jogo.rodada = 0;
    jogo.premio = 0;
    jogo.estado = SORTEA;

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawText("Jogo", 350, 280, 40, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
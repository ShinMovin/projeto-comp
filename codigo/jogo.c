/**
 * @file jogo.c
 * @author VGA
 * @brief Loop do jogo em raylib: rodadas, perguntas e premio.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "raylib.h"
#include "jogo.h"
#include "premio.h"
#include <stdlib.h>

void jogar(BancoPerguntas *banco)
{
    InitWindow(1280, 720, "Show do Milhao");
    SetTargetFPS(60);

    Jogo jogo;

    jogo.rodada = 0;
    jogo.premio = 0;
    jogo.estado = SORTEA;

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        
    }

    while(!WindowShouldClose())
    {
        switch (jogo.estado)
        {
        case SORTEA:
            jogo.dificuldade = dificuldadeRodada(jogo.rodada);
            jogo.categoria = rand() % 5 + 1;
            if((jogo.indicePergunta = sorteaPergunta(banco, jogo.dificuldade, jogo.categoria)) == -1)
            {

            }
            else
            {
                jogo.estado = PERGUNTA;
            }
            break;
        
        case PERGUNTA:
            break;

        case ACERTOU:
            break;

        case ERROU:
            break;

        case FIM:
            break;
        }

        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawText("Jogo", 640, 320, 40, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
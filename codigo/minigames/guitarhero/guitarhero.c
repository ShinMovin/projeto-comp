#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define LARGURA 800
#define ALTURA 600


#define TRILHAS 4
#define MAX_NOTAS 100

#define LARGURA_TRILHA 90
#define VELOCIDADE 250.0f
#define LINHA_ACERTO 480
#define DISTANCIA_ACERTO 35.0f


typedef struct {
    int trilha;
    float y;
    bool ativa;
} Nota;

int main(void)
{
    
    InitWindow(LARGURA, ALTURA, "Guitar Hero - Base");
    SetTargetFPS(60);

    
    int larguraCampo = TRILHAS * LARGURA_TRILHA;
    int inicioX = (LARGURA - larguraCampo) / 2;

    
    int teclas[TRILHAS] = {
        KEY_D, KEY_F, KEY_J, KEY_K
    };

    
    Color cores[TRILHAS] = {
        GREEN,
        RED,
        YELLOW,
        BLUE
    };


    Nota notas[MAX_NOTAS] = {0};

    
    float tempoNota = 1.0f;

    // Loop principal
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ----------------------------------
        // 1. GERAR NOTAS
        // ----------------------------------

        tempoNota -= dt;

        if (tempoNota <= 0.0f)
        {
            // Procura uma posicao livre no vetor
            for (int i = 0; i < MAX_NOTAS; i++)
            {
                if (!notas[i].ativa)
                {
                    notas[i].ativa = true;

                    // Escolhe uma trilha aleatoria
                    notas[i].trilha =
                        GetRandomValue(0, TRILHAS - 1);

                    // Comeca acima da tela
                    notas[i].y = -20;

                    break;
                }
            }

            // Intervalo entre notas
            tempoNota = 0.6f;
        }

        // ----------------------------------
        // 2. MOVIMENTAR NOTAS
        // ----------------------------------

        for (int i = 0; i < MAX_NOTAS; i++)
        {
            if (!notas[i].ativa)
                continue;

            // Move a nota para baixo
            notas[i].y += VELOCIDADE * dt;

            // Remove notas que sairam da tela
            if (notas[i].y > ALTURA + 20)
            {
                notas[i].ativa = false;
            }
        }

        // ----------------------------------
        // 3. DETECTAR ACERTOS
        // ----------------------------------

        for (int t = 0; t < TRILHAS; t++)
        {
            if (IsKeyPressed(teclas[t]))
            {
                int melhorNota = -1;
                float menorDistancia = DISTANCIA_ACERTO;

                // Procura a nota mais proxima
                for (int i = 0; i < MAX_NOTAS; i++)
                {
                    if (!notas[i].ativa)
                        continue;

                    if (notas[i].trilha != t)
                        continue;

                    float distancia =
                        fabsf(notas[i].y - LINHA_ACERTO);

                    if (distancia <= menorDistancia)
                    {
                        menorDistancia = distancia;
                        melhorNota = i;
                    }
                }

                // Acertou uma nota
                if (melhorNota != -1)
                {
                    notas[melhorNota].ativa = false;
                }
            }
        }

        // ----------------------------------
        // 4. DESENHAR O JOGO
        // ----------------------------------

        BeginDrawing();

        ClearBackground(BLACK);

        // Desenhar trilhas
        for (int t = 0; t < TRILHAS; t++)
        {
            int x = inicioX + t * LARGURA_TRILHA;

            // Fundo das trilhas
            DrawRectangle(
                x, 0,
                LARGURA_TRILHA, ALTURA,
                (Color){30, 30, 45, 255}
            );

            // Linha divisoria
            DrawLine(
                x, 0,
                x, ALTURA,
                DARKGRAY
            );
        }

        // Borda direita do campo
        DrawLine(
            inicioX + larguraCampo, 0,
            inicioX + larguraCampo, ALTURA,
            DARKGRAY
        );

        // Linha horizontal de acerto
        DrawRectangle(
            inicioX,
            LINHA_ACERTO - 2,
            larguraCampo,
            4,
            WHITE
        );

        // ----------------------------------
        // 5. DESENHAR BOLAS INFERIORES
        // ----------------------------------

        for (int t = 0; t < TRILHAS; t++)
        {
            int centroX =
                inicioX + t * LARGURA_TRILHA
                + LARGURA_TRILHA / 2;

            // Se a tecla esta pressionada
            if (IsKeyDown(teclas[t]))
            {
                DrawCircle(
                    centroX,
                    LINHA_ACERTO,
                    28,
                    cores[t]
                );
            }
            else
            {
                // Apenas o contorno do circulo
                DrawCircleLines(
                    centroX,
                    LINHA_ACERTO,
                    28,
                    cores[t]
                );
            }

            // Letras das teclas
            const char *nomes[TRILHAS] = {
                "D", "F", "J", "K"
            };

            DrawText(
                nomes[t],
                centroX - 8,
                LINHA_ACERTO + 45,
                24,
                cores[t]
            );
        }

        // ----------------------------------
        // 6. DESENHAR NOTAS CAINDO
        // ----------------------------------

        for (int i = 0; i < MAX_NOTAS; i++)
        {
            if (!notas[i].ativa)
                continue;

            int t = notas[i].trilha;

            int centroX =
                inicioX + t * LARGURA_TRILHA
                + LARGURA_TRILHA / 2;

            // Nota circular
            DrawCircle(
                centroX,
                (int)notas[i].y,
                22,
                cores[t]
            );

            // Contorno branco
            DrawCircleLines(
                centroX,
                (int)notas[i].y,
                22,
                WHITE
            );
        }

        EndDrawing();
    }

    // Fechar janela
    CloseWindow();

    return 0;
}
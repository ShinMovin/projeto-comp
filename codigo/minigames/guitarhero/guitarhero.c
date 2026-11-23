#include "raylib.h"
#include <math.h>


#define LARGURA 800
#define ALTURA 600


#define TRILHAS 4
#define MAX_NOTAS 100

#define LARGURA_TRILHA 90
#define VELOCIDADE_INICIAL 375.0f
#define AUMENTO_VELOCIDADE 37.5f
#define INTERVALO_AUMENTO 6
#define DURACAO_PARTIDA 30
#define LINHA_ACERTO 480
#define DISTANCIA_ACERTO 35.0f
#define DISTANCIA_PERFEITO 8.0f
#define DISTANCIA_OTIMO 20.0f
#define DURACAO_MENSAGEM 0.75f
#define MAX_ERROS 3
#define PONTOS_POR_NOTA 100
#define ACERTOS_POR_NIVEL 5
#define MULTIPLICADOR_MAXIMO 4

#define TELA_MENU 0
#define TELA_JOGO 1
#define TELA_DERROTA 2
#define TELA_VITORIA 3


typedef struct {
    int trilha;
    float y;
    bool ativa;
} Nota;

// Desenha o botão e retorna true quando ele é clicado.
bool Botao(Rectangle area, const char *texto)
{
    int tamanho = 24;
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = {30, 30, 45, 255};

    if (sobreBotao)
        cor = DARKGRAY;

    DrawRectangleRec(area, cor);
    int x = (int)(area.x + (area.width - MeasureText(texto, tamanho)) / 2);
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(texto, x, y, tamanho, WHITE);

    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

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

    
    int tela = TELA_MENU;
    int erros = 0;
    int pontos = 0;
    int combo = 0;
    int multiplicador = 1;
    float tempoNota = 1.0f;
    float tempoPartida = 0.0f;
    const char *mensagemNota = "";
    Color corMensagemNota = WHITE;
    float fimMensagemNota = 0.0f;
    const Rectangle areaBotaoJogar = {(LARGURA - 220) / 2, 260, 220, 52};
    const Rectangle areaBotaoInferior = {(LARGURA - 220) / 2, 330, 220, 52};

    // Loop principal
    while (!WindowShouldClose())
    {
        if (tela != TELA_JOGO)
        {
            BeginDrawing();
            ClearBackground(BLACK);

            bool sair = false;
            if (tela == TELA_MENU)
            {
                int xTitulo = (LARGURA - MeasureText("Guitar Hero", 48)) / 2;
                DrawText("Guitar Hero", xTitulo, 165, 48, WHITE);

                const char *objetivo = TextFormat("Sobreviva por %d segundos", DURACAO_PARTIDA);
                int xObjetivo = (LARGURA - MeasureText(objetivo, 20)) / 2;
                DrawText(objetivo, xObjetivo, 225, 20, WHITE);

                if (Botao(areaBotaoJogar, "Jogar"))
                {
                    for (int i = 0; i < MAX_NOTAS; i++)
                    {
                        notas[i].ativa = false;
                    }
                    erros = 0;
                    pontos = 0;
                    combo = 0;
                    multiplicador = 1;
                    tempoNota = 1.0f;
                    tempoPartida = 0.0f;
                    fimMensagemNota = 0.0f;
                    tela = TELA_JOGO;
                }
                sair = Botao(areaBotaoInferior, "Sair");
            }
            else
            {
                const char *titulo = "Você perdeu!";
                Color corTitulo = RED;
                if (tela == TELA_VITORIA)
                {
                    titulo = "Você venceu!";
                    corTitulo = GREEN;
                }
                int xTitulo = (LARGURA - MeasureText(titulo, 48)) / 2;
                DrawText(titulo, xTitulo, 165, 48, corTitulo);

                if (tela == TELA_DERROTA)
                {
                    int xNota = (LARGURA - MeasureText(mensagemNota, 24)) / 2;
                    DrawText(mensagemNota, xNota, 120, 24, corMensagemNota);
                }

                const char *mensagem = TextFormat("Notas perdidas: %d/%d", erros, MAX_ERROS);
                int xMensagem = (LARGURA - MeasureText(mensagem, 24)) / 2;
                DrawText(mensagem, xMensagem, 235, 24, WHITE);

                const char *pontuacaoFinal = TextFormat("Pontuação final: %d", pontos);
                int xPontuacao = (LARGURA - MeasureText(pontuacaoFinal, 24)) / 2;
                DrawText(pontuacaoFinal, xPontuacao, 270, 24, WHITE);

                if (Botao(areaBotaoInferior, "Voltar ao menu"))
                {
                    tela = TELA_MENU;
                }
            }

            EndDrawing();
            if (sair)
                break;
            continue;
        }

        float dt = GetFrameTime();
        int aumentos = (int)(tempoPartida / INTERVALO_AUMENTO);
        float velocidade = VELOCIDADE_INICIAL + aumentos * AUMENTO_VELOCIDADE;

        // No último quadro, só atualiza as notas até acabar o tempo.
        if (tempoPartida + dt > DURACAO_PARTIDA)
            dt = DURACAO_PARTIDA - tempoPartida;
        tempoPartida += dt;

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
            notas[i].y += velocidade * dt;

            // Uma nota conta como erro ao passar da zona de acerto.
            if (notas[i].y > LINHA_ACERTO + DISTANCIA_ACERTO)
            {
                notas[i].ativa = false;
                erros++;
                combo = 0;
                multiplicador = 1;
                mensagemNota = "Nota perdida!";
                corMensagemNota = RED;
                fimMensagemNota = tempoPartida + DURACAO_MENSAGEM;
                if (erros == MAX_ERROS)
                {
                    tela = TELA_DERROTA;
                    break;
                }
            }
        }

        if (tela == TELA_DERROTA)
            continue;

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
                    combo++;

                    if (menorDistancia <= DISTANCIA_PERFEITO)
                    {
                        mensagemNota = "Perfeito!";
                        corMensagemNota = GREEN;
                    }
                    else if (menorDistancia <= DISTANCIA_OTIMO)
                    {
                        mensagemNota = "Ótimo!";
                        corMensagemNota = SKYBLUE;
                    }
                    else
                    {
                        mensagemNota = "Bom!";
                        corMensagemNota = GOLD;
                    }
                    fimMensagemNota = tempoPartida + DURACAO_MENSAGEM;

                    // A divisão inteira sobe o multiplicador a cada 5 acertos seguidos.
                    multiplicador = 1 + combo / ACERTOS_POR_NIVEL;
                    if (multiplicador > MULTIPLICADOR_MAXIMO)
                        multiplicador = MULTIPLICADOR_MAXIMO;

                    pontos += PONTOS_POR_NOTA * multiplicador;
                }
            }
        }

        if (tempoPartida >= DURACAO_PARTIDA)
        {
            tela = TELA_VITORIA;
            continue;
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

        DrawText(TextFormat("Notas perdidas: %d/%d", erros, MAX_ERROS), 16, 16, 20, WHITE);
        DrawText(TextFormat("Pontuação: %d", pontos), 16, 44, 20, WHITE);
        DrawText(TextFormat("Combo: %d (x%d)", combo, multiplicador), 16, 72, 20, WHITE);
        int segundosRestantes = (int)ceilf(DURACAO_PARTIDA - tempoPartida);
        DrawText(TextFormat("Tempo restante: %d s", segundosRestantes), 16, 100, 20, WHITE);

        if (tempoPartida < fimMensagemNota)
        {
            int xNota = (LARGURA - MeasureText(mensagemNota, 24)) / 2;
            DrawText(mensagemNota, xNota, 560, 24, corMensagemNota);
        }

        EndDrawing();
    }

    // Fechar janela
    CloseWindow();

    return 0;
}

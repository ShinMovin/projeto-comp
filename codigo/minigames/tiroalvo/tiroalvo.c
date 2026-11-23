#include "raylib.h"

#define LARGURA 800
#define ALTURA 600

#define TELA_MENU 0
#define TELA_JOGO 1
#define TELA_FIM 2

#define INICIO_MUSICA_LOCAL 154.0f // 2:34

#define COMUM 0
#define RARO 1
#define EPICO 2
#define LENDARIO 3

#define TRILHA_COMUM_Y 428
#define TRILHA_RARA_Y 336
#define TRILHA_LENDARIA_Y 230
#define LIMITE_RARO_ESQUERDO 210
#define LIMITE_RARO_DIREITO 590
#define LIMITE_LENDARIO_ESQUERDO 268
#define LIMITE_LENDARIO_DIREITO 532
#define VELOCIDADE_RARO 110.0f
#define VELOCIDADE_LENDARIO 320.0f

typedef struct {
    Color cor;
    int pontos;
    float duracao;
    float pausa;
    float raio;
} TipoAlvo;

typedef struct {
    Vector2 centro;
    float velocidadeX;
    float tempoVisivel;
    float espera;
} Alvo;

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

// Define a posição inicial de cada alvo e a direção dos alvos que se movem.
Alvo CriarAlvo(int raridade, TipoAlvo tipo, const Rectangle janelas[])
{
    Alvo alvo = {{0, 0}, 0.0f, tipo.duracao, tipo.pausa};

    if (raridade == COMUM)
    {
        alvo.centro = (Vector2){GetRandomValue(140, 660), TRILHA_COMUM_Y};
    }
    else if (raridade == RARO)
    {
        alvo.centro = (Vector2){GetRandomValue(375, 425), TRILHA_RARA_Y};
        alvo.velocidadeX = VELOCIDADE_RARO;
        if (GetRandomValue(0, 1) == 0)
            alvo.velocidadeX = -VELOCIDADE_RARO;
    }
    else if (raridade == EPICO)
    {
        Rectangle janela = janelas[GetRandomValue(0, 1)];
        alvo.centro.x = janela.x + janela.width / 2;
        alvo.centro.y = janela.y + janela.height / 2;
    }
    else
    {
        alvo.centro = (Vector2){LIMITE_LENDARIO_ESQUERDO, TRILHA_LENDARIA_Y};
        alvo.velocidadeX = VELOCIDADE_LENDARIO;
        if (GetRandomValue(0, 1) == 0)
        {
            alvo.centro.x = LIMITE_LENDARIO_DIREITO;
            alvo.velocidadeX = -VELOCIDADE_LENDARIO;
        }
    }

    return alvo;
}

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Tiro ao Alvo");
    SetTargetFPS(60);

    // Os arquivos de áudio ficam ao lado do executável após a compilação.
    InitAudioDevice();
    bool audioPronto = IsAudioDeviceReady();
    Music ambiente = {0};
    Sound somAcerto = {0};
    bool temAmbiente = false;
    bool temSomAcerto = false;
    bool temFaixaLocal = false;
    if (audioPronto)
    {
        const char *pasta = GetApplicationDirectory();
        // Se houver uma faixa local, usa ela no lugar da trilha padrão.
        const char *musicaLocal = TextFormat("%saudio-tiroalvo/musica-local.mp3", pasta);
        if (FileExists(musicaLocal))
        {
            ambiente = LoadMusicStream(musicaLocal);
            if (IsMusicValid(ambiente))
            {
                temFaixaLocal = GetMusicTimeLength(ambiente) > INICIO_MUSICA_LOCAL;
                if (!temFaixaLocal)
                    UnloadMusicStream(ambiente);
            }
        }
        if (!temFaixaLocal)
            ambiente = LoadMusicStream(TextFormat("%saudio-tiroalvo/ambiente.wav", pasta));
        somAcerto = LoadSound(TextFormat("%saudio-tiroalvo/acerto.wav", pasta));
        temAmbiente = IsMusicValid(ambiente);
        temSomAcerto = IsSoundValid(somAcerto);

        if (temAmbiente)
        {
            ambiente.looping = !temFaixaLocal;
            SetMusicVolume(ambiente, 0.25f);
        }
        if (temSomAcerto)
            SetSoundVolume(somAcerto, 0.50f);
    }

    // Começa pelo menu e posiciona os botões no centro da janela.
    int tela = TELA_MENU;
    const Rectangle areaBotaoJogar = {(LARGURA - 220) / 2, 260, 220, 52};
    const Rectangle areaBotaoSair = {(LARGURA - 220) / 2, 330, 220, 52};

    const Rectangle janelas[] = {{86, 190, 130, 88}, {584, 190, 130, 88}};

    // Cor, pontos por acerto, tempo na tela, pausa até reaparecer e raio.
    const TipoAlvo tipos[] = {
        {GREEN,  100, 2.5f, 0.45f, 31.0f},
        {BLUE,   200, 2.0f, 1.25f, 23.0f},
        {PURPLE, 350, 1.5f, 3.75f, 21.0f},
        {YELLOW, 500, 1.0f, 6.50f, 14.0f}
    };
    const float esperasIniciais[] = {0.0f, 0.65f, 2.0f, 4.0f};
    Alvo alvos[LENDARIO + 1] = {0};
    int pontos = 0;

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
                pontos = 0;
                for (int raridade = COMUM; raridade <= LENDARIO; raridade++)
                    alvos[raridade] = (Alvo){.espera = esperasIniciais[raridade]};

                tela = TELA_JOGO;
                if (temAmbiente)
                {
                    if (temFaixaLocal)
                    {
                        // Prepara o trecho antes de começar a reproduzir.
                        SeekMusicStream(ambiente, INICIO_MUSICA_LOCAL);
                        UpdateMusicStream(ambiente);
                    }
                    PlayMusicStream(ambiente);
                }
            }
            sair = Botao(areaBotaoSair, "Sair");
        }
        else if (tela == TELA_FIM)
        {
            int xTitulo = (LARGURA - MeasureText("Fim da partida", 48)) / 2;
            DrawText("Fim da partida", xTitulo, 165, 48, WHITE);

            const char *placar = TextFormat("Pontuação final: %d", pontos);
            int xPlacar = (LARGURA - MeasureText(placar, 24)) / 2;
            DrawText(placar, xPlacar, 245, 24, WHITE);

            if (Botao(areaBotaoSair, "Voltar ao menu"))
                tela = TELA_MENU;
        }
        else
        {
            // Mantém a música tocando enquanto a partida roda.
            if (temAmbiente)
                UpdateMusicStream(ambiente);

            // A rodada termina junto com o trecho da faixa local.
            if (temFaixaLocal && !IsMusicStreamPlaying(ambiente))
            {
                tela = TELA_FIM;
                EndDrawing();
                continue;
            }

            float dt = GetFrameTime();
            Vector2 mouse = GetMousePosition();
            bool clicou = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            DesenharCabine(janelas);

            // Cada alvo tem seu próprio tempo e pode aparecer junto com os outros.
            for (int raridade = LENDARIO; raridade >= COMUM; raridade--)
            {
                Alvo alvo = alvos[raridade];
                TipoAlvo tipo = tipos[raridade];

                if (alvo.tempoVisivel > 0.0f)
                {
                    alvo.tempoVisivel -= dt;

                    // O clique usa a posição mostrada no quadro anterior.
                    if (alvo.tempoVisivel > 0.0f && clicou &&
                        CheckCollisionPointCircle(mouse, alvo.centro, tipo.raio))
                    {
                        pontos += tipo.pontos;
                        if (temSomAcerto)
                            PlaySound(somAcerto);
                        alvo.tempoVisivel = 0.0f;
                        clicou = false;
                    }

                    alvo.centro.x += alvo.velocidadeX * dt;

                    // O azul inverte a direção nas pontas da trilha.
                    if (raridade == RARO && alvo.centro.x <= LIMITE_RARO_ESQUERDO)
                    {
                        alvo.centro.x = LIMITE_RARO_ESQUERDO;
                        alvo.velocidadeX = VELOCIDADE_RARO;
                    }
                    else if (raridade == RARO && alvo.centro.x >= LIMITE_RARO_DIREITO)
                    {
                        alvo.centro.x = LIMITE_RARO_DIREITO;
                        alvo.velocidadeX = -VELOCIDADE_RARO;
                    }
                    // O amarelo desaparece ao cruzar a trilha do fundo.
                    else if (raridade == LENDARIO &&
                             (alvo.centro.x < LIMITE_LENDARIO_ESQUERDO ||
                              alvo.centro.x > LIMITE_LENDARIO_DIREITO))
                    {
                        alvo.tempoVisivel = 0.0f;
                    }
                }
                else
                {
                    alvo.espera -= dt;
                    if (alvo.espera <= 0.0f)
                        alvo = CriarAlvo(raridade, tipo, janelas);
                }

                alvos[raridade] = alvo;

                if (alvo.tempoVisivel > 0.0f)
                {
                    DrawCircleV(alvo.centro, tipo.raio, tipo.cor);
                    DrawCircleV(alvo.centro, tipo.raio * 0.66f, BLACK);
                    DrawCircleV(alvo.centro, tipo.raio * 0.33f, tipo.cor);
                }
            }

            DrawText(TextFormat("Pontuação: %d", pontos), 32, 33, 24, WHITE);
            int xTitulo = LARGURA - 32 - MeasureText("Tiro ao Alvo", 20);
            DrawText("Tiro ao Alvo", xTitulo, 35, 20, RAYWHITE);
        }

        EndDrawing();
        if (sair)
            break;
    }

    // Libera os sons antes de fechar o dispositivo de áudio e a janela.
    if (temSomAcerto)
        UnloadSound(somAcerto);
    if (temAmbiente)
        UnloadMusicStream(ambiente);
    if (audioPronto)
        CloseAudioDevice();
    CloseWindow();

    return 0;
}

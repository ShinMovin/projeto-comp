#include "raylib.h"
#include <math.h>
#include <stddef.h>

#define LARGURA 800
#define ALTURA 600

#define TELA_MENU 0
#define TELA_CONTAGEM 1
#define TELA_JOGO 2
#define TELA_FIM 3
#define TELA_MUSICA 4

#define DURACAO_PARTIDA 30
#define DURACAO_CONTAGEM 3
#define META_PONTOS 7500
#define PONTOS_LENDARIO 500

#define COMUM 0
#define RARO 1
#define EPICO 2
#define LENDARIO 3
#define FALSO 4

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

typedef struct {
    const char *nome;
    const char *arquivo;
    float inicio;
} Faixa;

static const Color COR_PAINEL = {14, 24, 35, 255};
static const Color COR_ACENTO = {98, 220, 236, 255};
static const Color COR_AMBAR = {255, 177, 80, 255};

static void DesenharTextoCentralizado(const char *texto, int y, int tamanho, Color cor)
{
    DrawText(texto, (LARGURA - MeasureText(texto, tamanho)) / 2, y, tamanho, cor);
}

// Desenha o botão e retorna true quando ele é clicado.
bool Botao(Rectangle area, const char *texto, bool selecionado)
{
    int tamanho = 24;
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = {32, 50, 66, 255};

    if (selecionado)
        cor = (Color){27, 76, 84, 255};
    else if (sobreBotao)
        cor = (Color){47, 74, 94, 255};

    DrawRectangleRec(area, cor);
    DrawRectangleLinesEx(area, 2, selecionado || sobreBotao ? COR_ACENTO : (Color){61, 86, 105, 255});

    // Centraliza o texto dentro do retângulo.
    int x = (int)(area.x + (area.width - MeasureText(texto, tamanho)) / 2);
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(texto, x, y, tamanho, WHITE);

    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void DesenharFace(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color cor)
{
    DrawTriangle(a, c, b, cor);
    DrawTriangle(a, d, c, cor);
}

void DesenharCabine(const Rectangle janelas[])
{
    const Color aco = {62, 83, 100, 255};
    const Color junta = {31, 49, 65, 255};
    const Color trilho = {53, 103, 118, 255};
    const Vector2 entrada[] = {{47, 110}, {753, 110}, {753, 505}, {47, 505}};
    const Vector2 fundo[] = {{244, 155}, {556, 155}, {556, 286}, {244, 286}};

    DrawRectangleGradientV(0, 0, LARGURA, ALTURA, (Color){31, 48, 63, 255}, COR_PAINEL);
    DrawRectangleGradientV(244, 155, 312, 131, (Color){44, 68, 85, 255}, (Color){26, 43, 58, 255});
    DesenharFace(entrada[0], entrada[1], fundo[1], fundo[0], (Color){24, 37, 50, 255});
    DesenharFace(entrada[0], fundo[0], fundo[3], entrada[3], aco);
    DesenharFace(fundo[1], entrada[1], entrada[2], fundo[2], aco);
    DesenharFace(fundo[3], fundo[2], entrada[2], entrada[3], (Color){36, 55, 71, 255});

    // As juntas do piso convergem para a parede do fundo.
    for (int x = -2; x <= 2; x++)
        DrawLine(400 + x * 56, 286, 400 + x * 132, 505, junta);
    DrawLineEx(fundo[3], entrada[3], 3, junta);
    DrawLineEx(fundo[2], entrada[2], 3, junta);
    DrawLineEx(entrada[0], fundo[0], 2, trilho);
    DrawLineEx(entrada[1], fundo[1], 2, trilho);
    DrawRectangleLinesEx((Rectangle){244, 155, 312, 131}, 2, junta);
    DrawRectangle(278, 167, 244, 3, trilho);
    DrawRectangle(335, 167, 130, 3, COR_ACENTO);

    for (int i = 0; i < 2; i++)
    {
        Rectangle janela = janelas[i];
        int x = (int)janela.x;
        int y = (int)janela.y;
        int largura = (int)janela.width;
        int altura = (int)janela.height;
        DrawRectangle(x - 8, y - 8, largura + 16, altura + 16, junta);
        DrawRectangleGradientV(x, y, largura, altura, COR_PAINEL, (Color){30, 47, 65, 255});
        DrawRectangleLinesEx(janela, 2, trilho);
        DrawRectangle(x + 10, y + 10, 22, 3, COR_AMBAR);
        DrawRectangle(x + 10, y + altura - 10, largura - 20, 2, trilho);
    }

    const Rectangle trilhas[] = {{260, 248, 280, 7}, {178, 363, 444, 9}, {100, 464, 600, 11}};
    for (int i = 0; i < 3; i++)
    {
        Rectangle trilha = trilhas[i];
        DrawRectangleRec(trilha, COR_PAINEL);
        DrawRectangle((int)trilha.x, (int)trilha.y, (int)trilha.width, 2, trilho);
        DrawRectangle((int)trilha.x, (int)trilha.y, 24, 2, COR_ACENTO);
        DrawRectangle((int)(trilha.x + trilha.width) - 24, (int)trilha.y, 24, 2, COR_ACENTO);
    }

    DrawRectangleLinesEx((Rectangle){39, 102, 722, 411}, 8, junta);
    DrawRectangle(47, 102, 706, 4, aco);
    DrawRectangle(302, 102, 196, 4, COR_AMBAR);
    DrawRectangleGradientV(29, 505, 742, 25, aco, junta);
    DrawRectangle(49, 506, 76, 4, COR_AMBAR);
    DrawRectangle(675, 506, 76, 4, COR_AMBAR);
}

static void DesenharPainelMenu(void)
{
    const Rectangle painel = {174, 91, 452, 418};
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.45f));
    DrawRectangleRec(painel, COR_PAINEL);
    DrawRectangleLinesEx(painel, 2, (Color){61, 86, 105, 255});
    DrawRectangle(368, 91, 64, 3, COR_AMBAR);
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
    else if (raridade == FALSO)
    {
        // O alvo falso fica fora do percurso do alvo azul.
        alvo.centro = (Vector2){GetRandomValue(0, 1) == 0 ? 135 : 665, TRILHA_RARA_Y};
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

    const Faixa faixas[] = {
        {"I Wonder", "musica-local.mp3", 154.0f},
        {"Master", "master.mp3", 340.0f},
        {"Eu, Você, o Mar e Ela", "eu-voce-o-mar-e-ela.mp3", 54.5f}
    };
    Music musicas[sizeof faixas / sizeof faixas[0]] = {0};
    const int quantidadeFaixas = sizeof faixas / sizeof faixas[0];
    int faixaEscolhida = 0;
    Music *trilha = NULL;

    // Os arquivos de áudio ficam ao lado do executável após a compilação.
    InitAudioDevice();
    bool audioPronto = IsAudioDeviceReady();
    Music ambiente = {0};
    Sound somAcerto = {0};
    bool temAmbiente = false;
    bool temSomAcerto = false;
    if (audioPronto)
    {
        const char *pasta = GetApplicationDirectory();
        for (int i = 0; i < quantidadeFaixas; i++)
        {
            const char *caminho = TextFormat("%saudio-tiroalvo/%s", pasta, faixas[i].arquivo);
            if (!FileExists(caminho))
                continue;

            Music musica = LoadMusicStream(caminho);
            if (!IsMusicValid(musica))
                continue;

            if (GetMusicTimeLength(musica) <= faixas[i].inicio)
            {
                UnloadMusicStream(musica);
                continue;
            }
            musica.looping = false;
            SetMusicVolume(musica, 0.25f);
            musicas[i] = musica;
        }
        ambiente = LoadMusicStream(TextFormat("%saudio-tiroalvo/ambiente.wav", pasta));
        somAcerto = LoadSound(TextFormat("%saudio-tiroalvo/acerto.wav", pasta));
        temAmbiente = IsMusicValid(ambiente);
        temSomAcerto = IsSoundValid(somAcerto);

        if (temAmbiente)
        {
            ambiente.looping = true;
            SetMusicVolume(ambiente, 0.25f);
        }
        if (temSomAcerto)
            SetSoundVolume(somAcerto, 0.50f);
    }

    // Começa pelo menu e posiciona os botões no centro da janela.
    int tela = TELA_MENU;
    const Rectangle areaBotaoJogar = {(LARGURA - 220) / 2, 262, 220, 50};
    const Rectangle areaBotaoMusica = {(LARGURA - 220) / 2, 324, 220, 50};
    const Rectangle areaBotaoSair = {(LARGURA - 220) / 2, 386, 220, 50};
    const Rectangle areaVoltarMusica = {(LARGURA - 220) / 2, 408, 220, 50};
    const Rectangle areaBotaoVoltar = {(LARGURA - 220) / 2, 330, 220, 52};

    const Rectangle janelas[] = {{86, 190, 130, 88}, {584, 190, 130, 88}};

    // Cor, pontos por acerto, tempo na tela, pausa até reaparecer e raio.
    const TipoAlvo tipos[] = {
        {GREEN,               100, 2.5f, 0.45f, 31.0f},
        {BLUE,                200, 2.0f, 1.25f, 23.0f},
        {PURPLE,              350, 1.5f, 3.75f, 21.0f},
        {YELLOW,  PONTOS_LENDARIO, 1.0f, 6.50f, 14.0f},
        {RED,    -PONTOS_LENDARIO, 1.8f, 2.80f, 23.0f}
    };
    const float esperasIniciais[] = {0.0f, 0.65f, 2.0f, 4.0f, 1.25f};
    Alvo alvos[FALSO + 1] = {0};
    int pontos = 0;
    double limite = 0.0;

    // Loop principal
    while (!WindowShouldClose())
    {
        double agora = GetTime();
        if (tela == TELA_CONTAGEM && agora >= limite)
        {
            tela = TELA_JOGO;
            limite = agora + DURACAO_PARTIDA;
            if (trilha)
                PlayMusicStream(*trilha);
        }
        else if (tela == TELA_JOGO && (agora >= limite || pontos >= META_PONTOS))
        {
            tela = TELA_FIM;
            if (trilha)
                StopMusicStream(*trilha);
        }

        // Limpa a janela e redesenha a tela atual a cada quadro.
        BeginDrawing();
        ClearBackground(BLACK);
        DesenharCabine(janelas);

        bool sair = false;
        if (tela == TELA_MENU)
        {
            DesenharPainelMenu();
            DesenharTextoCentralizado("Tiro ao Alvo", 129, 48, WHITE);
            DesenharTextoCentralizado(TextFormat("Faça %d pontos em %d segundos", META_PONTOS, DURACAO_PARTIDA), 189, 23, RAYWHITE);
            DesenharTextoCentralizado(TextFormat("Alvo falso: -%d pontos", PONTOS_LENDARIO), 221, 20, COR_AMBAR);
            DesenharTextoCentralizado(TextFormat("Faixa: %s", faixas[faixaEscolhida].nome), 461, 18, COR_ACENTO);

            if (Botao(areaBotaoJogar, "Jogar", false))
            {
                pontos = 0;
                for (int raridade = COMUM; raridade <= FALSO; raridade++)
                    alvos[raridade] = (Alvo){.espera = esperasIniciais[raridade]};

                trilha = NULL;
                if (IsMusicValid(musicas[faixaEscolhida]))
                {
                    trilha = &musicas[faixaEscolhida];
                    SeekMusicStream(*trilha, faixas[faixaEscolhida].inicio);
                }
                else if (temAmbiente)
                {
                    trilha = &ambiente;
                }
                if (trilha)
                    UpdateMusicStream(*trilha);

                tela = TELA_CONTAGEM;
                limite = GetTime() + DURACAO_CONTAGEM;
            }
            if (Botao(areaBotaoMusica, "Música", false))
                tela = TELA_MUSICA;
            sair = Botao(areaBotaoSair, "Sair", false);
        }
        else if (tela == TELA_MUSICA)
        {
            DesenharPainelMenu();
            DesenharTextoCentralizado("Música", 132, 46, WHITE);

            for (int i = 0; i < quantidadeFaixas; i++)
            {
                const Rectangle areaFaixa = {(LARGURA - 340) / 2, 210 + 55 * i, 340, 44};
                if (Botao(areaFaixa, faixas[i].nome, i == faixaEscolhida))
                    faixaEscolhida = i;
            }

            if (Botao(areaVoltarMusica, "Voltar", false))
                tela = TELA_MENU;
        }
        else if (tela == TELA_FIM)
        {
            DesenharPainelMenu();
            bool venceu = pontos >= META_PONTOS;
            DesenharTextoCentralizado(venceu ? "Vitória!" : "Tempo esgotado", 165, 48, venceu ? GREEN : RED);
            DesenharTextoCentralizado(TextFormat("Pontuação final: %d / %d", pontos, META_PONTOS), 245, 24, WHITE);

            if (Botao(areaBotaoVoltar, "Voltar ao menu", false))
                tela = TELA_MENU;
        }
        else if (tela == TELA_CONTAGEM)
        {
            DrawRectangle(340, 235, 120, 120, Fade(COR_PAINEL, 0.90f));
            DrawRectangleLinesEx((Rectangle){340, 235, 120, 120}, 2, COR_AMBAR);
            DesenharTextoCentralizado(TextFormat("%d", (int)ceil(limite - agora)), 252, 84, WHITE);
        }
        else
        {
            if (trilha)
                UpdateMusicStream(*trilha);

            float dt = GetFrameTime();
            Vector2 mouse = GetMousePosition();
            bool clicou = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            // Cada alvo tem seu próprio tempo e pode aparecer junto com os outros.
            for (int raridade = FALSO; raridade >= COMUM; raridade--)
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
                        if (temSomAcerto && raridade != FALSO)
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
                    if (raridade == FALSO)
                    {
                        Vector2 centro = alvo.centro;
                        DrawLineEx((Vector2){centro.x - 8, centro.y - 8}, (Vector2){centro.x + 8, centro.y + 8}, 4, WHITE);
                        DrawLineEx((Vector2){centro.x - 8, centro.y + 8}, (Vector2){centro.x + 8, centro.y - 8}, 4, WHITE);
                    }
                    else
                    {
                        DrawCircleV(alvo.centro, tipo.raio * 0.33f, tipo.cor);
                    }
                }
            }

            DrawText(TextFormat("Pontos: %d / %d", pontos, META_PONTOS), 32, 33, 24, WHITE);
            int segundos = (int)ceil(limite - agora);
            const char *relogio = TextFormat("Tempo: %ds", segundos);
            DrawText(relogio, LARGURA - 32 - MeasureText(relogio, 24), 33, 24, segundos <= 5 ? COR_AMBAR : COR_ACENTO);
            if (limite - agora > DURACAO_PARTIDA - 0.8)
                DesenharTextoCentralizado("Comece!", 546, 30, GREEN);
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
    for (int i = 0; i < quantidadeFaixas; i++)
    {
        if (IsMusicValid(musicas[i]))
            UnloadMusicStream(musicas[i]);
    }
    if (audioPronto)
        CloseAudioDevice();
    CloseWindow();

    return 0;
}

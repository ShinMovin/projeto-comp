#include "tiroalvo.h"
#include "desenho.h"
#include <math.h>
#include <stdlib.h>

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

enum { COMUM, RARO, EPICO, LENDARIO, FALSO, QUANTIDADE_ALVOS };

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
    float tempo;
    bool visivel;
} Alvo;

typedef struct {
    const char *nome;
    const char *arquivo;
    float inicio;
} Faixa;

const Color corPainelTiro = {14, 24, 35, 255};
const Color corAcentoTiro = {98, 220, 236, 255};
const Color corAmbarTiro = {255, 177, 80, 255};
const Rectangle janelasTiro[] = {{86, 190, 130, 88}, {584, 190, 130, 88}};

// Desenha o botão e retorna true quando ele é clicado.
#ifdef TIROALVO_INDEPENDENTE
bool botaoTiro(Rectangle area, const char *texto, bool selecionado)
{
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = {32, 50, 66, 255};

    if (selecionado)
        cor = (Color){27, 76, 84, 255};
    else if (sobreBotao)
        cor = (Color){47, 74, 94, 255};

    DrawRectangleRec(area, cor);
    DrawRectangleLinesEx(area, 2, selecionado || sobreBotao ? corAcentoTiro : (Color){61, 86, 105, 255});
    textoNoRetangulo(texto, area, 24, WHITE);
    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
#endif

void desenharCabineTiro(void)
{
    const Color aco = {62, 83, 100, 255};
    const Color junta = {31, 49, 65, 255};
    const Color trilho = {53, 103, 118, 255};
    const Vector2 entrada[] = {{47, 110}, {753, 110}, {753, 505}, {47, 505}};
    const Vector2 fundo[] = {{244, 155}, {556, 155}, {556, 286}, {244, 286}};

    DrawRectangleGradientV(0, 0, LARGURA, ALTURA, (Color){31, 48, 63, 255}, corPainelTiro);
    DrawRectangleGradientV(244, 155, 312, 131, (Color){44, 68, 85, 255}, (Color){26, 43, 58, 255});
    DrawTriangleFan((Vector2[]){entrada[0], fundo[0], fundo[1], entrada[1]}, 4, (Color){24, 37, 50, 255});
    DrawTriangleFan((Vector2[]){entrada[0], entrada[3], fundo[3], fundo[0]}, 4, aco);
    DrawTriangleFan((Vector2[]){fundo[1], fundo[2], entrada[2], entrada[1]}, 4, aco);
    DrawTriangleFan((Vector2[]){fundo[3], entrada[3], entrada[2], fundo[2]}, 4, (Color){36, 55, 71, 255});

    // As juntas do piso convergem para a parede do fundo.
    for (int x = -2; x <= 2; x++)
        DrawLine(400 + x * 56, 286, 400 + x * 132, 505, junta);
    DrawLineEx(fundo[3], entrada[3], 3, junta);
    DrawLineEx(fundo[2], entrada[2], 3, junta);
    DrawLineEx(entrada[0], fundo[0], 2, trilho);
    DrawLineEx(entrada[1], fundo[1], 2, trilho);
    DrawRectangleLinesEx((Rectangle){244, 155, 312, 131}, 2, junta);
    DrawRectangle(278, 167, 244, 3, trilho);
    DrawRectangle(335, 167, 130, 3, corAcentoTiro);

    for (int i = 0; i < 2; i++)
    {
        Rectangle janela = janelasTiro[i];
        int x = (int)janela.x;
        int y = (int)janela.y;
        int largura = (int)janela.width;
        int altura = (int)janela.height;
        DrawRectangle(x - 8, y - 8, largura + 16, altura + 16, junta);
        DrawRectangleGradientV(x, y, largura, altura, corPainelTiro, (Color){30, 47, 65, 255});
        DrawRectangleLinesEx(janela, 2, trilho);
        DrawRectangle(x + 10, y + 10, 22, 3, corAmbarTiro);
        DrawRectangle(x + 10, y + altura - 10, largura - 20, 2, trilho);
    }

    const Rectangle trilhas[] = {{260, 248, 280, 7}, {178, 363, 444, 9}, {100, 464, 600, 11}};
    for (int i = 0; i < 3; i++)
    {
        Rectangle trilha = trilhas[i];
        DrawRectangleRec(trilha, corPainelTiro);
        DrawRectangle((int)trilha.x, (int)trilha.y, (int)trilha.width, 2, trilho);
        DrawRectangle((int)trilha.x, (int)trilha.y, 24, 2, corAcentoTiro);
        DrawRectangle((int)(trilha.x + trilha.width) - 24, (int)trilha.y, 24, 2, corAcentoTiro);
    }

    DrawRectangleLinesEx((Rectangle){39, 102, 722, 411}, 8, junta);
    DrawRectangle(47, 102, 706, 4, aco);
    DrawRectangle(302, 102, 196, 4, corAmbarTiro);
    DrawRectangleGradientV(29, 505, 742, 25, aco, junta);
    DrawRectangle(49, 506, 76, 4, corAmbarTiro);
    DrawRectangle(675, 506, 76, 4, corAmbarTiro);
}

const Faixa faixasTiro[] = {
    {"I Wonder", "musica-local.mp3", 154.0f},
    {"Master of Puppets", "master.mp3", 340.0f},
    {"Eu, Você, o Mar e Ela", "eu-voce-o-mar-e-ela.mp3", 54.5f}
};
// Cor, pontos por acerto, tempo na tela, pausa até reaparecer e raio.
const TipoAlvo tiposAlvo[] = {
    {GREEN,               100, 2.5f, 0.45f, 31.0f},
    {BLUE,                200, 2.0f, 1.25f, 23.0f},
    {PURPLE,              350, 1.5f, 3.75f, 21.0f},
    {YELLOW,  PONTOS_LENDARIO, 1.0f, 6.50f, 14.0f},
    {RED,    -PONTOS_LENDARIO, 1.8f, 2.80f, 23.0f}
};
const float esperasIniciaisTiro[] = {0.0f, 0.65f, 2.0f, 4.0f, 1.25f};

struct SessaoTiro
{
    // Os alvos têm relógios próprios. "restante" pertence à contagem ou à partida.
    Alvo alvos[QUANTIDADE_ALVOS];
    int pontos;
    int tela;
    float restante;
    Music trilha;
    Sound somAcerto;
    ResultadoMinigame resultado;
};

const char *NomeOpcaoTiro(bool treino, int opcao)
{
    (void)treino;
    return opcao >= 0 && (size_t)opcao < sizeof faixasTiro / sizeof faixasTiro[0] ? faixasTiro[opcao].nome : NULL;
}

SessaoTiro *IniciarTiro(bool treino, int opcao)
{
    if (!NomeOpcaoTiro(treino, opcao))
        return NULL;
    SessaoTiro *sessao = calloc(1, sizeof *sessao);
    if (!sessao)
        return NULL;

    sessao->tela = TELA_CONTAGEM;
    sessao->restante = DURACAO_CONTAGEM;
    for (int i = 0; i < QUANTIDADE_ALVOS; i++)
        sessao->alvos[i].tempo = esperasIniciaisTiro[i];

    if (IsAudioDeviceReady())
    {
        const char *pasta = GetApplicationDirectory();
        const char *caminho = TextFormat("%saudio-tiroalvo/%s", pasta, faixasTiro[opcao].arquivo);
        if (FileExists(caminho))
            sessao->trilha = LoadMusicStream(caminho);
        if (IsMusicValid(sessao->trilha) && GetMusicTimeLength(sessao->trilha) <= faixasTiro[opcao].inicio)
        {
            UnloadMusicStream(sessao->trilha);
            sessao->trilha = (Music){0};
        }
        if (IsMusicValid(sessao->trilha))
        {
            sessao->trilha.looping = false;
            SeekMusicStream(sessao->trilha, faixasTiro[opcao].inicio);
        }
        else
        {
            // A trilha ambiente é o fallback original quando a faixa não está disponível.
            sessao->trilha = LoadMusicStream(TextFormat("%saudio-tiroalvo/ambiente.wav", pasta));
            sessao->trilha.looping = true;
        }
        if (IsMusicValid(sessao->trilha))
        {
            SetMusicVolume(sessao->trilha, 0.25f);
            UpdateMusicStream(sessao->trilha);
        }
        sessao->somAcerto = LoadSound(TextFormat("%saudio-tiroalvo/acerto.wav", pasta));
        if (IsSoundValid(sessao->somAcerto))
            SetSoundVolume(sessao->somAcerto, 0.50f);
    }
    return sessao;
}

ResultadoMinigame AtualizarTiro(SessaoTiro *sessao, float dt)
{
    if (sessao->resultado != MINIGAME_EM_CURSO)
        return sessao->resultado;
    sessao->restante -= dt;
    if (sessao->tela == TELA_CONTAGEM)
    {
        if (sessao->restante <= 0.0f)
        {
            sessao->tela = TELA_JOGO;
            sessao->restante = DURACAO_PARTIDA;
            if (IsMusicValid(sessao->trilha))
                PlayMusicStream(sessao->trilha);
        }
        return MINIGAME_EM_CURSO;
    }
    if (sessao->restante <= 0.0f)
    {
        sessao->resultado = MINIGAME_DERROTA;
        return sessao->resultado;
    }

    if (IsMusicValid(sessao->trilha))
        UpdateMusicStream(sessao->trilha);
    Vector2 mouse = GetMousePosition();
    bool clicou = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    // Um clique só atinge o alvo de maior prioridade sob o mouse.
    for (int raridade = FALSO; raridade >= COMUM; raridade--)
    {
        Alvo *alvo = &sessao->alvos[raridade];
        const TipoAlvo *tipo = &tiposAlvo[raridade];
        alvo->tempo -= dt;
        if (!alvo->visivel)
        {
            if (alvo->tempo > 0)
                continue;

            alvo->visivel = true;
            alvo->tempo = tipo->duracao;
            alvo->velocidadeX = 0;
            int posicao = raridade == FALSO ? GetRandomValue(COMUM, FALSO) : raridade;
            switch (posicao)
            {
            case COMUM:
                alvo->centro = (Vector2){GetRandomValue(140, 660), TRILHA_COMUM_Y};
                break;
            case RARO:
                alvo->centro = (Vector2){GetRandomValue(375, 425), TRILHA_RARA_Y};
                alvo->velocidadeX = GetRandomValue(0, 1) ? VELOCIDADE_RARO : -VELOCIDADE_RARO;
                break;
            case EPICO:
            {
                Rectangle janela = janelasTiro[GetRandomValue(0, 1)];
                alvo->centro = (Vector2){janela.x + janela.width / 2, janela.y + janela.height / 2};
                break;
            }
            case LENDARIO:
                alvo->velocidadeX = GetRandomValue(0, 1) ? VELOCIDADE_LENDARIO : -VELOCIDADE_LENDARIO;
                alvo->centro = (Vector2){
                    alvo->velocidadeX > 0 ? LIMITE_LENDARIO_ESQUERDO : LIMITE_LENDARIO_DIREITO, TRILHA_LENDARIA_Y
                };
                break;
            case FALSO:
                alvo->centro = (Vector2){GetRandomValue(0, 1) ? 665 : 135, TRILHA_RARA_Y};
                break;
            }
            if (raridade == FALSO)
                alvo->velocidadeX = 0;
            continue;
        }

        // O clique usa a posição mostrada no quadro anterior.
        if (clicou && CheckCollisionPointCircle(mouse, alvo->centro, tipo->raio))
        {
            sessao->pontos += tipo->pontos;
            if (raridade != FALSO && IsSoundValid(sessao->somAcerto))
                PlaySound(sessao->somAcerto);
            alvo->tempo = 0;
            clicou = false;
        }

        alvo->centro.x += alvo->velocidadeX * dt;
        if (raridade == RARO && alvo->centro.x <= LIMITE_RARO_ESQUERDO)
        {
            alvo->centro.x = LIMITE_RARO_ESQUERDO;
            alvo->velocidadeX = VELOCIDADE_RARO;
        }
        else if (raridade == RARO && alvo->centro.x >= LIMITE_RARO_DIREITO)
        {
            alvo->centro.x = LIMITE_RARO_DIREITO;
            alvo->velocidadeX = -VELOCIDADE_RARO;
        }
        else if (raridade == LENDARIO && (alvo->centro.x < LIMITE_LENDARIO_ESQUERDO ||
                                         alvo->centro.x > LIMITE_LENDARIO_DIREITO))
            alvo->tempo = 0;

        if (alvo->tempo <= 0)
        {
            alvo->visivel = false;
            alvo->tempo = tipo->pausa;
        }
    }
    if (sessao->pontos >= META_PONTOS)
        sessao->resultado = MINIGAME_VITORIA;
    return sessao->resultado;
}

void DesenharTiro(const SessaoTiro *sessao)
{
    ClearBackground(BLACK);
    desenharCabineTiro();
    if (sessao->tela == TELA_CONTAGEM)
    {
        DrawRectangle(340, 235, 120, 120, Fade(corPainelTiro, 0.90f));
        DrawRectangleLinesEx((Rectangle){340, 235, 120, 120}, 2, corAmbarTiro);
        textoCentralizado(TextFormat("%d", (int)ceilf(sessao->restante)), 252, 84, WHITE);
        return;
    }

    // A ordem visual acompanha a prioridade do clique quando alvos se sobrepõem.
    for (int raridade = 0; raridade < QUANTIDADE_ALVOS; raridade++)
    {
        if (!sessao->alvos[raridade].visivel)
            continue;
        Vector2 centro = sessao->alvos[raridade].centro;
        float raio = tiposAlvo[raridade].raio;
        Color cor = tiposAlvo[raridade].cor;
        DrawCircleV(centro, raio, cor);
        DrawCircleV(centro, raio * 0.66f, BLACK);
        if (raridade == FALSO)
        {
            DrawLineEx((Vector2){centro.x - 8, centro.y - 8}, (Vector2){centro.x + 8, centro.y + 8}, 4, WHITE);
            DrawLineEx((Vector2){centro.x - 8, centro.y + 8}, (Vector2){centro.x + 8, centro.y - 8}, 4, WHITE);
        }
        else
            DrawCircleV(centro, raio * 0.33f, cor);
    }
    DrawText(TextFormat("Pontos: %d / %d", sessao->pontos, META_PONTOS), 32, 33, 24, WHITE);
    int segundos = (int)ceilf(sessao->restante);
    const char *relogio = TextFormat("Tempo: %ds", segundos);
    DrawText(relogio, LARGURA - 32 - MeasureText(relogio, 24), 33, 24, segundos <= 5 ? corAmbarTiro : corAcentoTiro);
    if (sessao->restante > DURACAO_PARTIDA - 0.8f)
        textoCentralizado("Comece!", 546, 30, GREEN);
}

PlacarMinigame PlacarTiro(const SessaoTiro *sessao)
{
    return (PlacarMinigame){.pontos = sessao->pontos};
}

void EncerrarTiro(SessaoTiro *sessao)
{
    if (IsMusicValid(sessao->trilha))
    {
        StopMusicStream(sessao->trilha);
        UnloadMusicStream(sessao->trilha);
    }
    if (IsSoundValid(sessao->somAcerto))
    {
        StopSound(sessao->somAcerto);
        UnloadSound(sessao->somAcerto);
    }
    free(sessao);
}

#ifdef TIROALVO_INDEPENDENTE
int main(void)
{
    InitWindow(LARGURA, ALTURA, "Tiro ao Alvo");
    SetTargetFPS(60);
    InitAudioDevice();

    int tela = TELA_MENU;
    int faixaEscolhida = 0;
    SessaoTiro *sessao = NULL;
    while (!WindowShouldClose())
    {
        if (tela == TELA_JOGO && AtualizarTiro(sessao, GetFrameTime()) != MINIGAME_EM_CURSO)
        {
            if (IsMusicValid(sessao->trilha))
                StopMusicStream(sessao->trilha);
            tela = TELA_FIM;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        bool sair = false;
        if (tela == TELA_JOGO)
            DesenharTiro(sessao);
        else
        {
            desenharCabineTiro();
            const Rectangle painel = {174, 91, 452, 418};
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.45f));
            DrawRectangleRec(painel, corPainelTiro);
            DrawRectangleLinesEx(painel, 2, (Color){61, 86, 105, 255});
            DrawRectangle(368, 91, 64, 3, corAmbarTiro);
            if (tela == TELA_MENU)
            {
                textoCentralizado("Tiro ao Alvo", 129, 48, WHITE);
                textoCentralizado(TextFormat("Faça %d pontos em %d segundos", META_PONTOS, DURACAO_PARTIDA), 189, 23, RAYWHITE);
                textoCentralizado(TextFormat("Alvo falso: -%d pontos", PONTOS_LENDARIO), 221, 20, corAmbarTiro);
                textoCentralizado(TextFormat("Faixa: %s", faixasTiro[faixaEscolhida].nome), 461, 18, corAcentoTiro);
                if (botaoTiro((Rectangle){290, 262, 220, 50}, "Jogar", false))
                {
                    sessao = IniciarTiro(true, faixaEscolhida);
                    if (sessao)
                        tela = TELA_JOGO;
                }
                if (botaoTiro((Rectangle){290, 324, 220, 50}, "Música", false))
                    tela = TELA_MUSICA;
                sair = botaoTiro((Rectangle){290, 386, 220, 50}, "Sair", false);
            }
            else if (tela == TELA_MUSICA)
            {
                textoCentralizado("Música", 132, 46, WHITE);
                for (int i = 0; NomeOpcaoTiro(true, i); i++)
                {
                    if (botaoTiro((Rectangle){230, 210 + 55 * i, 340, 44}, faixasTiro[i].nome, i == faixaEscolhida))
                        faixaEscolhida = i;
                }
                if (botaoTiro((Rectangle){290, 408, 220, 50}, "Voltar", false))
                    tela = TELA_MENU;
            }
            else if (tela == TELA_FIM)
            {
                bool venceu = sessao->resultado == MINIGAME_VITORIA;
                textoCentralizado(venceu ? "Vitória!" : "Tempo esgotado", 165, 48, venceu ? GREEN : RED);
                textoCentralizado(TextFormat("Pontuação final: %d / %d", sessao->pontos, META_PONTOS), 245, 24, WHITE);
                if (botaoTiro((Rectangle){290, 330, 220, 52}, "Voltar ao menu", false))
                {
                    EncerrarTiro(sessao);
                    sessao = NULL;
                    tela = TELA_MENU;
                }
            }
        }
        EndDrawing();
        if (sair)
            break;
    }

    if (sessao)
        EncerrarTiro(sessao);
    if (IsAudioDeviceReady())
        CloseAudioDevice();
    CloseWindow();
    return 0;
}
#endif

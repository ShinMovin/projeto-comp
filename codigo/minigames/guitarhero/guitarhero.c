#include "raylib.h"
#include <math.h>
#include <stddef.h>

#define LARGURA 800
#define ALTURA 600

#define TRILHAS 4
#define MAX_NOTAS 256
#define LARGURA_TRILHA 90
#define LARGURA_CAMPO (TRILHAS * LARGURA_TRILHA)
#define INICIO_CAMPO ((LARGURA - LARGURA_CAMPO) / 2)
#define VELOCIDADE_NOTAS 375.0f
#define DURACAO_PARTIDA 30
#define PREPARACAO_MUSICA 1.5f
#define INTERVALO_NOTAS 0.6f
#define DURACAO_SUSTENTADA 0.9f
#define LINHA_ACERTO 480
#define RAIO_NOTA 22
#define DISTANCIA_ACERTO 35.0f
#define DISTANCIA_PERFEITO 8.0f
#define DISTANCIA_OTIMO 20.0f
#define DURACAO_MENSAGEM 0.75f
#define DURACAO_PULSO 0.24f
#define MAX_ERROS 3
#define ACERTOS_POR_NIVEL 5
#define MULTIPLICADOR_MAXIMO 4

typedef enum {
    TELA_MENU,
    TELA_MUSICAS,
    TELA_JOGO,
    TELA_RESULTADO
} Tela;

typedef enum {
    PENDENTE,
    SUSTENTANDO,
    RESOLVIDA
} EstadoNota;

typedef enum {
    PERFEITO,
    OTIMO,
    BOM,
    NIVEIS_PRECISAO
} Precisao;

typedef struct {
    int pontos;
    const char *mensagem;
    Color cor;
} NivelPrecisao;

typedef struct {
    float inicio;
    float duracao;
    int trilha;
} NotaMapa;

#include "mapas.h"

typedef struct {
    const char *nome;
    const char *arquivo;
    const NotaMapa *mapa;
    size_t quantidade;
} Faixa;

#define TAMANHO_VETOR(vetor) (sizeof(vetor) / sizeof((vetor)[0]))

static const Faixa faixas[] = {
    {"Hedwig's Theme", "harry-potter.wav", harryPotter, TAMANHO_VETOR(harryPotter)},
    {"Blue Bird", "blue-bird.wav", blueBird, TAMANHO_VETOR(blueBird)},
    {"Sweden", "sweden.wav", sweden, TAMANHO_VETOR(sweden)}
};

typedef struct {
    NotaMapa mapa;
    EstadoNota estado;
    Precisao precisao;
} Nota;

typedef struct {
    Nota notas[MAX_NOTAS];
    float duracao;
    bool modoBonus;
    int quantidadeNotas;
    int pontos;
    int erros;
    int acertos[NIVEIS_PRECISAO];
    int combo;
    int maiorCombo;
    int multiplicador;
    const char *mensagem;
    Color corMensagem;
    float fimMensagem;
    Color coresPulso[TRILHAS];
    float finsPulso[TRILHAS];
} Partida;

static const int teclas[TRILHAS] = {KEY_D, KEY_F, KEY_J, KEY_K};
static const char *const nomesTeclas[TRILHAS] = {"D", "F", "J", "K"};
static const Color coresTrilhas[TRILHAS] = {GREEN, RED, YELLOW, BLUE};
static const NivelPrecisao niveis[NIVEIS_PRECISAO] = {
    {100, "Perfeito!", GREEN},
    {75, "Ótimo!", SKYBLUE},
    {50, "Bom!", GOLD}
};

static void TextoCentralizado(const char *texto, int y, int tamanho, Color cor)
{
    DrawText(texto, (LARGURA - MeasureText(texto, tamanho)) / 2, y, tamanho, cor);
}

static bool Botao(Rectangle area, const char *texto)
{
    const int tamanho = 24;
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = sobreBotao ? DARKGRAY : (Color){30, 30, 45, 255};
    DrawRectangleRec(area, cor);
    int x = (int)(area.x + (area.width - MeasureText(texto, tamanho)) / 2);
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(texto, x, y, tamanho, WHITE);

    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void IniciarTreino(Partida *partida)
{
    *partida = (Partida){.multiplicador = 1, .duracao = DURACAO_PARTIDA};
    float livreEm[TRILHAS] = {0};
    const float janela = DISTANCIA_ACERTO / VELOCIDADE_NOTAS;

    // Os tempos indicam quando a cabeça e a cauda chegam à linha de acerto.
    for (int i = 0; i < MAX_NOTAS; i++)
    {
        float inicio = 2.3f + i * INTERVALO_NOTAS;
        float duracao = (i + 1) % 5 == 0 ? DURACAO_SUSTENTADA : 0.0f;
        float fim = inicio + fmaxf(janela, duracao);
        if (fim >= DURACAO_PARTIDA)
            break;

        // A cauda de uma nota pode avançar sobre o intervalo da próxima.
        int disponiveis[TRILHAS];
        int quantidade = 0;
        for (int t = 0; t < TRILHAS; t++)
        {
            if (inicio - janela > livreEm[t])
                disponiveis[quantidade++] = t;
        }

        // Há apenas uma nota sustentada a cada cinco no treino.
        int trilha = disponiveis[GetRandomValue(0, quantidade - 1)];
        livreEm[trilha] = fim;
        partida->notas[partida->quantidadeNotas++] = (Nota){
            .mapa = {inicio, duracao, trilha}
        };
    }
}

static Music CarregarFaixa(const Faixa *faixa, Partida *partida)
{
    Music musica = LoadMusicStream(TextFormat("%sguitarhero-musicas/%s", GetApplicationDirectory(), faixa->arquivo));
    if (!IsMusicValid(musica))
        return (Music){0};

    float duracao = GetMusicTimeLength(musica);
    if (faixa->quantidade > MAX_NOTAS || duracao <= 0.0f || duracao > 60.0f)
    {
        UnloadMusicStream(musica);
        return (Music){0};
    }

    *partida = (Partida){
        .multiplicador = 1, .duracao = duracao, .modoBonus = true,
        .quantidadeNotas = (int)faixa->quantidade
    };
    for (size_t i = 0; i < faixa->quantidade; i++)
        partida->notas[i].mapa = faixa->mapa[i];

    musica.looping = false;
    SetMusicVolume(musica, 0.65f);
    return musica;
}

static void MostrarFeedback(Partida *partida, int trilha, float tempo, const char *texto, Color cor)
{
    partida->mensagem = texto;
    partida->corMensagem = cor;
    partida->fimMensagem = tempo + DURACAO_MENSAGEM;
    partida->coresPulso[trilha] = cor;
    partida->finsPulso[trilha] = tempo + DURACAO_PULSO;
}

static Precisao AvaliarPrecisao(float distancia)
{
    if (distancia <= DISTANCIA_PERFEITO)
        return PERFEITO;
    if (distancia <= DISTANCIA_OTIMO)
        return OTIMO;
    return BOM;
}

static void RegistrarAcerto(Partida *partida, Nota *nota, float tempo)
{
    NivelPrecisao nivel = niveis[nota->precisao];
    nota->estado = RESOLVIDA;
    partida->acertos[nota->precisao]++;
    partida->combo++;
    if (partida->combo > partida->maiorCombo)
        partida->maiorCombo = partida->combo;

    int multiplicador = 1 + partida->combo / ACERTOS_POR_NIVEL;
    partida->multiplicador = multiplicador > MULTIPLICADOR_MAXIMO ? MULTIPLICADOR_MAXIMO : multiplicador;
    partida->pontos += nivel.pontos * partida->multiplicador;
    MostrarFeedback(partida, nota->mapa.trilha, tempo, nivel.mensagem, nivel.cor);
}

static void RegistrarErro(Partida *partida, Nota *nota, float tempo, const char *texto)
{
    nota->estado = RESOLVIDA;
    partida->erros++;
    partida->combo = 0;
    partida->multiplicador = 1;
    MostrarFeedback(partida, nota->mapa.trilha, tempo, texto, RED);
}

static bool EsgotouErros(const Partida *partida)
{
    return !partida->modoBonus && partida->erros >= MAX_ERROS;
}

static const char *TextoErros(const Partida *partida)
{
    return partida->modoBonus
        ? TextFormat("Erros: %d", partida->erros)
        : TextFormat("Erros: %d/%d", partida->erros, MAX_ERROS);
}

static void AtualizarPartida(Partida *partida, float tempo)
{
    for (int i = 0; i < partida->quantidadeNotas; i++)
    {
        Nota *nota = &partida->notas[i];
        if (nota->estado == SUSTENTANDO)
        {
            if (tempo >= nota->mapa.inicio + nota->mapa.duracao)
                RegistrarAcerto(partida, nota, tempo);
            else if (!IsKeyDown(teclas[nota->mapa.trilha]))
                RegistrarErro(partida, nota, tempo, "Soltou cedo!");
        }
        else if (nota->estado == PENDENTE && (tempo - nota->mapa.inicio) * VELOCIDADE_NOTAS > DISTANCIA_ACERTO)
        {
            RegistrarErro(partida, nota, tempo, "Nota perdida!");
        }

        if (EsgotouErros(partida))
            return;
    }

    for (int t = 0; t < TRILHAS; t++)
    {
        if (!IsKeyPressed(teclas[t]))
            continue;

        Nota *melhorNota = NULL;
        float menorDistancia = DISTANCIA_ACERTO;
        for (int i = 0; i < partida->quantidadeNotas; i++)
        {
            Nota *nota = &partida->notas[i];
            if (nota->estado != PENDENTE || nota->mapa.trilha != t)
                continue;

            float distancia = fabsf(tempo - nota->mapa.inicio) * VELOCIDADE_NOTAS;
            if (distancia <= menorDistancia)
            {
                menorDistancia = distancia;
                melhorNota = nota;
            }
        }

        if (melhorNota == NULL)
            continue;

        melhorNota->precisao = AvaliarPrecisao(menorDistancia);
        if (melhorNota->mapa.duracao > 0.0f)
        {
            melhorNota->estado = SUSTENTANDO;
            MostrarFeedback(partida, t, tempo, "Segure!", coresTrilhas[t]);
        }
        else
        {
            RegistrarAcerto(partida, melhorNota, tempo);
        }
    }
}

static void DesenharPartida(const Partida *partida, float tempo)
{
    for (int t = 0; t < TRILHAS; t++)
    {
        int x = INICIO_CAMPO + t * LARGURA_TRILHA;
        DrawRectangle(x, 0, LARGURA_TRILHA, ALTURA, (Color){30, 30, 45, 255});
        if (IsKeyDown(teclas[t]))
            DrawRectangleGradientV(x, 0, LARGURA_TRILHA, ALTURA, BLANK, Fade(coresTrilhas[t], 0.35f));
        DrawLine(x, 0, x, ALTURA, DARKGRAY);
    }
    DrawLine(INICIO_CAMPO + LARGURA_CAMPO, 0, INICIO_CAMPO + LARGURA_CAMPO, ALTURA, DARKGRAY);
    DrawRectangle(INICIO_CAMPO, LINHA_ACERTO - 2, LARGURA_CAMPO, 4, WHITE);

    for (int t = 0; t < TRILHAS; t++)
    {
        int centroX = INICIO_CAMPO + t * LARGURA_TRILHA + LARGURA_TRILHA / 2;
        Color cor = coresTrilhas[t];
        if (IsKeyDown(teclas[t]))
            DrawCircle(centroX, LINHA_ACERTO, 28, cor);
        else
            DrawCircleLines(centroX, LINHA_ACERTO, 28, cor);

        float pulso = (partida->finsPulso[t] - tempo) / DURACAO_PULSO;
        if (partida->finsPulso[t] > 0.0f && pulso > 0.0f)
        {
            Vector2 centro = {centroX, LINHA_ACERTO};
            float raio = 28.0f + (1.0f - pulso) * 15.0f;
            DrawRing(centro, raio, raio + 4.0f, 0, 360, 36, Fade(partida->coresPulso[t], pulso));
        }
        DrawText(nomesTeclas[t], centroX - 8, LINHA_ACERTO + 45, 24, cor);
    }

    // Começa pelas notas mais distantes para que a próxima fique por cima.
    for (int i = partida->quantidadeNotas - 1; i >= 0; i--)
    {
        const Nota *nota = &partida->notas[i];
        if (nota->estado == RESOLVIDA)
            continue;

        float y = LINHA_ACERTO + (tempo - nota->mapa.inicio) * VELOCIDADE_NOTAS;
        if (y < -RAIO_NOTA)
            continue;

        int centroX = INICIO_CAMPO + nota->mapa.trilha * LARGURA_TRILHA + LARGURA_TRILHA / 2;
        Color cor = coresTrilhas[nota->mapa.trilha];
        if (nota->mapa.duracao > 0.0f)
        {
            float yCauda = y - nota->mapa.duracao * VELOCIDADE_NOTAS;
            if (nota->estado == SUSTENTANDO)
                y = LINHA_ACERTO;
            float topo = fmaxf(0, yCauda);
            if (y > topo)
            {
                DrawRectangleRec((Rectangle){centroX - 11, topo, 22, y - topo}, Fade(cor, 0.55f));
                DrawRectangleRec((Rectangle){centroX - 3, topo, 6, y - topo}, cor);
            }
        }
        DrawCircle(centroX, (int)y, RAIO_NOTA, cor);
        DrawCircleLines(centroX, (int)y, RAIO_NOTA, WHITE);
    }

    DrawText(TextoErros(partida), 16, 16, 20, WHITE);
    DrawText(TextFormat("Pontos: %d", partida->pontos), 16, 44, 20, WHITE);
    DrawText(TextFormat("Combo: %d (x%d)", partida->combo, partida->multiplicador), 16, 72, 20, WHITE);
    int segundosRestantes = (int)ceilf(partida->duracao - fmaxf(0.0f, tempo));
    DrawText(TextFormat("Tempo: %d s", segundosRestantes), 16, 100, 20, WHITE);
    if (tempo < 0.0f)
        TextoCentralizado("Prepare-se!", 560, 24, RAYWHITE);
    else if (tempo < partida->fimMensagem)
        TextoCentralizado(partida->mensagem, 560, 24, partida->corMensagem);
}

static void DesenharResultado(const Partida *partida)
{
    bool perdeu = EsgotouErros(partida);
    const char *titulo = partida->modoBonus ? "Faixa concluída!" : (perdeu ? "Você perdeu!" : "Você venceu!");
    TextoCentralizado(titulo, 94, 48, perdeu ? RED : GREEN);
    if (perdeu)
        TextoCentralizado(partida->mensagem, 153, 22, partida->corMensagem);

    int julgadas = partida->erros;
    int pontosBase = 0;
    for (int i = 0; i < NIVEIS_PRECISAO; i++)
    {
        julgadas += partida->acertos[i];
        pontosBase += partida->acertos[i] * niveis[i].pontos;
    }
    float precisao = julgadas > 0 ? 100.0f * pontosBase / (julgadas * niveis[PERFEITO].pontos) : 0.0f;
    TextoCentralizado(TextFormat("Pontuação final: %d", partida->pontos), 204, 28, WHITE);
    TextoCentralizado(TextFormat("Precisão: %.1f%%", precisao), 244, 24, WHITE);
    TextoCentralizado(TextFormat("Maior combo: %d", partida->maiorCombo), 281, 24, WHITE);
    TextoCentralizado(TextFormat("Perfeitos: %d   Ótimos: %d   Bons: %d",
        partida->acertos[PERFEITO], partida->acertos[OTIMO], partida->acertos[BOM]), 332, 20, RAYWHITE);
    TextoCentralizado(TextoErros(partida), 363, 20, RAYWHITE);
}

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Guitar Hero - Base");
    SetTargetFPS(60);
    InitAudioDevice();
    const bool audioPronto = IsAudioDeviceReady();

    Partida partida = {0};
    Tela tela = TELA_MENU;
    double inicioPartida = 0.0;
    float tempoMusica = 0.0f;
    Music musica = {0};
    bool tocandoFaixa = false;
    const char *erroFaixa = NULL;
    const Rectangle areaJogar = {(LARGURA - 220) / 2, 278, 220, 52};
    const Rectangle areaMusicas = {(LARGURA - 220) / 2, 346, 220, 52};
    const Rectangle areaSair = {(LARGURA - 220) / 2, 414, 220, 52};
    const Rectangle areaVoltar = {(LARGURA - 240) / 2, 438, 240, 52};

    while (!WindowShouldClose())
    {
        float tempo = 0.0f;
        if (tela == TELA_JOGO)
        {
            tempo = (float)(GetTime() - inicioPartida);
            if (IsMusicValid(musica))
            {
                tempo -= PREPARACAO_MUSICA;
                if (tempo >= 0.0f && !tocandoFaixa)
                {
                    UpdateMusicStream(musica);
                    PlayMusicStream(musica);
                    tocandoFaixa = true;
                }
                if (tocandoFaixa)
                {
                    UpdateMusicStream(musica);
                    // No fim da reprodução, o relógio do stream volta a zero.
                    tempoMusica = IsMusicStreamPlaying(musica)
                        ? fmaxf(tempoMusica, GetMusicTimePlayed(musica))
                        : partida.duracao;
                    tempo = tempoMusica;
                }
            }
            tempo = fminf(tempo, partida.duracao);
            if (tempo >= 0.0f)
                AtualizarPartida(&partida, tempo);
            if (EsgotouErros(&partida) || tempo >= partida.duracao)
            {
                tela = TELA_RESULTADO;
                if (IsMusicValid(musica))
                {
                    StopMusicStream(musica);
                    UnloadMusicStream(musica);
                    musica = (Music){0};
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        bool iniciarTreino = false;
        bool sair = false;
        const Faixa *faixaEscolhida = NULL;
        switch (tela)
        {
            case TELA_MENU:
                TextoCentralizado("Guitar Hero", 113, 48, WHITE);
                TextoCentralizado(TextFormat("Sobreviva por %d segundos", DURACAO_PARTIDA), 185, 20, WHITE);
                TextoCentralizado("D F J K  |  Segure as notas com cauda", 217, 20, LIGHTGRAY);
                iniciarTreino = Botao(areaJogar, "Jogar");
                if (Botao(areaMusicas, "Músicas"))
                {
                    tela = TELA_MUSICAS;
                    erroFaixa = NULL;
                }
                sair = Botao(areaSair, "Sair");
                break;

            case TELA_MUSICAS:
            {
                TextoCentralizado("Músicas bônus", 48, 48, WHITE);
                float y = 140;
                for (size_t i = 0; i < TAMANHO_VETOR(faixas); i++)
                {
                    Rectangle area = {(LARGURA - 356) / 2, y, 356, 46};
                    if (Botao(area, faixas[i].nome))
                        faixaEscolhida = &faixas[i];
                    y += 56;
                }
                iniciarTreino = Botao((Rectangle){(LARGURA - 220) / 2, y + 20, 220, 50}, "Treino livre");
                if (Botao((Rectangle){(LARGURA - 220) / 2, y + 90, 220, 50}, "Voltar"))
                    tela = TELA_MENU;
                if (erroFaixa)
                    TextoCentralizado(erroFaixa, 530, 20, ORANGE);
                break;
            }

            case TELA_JOGO:
                DesenharPartida(&partida, tempo);
                break;

            case TELA_RESULTADO:
                DesenharResultado(&partida);
                if (Botao(areaVoltar, "Voltar ao menu"))
                    tela = TELA_MENU;
                break;
        }
        EndDrawing();

        if (iniciarTreino)
        {
            IniciarTreino(&partida);
            inicioPartida = GetTime();
            tela = TELA_JOGO;
        }
        else if (faixaEscolhida)
        {
            if (audioPronto)
                musica = CarregarFaixa(faixaEscolhida, &partida);
            if (IsMusicValid(musica))
            {
                inicioPartida = GetTime();
                tempoMusica = 0.0f;
                tocandoFaixa = false;
                tela = TELA_JOGO;
            }
            else
            {
                erroFaixa = audioPronto ? "Não foi possível carregar a faixa." : "Dispositivo de áudio indisponível.";
            }
        }
        if (sair)
            break;
    }

    if (IsMusicValid(musica))
        UnloadMusicStream(musica);
    if (audioPronto)
        CloseAudioDevice();
    CloseWindow();
    return 0;
}

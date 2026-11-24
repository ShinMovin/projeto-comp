#include "guitarhero.h"
#include "../../desenho.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#define LARGURA 800
#define ALTURA 600

#define TRILHAS 4
#define MAX_NOTAS 256
#define LARGURA_TRILHA 90
#define LARGURA_CAMPO (TRILHAS * LARGURA_TRILHA)
#define INICIO_CAMPO ((LARGURA - LARGURA_CAMPO) / 2)
#define VELOCIDADE_NOTAS 337.5f
#define DURACAO_PARTIDA 30
#define PREPARACAO_MUSICA 1.5f
#define MARGEM_SAIDA_AUDIO 0.65f
#define INTERVALO_NOTAS 0.6f
#define DURACAO_SUSTENTADA 0.9f
#define LINHA_ACERTO 480
#define RAIO_NOTA 22
// As janelas mantêm a tolerância original em segundos, independente da descida.
#define JANELA_ACERTO (35.0f / 375.0f)
#define JANELA_PERFEITO (8.0f / 375.0f)
#define JANELA_OTIMO (20.0f / 375.0f)
#define TOLERANCIA_SOLTURA 0.30f
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

#ifdef GUITARHERO_INDEPENDENTE
static bool Botao(Rectangle area, const char *texto)
{
    const int tamanho = 24;
    bool sobreBotao = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = sobreBotao ? DARKGRAY : (Color){30, 30, 45, 255};
    DrawRectangleRec(area, cor);
    textoNoRetangulo(texto, area, tamanho, WHITE);

    return sobreBotao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

#endif

static void IniciarTreino(Partida *partida)
{
    *partida = (Partida){.multiplicador = 1, .duracao = DURACAO_PARTIDA};
    float livreEm[TRILHAS] = {0};
    const float janela = JANELA_ACERTO;

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

static Sound CarregarFaixa(const Faixa *faixa, Partida *partida)
{
    Wave onda = LoadWave(TextFormat("%sguitarhero-musicas/%s", GetApplicationDirectory(), faixa->arquivo));
    if (!IsWaveValid(onda))
        return (Sound){0};

    float duracao = (float)onda.frameCount / onda.sampleRate;
    if (faixa->quantidade > MAX_NOTAS || duracao <= 0.0f || duracao > 60.0f)
    {
        UnloadWave(onda);
        return (Sound){0};
    }

    // Os WAVs são curtos. O arquivo é lido inteiro antes da contagem inicial.
    Sound som = LoadSoundFromWave(onda);
    UnloadWave(onda);
    if (!IsSoundValid(som))
        return (Sound){0};

    *partida = (Partida){.multiplicador = 1, .duracao = duracao, .modoBonus = true};
    for (size_t i = 0; i < faixa->quantidade; i++)
    {
        NotaMapa nota = faixa->mapa[i];
        if (nota.inicio >= duracao)
            break;
        nota.duracao = fminf(nota.duracao, duracao - nota.inicio);
        partida->notas[partida->quantidadeNotas++].mapa = nota;
    }
    if (partida->quantidadeNotas == 0)
    {
        UnloadSound(som);
        return (Sound){0};
    }
    SetSoundVolume(som, 0.65f);
    return som;
}

static void MostrarFeedback(Partida *partida, int trilha, float tempo, const char *texto, Color cor)
{
    partida->mensagem = texto;
    partida->corMensagem = cor;
    partida->fimMensagem = tempo + DURACAO_MENSAGEM;
    partida->coresPulso[trilha] = cor;
    partida->finsPulso[trilha] = tempo + DURACAO_PULSO;
}

static Precisao AvaliarPrecisao(float diferenca)
{
    if (diferenca <= JANELA_PERFEITO)
        return PERFEITO;
    if (diferenca <= JANELA_OTIMO)
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
            float fim = nota->mapa.inicio + nota->mapa.duracao;
            float toleranciaCedo = fminf(TOLERANCIA_SOLTURA, nota->mapa.duracao * 0.4f);

            if (tempo > fim + TOLERANCIA_SOLTURA)
                RegistrarErro(partida, nota, tempo, "Soltou tarde!");
            else if (!IsKeyDown(teclas[nota->mapa.trilha]) || tempo >= partida->duracao)
            {
                if (tempo < fim - toleranciaCedo)
                    RegistrarErro(partida, nota, tempo, "Soltou cedo!");
                else
                    RegistrarAcerto(partida, nota, tempo);
            }
        }
        else if (nota->estado == PENDENTE && tempo - nota->mapa.inicio > JANELA_ACERTO)
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
        float menorDiferenca = JANELA_ACERTO;
        for (int i = 0; i < partida->quantidadeNotas; i++)
        {
            Nota *nota = &partida->notas[i];
            if (nota->estado != PENDENTE || nota->mapa.trilha != t)
                continue;

            float diferenca = fabsf(tempo - nota->mapa.inicio);
            if (diferenca <= menorDiferenca)
            {
                menorDiferenca = diferenca;
                melhorNota = nota;
            }
        }

        if (melhorNota == NULL)
            continue;

        melhorNota->precisao = AvaliarPrecisao(menorDiferenca);
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

static void DesenharPartida(const Partida *partida, float tempo, bool finalizando)
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
            {
                if (tempo >= nota->mapa.inicio + nota->mapa.duracao)
                    continue;
                y = LINHA_ACERTO;
            }
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
    if (finalizando)
        textoCentralizado("Finalizando faixa...", 560, 24, RAYWHITE);
    else if (tempo < 0.0f)
        textoCentralizado("Prepare-se!", 560, 24, RAYWHITE);
    else if (tempo < partida->fimMensagem)
        textoCentralizado(partida->mensagem, 560, 24, partida->corMensagem);
}

#ifdef GUITARHERO_INDEPENDENTE
static void DesenharResultado(const Partida *partida)
{
    bool perdeu = EsgotouErros(partida);
    const char *titulo = partida->modoBonus ? "Faixa concluída!" : (perdeu ? "Você perdeu!" : "Você venceu!");
    textoCentralizado(titulo, 94, 48, perdeu ? RED : GREEN);
    if (perdeu)
        textoCentralizado(partida->mensagem, 153, 22, partida->corMensagem);

    int julgadas = partida->erros;
    int pontosBase = 0;
    for (int i = 0; i < NIVEIS_PRECISAO; i++)
    {
        julgadas += partida->acertos[i];
        pontosBase += partida->acertos[i] * niveis[i].pontos;
    }
    float precisao = julgadas > 0 ? 100.0f * pontosBase / (julgadas * niveis[PERFEITO].pontos) : 0.0f;
    textoCentralizado(TextFormat("Pontuação final: %d", partida->pontos), 204, 28, WHITE);
    textoCentralizado(TextFormat("Precisão: %.1f%%", precisao), 244, 24, WHITE);
    textoCentralizado(TextFormat("Maior combo: %d", partida->maiorCombo), 281, 24, WHITE);
    textoCentralizado(TextFormat("Perfeitos: %d   Ótimos: %d   Bons: %d",
        partida->acertos[PERFEITO], partida->acertos[OTIMO], partida->acertos[BOM]), 332, 20, RAYWHITE);
    textoCentralizado(TextoErros(partida), 363, 20, RAYWHITE);
}

#endif

typedef struct
{
    Partida partida;
    Sound somFaixa;
    float tempo;
    float esperaSaidaAudio;
    bool tocandoFaixa;
    bool treino;
    ResultadoMinigame resultado;
} SessaoGuitarra;

static const char *NomeOpcaoGuitarra(bool treino, int opcao)
{
    if (treino && opcao == 0)
        return "Treino livre";
    int faixa = opcao - (treino ? 1 : 0);
    return faixa >= 0 && (size_t)faixa < TAMANHO_VETOR(faixas) ? faixas[faixa].nome : NULL;
}

static void *IniciarGuitarra(bool treino, int opcao)
{
    if (!NomeOpcaoGuitarra(treino, opcao))
        return NULL;

    SessaoGuitarra *sessao = calloc(1, sizeof *sessao);
    if (!sessao)
        return NULL;
    sessao->treino = treino;
    if (treino && opcao == 0)
        IniciarTreino(&sessao->partida);
    else
    {
        if (IsAudioDeviceReady())
            sessao->somFaixa = CarregarFaixa(&faixas[opcao - (treino ? 1 : 0)], &sessao->partida);
        if (!IsSoundValid(sessao->somFaixa))
        {
            free(sessao);
            return NULL;
        }
        sessao->tempo = -PREPARACAO_MUSICA;
    }
    return sessao;
}

static int TotalAcertos(const Partida *partida)
{
    int total = 0;
    for (int i = 0; i < NIVEIS_PRECISAO; i++)
        total += partida->acertos[i];
    return total;
}

static void ResolverNotasRestantes(Partida *partida, float tempo)
{
    for (int i = 0; i < partida->quantidadeNotas; i++)
    {
        if (partida->notas[i].estado != RESOLVIDA)
            RegistrarErro(partida, &partida->notas[i], tempo, "Nota perdida!");
    }
}

static ResultadoMinigame ConcluirGuitarra(SessaoGuitarra *sessao)
{
    const Partida *partida = &sessao->partida;
    bool venceu = sessao->treino ||
                  (partida->quantidadeNotas > 0 && 100 * TotalAcertos(partida) >= 70 * partida->quantidadeNotas);
    sessao->resultado = venceu ? MINIGAME_VITORIA : MINIGAME_DERROTA;
    return sessao->resultado;
}

static ResultadoMinigame AtualizarGuitarra(void *dados, float tempoQuadro)
{
    SessaoGuitarra *sessao = dados;
    Partida *partida = &sessao->partida;
    if (sessao->resultado != MINIGAME_EM_CURSO)
        return sessao->resultado;
    if (sessao->esperaSaidaAudio > 0.0f)
    {
        sessao->esperaSaidaAudio -= tempoQuadro;
        return sessao->esperaSaidaAudio > 0.0f ? MINIGAME_EM_CURSO : ConcluirGuitarra(sessao);
    }

    bool temFaixa = IsSoundValid(sessao->somFaixa);
    sessao->tempo += tempoQuadro;
    if (temFaixa && !sessao->tocandoFaixa)
    {
        if (sessao->tempo < 0.0f)
            return MINIGAME_EM_CURSO;
        // O relógio das notas começa no mesmo quadro que dispara a faixa.
        sessao->tempo = 0.0f;
        PlaySound(sessao->somFaixa);
        sessao->tocandoFaixa = true;
    }

    sessao->tempo = fminf(sessao->tempo, partida->duracao);
    if (temFaixa && !IsSoundPlaying(sessao->somFaixa))
    {
        // O dispositivo ainda pode tocar amostras que o mixer já entregou.
        sessao->tempo = partida->duracao;
        ResolverNotasRestantes(partida, sessao->tempo);
        sessao->esperaSaidaAudio = MARGEM_SAIDA_AUDIO;
        return MINIGAME_EM_CURSO;
    }

    AtualizarPartida(partida, sessao->tempo);
    if (EsgotouErros(partida))
        sessao->resultado = MINIGAME_DERROTA;
    else if (!temFaixa && sessao->tempo >= partida->duracao)
    {
        ResolverNotasRestantes(partida, sessao->tempo);
        return ConcluirGuitarra(sessao);
    }
    return sessao->resultado;
}

static void DesenharGuitarra(const void *dados)
{
    const SessaoGuitarra *sessao = dados;
    ClearBackground(BLACK);
    DesenharPartida(&sessao->partida, sessao->tempo, sessao->esperaSaidaAudio > 0.0f);
    if (!sessao->treino)
        DrawText(TextFormat("Notas acertadas: %d/%d", TotalAcertos(&sessao->partida),
                            sessao->partida.quantidadeNotas), 16, 129, 17, GOLD);
}

static PlacarMinigame PlacarGuitarra(const void *dados)
{
    const SessaoGuitarra *sessao = dados;
    return (PlacarMinigame){.pontos = sessao->partida.pontos, .acertos = TotalAcertos(&sessao->partida),
                           .notas = sessao->partida.quantidadeNotas};
}

static void EncerrarGuitarra(void *dados)
{
    SessaoGuitarra *sessao = dados;
    if (IsSoundValid(sessao->somFaixa))
    {
        if (IsSoundPlaying(sessao->somFaixa))
            StopSound(sessao->somFaixa);
        UnloadSound(sessao->somFaixa);
    }
    free(sessao);
}

const Minigame guitarHero = {
    .nomeOpcao = NomeOpcaoGuitarra,
    .iniciar = IniciarGuitarra,
    .atualizar = AtualizarGuitarra,
    .desenhar = DesenharGuitarra,
    .placar = PlacarGuitarra,
    .encerrar = EncerrarGuitarra
};

#ifdef GUITARHERO_INDEPENDENTE
int main(void)
{
    InitWindow(LARGURA, ALTURA, "Guitar Hero - Base");
    SetTargetFPS(60);
    InitAudioDevice();

    SessaoGuitarra *sessao = NULL;
    Tela tela = TELA_MENU;
    const char *erroFaixa = NULL;

    while (!WindowShouldClose())
    {
        if (tela == TELA_JOGO && guitarHero.atualizar(sessao, GetFrameTime()) != MINIGAME_EM_CURSO)
            tela = TELA_RESULTADO;

        int iniciarOpcao = -1;
        bool sair = false;
        BeginDrawing();
        ClearBackground(BLACK);
        switch (tela)
        {
        case TELA_MENU:
            textoCentralizado("Guitar Hero", 113, 48, WHITE);
            textoCentralizado(TextFormat("Sobreviva por %d segundos", DURACAO_PARTIDA), 185, 20, WHITE);
            textoCentralizado("D F J K  |  Segure as notas com cauda", 217, 20, LIGHTGRAY);
            if (Botao((Rectangle){290, 278, 220, 52}, "Jogar"))
                iniciarOpcao = 0;
            if (Botao((Rectangle){290, 346, 220, 52}, "Músicas"))
            {
                tela = TELA_MUSICAS;
                erroFaixa = NULL;
            }
            sair = Botao((Rectangle){290, 414, 220, 52}, "Sair");
            break;

        case TELA_MUSICAS:
            textoCentralizado("Músicas bônus", 48, 48, WHITE);
            for (size_t i = 0; i < TAMANHO_VETOR(faixas); i++)
            {
                if (Botao((Rectangle){222, 140 + 56 * (float)i, 356, 46}, faixas[i].nome))
                    iniciarOpcao = (int)i + 1;
            }
            if (Botao((Rectangle){290, 326, 220, 50}, "Treino livre"))
                iniciarOpcao = 0;
            if (Botao((Rectangle){290, 396, 220, 50}, "Voltar"))
                tela = TELA_MENU;
            if (erroFaixa)
                textoCentralizado(erroFaixa, 530, 20, ORANGE);
            break;

        case TELA_JOGO:
            guitarHero.desenhar(sessao);
            break;

        case TELA_RESULTADO:
            DesenharResultado(&sessao->partida);
            if (Botao((Rectangle){280, 438, 240, 52}, "Voltar ao menu"))
            {
                guitarHero.encerrar(sessao);
                sessao = NULL;
                tela = TELA_MENU;
            }
            break;
        }
        EndDrawing();

        if (iniciarOpcao >= 0)
        {
            sessao = guitarHero.iniciar(true, iniciarOpcao);
            if (sessao)
                tela = TELA_JOGO;
            else
                erroFaixa = "Não foi possível iniciar a faixa.";
        }
        if (sair)
            break;
    }

    if (sessao)
        guitarHero.encerrar(sessao);
    if (IsAudioDeviceReady())
        CloseAudioDevice();
    CloseWindow();
    return 0;
}
#endif

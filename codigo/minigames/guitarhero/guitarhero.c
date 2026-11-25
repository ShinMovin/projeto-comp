#include "guitarhero.h"
#include "desenho.h"
#include <math.h>
#include <stdlib.h>

#define LARGURA 800
#define ALTURA 600

#define TRILHAS 4
#define MAX_NOTAS 256
#define LARGURA_TRILHA 90
#define LARGURA_CAMPO 360
#define INICIO_CAMPO 220
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

const Faixa faixasGuitarra[] = {
    {"Hedwig's Theme", "harry-potter.wav", harryPotter, sizeof harryPotter / sizeof harryPotter[0]},
    {"Blue Bird", "blue-bird.wav", blueBird, sizeof blueBird / sizeof blueBird[0]},
    {"Sweden", "sweden.wav", sweden, sizeof sweden / sizeof sweden[0]}
};

const int quantidadeFaixasGuitarra = sizeof faixasGuitarra / sizeof faixasGuitarra[0];

typedef struct {
    NotaMapa mapa;
    EstadoNota estado;
    Precisao precisao;
} Nota;

struct SessaoGuitarra
{
    Nota notas[MAX_NOTAS];
    float duracao;
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
    Sound somFaixa;
    float tempo;
    float esperaSaidaAudio;
    bool treino;
    ResultadoMinigame resultado;
};

const int teclasGuitarra[TRILHAS] = {KEY_D, KEY_F, KEY_J, KEY_K};
const char *const nomesTeclasGuitarra[TRILHAS] = {"D", "F", "J", "K"};
const Color coresTrilhasGuitarra[TRILHAS] = {GREEN, RED, YELLOW, BLUE};
const NivelPrecisao niveisGuitarra[NIVEIS_PRECISAO] = {
    {100, "Perfeito!", GREEN},
    {75, "Ótimo!", SKYBLUE},
    {50, "Bom!", GOLD}
};

void prepararTreinoGuitarra(SessaoGuitarra *partida)
{
    partida->duracao = DURACAO_PARTIDA;
    int trilhaBloqueada = -1;
    for (int i = 0; i < MAX_NOTAS; i++)
    {
        float inicio = 2.3f + i * INTERVALO_NOTAS;
        float duracao = (i + 1) % 5 == 0 ? DURACAO_SUSTENTADA : 0.0f;
        if (inicio + fmaxf(JANELA_ACERTO, duracao) >= DURACAO_PARTIDA)
            break;

        // Uma nota sustentada bloqueia sua trilha para a nota seguinte.
        int disponiveis[TRILHAS];
        int quantidade = 0;
        for (int t = 0; t < TRILHAS; t++)
            if (t != trilhaBloqueada)
                disponiveis[quantidade++] = t;

        int trilha = disponiveis[GetRandomValue(0, quantidade - 1)];
        partida->notas[partida->quantidadeNotas++].mapa = (NotaMapa){inicio, duracao, trilha};
        trilhaBloqueada = duracao > 0 ? trilha : -1;
    }
}

Sound carregarFaixaGuitarra(const Faixa *faixa, SessaoGuitarra *partida)
{
    Wave onda = LoadWave(TextFormat("%sguitarhero-musicas/%s", GetApplicationDirectory(), faixa->arquivo));
    if (!IsWaveValid(onda))
        return (Sound){0};

    float duracao = (float)onda.frameCount / onda.sampleRate;
    // Os WAVs são curtos. O arquivo é lido inteiro antes da contagem inicial.
    Sound som = {0};
    if (duracao > 0 && duracao <= 60 && faixa->quantidade <= MAX_NOTAS && faixa->mapa[0].inicio < duracao)
        som = LoadSoundFromWave(onda);
    UnloadWave(onda);
    if (!IsSoundValid(som))
        return som;

    partida->duracao = duracao;
    for (size_t i = 0; i < faixa->quantidade && faixa->mapa[i].inicio < duracao; i++)
    {
        NotaMapa nota = faixa->mapa[i];
        nota.duracao = fminf(nota.duracao, duracao - nota.inicio);
        partida->notas[partida->quantidadeNotas++].mapa = nota;
    }
    SetSoundVolume(som, 0.65f);
    return som;
}

void julgarNotaGuitarra(SessaoGuitarra *partida, Nota *nota, float tempo, const char *erro)
{
    int trilha = nota->mapa.trilha;
    const char *mensagem;
    Color cor;
    if (erro)
    {
        nota->estado = RESOLVIDA;
        partida->erros++;
        partida->combo = 0;
        partida->multiplicador = 1;
        mensagem = erro;
        cor = RED;
    }
    else if (nota->estado == PENDENTE && nota->mapa.duracao > 0)
    {
        nota->estado = SUSTENTANDO;
        mensagem = "Segure!";
        cor = coresTrilhasGuitarra[trilha];
    }
    else
    {
        const NivelPrecisao *nivel = &niveisGuitarra[nota->precisao];
        nota->estado = RESOLVIDA;
        partida->acertos[nota->precisao]++;
        partida->combo++;
        if (partida->combo > partida->maiorCombo)
            partida->maiorCombo = partida->combo;
        partida->multiplicador = 1 + partida->combo / ACERTOS_POR_NIVEL;
        if (partida->multiplicador > MULTIPLICADOR_MAXIMO)
            partida->multiplicador = MULTIPLICADOR_MAXIMO;
        partida->pontos += nivel->pontos * partida->multiplicador;
        mensagem = nivel->mensagem;
        cor = nivel->cor;
    }
    partida->mensagem = mensagem;
    partida->corMensagem = cor;
    partida->fimMensagem = tempo + DURACAO_MENSAGEM;
    partida->coresPulso[trilha] = cor;
    partida->finsPulso[trilha] = tempo + DURACAO_PULSO;
}

const char *textoErrosGuitarra(const SessaoGuitarra *partida)
{
    return IsSoundValid(partida->somFaixa)
        ? TextFormat("Erros: %d", partida->erros)
        : TextFormat("Erros: %d/%d", partida->erros, MAX_ERROS);
}

void atualizarNotasGuitarra(SessaoGuitarra *partida)
{
    float tempo = partida->tempo;
    bool temFaixa = IsSoundValid(partida->somFaixa);
    bool apertou[TRILHAS];
    Nota *maisProxima[TRILHAS] = {NULL};
    float menorDiferenca[TRILHAS] = {0};
    for (int t = 0; t < TRILHAS; t++)
        apertou[t] = IsKeyPressed(teclasGuitarra[t]);

    for (int i = 0; i < partida->quantidadeNotas; i++)
    {
        Nota *nota = &partida->notas[i];
        int trilha = nota->mapa.trilha;
        float atraso = tempo - nota->mapa.inicio;
        if (nota->estado == SUSTENTANDO)
        {
            float fim = nota->mapa.inicio + nota->mapa.duracao;
            if (tempo > fim + TOLERANCIA_SOLTURA)
                julgarNotaGuitarra(partida, nota, tempo, "Soltou tarde!");
            else if (!IsKeyDown(teclasGuitarra[trilha]) || tempo >= partida->duracao)
            {
                float tolerancia = fminf(TOLERANCIA_SOLTURA, nota->mapa.duracao * 0.4f);
                julgarNotaGuitarra(partida, nota, tempo,
                    tempo < fim - tolerancia ? "Soltou cedo!" : NULL);
            }
        }
        else if (nota->estado == PENDENTE)
        {
            if (atraso > JANELA_ACERTO)
                julgarNotaGuitarra(partida, nota, tempo, "Nota perdida!");
            else if (apertou[trilha] && atraso >= -JANELA_ACERTO)
            {
                float diferenca = fabsf(atraso);
                if (!maisProxima[trilha] || diferenca <= menorDiferenca[trilha])
                {
                    maisProxima[trilha] = nota;
                    menorDiferenca[trilha] = diferenca;
                }
            }
        }

        if (!temFaixa && partida->erros >= MAX_ERROS)
            return;
    }

    // As teclas só pontuam depois de julgar as notas perdidas e as caudas.
    for (int t = 0; t < TRILHAS; t++)
    {
        Nota *nota = maisProxima[t];
        if (!nota)
            continue;

        nota->precisao = BOM;
        if (menorDiferenca[t] <= JANELA_PERFEITO)
            nota->precisao = PERFEITO;
        else if (menorDiferenca[t] <= JANELA_OTIMO)
            nota->precisao = OTIMO;
        julgarNotaGuitarra(partida, nota, tempo, NULL);
    }
}

int totalAcertosGuitarra(const SessaoGuitarra *partida)
{
    int total = 0;
    for (int i = 0; i < NIVEIS_PRECISAO; i++)
        total += partida->acertos[i];
    return total;
}

void DesenharGuitarra(const SessaoGuitarra *partida)
{
    float tempo = partida->tempo;
    ClearBackground(BLACK);
    DrawRectangle(INICIO_CAMPO, 0, LARGURA_CAMPO, ALTURA, (Color){30, 30, 45, 255});
    for (int t = 0; t < TRILHAS; t++)
    {
        int x = INICIO_CAMPO + t * LARGURA_TRILHA;
        if (IsKeyDown(teclasGuitarra[t]))
            DrawRectangleGradientV(x, 0, LARGURA_TRILHA, ALTURA, BLANK, Fade(coresTrilhasGuitarra[t], 0.35f));
        DrawLine(x, 0, x, ALTURA, DARKGRAY);
    }
    DrawLine(INICIO_CAMPO + LARGURA_CAMPO, 0, INICIO_CAMPO + LARGURA_CAMPO, ALTURA, DARKGRAY);
    DrawRectangle(INICIO_CAMPO, LINHA_ACERTO - 2, LARGURA_CAMPO, 4, WHITE);

    for (int t = 0; t < TRILHAS; t++)
    {
        int centroX = INICIO_CAMPO + t * LARGURA_TRILHA + LARGURA_TRILHA / 2;
        Color cor = coresTrilhasGuitarra[t];
        if (IsKeyDown(teclasGuitarra[t]))
            DrawCircle(centroX, LINHA_ACERTO, 28, cor);
        else
            DrawCircleLines(centroX, LINHA_ACERTO, 28, cor);

        float pulso = (partida->finsPulso[t] - tempo) / DURACAO_PULSO;
        if (partida->finsPulso[t] > 0.0f && pulso > 0.0f)
        {
            float raio = 28.0f + (1.0f - pulso) * 15.0f;
            DrawRing((Vector2){centroX, LINHA_ACERTO}, raio, raio + 4.0f,
                0, 360, 36, Fade(partida->coresPulso[t], pulso));
        }
        DrawText(nomesTeclasGuitarra[t], centroX - 8, LINHA_ACERTO + 45, 24, cor);
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
        Color cor = coresTrilhasGuitarra[nota->mapa.trilha];
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

    DrawText(textoErrosGuitarra(partida), 16, 16, 20, WHITE);
    DrawText(TextFormat("Pontos: %d", partida->pontos), 16, 44, 20, WHITE);
    DrawText(TextFormat("Combo: %d (x%d)", partida->combo, partida->multiplicador), 16, 72, 20, WHITE);
    int segundosRestantes = (int)ceilf(partida->duracao - fmaxf(0.0f, tempo));
    DrawText(TextFormat("Tempo: %d s", segundosRestantes), 16, 100, 20, WHITE);
    if (partida->esperaSaidaAudio > 0.0f)
        textoCentralizado("Finalizando faixa...", 560, 24, RAYWHITE);
    else if (tempo < 0.0f)
        textoCentralizado("Prepare-se!", 560, 24, RAYWHITE);
    else if (tempo < partida->fimMensagem)
        textoCentralizado(partida->mensagem, 560, 24, partida->corMensagem);
    if (!partida->treino)
        DrawText(TextFormat("Notas acertadas: %d/%d", totalAcertosGuitarra(partida), partida->quantidadeNotas),
            16, 129, 17, GOLD);
}

const char *NomeOpcaoGuitarra(bool treino, int opcao)
{
    if (treino && opcao == 0)
        return "Treino livre";
    int faixa = opcao - (treino ? 1 : 0);
    return faixa >= 0 && faixa < quantidadeFaixasGuitarra ? faixasGuitarra[faixa].nome : NULL;
}

SessaoGuitarra *IniciarGuitarra(bool treino, int opcao)
{
    if (!NomeOpcaoGuitarra(treino, opcao))
        return NULL;

    SessaoGuitarra *sessao = calloc(1, sizeof *sessao);
    if (!sessao)
        return NULL;
    sessao->treino = treino;
    sessao->multiplicador = 1;
    if (treino && opcao == 0)
        prepararTreinoGuitarra(sessao);
    else
    {
        if (IsAudioDeviceReady())
            sessao->somFaixa = carregarFaixaGuitarra(&faixasGuitarra[opcao - (treino ? 1 : 0)], sessao);
        if (!IsSoundValid(sessao->somFaixa))
        {
            free(sessao);
            return NULL;
        }
        sessao->tempo = -PREPARACAO_MUSICA;
    }
    return sessao;
}

void resolverNotasRestantesGuitarra(SessaoGuitarra *sessao)
{
    for (int i = 0; i < sessao->quantidadeNotas; i++)
    {
        if (sessao->notas[i].estado != RESOLVIDA)
            julgarNotaGuitarra(sessao, &sessao->notas[i], sessao->tempo, "Nota perdida!");
    }
}

ResultadoMinigame concluirGuitarra(SessaoGuitarra *sessao)
{
    bool venceu = sessao->treino ||
                  (sessao->quantidadeNotas > 0 && 100 * totalAcertosGuitarra(sessao) >= 70 * sessao->quantidadeNotas);
    sessao->resultado = venceu ? MINIGAME_VITORIA : MINIGAME_DERROTA;
    return sessao->resultado;
}

ResultadoMinigame AtualizarGuitarra(SessaoGuitarra *sessao, float tempoQuadro)
{
    if (sessao->resultado != MINIGAME_EM_CURSO)
        return sessao->resultado;
    // O atraso de saída é contado em quadros para a janela continuar respondendo.
    if (sessao->esperaSaidaAudio > 0.0f)
    {
        sessao->esperaSaidaAudio -= tempoQuadro;
        return sessao->esperaSaidaAudio > 0.0f ? MINIGAME_EM_CURSO : concluirGuitarra(sessao);
    }

    bool temFaixa = IsSoundValid(sessao->somFaixa);
    bool contando = sessao->tempo < 0;
    sessao->tempo += tempoQuadro;
    if (contando)
    {
        if (sessao->tempo < 0.0f)
            return MINIGAME_EM_CURSO;
        // O relógio das notas começa no mesmo quadro que dispara a faixa.
        sessao->tempo = 0.0f;
        PlaySound(sessao->somFaixa);
    }

    sessao->tempo = fminf(sessao->tempo, sessao->duracao);
    if (temFaixa && !IsSoundPlaying(sessao->somFaixa))
    {
        // O dispositivo ainda pode tocar amostras que o mixer já entregou.
        sessao->tempo = sessao->duracao;
        resolverNotasRestantesGuitarra(sessao);
        sessao->esperaSaidaAudio = MARGEM_SAIDA_AUDIO;
        return MINIGAME_EM_CURSO;
    }

    atualizarNotasGuitarra(sessao);
    if (!temFaixa && sessao->erros >= MAX_ERROS)
        sessao->resultado = MINIGAME_DERROTA;
    else if (!temFaixa && sessao->tempo >= sessao->duracao)
    {
        resolverNotasRestantesGuitarra(sessao);
        return concluirGuitarra(sessao);
    }
    return sessao->resultado;
}

PlacarMinigame PlacarGuitarra(const SessaoGuitarra *sessao)
{
    return (PlacarMinigame){.pontos = sessao->pontos, .acertos = totalAcertosGuitarra(sessao),
                           .notas = sessao->quantidadeNotas};
}

void EncerrarGuitarra(SessaoGuitarra *sessao)
{
    if (IsSoundValid(sessao->somFaixa))
    {
        if (IsSoundPlaying(sessao->somFaixa))
            StopSound(sessao->somFaixa);
        UnloadSound(sessao->somFaixa);
    }
    free(sessao);
}

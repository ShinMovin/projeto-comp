/**
 * @file travessia.c
 * @author VGA
 * @brief Minigame Travessia: avance pelas pistas, ruas, rios e trilhos.
 */
#include "travessia.h"
#include "desenho.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>

#define LARGURA 800
#define ALTURA 600
#define CELULA 50                          // tamanho de uma casa da grade, em pixels
#define COLUNAS (LARGURA / CELULA)         // 16 casas na horizontal
#define LINHAS_VISIVEIS (ALTURA / CELULA)  // 12 pistas cabem na tela
#define BUFFER_PISTAS 16                   // pistas guardadas na memória (buffer circular)
#define MAX_CARROS 4
#define META_PISTAS 25                     // pistas para vencer
#define VIDAS_INICIAIS 3
#define DURACAO_PULO 0.12f
#define DURACAO_CONTAGEM 2.0f
#define DURACAO_INVENCIVEL 0.8f
#define DURACAO_AVISO_TREM 1.0f           // a luz pisca este tempo antes do trem aparecer
#define LINHA_JOGADOR_TELA 3
#define FOLGA_CARRO 6.0f                   // pixels de cada ponta do carro que não contam como batida               // a câmera tenta manter o jogador 3 pistas acima do fundo

static const Color corGramaClara = {86, 150, 78, 255};
static const Color corGramaEscura = {74, 136, 68, 255};
static const Color corRua = {52, 56, 66, 255};
static const Color corFaixa = {214, 214, 200, 255};
static const Color corJogador = {207, 166, 78, 255};
static const Color corAgua = {46, 102, 168, 255};
static const Color corTronco = {124, 84, 48, 255};
static const Color corBrita = {104, 96, 88, 255};
static const Color corTrem = {176, 42, 52, 255};
static const Color coresCarro[] = {
    {196, 70, 70, 255}, {70, 120, 196, 255}, {226, 182, 60, 255}, {150, 90, 190, 255}, {235, 235, 235, 255}
};

typedef enum { PISTA_GRAMA, PISTA_RUA, PISTA_RIO, PISTA_TRILHO } TipoPista;

typedef struct
{
    TipoPista tipo;
    int linha;                 // qual linha do mapa ocupa este espaço do buffer (-1 = vazio)
    bool arvores[COLUNAS];     // só na grama: casas bloqueadas
    float carros[MAX_CARROS];  // rua: carros; rio: troncos; trilho: o trem. Guarda o x do lado esquerdo
    int quantidadeCarros;
    float larguraCarro;
    float velocidade;          // pixels por segundo; negativo anda para a esquerda
    Color corCarro;
    float espera;              // só no trilho: segundos até o próximo trem
    bool tremPassando;         // só no trilho: o trem está atravessando a tela
} Pista;

struct SessaoTravessia
{
    Pista pistas[BUFFER_PISTAS];
    int coluna, linha;                  // posição do jogador na grade (a linha cresce para cima)
    int colunaAnterior, linhaAnterior;  // de onde ele pulou, para animar o salto
    float arrasto, arrastoAnterior;     // pixels que o tronco já carregou o jogador para fora da casa
    float pulo;                         // tempo restante da animação do pulo
    float camera;                       // linha do mapa que aparece no fundo da tela
    int vidas;
    int maiorLinha;                     // pontos = pista mais longe alcançada
    int colunaSegura, linhaSegura;      // última casa de grama pisada: volta para cá ao ser atropelado
    float invencivel;                   // pisca e não pode ser atropelado
    float contagem;                     // "3, 2, 1" antes de começar
    bool infinito;                      // treino: sem chegada, vai até acabarem as vidas
    ResultadoMinigame resultado;
};

/* ---------- Mapa ---------- */

// O mapa é infinito, mas só BUFFER_PISTAS ficam na memória: a linha L mora no índice L % BUFFER_PISTAS.
static Pista *pista(SessaoTravessia *sessao, int linha)
{
    return &sessao->pistas[linha % BUFFER_PISTAS];
}

static const Pista *pistaConst(const SessaoTravessia *sessao, int linha)
{
    return &sessao->pistas[linha % BUFFER_PISTAS];
}

static void gerarPista(SessaoTravessia *sessao, int linha)
{
    Pista *p = pista(sessao, linha);
    *p = (Pista){.linha = linha, .tipo = PISTA_GRAMA};

    // As primeiras pistas e a chegada são grama livre, para o jogador começar e terminar em paz.
    bool areaSegura = linha < 3 || (!sessao->infinito && linha >= META_PISTAS);
    int sorteio = rand() % 100;
    if (!areaSegura && sorteio < 20)
    {
        p->tipo = PISTA_RIO;
        // Dois rios seguidos correm em sentidos opostos, para obrigar a trocar de tronco com atenção.
        const Pista *anterior = pista(sessao, linha - 1);
        int direcao = anterior->linha == linha - 1 && anterior->tipo == PISTA_RIO ?
                      (anterior->velocidade > 0 ? -1 : 1) : (rand() % 2 ? 1 : -1);
        p->velocidade = direcao * (float)(60 + rand() % 50 + linha * 2);
        p->quantidadeCarros = 2 + rand() % 2;
        p->larguraCarro = (float)(CELULA * (2 + rand() % 2) - 6);
        float trecho = LARGURA + 200.0f;
        for (int i = 0; i < p->quantidadeCarros; i++)
            p->carros[i] = -100.0f + i * trecho / p->quantidadeCarros + rand() % 60;
        return;
    }
    if (!areaSegura && sorteio < 30)
    {
        p->tipo = PISTA_TRILHO;
        p->velocidade = (rand() % 2 ? 1 : -1) * (float)(1000 + linha * 10);
        p->quantidadeCarros = 1;
        p->larguraCarro = CELULA * 8.0f;
        p->carros[0] = -10000.0f;               // fora da tela enquanto não passa
        p->espera = 1.5f + (rand() % 30) / 10.0f; // cada trilho começa num momento diferente
        return;
    }
    if (!areaSegura && sorteio < 75)
    {
        p->tipo = PISTA_RUA;
        int direcao = rand() % 2 ? 1 : -1;
        // Quanto mais longe, mais rápidos os carros.
        p->velocidade = direcao * (float)(130 + rand() % 80 + linha * 6);
        p->quantidadeCarros = 2 + rand() % 2;
        p->larguraCarro = (float)(CELULA * (1 + rand() % 2) - 6);
        p->corCarro = coresCarro[rand() % (int)(sizeof coresCarro / sizeof coresCarro[0])];
        // Espalha os carros por um trecho um pouco maior que a tela, para entrarem e saírem dela.
        float trecho = LARGURA + 200.0f;
        for (int i = 0; i < p->quantidadeCarros; i++)
            p->carros[i] = -100.0f + i * trecho / p->quantidadeCarros + rand() % 60;
        return;
    }

    if (!areaSegura)
    {
        // TODO: garantir que sempre exista caminho (hoje é muito raro, mas pode travar).
        for (int c = 0; c < COLUNAS; c++)
            p->arvores[c] = rand() % 100 < 15;
    }
}

// Gera as pistas que vão aparecer na tela e ainda não existem no buffer.
static void garantirPistas(SessaoTravessia *sessao)
{
    int primeira = (int)sessao->camera;
    for (int linha = primeira; linha <= primeira + LINHAS_VISIVEIS + 1; linha++)
    {
        if (pista(sessao, linha)->linha != linha)
            gerarPista(sessao, linha);
    }
}

static bool casaLivre(SessaoTravessia *sessao, int coluna, int linha)
{
    const Pista *p = pista(sessao, linha);
    return coluna >= 0 && coluna < COLUNAS && !(p->tipo == PISTA_GRAMA && p->arvores[coluna]);
}

/* ---------- Regras ---------- */

static void perderVida(SessaoTravessia *sessao, bool ficouParaTras)
{
    sessao->vidas--;
    if (sessao->vidas <= 0)
    {
        // No modo infinito não existe "perder": acabar as vidas encerra o treino com a pontuação feita.
        sessao->resultado = sessao->infinito ? MINIGAME_VITORIA : MINIGAME_DERROTA;
        return;
    }
    sessao->invencivel = DURACAO_INVENCIVEL;
    // Atropelado: volta para a última grama, se ela ainda estiver na tela.
    if (!ficouParaTras && sessao->linhaSegura >= (int)ceilf(sessao->camera))
    {
        sessao->coluna = sessao->colunaAnterior = sessao->colunaSegura;
        sessao->linha = sessao->linhaAnterior = sessao->linhaSegura;
        sessao->arrasto = sessao->arrastoAnterior = 0.0f;
        sessao->pulo = 0.0f;
        return;
    }
    // Ficou para trás (ou a grama já saiu da tela): volta para dentro dela, na casa livre mais próxima.
    sessao->arrasto = sessao->arrastoAnterior = 0.0f;
    sessao->linha = (int)ceilf(sessao->camera) + 1;
    garantirPistas(sessao);
    // Não renasce dentro do rio.
    while (pista(sessao, sessao->linha)->tipo == PISTA_RIO && sessao->linha < (int)sessao->camera + LINHAS_VISIVEIS - 2)
        sessao->linha++;
    for (int d = 0; d < COLUNAS && !casaLivre(sessao, sessao->coluna, sessao->linha); d++)
        sessao->coluna = (sessao->coluna + 1) % COLUNAS;
    sessao->colunaAnterior = sessao->coluna;
    sessao->linhaAnterior = sessao->linha;
    sessao->pulo = 0.0f;
}

// Ciclo do trilho: espera -> (luz piscando no último segundo) -> trem atravessa -> espera de novo.
static void moverTrem(Pista *p, float dt)
{
    if (!p->tremPassando)
    {
        p->espera -= dt;
        if (p->espera <= 0.0f)
        {
            p->tremPassando = true;
            p->carros[0] = p->velocidade > 0 ? -p->larguraCarro : (float)LARGURA;
        }
        return;
    }
    p->carros[0] += p->velocidade * dt;
    bool saiu = p->velocidade > 0 ? p->carros[0] > LARGURA : p->carros[0] + p->larguraCarro < 0;
    if (saiu)
    {
        p->tremPassando = false;
        p->carros[0] = -10000.0f;
        p->espera = 3.0f + (rand() % 30) / 10.0f;
    }
}

static void moverCarros(SessaoTravessia *sessao, float dt)
{
    const float trecho = LARGURA + 200.0f;
    for (int i = 0; i < BUFFER_PISTAS; i++)
    {
        Pista *p = &sessao->pistas[i];
        if (p->linha < 0 || p->tipo == PISTA_GRAMA)
            continue;
        if (p->tipo == PISTA_TRILHO)
        {
            moverTrem(p, dt);
            continue;
        }
        for (int c = 0; c < p->quantidadeCarros; c++)
        {
            p->carros[c] += p->velocidade * dt;
            // Saiu de um lado, reaparece do outro.
            if (p->velocidade > 0 && p->carros[c] > LARGURA + 100)
                p->carros[c] -= trecho;
            else if (p->velocidade < 0 && p->carros[c] + p->larguraCarro < -100)
                p->carros[c] += trecho;
        }
    }
}

static void lerMovimento(SessaoTravessia *sessao)
{
    int dc = 0, dl = 0;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        dl = 1;
    else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        dl = -1;
    else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        dc = -1;
    else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        dc = 1;
    if (dc == 0 && dl == 0)
        return;

    int novaColuna = sessao->coluna + dc;
    int novaLinha = sessao->linha + dl;
    float novoArrasto = sessao->arrasto;
    if (dl != 0)
    {
        // Ao trocar de pista, volta a encaixar na casa mais próxima de onde o tronco o deixou.
        novaColuna = (int)lroundf((sessao->coluna * CELULA + sessao->arrasto) / CELULA);
        novoArrasto = 0.0f;
    }
    // Não deixa andar para fora da tela por baixo nem entrar em árvore.
    if (novaLinha < (int)sessao->camera || !casaLivre(sessao, novaColuna, novaLinha))
        return;

    sessao->colunaAnterior = sessao->coluna;
    sessao->linhaAnterior = sessao->linha;
    sessao->arrastoAnterior = sessao->arrasto;
    sessao->coluna = novaColuna;
    sessao->linha = novaLinha;
    sessao->arrasto = novoArrasto;
    sessao->pulo = DURACAO_PULO;
}

static bool atropelado(SessaoTravessia *sessao)
{
    const Pista *p = pista(sessao, sessao->linha);
    if (p->tipo != PISTA_RUA && p->tipo != PISTA_TRILHO)
        return false;
    // Na primeira metade do pulo o personagem ainda está saindo da pista anterior: não conta.
    if (sessao->pulo > DURACAO_PULO / 2)
        return false;
    // Todos estão na mesma pista, então basta comparar na horizontal.
    // As caixas são um pouco menores que o desenho (cantos arredondados), para a batida parecer justa.
    Rectangle jogador = {sessao->coluna * CELULA + sessao->arrasto + 13.0f, 0, CELULA - 26.0f, 1};
    for (int c = 0; c < p->quantidadeCarros; c++)
    {
        Rectangle carro = {p->carros[c] + FOLGA_CARRO, 0, p->larguraCarro - 2 * FOLGA_CARRO, 1};
        if (CheckCollisionRecs(jogador, carro))
            return true;
    }
    return false;
}

// No rio: se está sobre um tronco, é carregado junto; se não, afoga.
// Devolve true se o jogador caiu na água ou foi levado para fora da tela.
static bool levarPeloRio(SessaoTravessia *sessao, float dt)
{
    const Pista *p = pista(sessao, sessao->linha);
    if (p->tipo != PISTA_RIO || sessao->pulo > 0.0f)
        return false;

    float centro = sessao->coluna * CELULA + sessao->arrasto + CELULA / 2.0f;
    for (int c = 0; c < p->quantidadeCarros; c++)
    {
        if (centro < p->carros[c] || centro > p->carros[c] + p->larguraCarro)
            continue;
        sessao->arrasto += p->velocidade * dt;
        // Andou mais de meia casa: passa a contar como a casa vizinha.
        while (sessao->arrasto > CELULA / 2.0f)
        {
            sessao->coluna++;
            sessao->arrasto -= CELULA;
        }
        while (sessao->arrasto < -CELULA / 2.0f)
        {
            sessao->coluna--;
            sessao->arrasto += CELULA;
        }
        return sessao->coluna < 0 || sessao->coluna >= COLUNAS;
    }
    return true;
}

SessaoTravessia *IniciarTravessia(bool treino, int opcao)
{
    if (opcao != 0)
        return NULL;
    SessaoTravessia *sessao = calloc(1, sizeof *sessao);
    if (!sessao)
        return NULL;
    for (int i = 0; i < BUFFER_PISTAS; i++)
        sessao->pistas[i].linha = -1;
    sessao->coluna = sessao->colunaAnterior = COLUNAS / 2;
    sessao->vidas = VIDAS_INICIAIS;
    sessao->contagem = DURACAO_CONTAGEM;
    sessao->infinito = treino;
    sessao->resultado = MINIGAME_EM_CURSO;
    garantirPistas(sessao);
    return sessao;
}

ResultadoMinigame AtualizarTravessia(SessaoTravessia *sessao, float dt)
{
    if (sessao->resultado != MINIGAME_EM_CURSO)
        return sessao->resultado;
    moverCarros(sessao, dt);
    if (sessao->contagem > 0.0f)
    {
        sessao->contagem -= dt;
        return MINIGAME_EM_CURSO;
    }

    if (sessao->pulo > 0.0f)
        sessao->pulo -= dt;
    else
        lerMovimento(sessao);

    if (sessao->linha > sessao->maiorLinha)
        sessao->maiorLinha = sessao->linha;
    if (sessao->pulo <= 0.0f && pista(sessao, sessao->linha)->tipo == PISTA_GRAMA)
    {
        sessao->colunaSegura = sessao->coluna;
        sessao->linhaSegura = sessao->linha;
    }
    if (!sessao->infinito && sessao->maiorLinha >= META_PISTAS)
    {
        sessao->resultado = MINIGAME_VITORIA;
        return sessao->resultado;
    }

    // A câmera sobe sozinha (cada vez mais rápido) e também acompanha o jogador quando ele avança.
    sessao->camera += dt * (0.25f + 0.015f * sessao->maiorLinha);
    float alvo = (float)(sessao->linha - LINHA_JOGADOR_TELA);
    if (sessao->camera < alvo)
        sessao->camera += (alvo - sessao->camera) * fminf(1.0f, dt * 6.0f);
    garantirPistas(sessao);

    // A invencibilidade protege dos carros, mas não da água.
    if (levarPeloRio(sessao, dt))
        perderVida(sessao, false);
    else if (sessao->invencivel > 0.0f)
        sessao->invencivel -= dt;
    else if (sessao->linha + 0.5f < sessao->camera)
        perderVida(sessao, true);
    else if (atropelado(sessao))
        perderVida(sessao, false);
    return sessao->resultado;
}

/* ---------- Desenho ---------- */

// Converte uma linha do mapa (pode ter fração) no y do topo dela na tela.
static float telaY(const SessaoTravessia *sessao, float linha)
{
    return ALTURA - (linha - sessao->camera + 1.0f) * CELULA;
}

static void desenharPista(const SessaoTravessia *sessao, int linha)
{
    const Pista *p = pistaConst(sessao, linha);
    float y = telaY(sessao, (float)linha);
    if (p->tipo == PISTA_RUA)
    {
        DrawRectangle(0, (int)y, LARGURA, CELULA, corRua);
        // Faixa tracejada entre duas ruas seguidas.
        if (pistaConst(sessao, linha + 1)->tipo == PISTA_RUA)
            for (int x = 10; x < LARGURA; x += 60)
                DrawRectangle(x, (int)y - 2, 30, 4, corFaixa);
        for (int c = 0; c < p->quantidadeCarros; c++)
        {
            Rectangle carro = {p->carros[c], y + 7, p->larguraCarro, CELULA - 14};
            DrawRectangleRounded(carro, 0.35f, 6, p->corCarro);
            // Para-brisa do lado para onde o carro anda.
            float frente = p->velocidade > 0 ? carro.x + carro.width - 16 : carro.x + 6;
            DrawRectangle((int)frente, (int)carro.y + 6, 10, (int)carro.height - 12, Fade(BLACK, 0.45f));
        }
        return;
    }

    if (p->tipo == PISTA_TRILHO)
    {
        DrawRectangle(0, (int)y, LARGURA, CELULA, corBrita);
        for (int x = 4; x < LARGURA; x += 25)
            DrawRectangle(x, (int)y + 8, 10, CELULA - 16, (Color){92, 64, 44, 255}); // dormentes
        DrawRectangle(0, (int)y + 15, LARGURA, 4, LIGHTGRAY);                       // trilhos
        DrawRectangle(0, (int)y + CELULA - 19, LARGURA, 4, LIGHTGRAY);

        // A luz pisca no aviso e fica acesa enquanto o trem passa.
        bool aviso = !p->tremPassando && p->espera < DURACAO_AVISO_TREM;
        bool acesa = p->tremPassando || (aviso && fmodf(p->espera, 0.25f) < 0.125f);
        if (aviso)
            DrawRectangle(0, (int)y, LARGURA, CELULA, Fade(RED, acesa ? 0.18f : 0.06f));
        for (int lado = 0; lado < 2; lado++)
        {
            int x = lado == 0 ? 14 : LARGURA - 14;
            DrawRectangle(x - 2, (int)y + 6, 4, 26, DARKGRAY);
            DrawCircle(x, (int)y + 8, 7, BLACK);
            DrawCircle(x, (int)y + 8, 5, acesa ? RED : (Color){70, 20, 20, 255});
        }

        if (p->tremPassando)
        {
            Rectangle trem = {p->carros[0], y + 5, p->larguraCarro, CELULA - 10};
            DrawRectangleRounded(trem, 0.3f, 6, corTrem);
            // Vagões: janelas a cada duas casas e uma divisão entre eles.
            for (float x = trem.x + 12; x < trem.x + trem.width - 20; x += 22)
                DrawRectangle((int)x, (int)trem.y + 8, 12, 12, (Color){240, 220, 160, 255});
            for (float x = trem.x + 2 * CELULA; x < trem.x + trem.width - 1; x += 2 * CELULA)
                DrawRectangle((int)x - 2, (int)trem.y, 4, (int)trem.height, ColorBrightness(corTrem, -0.4f));
            float frente = p->velocidade > 0 ? trem.x + trem.width - 14 : trem.x + 4;
            DrawRectangle((int)frente, (int)trem.y + 4, 10, (int)trem.height - 8, (Color){60, 60, 70, 255});
        }
        return;
    }

    if (p->tipo == PISTA_RIO)
    {
        DrawRectangle(0, (int)y, LARGURA, CELULA, corAgua);
        // Ondinhas que andam no sentido da correnteza.
        float deslocamento = fmodf((float)GetTime() * p->velocidade * 0.4f, 80.0f);
        for (float x = -80 + deslocamento; x < LARGURA; x += 80)
            DrawRectangle((int)x, (int)y + 14 + (linha % 2) * 18, 24, 3, Fade(RAYWHITE, 0.25f));
        for (int c = 0; c < p->quantidadeCarros; c++)
        {
            Rectangle tronco = {p->carros[c], y + 8, p->larguraCarro, CELULA - 16};
            DrawRectangleRounded(tronco, 0.6f, 8, corTronco);
            // Linhas da casca e os anéis nas pontas.
            for (float x = tronco.x + 22; x < tronco.x + tronco.width - 16; x += 26)
                DrawLine((int)x, (int)tronco.y + 6, (int)x + 8, (int)tronco.y + (int)tronco.height - 6,
                         ColorBrightness(corTronco, -0.3f));
            DrawCircle((int)(tronco.x + 10), (int)(tronco.y + tronco.height / 2), 8, ColorBrightness(corTronco, 0.25f));
            DrawCircleLines((int)(tronco.x + 10), (int)(tronco.y + tronco.height / 2), 4, ColorBrightness(corTronco, -0.3f));
        }
        return;
    }

    DrawRectangle(0, (int)y, LARGURA, CELULA, linha % 2 ? corGramaClara : corGramaEscura);
    if (linha == META_PISTAS && !sessao->infinito)
    {
        for (int x = 0; x < LARGURA; x += 20)
            DrawRectangle(x, (int)y + ((x / 20) % 2) * 25, 20, 25, Fade(corJogador, 0.7f));
        textoCentralizado("CHEGADA", (int)y + 13, 24, RAYWHITE);
    }
    for (int c = 0; c < COLUNAS; c++)
    {
        if (!p->arvores[c])
            continue;
        int cx = c * CELULA + CELULA / 2;
        DrawRectangle(cx - 5, (int)y + 30, 10, 14, (Color){110, 72, 40, 255});
        DrawCircle(cx, (int)y + 22, 17, (Color){40, 96, 52, 255});
    }
}

static void desenharJogador(const SessaoTravessia *sessao)
{
    // Pisca enquanto está invencível.
    if (sessao->invencivel > 0.0f && fmodf(sessao->invencivel, 0.2f) < 0.1f)
        return;
    // Interpola entre a casa de onde saiu e a de destino, com um "saltinho" no meio.
    float t = 1.0f - fmaxf(sessao->pulo, 0.0f) / DURACAO_PULO;
    float xAnterior = sessao->colunaAnterior * CELULA + sessao->arrastoAnterior;
    float xAtual = sessao->coluna * CELULA + sessao->arrasto;
    float linha = sessao->linhaAnterior + (sessao->linha - sessao->linhaAnterior) * t;
    float salto = sinf(t * PI) * 10.0f;

    float x = xAnterior + (xAtual - xAnterior) * t + 9;
    float y = telaY(sessao, linha) + 9 - salto;
    // TODO: desenhem o personagem de vocês aqui. Por enquanto é um cubo dourado com olhos.
    DrawRectangle((int)x + 3, (int)y + 34, 32, 5, Fade(BLACK, 0.25f));
    DrawRectangleRounded((Rectangle){x, y, 32, 32}, 0.25f, 6, corJogador);
    DrawRectangleRoundedLines((Rectangle){x, y, 32, 32}, 0.25f, 6, (Color){120, 90, 30, 255});
    DrawCircle((int)x + 10, (int)y + 12, 5, RAYWHITE);
    DrawCircle((int)x + 22, (int)y + 12, 5, RAYWHITE);
    DrawCircle((int)x + 11, (int)y + 12, 2, BLACK);
    DrawCircle((int)x + 23, (int)y + 12, 2, BLACK);
}

void DesenharTravessia(const SessaoTravessia *sessao)
{
    ClearBackground(BLACK);
    int primeira = (int)sessao->camera;
    for (int linha = primeira; linha <= primeira + LINHAS_VISIVEIS + 1; linha++)
        desenharPista(sessao, linha);
    desenharJogador(sessao);

    // Barra de informações no topo.
    DrawRectangle(0, 0, LARGURA, 40, Fade(BLACK, 0.6f));
    DrawText("TRAVESSIA", 16, 10, 22, corJogador);
    if (sessao->infinito)
        textoCentralizado(TextFormat("PISTAS %d", sessao->maiorLinha), 10, 22, RAYWHITE);
    else
        textoCentralizado(TextFormat("PISTAS %d / %d", sessao->maiorLinha, META_PISTAS), 10, 22, RAYWHITE);
    for (int i = 0; i < VIDAS_INICIAIS; i++)
        DrawCircle(LARGURA - 30 - i * 28, 20, 9, i < sessao->vidas ? (Color){226, 80, 90, 255} : DARKGRAY);

    if (sessao->contagem > 0.0f)
    {
        DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.35f));
        textoCentralizado(TextFormat("%d", (int)ceilf(sessao->contagem)), 230, 90, RAYWHITE);
        textoCentralizado("Setas ou WASD para pular", 340, 22, RAYWHITE);
    }
}

PlacarMinigame PlacarTravessia(const SessaoTravessia *sessao)
{
    return (PlacarMinigame){.pontos = sessao->maiorLinha};
}

void EncerrarTravessia(SessaoTravessia *sessao)
{
    free(sessao);
}

#ifdef TRAVESSIA_INDEPENDENTE
// Permite testar só o minigame: make -C codigo run-travessia
int main(void)
{
    InitWindow(LARGURA, ALTURA, "Travessia");
    SetTargetFPS(60);
    SessaoTravessia *sessao = IniciarTravessia(true, 0);
    while (sessao && !WindowShouldClose())
    {
        ResultadoMinigame resultado = AtualizarTravessia(sessao, GetFrameTime());
        if (resultado != MINIGAME_EM_CURSO && IsKeyPressed(KEY_R))
        {
            EncerrarTravessia(sessao);
            sessao = IniciarTravessia(true, 0);
            continue;
        }

        BeginDrawing();
        DesenharTravessia(sessao);
        if (resultado != MINIGAME_EM_CURSO)
        {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.6f));
            const SessaoTravessia *s = sessao;
            textoCentralizado("FIM DE JOGO", 200, 44, corJogador);
            textoCentralizado(TextFormat("Você avançou %d pistas", s->maiorLinha), 258, 24, RAYWHITE);
            textoCentralizado("R para jogar de novo  |  ESC para sair", 310, 20, RAYWHITE);
        }
        EndDrawing();
    }
    if (sessao)
        EncerrarTravessia(sessao);
    CloseWindow();
    return 0;
}
#endif

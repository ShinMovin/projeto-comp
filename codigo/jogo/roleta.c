#include "roleta.h"
#include "interface.h"
#include <math.h>
#include <stdlib.h>

#define DURACAO_GIRO 3.4
#define DURACAO_ANUNCIO 0.42f
#define ANGULO_SETOR (360.0f / SETORES_ROLETA)
#define ANGULO_PONTEIRO (-90.0f)
#define GRAUS_EM_RADIANOS (3.14159265358979323846f / 180.0f)

static const struct
{
    Categoria categoria;
    Color cor;
} setores[SETORES_ROLETA] = {
    {GEEK, {101, 63, 140, 255}},
    {ESPORTES, {39, 122, 83, 255}},
    {MATEMATICA, {45, 94, 150, 255}},
    {GEOGRAFIA, {140, 80, 47, 255}},
    {BIOLOGIA, {139, 55, 80, 255}}
};

void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade dificuldade,
                   Categoria bloqueada)
{
    *roleta = (Roleta){0};
    if (!banco)
        return;

    // Os cinco setores continuam visíveis, mas só entram no sorteio os que têm perguntas inéditas.
    int indiceBloqueado = -1;
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        roleta->disponiveis[i] = contaPerguntasDisponiveis(banco, dificuldade, setores[i].categoria) > 0;
        if (roleta->disponiveis[i])
            roleta->quantidadeDisponivel++;
        if (setores[i].categoria == bloqueada)
            indiceBloqueado = i;
    }

    // Evita a terceira repetição seguida, mas só se ainda sobrar outra categoria para sortear.
    if (indiceBloqueado >= 0 && roleta->disponiveis[indiceBloqueado] && roleta->quantidadeDisponivel > 1)
    {
        roleta->disponiveis[indiceBloqueado] = false;
        roleta->quantidadeDisponivel--;
    }
}

bool girarRoleta(Roleta *roleta)
{
    if (roleta->animacao.estado != ROLETA_PRONTA || roleta->quantidadeDisponivel == 0)
        return false;

    int sorteado = rand() % roleta->quantidadeDisponivel;
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        if (roleta->disponiveis[i] && sorteado-- == 0)
        {
            roleta->categoria = setores[i].categoria;
            // O setor zero já começa centralizado no ponteiro. Compensa a posição do setor sorteado.
            roleta->animacao.anguloFinal = 5 * 360.0f + fmodf(360.0f - i * ANGULO_SETOR, 360.0f);
            roleta->animacao.estado = ROLETA_GIRANDO;
            return true;
        }
    }
    return false;
}

/* O giro desacelera até o ponto sorteado. Só depois começa o anúncio. */
static void atualizarAnimacao(AnimacaoRoleta *animacao, float tempoQuadro)
{
    if (animacao->estado == ROLETA_GIRANDO)
    {
        animacao->tempoGiro += tempoQuadro;
        if (animacao->tempoGiro >= DURACAO_GIRO)
        {
            animacao->angulo = animacao->anguloFinal;
            animacao->estado = ROLETA_RESULTADO;
        }
        else
        {
            // A curva cúbica começa rápida e chega ao ângulo final com velocidade zero.
            double restante = 1.0 - animacao->tempoGiro / DURACAO_GIRO;
            animacao->angulo = (float)(animacao->anguloFinal * (1.0 - restante * restante * restante));
        }
    }
    else if (animacao->estado == ROLETA_RESULTADO)
    {
        animacao->tempoAnuncio = fminf(animacao->tempoAnuncio + tempoQuadro, DURACAO_ANUNCIO);
    }
}

void atualizarRoleta(Roleta *roleta, float tempoQuadro)
{
    atualizarAnimacao(&roleta->animacao, tempoQuadro);
}

static void desenharPonteiro(void)
{
    // A ponta fixa fica sobre o disco, no ângulo usado pelo cálculo do giro.
    DrawTriangle((Vector2){384, 68}, (Vector2){400, 113}, (Vector2){416, 68}, COR_FUNDO);
    DrawTriangle((Vector2){388, 71}, (Vector2){400, 106}, (Vector2){412, 71}, COR_DOURADO);
}

static void desenharDisco(const Roleta *roleta)
{
    const Vector2 centro = {400, 267};
    const float raio = 174.0f;
    const float giro = fmodf(roleta->animacao.angulo, 360.0f);
    const bool resultado = roleta->animacao.estado == ROLETA_RESULTADO;
    const float transicao = roleta->animacao.tempoAnuncio / DURACAO_ANUNCIO;

    DrawCircleV(centro, raio + 8, COR_FUNDO);
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        float meio = ANGULO_PONTEIRO + i * ANGULO_SETOR + giro;
        float inicio = meio - ANGULO_SETOR / 2;
        float fim = meio + ANGULO_SETOR / 2;
        bool escolhido = resultado && setores[i].categoria == roleta->categoria;
        float brilho = roleta->disponiveis[i] ? 0.0f : -0.56f;
        if (resultado)
            brilho = escolhido ? 0.16f * transicao : fminf(brilho, -0.64f * transicao);

        DrawCircleSector(centro, raio, inicio, fim, 24, ColorBrightness(setores[i].cor, brilho));
        DrawCircleSectorLines(centro, raio, inicio, fim, 24, COR_DOURADO);
        if (escolhido)
            DrawRing(centro, raio - 7, raio - 2, inicio + 2, fim - 2, 24, Fade(COR_CLARO, transicao));

        float angulo = meio * GRAUS_EM_RADIANOS;
        const int tamanho = 17;
        const char *nome = nomeCategoria(setores[i].categoria);
        int x = (int)(centro.x + cosf(angulo) * 121);
        int y = (int)(centro.y + sinf(angulo) * 121 - tamanho / 2.0f);
        textoCentradoEm(nome, x + 1, y + 1, tamanho, COR_FUNDO);
        textoCentradoEm(nome, x, y, tamanho, ColorBrightness(COR_CLARO, fminf(brilho, 0.0f) * 0.6f));
    }

    DrawCircleLines((int)centro.x, (int)centro.y, raio + 5, COR_DOURADO);
    DrawCircleV(centro, 38, COR_DOURADO);
    DrawCircleV(centro, 35, COR_AZUL);
    textoCentralizado("?", (int)centro.y - 22, 44, COR_CLARO);

    desenharPonteiro();
}

// Botão redondo ao lado da roleta de categorias que abre a roleta de dificuldade.
static bool desenharBotaoDificuldade(Dificuldade dificuldade, bool disponivel)
{
    const Vector2 centro = {690, 267};
    const float raio = 56.0f;
    bool sobre = disponivel && CheckCollisionPointCircle(GetMousePosition(), centro, raio);
    float pulso = disponivel ? 0.5f + 0.5f * sinf((float)GetTime() * 3.0f) : 0.0f;
    Color borda = disponivel ? COR_DOURADO : Fade(COR_DOURADO, 0.35f);
    Color texto = disponivel ? COR_CLARO : Fade(COR_CLARO, 0.4f);

    if (disponivel)
        DrawCircleV(centro, raio + 6 + 3 * pulso, Fade(COR_DOURADO, 0.12f + 0.1f * pulso));
    DrawCircleV(centro, raio, sobre ? ColorBrightness(COR_AZUL, 0.25f) : COR_AZUL);
    DrawCircleLines((int)centro.x, (int)centro.y, raio, borda);
    DrawCircleLines((int)centro.x, (int)centro.y, raio - 4, borda);
    textoCentradoEm("NÍVEL", (int)centro.x, (int)centro.y - 36, 13, texto);
    textoCentradoEm(TextFormat("%d", dificuldade), (int)centro.x, (int)centro.y - 21, 36, borda);
    textoCentradoEm(disponivel ? "MUDAR?" : "GIRADA", (int)centro.x, (int)centro.y + 19, 13, texto);
    textoCentradoEm("ROLETA DE", (int)centro.x, (int)(centro.y + raio + 12), 14, texto);
    textoCentradoEm("DIFICULDADE", (int)centro.x, (int)(centro.y + raio + 29), 14, texto);

    return sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso, Dificuldade dificuldade,
                          bool podeMudarDificuldade)
{
    desenharFundo();
    textoCentralizado("ROLETA DE CATEGORIAS", 33, 26, COR_DOURADO);
    desenharDisco(roleta);
    // Depois de girar as categorias, a dificuldade já ficou definida para esta pergunta.
    if (desenharBotaoDificuldade(dificuldade, podeMudarDificuldade && roleta->animacao.estado == ROLETA_PRONTA))
        return ROLETA_DIFICULDADE;

    bool anuncioCompleto = roleta->animacao.tempoAnuncio >= DURACAO_ANUNCIO;
    if (roleta->animacao.estado == ROLETA_RESULTADO)
    {
        float progresso = roleta->animacao.tempoAnuncio / DURACAO_ANUNCIO;
        float suavizado = progresso * progresso * (3.0f - 2.0f * progresso);
        int deslocamento = (int)((1.0f - suavizado) * 9);
        textoCentralizado("O TEMA ESCOLHIDO FOI", 456 + deslocamento, 18, Fade(COR_CLARO, suavizado));
        textoCentralizado(nomeCategoria(roleta->categoria), 479 + deslocamento, 32, Fade(COR_DOURADO, suavizado));
    }

    if (desenharBotao("VOLTAR", (Rectangle){36, 526, 132, 40}))
        return ROLETA_VOLTAR;

    const Rectangle areaAcao = {202, 526, 396, 40};
    if (aviso)
        textoCentralizado(aviso, 538, 18, COR_CLARO);
    else if (roleta->quantidadeDisponivel == 0)
        textoCentralizado("Sem perguntas disponíveis nesta dificuldade.", 538, 18, COR_CLARO);
    else if (roleta->animacao.estado == ROLETA_GIRANDO)
        textoCentralizado("SORTEANDO...", 538, 18, COR_CLARO);
    else if (roleta->animacao.estado == ROLETA_PRONTA && desenharBotao("GIRAR ROLETA", areaAcao))
        return ROLETA_GIRAR;
    else if (anuncioCompleto && desenharBotao("PROSSEGUIR PARA A PERGUNTA", areaAcao))
        return ROLETA_PROSSEGUIR;

    return ROLETA_NENHUMA_ACAO;
}

/* ---------- Roleta de dificuldade ---------- */

static const struct
{
    const char *nome;
    const char *efeito;
    Color cor;
} opcoesDificuldade[QUANTIDADE_OPCOES_DIF] = {
    {"DIMINUIR", "-1 nível", {39, 122, 83, 255}},
    {"MANTER", "mesmo nível", {45, 94, 150, 255}},
    {"AUMENTAR", "+1 nível", {150, 52, 64, 255}}
};

static bool temPerguntasNoNivel(const BancoPerguntas *banco, int nivel)
{
    if (!banco || nivel < MUITO_FACIL || nivel > IMPOSSIVEL)
        return false;
    for (int categoria = GEEK; categoria <= BIOLOGIA; categoria++)
    {
        if (contaPerguntasDisponiveis(banco, (Dificuldade)nivel, (Categoria)categoria) > 0)
            return true;
    }
    return false;
}

static int totalChances(const RoletaDificuldade *roleta)
{
    int total = 0;
    for (int i = 0; i < QUANTIDADE_OPCOES_DIF; i++)
        total += roleta->chances[i];
    return total;
}

void iniciarRoletaDificuldade(RoletaDificuldade *roleta, const BancoPerguntas *banco, int rodada)
{
    *roleta = (RoletaDificuldade){.base = dificuldadeRodada(rodada)};
    chancesDificuldade(rodada, roleta->chances);
    // Uma opção sai da roleta se passar do nível 1 a 5 ou se não houver pergunta inédita nela.
    if (!temPerguntasNoNivel(banco, roleta->base - 1))
        roleta->chances[DIF_DIMINUIR] = 0;
    if (!temPerguntasNoNivel(banco, roleta->base + 1))
        roleta->chances[DIF_AUMENTAR] = 0;
}

bool girarRoletaDificuldade(RoletaDificuldade *roleta)
{
    int total = totalChances(roleta);
    if (roleta->animacao.estado != ROLETA_PRONTA || total == 0)
        return false;

    // Sorteio ponderado: cada opção ocupa uma faixa do tamanho da sua chance.
    int sorteado = rand() % total;
    int inicio = 0;
    for (int i = 0; i < QUANTIDADE_OPCOES_DIF; i++)
    {
        if (sorteado < inicio + roleta->chances[i])
        {
            roleta->resultado = (OpcaoDificuldade)i;
            // Para em um ponto aleatório dentro do setor, longe das bordas.
            // As opções descartadas deixam o total abaixo de 100. Converte o total real em 360°.
            float grausPorPeso = 360.0f / total;
            float largura = roleta->chances[i] * grausPorPeso;
            float folga = largura * 0.35f;
            float alvo = inicio * grausPorPeso + largura / 2 + folga * (rand() % 201 - 100) / 100.0f;
            roleta->animacao.anguloFinal = 5 * 360.0f + fmodf(360.0f - alvo, 360.0f);
            roleta->animacao.estado = ROLETA_GIRANDO;
            return true;
        }
        inicio += roleta->chances[i];
    }
    return false;
}

void atualizarRoletaDificuldade(RoletaDificuldade *roleta, float tempoQuadro)
{
    atualizarAnimacao(&roleta->animacao, tempoQuadro);
}

int variacaoDificuldade(const RoletaDificuldade *roleta)
{
    return (int)roleta->resultado - DIF_MANTER;
}

static void desenharDiscoDificuldade(const RoletaDificuldade *roleta)
{
    const Vector2 centro = {400, 267};
    const float raio = 174.0f;
    const float giro = fmodf(roleta->animacao.angulo, 360.0f);
    const bool resultado = roleta->animacao.estado == ROLETA_RESULTADO;
    const float transicao = roleta->animacao.tempoAnuncio / DURACAO_ANUNCIO;
    const int total = totalChances(roleta);

    DrawCircleV(centro, raio + 8, COR_FUNDO);
    float acumulado = 0.0f;
    for (int i = 0; i < QUANTIDADE_OPCOES_DIF; i++)
    {
        if (roleta->chances[i] == 0)
            continue;
        float largura = roleta->chances[i] * 360.0f / total;
        float inicio = ANGULO_PONTEIRO + acumulado + giro;
        float fim = inicio + largura;
        float meio = inicio + largura / 2;
        acumulado += largura;

        bool escolhido = resultado && roleta->resultado == (OpcaoDificuldade)i;
        float brilho = resultado ? (escolhido ? 0.16f * transicao : -0.64f * transicao) : 0.0f;
        DrawCircleSector(centro, raio, inicio, fim, 32, ColorBrightness(opcoesDificuldade[i].cor, brilho));
        DrawCircleSectorLines(centro, raio, inicio, fim, 32, COR_DOURADO);
        if (escolhido)
            DrawRing(centro, raio - 7, raio - 2, inicio + 2, fim - 2, 32, Fade(COR_CLARO, transicao));

        float angulo = meio * GRAUS_EM_RADIANOS;
        int x = (int)(centro.x + cosf(angulo) * 112);
        int y = (int)(centro.y + sinf(angulo) * 112);
        Color cor = ColorBrightness(COR_CLARO, fminf(brilho, 0.0f) * 0.6f);
        textoCentradoEm(opcoesDificuldade[i].nome, x, y - 20, 17, cor);
        textoCentradoEm(opcoesDificuldade[i].efeito, x, y, 14, cor);
        textoCentradoEm(TextFormat("%d%%", roleta->chances[i] * 100 / total), x, y + 16, 14, cor);
    }

    DrawCircleLines((int)centro.x, (int)centro.y, raio + 5, COR_DOURADO);
    DrawCircleV(centro, 38, COR_DOURADO);
    DrawCircleV(centro, 35, COR_AZUL);
    textoCentradoEm(TextFormat("%d", roleta->base), (int)centro.x, (int)centro.y - 18, 36, COR_CLARO);
    desenharPonteiro();
}

AcaoRoleta desenharRoletaDificuldade(const RoletaDificuldade *roleta)
{
    desenharFundo();
    textoCentralizado("ROLETA DE DIFICULDADE", 33, 26, COR_DOURADO);
    desenharDiscoDificuldade(roleta);

    // Explicação curta das regras nas laterais.
    DrawText("DIMINUIR", 34, 150, 16, opcoesDificuldade[DIF_DIMINUIR].cor);
    DrawText("Pergunta mais", 34, 170, 14, COR_CLARO);
    DrawText("fácil, prêmio", 34, 186, 14, COR_CLARO);
    DrawText("normal.", 34, 202, 14, COR_CLARO);
    DrawText("AUMENTAR", 34, 300, 16, ColorBrightness(opcoesDificuldade[DIF_AUMENTAR].cor, 0.4f));
    DrawText("Pergunta mais", 34, 320, 14, COR_CLARO);
    DrawText("difícil. Acertou:", 34, 336, 14, COR_CLARO);
    DrawText("+50% de bônus", 34, 352, 14, COR_DOURADO);
    DrawText("por nível.", 34, 368, 14, COR_CLARO);
    DrawText("Uma vez", 652, 230, 16, COR_DOURADO);
    DrawText("por rodada.", 652, 250, 16, COR_DOURADO);

    if (roleta->animacao.estado == ROLETA_RESULTADO)
    {
        float progresso = roleta->animacao.tempoAnuncio / DURACAO_ANUNCIO;
        float suavizado = progresso * progresso * (3.0f - 2.0f * progresso);
        int deslocamento = (int)((1.0f - suavizado) * 9);
        Dificuldade nova = (Dificuldade)(roleta->base + variacaoDificuldade(roleta));
        textoCentralizado(TextFormat("NÍVEL %d  ->  %d", roleta->base, nova), 456 + deslocamento, 18,
                          Fade(COR_CLARO, suavizado));
        textoCentralizado(nomeDificuldade(nova), 479 + deslocamento, 32, Fade(COR_DOURADO, suavizado));
    }

    const Rectangle areaAcao = {202, 526, 396, 40};
    if (roleta->animacao.estado == ROLETA_PRONTA)
    {
        // Antes de girar ainda dá para desistir sem gastar a roleta da rodada.
        if (desenharBotao("VOLTAR", (Rectangle){36, 526, 132, 40}) || IsKeyPressed(KEY_BACKSPACE))
            return ROLETA_VOLTAR;
        if (desenharBotao("GIRAR ROLETA", areaAcao))
            return ROLETA_GIRAR;
    }
    else if (roleta->animacao.estado == ROLETA_GIRANDO)
        textoCentralizado("SORTEANDO...", 538, 18, COR_CLARO);
    else if (roleta->animacao.tempoAnuncio >= DURACAO_ANUNCIO && desenharBotao("CONTINUAR", areaAcao))
        return ROLETA_PROSSEGUIR;

    return ROLETA_NENHUMA_ACAO;
}

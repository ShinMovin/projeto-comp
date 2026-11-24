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

void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade dificuldade)
{
    *roleta = (Roleta){.estado = ROLETA_PRONTA};
    if (!banco)
        return;

    // Os cinco setores continuam visíveis, mas só entram no sorteio os que têm perguntas inéditas.
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        roleta->disponiveis[i] = contaPerguntasDisponiveis(banco, dificuldade, setores[i].categoria) > 0;
        if (roleta->disponiveis[i])
            roleta->quantidadeDisponivel++;
    }
}

bool girarRoleta(Roleta *roleta)
{
    if (roleta->estado != ROLETA_PRONTA || roleta->quantidadeDisponivel == 0)
        return false;

    int sorteado = rand() % roleta->quantidadeDisponivel;
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        if (roleta->disponiveis[i] && sorteado-- == 0)
        {
            roleta->categoria = setores[i].categoria;
            // O setor zero já começa centralizado no ponteiro. Compensa a posição do setor sorteado.
            roleta->anguloFinal = 5 * 360.0f + fmodf(360.0f - i * ANGULO_SETOR, 360.0f);
            roleta->estado = ROLETA_GIRANDO;
            return true;
        }
    }
    return false;
}

void atualizarRoleta(Roleta *roleta, float tempoQuadro)
{
    if (roleta->estado == ROLETA_GIRANDO)
    {
        roleta->tempoGiro += tempoQuadro;
        if (roleta->tempoGiro >= DURACAO_GIRO)
        {
            roleta->angulo = roleta->anguloFinal;
            roleta->estado = ROLETA_RESULTADO;
        }
        else
        {
            // A curva cúbica começa rápida e chega ao ângulo final com velocidade zero.
            double restante = 1.0 - roleta->tempoGiro / DURACAO_GIRO;
            roleta->angulo = (float)(roleta->anguloFinal * (1.0 - restante * restante * restante));
        }
    }
    else if (roleta->estado == ROLETA_RESULTADO)
    {
        roleta->tempoAnuncio = fminf(roleta->tempoAnuncio + tempoQuadro, DURACAO_ANUNCIO);
    }
}

static void desenharDisco(const Roleta *roleta)
{
    const Vector2 centro = {400, 267};
    const float raio = 174.0f;
    const float giro = fmodf(roleta->angulo, 360.0f);
    const bool resultado = roleta->estado == ROLETA_RESULTADO;
    const float transicao = roleta->tempoAnuncio / DURACAO_ANUNCIO;

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

    // A ponta fixa fica sobre o disco, no ângulo usado pelo cálculo do giro.
    DrawTriangle((Vector2){384, 68}, (Vector2){400, 113}, (Vector2){416, 68}, COR_FUNDO);
    DrawTriangle((Vector2){388, 71}, (Vector2){400, 106}, (Vector2){412, 71}, COR_DOURADO);
}

AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso)
{
    desenharFundo();
    textoCentralizado("ROLETA DE CATEGORIAS", 33, 26, COR_DOURADO);
    desenharDisco(roleta);

    bool anuncioCompleto = roleta->tempoAnuncio >= DURACAO_ANUNCIO;
    if (roleta->estado == ROLETA_RESULTADO)
    {
        float progresso = roleta->tempoAnuncio / DURACAO_ANUNCIO;
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
    else if (roleta->estado == ROLETA_GIRANDO)
        textoCentralizado("SORTEANDO...", 538, 18, COR_CLARO);
    else if (roleta->estado == ROLETA_PRONTA && desenharBotao("GIRAR ROLETA", areaAcao))
        return ROLETA_GIRAR;
    else if (anuncioCompleto && desenharBotao("PROSSEGUIR PARA A PERGUNTA", areaAcao))
        return ROLETA_PROSSEGUIR;

    return ROLETA_NENHUMA_ACAO;
}

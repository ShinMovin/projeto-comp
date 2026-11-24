#include "roleta.h"
#include "interface.h"
#include <math.h>
#include <stdlib.h>

const float duracaoAnuncioRoleta = 0.42f;
const Color coresCategorias[SETORES_ROLETA] = {
    {101, 63, 140, 255}, {39, 122, 83, 255}, {45, 94, 150, 255},
    {140, 80, 47, 255}, {139, 55, 80, 255}
};
const Color coresDificuldade[QUANTIDADE_OPCOES_DIF] = {
    {39, 122, 83, 255}, {45, 94, 150, 255}, {150, 52, 64, 255}
};
const char *const rotulosDificuldade[] = {"DIMINUIR", "MANTER", "AUMENTAR"};
const char *const efeitosDificuldade[] = {"-1 nível", "mesmo nível", "+1 nível"};

void iniciarRoleta(Roleta *roleta, const BancoPerguntas *banco, Dificuldade nivel, Categoria bloqueada)
{
    *roleta = (Roleta){0};
    if (!banco)
        return;
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        if (contaPerguntasDisponiveis(banco, nivel, (Categoria)(GEEK + i)) > 0)
        {
            roleta->pesos[i] = 1;
            roleta->total++;
        }
    }
    // Só impede a terceira categoria seguida quando ainda existe outra opção.
    if (bloqueada >= GEEK && bloqueada <= BIOLOGIA && roleta->total > 1)
    {
        int indice = bloqueada - GEEK;
        roleta->total -= roleta->pesos[indice];
        roleta->pesos[indice] = 0;
    }
}

void iniciarRoletaDificuldade(Roleta *roleta, const BancoPerguntas *banco, int rodada)
{
    *roleta = (Roleta){.deDificuldade = true, .nivelBase = dificuldadeRodada(rodada)};
    chancesDificuldade(rodada, roleta->pesos);

    // Um nível fora do intervalo ou sem pergunta inédita não pode sair.
    for (int opcao = DIF_DIMINUIR; opcao <= DIF_AUMENTAR; opcao++)
    {
        if (opcao != DIF_MANTER)
        {
            int nivel = (int)roleta->nivelBase + opcao - DIF_MANTER;
            bool disponivel = false;
            if (banco && nivel >= MUITO_FACIL && nivel <= IMPOSSIVEL)
            {
                for (int categoria = GEEK; categoria <= BIOLOGIA; categoria++)
                    if (contaPerguntasDisponiveis(banco, (Dificuldade)nivel, (Categoria)categoria) > 0)
                    {
                        disponivel = true;
                        break;
                    }
            }
            if (!disponivel)
                roleta->pesos[opcao] = 0;
        }
        roleta->total += roleta->pesos[opcao];
    }
}

bool girarRoleta(Roleta *roleta)
{
    if (roleta->estado != ROLETA_PRONTA || roleta->total == 0)
        return false;

    int sorteado = rand() % roleta->total;
    int inicio = 0;
    for (int i = 0; i < SETORES_ROLETA; i++)
    {
        int peso = roleta->pesos[i];
        if (sorteado >= inicio + peso)
        {
            inicio += peso;
            continue;
        }

        float alvo = i * (360.0f / SETORES_ROLETA);
        if (roleta->deDificuldade)
        {
            float grausPorPeso = 360.0f / roleta->total;
            float largura = peso * grausPorPeso;
            float folga = largura * 0.35f;
            alvo = inicio * grausPorPeso + largura / 2 + folga * (rand() % 201 - 100) / 100.0f;
        }
        roleta->resultado = i;
        roleta->anguloFinal = 5 * 360.0f + fmodf(360.0f - alvo, 360.0f);
        roleta->estado = ROLETA_GIRANDO;
        return true;
    }
    return false;
}

void atualizarRoleta(Roleta *roleta, float segundos)
{
    const double duracaoGiro = 3.4;
    if (roleta->estado == ROLETA_GIRANDO)
    {
        roleta->tempoGiro += segundos;
        if (roleta->tempoGiro >= duracaoGiro)
        {
            roleta->angulo = roleta->anguloFinal;
            roleta->estado = ROLETA_RESULTADO;
        }
        else
        {
            double restante = 1.0 - roleta->tempoGiro / duracaoGiro;
            roleta->angulo = (float)(roleta->anguloFinal * (1.0 - restante * restante * restante));
        }
    }
    else if (roleta->estado == ROLETA_RESULTADO)
        roleta->tempoAnuncio = fminf(roleta->tempoAnuncio + segundos, duracaoAnuncioRoleta);
}

void desenharDiscoRoleta(const Roleta *roleta)
{
    const Vector2 centro = {400, 267};
    const float raio = 174.0f;
    float giro = fmodf(roleta->angulo, 360.0f);
    float transicao = roleta->tempoAnuncio / duracaoAnuncioRoleta;
    bool deDificuldade = roleta->deDificuldade;
    int setores = deDificuldade ? QUANTIDADE_OPCOES_DIF : SETORES_ROLETA;
    int detalhes = deDificuldade ? 32 : 24;
    float acumulado = 0.0f;

    DrawCircleV(centro, raio + 8, COR_FUNDO);
    for (int i = 0; i < setores; i++)
    {
        if (deDificuldade && roleta->pesos[i] == 0)
            continue;

        float inicio, fim, meio;
        Color corSetor;
        if (deDificuldade)
        {
            float largura = roleta->pesos[i] * 360.0f / roleta->total;
            inicio = -90.0f + acumulado + giro;
            fim = inicio + largura;
            meio = inicio + largura / 2;
            acumulado += largura;
            corSetor = coresDificuldade[i];
        }
        else
        {
            float largura = 360.0f / SETORES_ROLETA;
            meio = -90.0f + i * largura + giro;
            inicio = meio - largura / 2;
            fim = meio + largura / 2;
            corSetor = coresCategorias[i];
        }

        bool escolhido = roleta->estado == ROLETA_RESULTADO && roleta->resultado == i;
        float brilho = roleta->pesos[i] ? 0.0f : -0.56f;
        if (escolhido)
            brilho = 0.16f * transicao;
        else if (roleta->estado == ROLETA_RESULTADO)
            brilho = fminf(brilho, -0.64f * transicao);

        DrawCircleSector(centro, raio, inicio, fim, detalhes, ColorBrightness(corSetor, brilho));
        DrawCircleSectorLines(centro, raio, inicio, fim, detalhes, COR_DOURADO);
        if (escolhido)
            DrawRing(centro, raio - 7, raio - 2, inicio + 2, fim - 2, detalhes, Fade(COR_CLARO, transicao));

        float angulo = meio * DEG2RAD;
        Color corTexto = ColorBrightness(COR_CLARO, fminf(brilho, 0.0f) * 0.6f);
        if (deDificuldade)
        {
            int x = (int)(centro.x + cosf(angulo) * 112);
            int y = (int)(centro.y + sinf(angulo) * 112);
            textoCentradoEm(rotulosDificuldade[i], x, y - 20, 17, corTexto);
            textoCentradoEm(efeitosDificuldade[i], x, y, 14, corTexto);
            textoCentradoEm(TextFormat("%d%%", roleta->pesos[i] * 100 / roleta->total), x, y + 16, 14, corTexto);
        }
        else
        {
            int x = (int)(centro.x + cosf(angulo) * 121);
            int y = (int)(centro.y + sinf(angulo) * 121 - 8.5f);
            const char *nome = nomeCategoria((Categoria)(GEEK + i));
            textoCentradoEm(nome, x + 1, y + 1, 17, COR_FUNDO);
            textoCentradoEm(nome, x, y, 17, corTexto);
        }
    }

    DrawCircleLines(400, 267, raio + 5, COR_DOURADO);
    DrawCircleV(centro, 38, COR_DOURADO);
    DrawCircleV(centro, 35, COR_AZUL);
    if (deDificuldade)
        textoCentralizado(TextFormat("%d", roleta->nivelBase), 249, 36, COR_CLARO);
    else
        textoCentralizado("?", 245, 44, COR_CLARO);
    DrawTriangle((Vector2){384, 68}, (Vector2){400, 113}, (Vector2){416, 68}, COR_FUNDO);
    DrawTriangle((Vector2){388, 71}, (Vector2){400, 106}, (Vector2){412, 71}, COR_DOURADO);
}

bool desenharBotaoDificuldade(Dificuldade nivel, bool disponivel)
{
    const Vector2 centro = {690, 267};
    bool sobre = disponivel && CheckCollisionPointCircle(GetMousePosition(), centro, 56.0f);
    Color borda = disponivel ? COR_DOURADO : Fade(COR_DOURADO, 0.35f);
    Color texto = disponivel ? COR_CLARO : Fade(COR_CLARO, 0.4f);
    if (disponivel)
    {
        float pulso = 0.5f + 0.5f * sinf((float)GetTime() * 3.0f);
        DrawCircleV(centro, 62 + 3 * pulso, Fade(COR_DOURADO, 0.12f + 0.1f * pulso));
    }
    DrawCircleV(centro, 56.0f, sobre ? ColorBrightness(COR_AZUL, 0.25f) : COR_AZUL);
    DrawCircleLines(690, 267, 56.0f, borda);
    DrawCircleLines(690, 267, 52.0f, borda);
    textoCentradoEm("NÍVEL", 690, 231, 13, texto);
    textoCentradoEm(TextFormat("%d", nivel), 690, 246, 36, borda);
    textoCentradoEm(disponivel ? "MUDAR?" : "GIRADA", 690, 286, 13, texto);
    textoCentradoEm("ROLETA DE", 690, 335, 14, texto);
    textoCentradoEm("DIFICULDADE", 690, 352, 14, texto);
    return sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

AcaoRoleta desenharRoleta(const Roleta *roleta, const char *aviso, Dificuldade nivel, bool podeMudar)
{
    bool deDificuldade = roleta->deDificuldade;
    desenharFundo();
    textoCentralizado(deDificuldade ? "ROLETA DE DIFICULDADE" : "ROLETA DE CATEGORIAS", 33, 26, COR_DOURADO);
    desenharDiscoRoleta(roleta);

    if (deDificuldade)
    {
        DrawText("DIMINUIR", 34, 150, 16, coresDificuldade[DIF_DIMINUIR]);
        DrawText("Pergunta mais", 34, 170, 14, COR_CLARO);
        DrawText("fácil, prêmio", 34, 186, 14, COR_CLARO);
        DrawText("normal.", 34, 202, 14, COR_CLARO);
        DrawText("AUMENTAR", 34, 300, 16, ColorBrightness(coresDificuldade[DIF_AUMENTAR], 0.4f));
        DrawText("Pergunta mais", 34, 320, 14, COR_CLARO);
        DrawText("difícil. Acertou:", 34, 336, 14, COR_CLARO);
        DrawText("+50% de bônus", 34, 352, 14, COR_DOURADO);
        DrawText("por nível.", 34, 368, 14, COR_CLARO);
        DrawText("Uma vez", 652, 230, 16, COR_DOURADO);
        DrawText("por rodada.", 652, 250, 16, COR_DOURADO);
    }
    else if (desenharBotaoDificuldade(nivel, podeMudar && roleta->estado == ROLETA_PRONTA))
        return ROLETA_DIFICULDADE;

    if (roleta->estado == ROLETA_RESULTADO)
    {
        float progresso = roleta->tempoAnuncio / duracaoAnuncioRoleta;
        float brilho = progresso * progresso * (3.0f - 2.0f * progresso);
        int deslocamento = (int)((1.0f - brilho) * 9);
        const char *titulo = "O TEMA ESCOLHIDO FOI";
        const char *resultado;
        if (deDificuldade)
        {
            Dificuldade nova = (Dificuldade)((int)roleta->nivelBase + roleta->resultado - DIF_MANTER);
            titulo = TextFormat("NÍVEL %d  ->  %d", roleta->nivelBase, nova);
            resultado = nomeDificuldade(nova);
        }
        else
            resultado = nomeCategoria((Categoria)(GEEK + roleta->resultado));
        textoCentralizado(titulo, 456 + deslocamento, 18, Fade(COR_CLARO, brilho));
        textoCentralizado(resultado, 479 + deslocamento, 32, Fade(COR_DOURADO, brilho));
    }

    if (!deDificuldade || roleta->estado == ROLETA_PRONTA)
    {
        if (desenharBotao("VOLTAR", (Rectangle){36, 526, 132, 40}) ||
            (deDificuldade && IsKeyPressed(KEY_BACKSPACE)))
            return ROLETA_VOLTAR;
    }

    const Rectangle areaAcao = {202, 526, 396, 40};
    if (aviso)
        textoCentralizado(aviso, 538, 18, COR_CLARO);
    else if (roleta->total == 0)
        textoCentralizado("Sem perguntas disponíveis nesta dificuldade.", 538, 18, COR_CLARO);
    else if (roleta->estado == ROLETA_GIRANDO)
        textoCentralizado("SORTEANDO...", 538, 18, COR_CLARO);
    else if (roleta->estado == ROLETA_PRONTA && desenharBotao("GIRAR ROLETA", areaAcao))
        return ROLETA_GIRAR;
    else if (roleta->tempoAnuncio >= duracaoAnuncioRoleta &&
             desenharBotao(deDificuldade ? "CONTINUAR" : "PROSSEGUIR PARA A PERGUNTA", areaAcao))
        return ROLETA_PROSSEGUIR;
    return ROLETA_NENHUMA_ACAO;
}

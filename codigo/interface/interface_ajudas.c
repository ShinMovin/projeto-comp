#include "interface_ajudas.h"
#include "interface.h"
#include "premio.h"
#include <math.h>
#include <stddef.h>

static const Rectangle areaAbandonar = {615, 538, 165, 40};
static const Rectangle areaVoltar = {43, 522, 185, 42};
static const struct
{
    const char *nome;
    const char *recompensa;
} cartoes[QUANTIDADE_MINIGAMES] = {
    [GUITAR_HERO] = {"GUITAR HERO", "PULAR E GANHAR"},
    [TRAVESSIA] = {"TRAVESSIA", "SEGUNDA CHANCE"},
    [TIRO_AO_ALVO] = {"TIRO AO ALVO", "ELIMINAR DUAS\nALTERNATIVAS"}
};

static bool disponivel(const Jogo *jogo, TipoMinigame tipo)
{
    return obterMinigame(tipo) && (!jogo || ajudaDisponivel(jogo, tipo));
}

int contarAjudasDisponiveis(const Jogo *jogo)
{
    int total = 0;
    for (int i = GUITAR_HERO; i < QUANTIDADE_MINIGAMES; i++)
        total += disponivel(jogo, (TipoMinigame)i);
    return total;
}

static void encerrarSessao(Desafios *desafios)
{
    if (desafios->sessao)
    {
        obterMinigame(desafios->tipo)->encerrar(desafios->sessao);
        desafios->sessao = NULL;
    }
}

void liberarDesafios(Desafios *desafios)
{
    encerrarSessao(desafios);
    *desafios = (Desafios){0};
}

void abrirDesafios(Desafios *desafios)
{
    liberarDesafios(desafios);
    desafios->tela = DESAFIOS_CARTOES;
}

static bool teclaVoltar(void)
{
    return IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE);
}

static void finalizarDesafio(Desafios *desafios, Jogo *jogo, const Pergunta *pergunta, ResultadoMinigame resultado)
{
    const Minigame *minigame = obterMinigame(desafios->tipo);
    desafios->placar = minigame->placar(desafios->sessao);
    // A faixa do Guitar Hero já terminou. Libera o PCM ao sair do resultado.
    if (desafios->tipo != GUITAR_HERO || resultado == MINIGAME_ABANDONADO)
        encerrarSessao(desafios);
    desafios->resultado = resultado;
    desafios->recompensa = jogo && concluirAjuda(jogo, pergunta, resultado);
    desafios->tempoResultado = 0;
    desafios->tela = DESAFIOS_RESULTADO;
}

void atualizarDesafios(Desafios *desafios, Jogo *jogo, const Pergunta *pergunta, float tempoQuadro)
{
    if (desafios->tela == DESAFIOS_RESULTADO)
        desafios->tempoResultado = fminf(desafios->tempoResultado + tempoQuadro, 1.0f);
    else if (desafios->tela == DESAFIOS_JOGANDO)
    {
        bool abandonou = teclaVoltar() ||
                        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                         CheckCollisionPointRec(GetMousePosition(), areaAbandonar));
        ResultadoMinigame resultado = abandonou ? MINIGAME_ABANDONADO :
            obterMinigame(desafios->tipo)->atualizar(desafios->sessao, tempoQuadro);
        if (resultado != MINIGAME_EM_CURSO)
            finalizarDesafio(desafios, jogo, pergunta, resultado);
    }
}

static void desenharSimbolo(TipoMinigame tipo, int x, int y, Color cor)
{
    DrawCircleLines(x, y, 38, cor);
    if (tipo == GUITAR_HERO)
    {
        DrawCircleLines(x - 10, y + 9, 14, cor);
        DrawLineEx((Vector2){x - 9, y + 8}, (Vector2){x + 19, y - 21}, 5, cor);
        DrawCircleV((Vector2){x + 18, y - 20}, 5, cor);
    }
    else if (tipo == TRAVESSIA)
    {
        DrawLine(x - 24, y + 15, x + 24, y + 15, cor);
        DrawLine(x - 24, y - 15, x + 24, y - 15, cor);
        for (int i = -19; i < 20; i += 15)
            DrawRectangle(x + i, y - 1, 8, 2, cor);
    }
    else
    {
        DrawCircleLines(x, y, 25, cor);
        DrawCircleLines(x, y, 14, cor);
        DrawCircle(x, y, 4, cor);
    }
}

static bool desenharCartoes(Desafios *desafios, const Jogo *jogo)
{
    desenharFundo();
    textoCentralizado(jogo ? "ESCOLHA SUA AJUDA" : "TREINO DE MINIGAMES", 52, 33, COR_DOURADO);
    if (jogo)
    {
        int total = contarAjudasDisponiveis(jogo);
        textoCentralizado(total == 1 ? "1 ajuda disponível nesta rodada" :
                          TextFormat("%d ajudas disponíveis nesta rodada", total), 103, 19, COR_CLARO);
    }

    for (int i = GUITAR_HERO; i < QUANTIDADE_MINIGAMES; i++)
    {
        TipoMinigame tipo = (TipoMinigame)i;
        Rectangle area = {36 + (i - GUITAR_HERO) * 248, 155, 232, 322};
        int centro = (int)(area.x + area.width / 2);
        bool ativo = disponivel(jogo, tipo);
        bool sobre = ativo && CheckCollisionPointRec(GetMousePosition(), area);
        Color borda = ativo ? COR_DOURADO : Fade(COR_DOURADO, 0.45f);
        Color letras = ativo ? COR_CLARO : Fade(COR_CLARO, 0.58f);
        DrawRectangleRec(area, sobre ? ColorBrightness(COR_AZUL, 0.16f) : COR_AZUL);
        DrawRectangleLinesEx(area, sobre ? 3 : 1, borda);
        desenharSimbolo(tipo, centro, 221, borda);
        textoCentradoEm(cartoes[i].nome, centro, 277, 21, letras);

        if (jogo && tipo == TIRO_AO_ALVO)
        {
            textoCentradoEm("ELIMINAR DUAS", centro, 329, 17, borda);
            textoCentradoEm("ALTERNATIVAS", centro, 352, 17, borda);
        }
        else if (jogo)
            textoCentradoEm(cartoes[i].recompensa, centro, 338, 17, borda);

        if (jogo && tipo == GUITAR_HERO && jogo->rodada == TOTAL_RODADAS - 1)
        {
            textoCentradoEm("Indisponível na pergunta", centro, 409, 15, letras);
            textoCentradoEm("do milhão", centro, 430, 15, letras);
        }
        else
        {
            const char *situacao = ativo ? "DISPONÍVEL" :
                !obterMinigame(tipo) ? "CÓDIGO INDISPONÍVEL" : "JÁ UTILIZADO";
            textoCentradoEm(situacao, centro, 421, 15, letras);
        }

        if (sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            desafios->tipo = tipo;
            desafios->opcao = 0;
            desafios->erroAoIniciar = false;
            desafios->tela = DESAFIOS_INSTRUCOES;
        }
    }

    return desenharBotao("VOLTAR", areaVoltar) || teclaVoltar();
}

static void iniciarDesafio(Desafios *desafios, Jogo *jogo)
{
    if (!disponivel(jogo, desafios->tipo))
        return;

    const Minigame *minigame = obterMinigame(desafios->tipo);
    encerrarSessao(desafios);
    desafios->sessao = minigame->iniciar(!jogo, desafios->opcao);
    if (!desafios->sessao)
    {
        desafios->erroAoIniciar = true;
        return;
    }
    // Uma faixa indisponível ou uma falha antes de criar a sessão não consome a tentativa.
    if (jogo && !iniciarAjuda(jogo, desafios->tipo))
    {
        encerrarSessao(desafios);
        return;
    }
    desafios->erroAoIniciar = false;
    desafios->tela = DESAFIOS_JOGANDO;
}

static void desenharInstrucoes(Desafios *desafios, Jogo *jogo)
{
    desenharFundo();
    const Rectangle painel = {66, 43, 668, 461};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, COR_DOURADO);
    textoCentralizado(cartoes[desafios->tipo].nome, 73, 34, COR_DOURADO);
    if (desafios->tipo == GUITAR_HERO)
    {
        textoCentralizado("Use D, F, J e K quando a nota chegar à linha.", 146, 19, COR_CLARO);
        textoCentralizado("Segure as teclas nas notas com cauda.", 179, 19, COR_CLARO);
        const char *objetivo = jogo ? "Acerte pelo menos 70% das notas da faixa." :
            desafios->opcao == 0 ? "Sobreviva 30 segundos com menos de três erros." :
            "Toque a faixa inteira e acompanhe seus acertos.";
        textoCentralizado(objetivo, 224, 20, COR_DOURADO);
        if (jogo)
            textoCentralizado("Recompensa: conquiste o valor e avance pela roleta.", 268, 18, COR_CLARO);
    }
    else if (desafios->tipo == TIRO_AO_ALVO)
    {
        textoCentralizado("Clique nos alvos antes que desapareçam.", 146, 19, COR_CLARO);
        textoCentralizado("O alvo falso desconta 500 pontos.", 179, 19, COR_CLARO);
        textoCentralizado("Alcance 7.500 pontos em até 30 segundos.", 224, 20, COR_DOURADO);
        if (jogo)
            textoCentralizado("Recompensa: eliminar duas alternativas incorretas.", 268, 18, COR_CLARO);
    }
    else
    {
        textoCentralizado("Use as setas ou WASD para pular de casa em casa.", 146, 19, COR_CLARO);
        textoCentralizado("Desvie dos carros e não fique para trás: você tem 3 vidas.", 179, 19, COR_CLARO);
        textoCentralizado("Avance 25 pistas para completar a travessia.", 224, 20, COR_DOURADO);
        if (jogo)
            textoCentralizado("Recompensa: uma segunda chance nesta pergunta.", 268, 18, COR_CLARO);
    }

    const Minigame *minigame = obterMinigame(desafios->tipo);
    if (minigame->nomeOpcao)
    {
        const char *nome = minigame->nomeOpcao(!jogo, desafios->opcao);
        textoCentralizado("FAIXA", 326, 15, COR_DOURADO);
        textoCentralizado(nome, 364, 24, COR_CLARO);
        if (desafios->opcao > 0 && desenharBotao("<", (Rectangle){164, 354, 45, 38}))
        {
            desafios->opcao--;
            desafios->erroAoIniciar = false;
        }
        if (minigame->nomeOpcao(!jogo, desafios->opcao + 1) &&
            desenharBotao(">", (Rectangle){591, 354, 45, 38}))
        {
            desafios->opcao++;
            desafios->erroAoIniciar = false;
        }
    }

    if (desafios->erroAoIniciar)
        textoCentralizado(desafios->tipo == GUITAR_HERO ?
            "Falha ao iniciar. Confira o áudio e a faixa escolhida." :
            "Não foi possível iniciar o desafio.", 449, 17, COR_CLARO);

    if (desenharBotao("VOLTAR", areaVoltar) || teclaVoltar())
        desafios->tela = DESAFIOS_CARTOES;
    else if (desenharBotao("INICIAR DESAFIO", (Rectangle){489, 522, 268, 42}))
        iniciarDesafio(desafios, jogo);
}

static bool desenharResultadoDesafio(Desafios *desafios, Jogo *jogo)
{
    desenharFundo();
    bool venceu = jogo ? desafios->recompensa : desafios->resultado == MINIGAME_VITORIA;
    Color cor = venceu ? COR_DOURADO : (Color){247, 142, 139, 255};
    float entrada = animarEntrada(desafios->tempoResultado, 0.55f);
    const Rectangle painel = {85, 102, 630, 368};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, Fade(cor, entrada));
    textoCentralizado(venceu ? (jogo ? "AJUDA CONQUISTADA!" : "TREINO CONCLUÍDO!") :
                      "DESAFIO NÃO CONCLUÍDO", 156 + (int)((1 - entrada) * 18), 32, Fade(cor, entrada));
    if (desafios->placar.notas > 0)
        textoCentralizado(TextFormat("%d/%d notas acertadas  |  %.1f%%",
                           desafios->placar.acertos, desafios->placar.notas,
                           100.0f * desafios->placar.acertos / desafios->placar.notas), 232, 22, COR_CLARO);
    else
        textoCentralizado(TextFormat("%d pontos", desafios->placar.pontos), 232, 22, COR_CLARO);

    if (jogo && venceu)
    {
        const char *mensagem = desafios->tipo == GUITAR_HERO ? "PULAR E GANHAR: valor da rodada conquistado." :
            desafios->tipo == TRAVESSIA ? "SEGUNDA CHANCE: você pode errar uma vez." :
            "Duas alternativas incorretas foram eliminadas.";
        textoCentralizado(mensagem, 308, 20, Fade(COR_CLARO, entrada));
    }
    else if (jogo)
        textoCentralizado("Recompensa não conquistada.", 308, 20, Fade(COR_CLARO, entrada));
    else if (desafios->resultado == MINIGAME_ABANDONADO)
        textoCentralizado("Treino abandonado.", 308, 20, Fade(COR_CLARO, entrada));

    if (jogo)
    {
        const char *botao = venceu && desafios->tipo == GUITAR_HERO ? "PRÓXIMA RODADA" : "VOLTAR À PERGUNTA";
        bool clicou = desenharBotao(botao, (Rectangle){244, 389, 312, 46});
        return desafios->tempoResultado >= 0.15f && (clicou || teclaVoltar());
    }
    bool voltou = desenharBotao("VOLTAR", (Rectangle){129, 389, 214, 46});
    if (desafios->tempoResultado >= 0.15f && (voltou || teclaVoltar()))
    {
        encerrarSessao(desafios);
        desafios->tela = DESAFIOS_CARTOES;
    }
    else if (desenharBotao("JOGAR NOVAMENTE", (Rectangle){419, 389, 252, 46}))
    {
        desafios->tela = DESAFIOS_INSTRUCOES;
        iniciarDesafio(desafios, NULL);
    }
    return false;
}

bool desenharDesafios(Desafios *desafios, Jogo *jogo)
{
    switch (desafios->tela)
    {
    case DESAFIOS_CARTOES:
        return desenharCartoes(desafios, jogo);
    case DESAFIOS_INSTRUCOES:
        desenharInstrucoes(desafios, jogo);
        break;
    case DESAFIOS_JOGANDO:
        obterMinigame(desafios->tipo)->desenhar(desafios->sessao);
        desenharBotao("ABANDONAR", areaAbandonar);
        break;
    case DESAFIOS_RESULTADO:
        return desenharResultadoDesafio(desafios, jogo);
    case DESAFIOS_FECHADOS:
        break;
    }
    return false;
}

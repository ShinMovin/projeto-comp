#include "interface.h"
#include "interface_ajudas.h"
#include "premio.h"
#include <math.h>
#include <string.h>

bool botao(const char *texto, Rectangle area, bool dourado)
{
    bool selecionado = CheckCollisionPointRec(GetMousePosition(), area);
    Color cor = selecionado || dourado ? COR_DOURADO : COR_AZUL;
    Color corTexto = selecionado || dourado ? COR_FUNDO : COR_CLARO;
    if (selecionado && dourado)
        cor = ColorBrightness(COR_DOURADO, 0.25f);

    DrawRectangleRec(area, cor);
    DrawRectangleLinesEx(area, 2, COR_DOURADO);

    textoNoRetangulo(texto, area, 22, corTexto);

    return selecionado && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool desenharBotao(const char *texto, Rectangle area)
{
    return botao(texto, area, false);
}

void desenharFundo(void)
{
    ClearBackground(COR_FUNDO);

    // Linhas decorativas
    for (int i = 0; i < 900; i += 100)
    {
        DrawLine(i, 0, 400, 300, COR_AZUL);
        DrawLine(i, 600, 400, 300, COR_AZUL);
    }

    // Moldura externa
    DrawRectangleLinesEx(
        (Rectangle){20, 20, 760, 560},
        2, COR_DOURADO
    );
}

// Interface principal
Tela desenharMenu(void)
{
    desenharFundo();

    // Logotipo
    DrawCircle(400, 172, 133, COR_AZUL);
    DrawCircleLines(400, 172, 133, COR_DOURADO);
    DrawCircleLines(400, 172, 122, COR_DOURADO);
    DrawCircle(400, 172, 108, COR_FUNDO);

    DrawLine(290, 142, 510, 142, COR_DOURADO);
    DrawLine(290, 210, 510, 210, COR_DOURADO);

    textoCentralizado("JOGO DO", 105, 26, COR_CLARO);
    textoCentralizado("MILHAO", 151, 48, COR_DOURADO);
    textoCentralizado("DESAFIE SEU SABER", 222, 15, COR_CLARO);

    const struct
    {
        const char *rotulo;
        Tela destino;
    } opcoes[] = {
        {"JOGAR", TELA_JOGAR},
        {"TESTAR MINIGAMES", TELA_MINIGAMES},
        {"COMO JOGAR", TELA_INSTRUCOES},
        {"SAIR", TELA_SAIR}
    };
    Tela proximaTela = MENU_PRINCIPAL;
    for (size_t i = 0; i < sizeof opcoes / sizeof opcoes[0]; i++)
    {
        if (desenharBotao(opcoes[i].rotulo, (Rectangle){225, 315 + 47 * i, 350, 38}))
            proximaTela = opcoes[i].destino;
    }

    textoCentralizado(
        "ENGENHARIA DE COMPUTACAO - UTFPR",
        557, 12, GRAY
    );

    return proximaTela;
}

const Color corAcerto = {112, 224, 163, 255};
const Color corErro = {255, 135, 145, 255};
const Color corLegenda = {176, 184, 208, 255};

const char *emReais(int valor)
{
    if (valor >= 1000000)
        return TextFormat("R$ %d.%03d.%03d", valor / 1000000, valor / 1000 % 1000, valor % 1000);
    if (valor >= 1000)
        return TextFormat("R$ %d.%03d", valor / 1000, valor % 1000);
    return TextFormat("R$ %d", valor);
}

void textoADireita(const char *texto, int direita, int y, int tamanho, Color cor)
{
    DrawText(texto, direita - MeasureText(texto, tamanho), y, tamanho, cor);
}

void desenharTextoPergunta(const char texto[LIMITE_TEXTO_PERGUNTA + 1], Rectangle area,
                           int tamanho, bool centralizar, Color cor)
{
    int limite = 0;
    while (limite < LIMITE_TEXTO_PERGUNTA && texto[limite])
        limite++;
    if (limite == 0)
        return;

    Font fonte = GetFontDefault();
    char buffer[2 * (LIMITE_TEXTO_PERGUNTA + 1)];
    int linhas[LIMITE_TEXTO_PERGUNTA + 1];
    int quantidade;
    float passo, altura;
    // Mede cada linha na fonte real. Se o texto não couber, tenta uma fonte menor.
    for (;; tamanho--)
    {
        int inicio = 0;
        int gravados = 0;
        quantidade = 0;
        while (inicio < limite)
        {
            linhas[quantidade++] = gravados;
            int fim = inicio;
            int ultimoEspaco = -1;

            while (fim < limite && texto[fim] != '\n')
            {
                int bytes;
                GetCodepointNext(texto + fim, &bytes);
                if (bytes < 1 || bytes > limite - fim)
                    bytes = 1;
                int posicao = gravados + fim - inicio;
                for (int i = 0; i < bytes; i++)
                    buffer[posicao + i] = texto[fim + i];
                buffer[posicao + bytes] = '\0';
                if (fim > inicio && MeasureTextEx(fonte, buffer + gravados, tamanho, 1).x > area.width)
                    break;
                if (texto[fim] == ' ' && fim > inicio)
                    ultimoEspaco = fim;
                fim += bytes;
            }
            if (fim < limite && texto[fim] != '\n' && ultimoEspaco >= 0)
                fim = ultimoEspaco;
            gravados += fim - inicio;
            buffer[gravados++] = '\0';
            inicio = fim;
            if (inicio < limite && texto[inicio] == '\n')
                inicio++;
            while (inicio < limite && texto[inicio] == ' ')
                inicio++;
        }

        passo = tamanho + (tamanho >= 15 ? 3 : 1);
        altura = tamanho + (quantidade - 1) * passo;
        if (altura <= area.height || tamanho == 1)
            break;
    }

    float y = area.y + (area.height - altura) / 2;
    for (int i = 0; i < quantidade; i++)
    {
        const char *linha = buffer + linhas[i];
        float x = area.x;
        if (centralizar)
            x += (area.width - MeasureTextEx(fonte, linha, tamanho, 1).x) / 2;
        DrawTextEx(fonte, linha, (Vector2){x, y}, tamanho, 1, cor);
        y += passo;
    }
}

void desenharPremioCabecalho(const char *rotulo, int valor, int x, Color cor)
{
    const Rectangle painel = {x, 34, 174, 68};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 1, COR_DOURADO);
    textoCentradoEm(rotulo, x + 87, 45, 13, corLegenda);
    textoCentradoEm(emReais(valor), x + 87, 65, 22, cor);
}

void desenharCabecalho(const Jogo *jogo)
{
    DrawCircle(70, 69, 36, COR_AZUL);
    DrawCircleLines(70, 69, 36, COR_DOURADO);
    DrawCircleLines(70, 69, 32, COR_DOURADO);
    textoCentradoEm("JOGO DO", 70, 55, 10, COR_CLARO);
    textoCentradoEm("MILHÃO", 70, 69, 14, COR_DOURADO);

    DrawText(nomeCategoria(jogo->categoria), 123, 41, 30, COR_CLARO);
    DrawText(TextFormat("PERGUNTA %02d/%02d", jogo->rodada + 1, TOTAL_RODADAS), 124, 80, 16, COR_DOURADO);
    Dificuldade base = dificuldadeRodada(jogo->rodada);
    if (jogo->dificuldade == base)
        DrawText(TextFormat("NÍVEL %d/%d", jogo->dificuldade, IMPOSSIVEL), 295, 80, 16, corLegenda);
    else
        // Mostra que o nível veio da roleta: verde se diminuiu, dourado se aumentou.
        DrawText(TextFormat("NÍVEL %d (%+d)", jogo->dificuldade, jogo->dificuldade - base),
                 295, 80, 16, jogo->dificuldade < base ? corAcerto : COR_DOURADO);

    desenharPremioCabecalho("PRÊMIO ATUAL", jogo->premio, 410, COR_CLARO);
    desenharPremioCabecalho("PRÊMIO GARANTIDO", jogo->premioGarantido, 594, COR_DOURADO);
}

void desenharEscudo(float x, float y, float escala, Color cor)
{
    Vector2 pontos[] = {
        {x, y}, {x - escala, y - escala}, {x - escala, y},
        {x - escala * 0.65f, y + escala * 0.65f}, {x, y + escala},
        {x + escala * 0.65f, y + escala * 0.65f}, {x + escala, y},
        {x + escala, y - escala}, {x - escala, y - escala}
    };
    DrawTriangleFan(pontos, (int)(sizeof pontos / sizeof pontos[0]), cor);
    DrawLineEx((Vector2){x - escala * 0.43f, y}, (Vector2){x - escala * 0.12f, y + escala * 0.3f},
               escala * 0.16f, COR_FUNDO);
    DrawLineEx((Vector2){x - escala * 0.12f, y + escala * 0.3f}, (Vector2){x + escala * 0.5f, y - escala * 0.43f},
               escala * 0.16f, COR_FUNDO);
}

void desenharEscada(const Jogo *jogo)
{
    const Rectangle painel = {568, 115, 200, 454};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 1, COR_DOURADO);
    textoCentradoEm("ESCADA DE PRÊMIOS", 668, 130, 16, COR_DOURADO);

    for (int rodada = TOTAL_RODADAS - 1; rodada >= 0; rodada--)
    {
        int y = 155 + (TOTAL_RODADAS - 1 - rodada) * 27;
        bool atual = rodada == jogo->rodada;
        bool checkpoint = rodadaTemCheckpoint(rodada);
        Color cor = rodada < jogo->rodada ? corAcerto : COR_CLARO;
        if (atual)
        {
            DrawRectangleRec((Rectangle){577, y, 182, 24}, COR_DOURADO);
            cor = COR_FUNDO;
        }
        else if (checkpoint)
            DrawRectangleRec((Rectangle){577, y, 182, 24}, Fade(COR_DOURADO, 0.13f));
        else if ((rodada / RODADAS_POR_NIVEL) % 2 == 0)
            DrawRectangleRec((Rectangle){577, y, 182, 24}, Fade(COR_FUNDO, 0.55f));

        DrawText(TextFormat("%02d", rodada + 1), 587, y + 4, 15, cor);
        if (checkpoint)
            desenharEscudo(624, y + 12, 6, atual ? COR_FUNDO : COR_DOURADO);
        textoADireita(emReais(valorRodada(rodada)), 749, y + 4, 15, cor);
    }
}

bool desenharAlternativa(const Jogo *jogo, const Pergunta *pergunta, int indice)
{
    char letra = (char)('A' + indice);
    Rectangle area = {32, 259 + indice * 65, 520, 58};
    bool eliminada = jogo->eliminadas[indice];
    bool bloqueada = alternativaBloqueada(jogo, letra);
    bool mouseSobre = jogo->estado == PERGUNTA && !bloqueada && CheckCollisionPointRec(GetMousePosition(), area);
    bool marcada = letra == jogo->resposta;
    bool revelada = jogo->estado == ACERTOU || jogo->estado == ERROU;
    Color fundo = COR_AZUL;
    Color contorno = Fade(COR_DOURADO, 0.48f);
    if (revelada && letra == pergunta->alternativa_correta)
    {
        fundo = (Color){23, 71, 55, 255};
        contorno = corAcerto;
    }
    else if (jogo->estado == ERROU && marcada)
    {
        fundo = (Color){96, 38, 55, 255};
        contorno = corErro;
    }
    else if (bloqueada)
    {
        fundo = COR_FUNDO;
        contorno = eliminada ? Fade(COR_DOURADO, 0.38f) : Fade(corErro, 0.55f);
    }
    else if (marcada || mouseSobre)
    {
        fundo = (Color){39, 56, 98, 255};
        contorno = COR_DOURADO;
        if (jogo->estado == SUSPENSE)
        {
            float pulso = 0.5f + 0.5f * sinf(jogo->tempoEstado * 17.0f);
            fundo = ColorBrightness(fundo, 0.22f * pulso);
            contorno = ColorBrightness(contorno, 0.35f * pulso);
        }
    }

    DrawRectangleRec(area, fundo);
    DrawRectangleLinesEx(area, 2, contorno);
    Rectangle selo = {44, area.y + 13, 34, 32};
    DrawRectangleRec(selo, contorno);
    const char textoLetra[] = {letra, '\0'};
    textoNoRetangulo(textoLetra, selo, 19, COR_FUNDO);
    if (eliminada)
        DrawText("ELIMINADA", 90, (int)area.y + 21, 16, corLegenda);
    else
        desenharTextoPergunta(pergunta->alternativas[indice], (Rectangle){90, area.y + 4, 448, 50},
                               19, false, bloqueada ? corLegenda : COR_CLARO);
    return mouseSobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

AcaoPergunta desenharConfirmacao(char resposta)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(COR_FUNDO, 0.84f));
    const Rectangle painel = {166, 201, 468, 198};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, COR_DOURADO);
    textoCentralizado("Confirmar resposta?", 227, 30, COR_CLARO);
    textoCentralizado(TextFormat("Alternativa %c", resposta), 271, 21, COR_DOURADO);

    if (desenharBotao("CANCELAR", (Rectangle){194, 324, 196, 44}) || IsKeyPressed(KEY_BACKSPACE))
        return (AcaoPergunta){.tipo = PERGUNTA_CANCELAR};
    if (botao("CONFIRMAR", (Rectangle){410, 324, 196, 44}, true))
        return (AcaoPergunta){.tipo = PERGUNTA_CONFIRMAR};
    return (AcaoPergunta){0};
}

AcaoPergunta desenharTelaPergunta(const Jogo *jogo, const Pergunta *pergunta)
{
    desenharFundo();
    desenharCabecalho(jogo);
    desenharEscada(jogo);

    const Rectangle painel = {32, 115, 520, 134};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 1, COR_DOURADO);
    DrawText(TextFormat("VALE %s", emReais(valorRodada(jogo->rodada))), 49, 128, 17, COR_DOURADO);
    int bonus = bonusDificuldade(jogo->rodada, dificuldadeRodada(jogo->rodada), jogo->dificuldade);
    if (bonus > 0)
        textoADireita(TextFormat("+ BÔNUS %s", emReais(bonus)), 535, 128, 17, corAcerto);
    DrawLine(49, 152, 535, 152, Fade(COR_DOURADO, 0.50f));
    desenharTextoPergunta(pergunta->conteudo_da_pergunta, (Rectangle){50, 159, 484, 80}, 23, true, COR_CLARO);

    AcaoPergunta acao = {0};
    for (int i = 0; i < 4; i++)
    {
        if (desenharAlternativa(jogo, pergunta, i))
            acao = (AcaoPergunta){PERGUNTA_ESCOLHER, (char)('A' + i)};
    }

    switch (jogo->estado)
    {
        case PERGUNTA:
        {
            const char *status = NULL;
            if (jogo->segundaChance)
                status = "Segunda chance ativa";
            else if (jogo->respostaBloqueada)
                status = "Segunda chance utilizada";
            DrawText(TextFormat("Ajudas disponíveis: %d", contarAjudasDisponiveis(jogo)),
                     229, status ? 532 : 542, 17, corLegenda);
            if (status)
                DrawText(status, 229, 551, 14, COR_DOURADO);
            if (botao("PEDIR AJUDA", (Rectangle){32, 528, 182, 41}, true))
                acao.tipo = PERGUNTA_AJUDA;
            break;
        }
        case CONFIRMANDO:
            return desenharConfirmacao(jogo->resposta);
        case SUSPENSE:
            DrawText("REVELANDO RESPOSTA...", 38, 541, 19, COR_DOURADO);
            break;
        case ACERTOU:
            DrawText("RESPOSTA CORRETA", 38, 541, 19, corAcerto);
            if (jogo->rodada < TOTAL_RODADAS - 1 &&
                botao("PRÓXIMA RODADA", (Rectangle){303, 528, 249, 41}, true))
                acao.tipo = PERGUNTA_AVANCAR;
            break;
        case ERROU:
            DrawText("RESPOSTA INCORRETA", 38, 541, 19, corErro);
            break;
        default:
            break;
    }
    return acao;
}

bool desenharNovoNivel(const Jogo *jogo)
{
    desenharFundo();
    const Rectangle painel = {78, 59, 644, 482};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, COR_DOURADO);
    float entrada = animarEntrada(jogo->tempoEstado, 0.55f);
    float brilho = 0.17f + 0.08f * sinf(jogo->tempoEstado * 4.0f);
    int deslocamento = (int)(12 * (1.0f - entrada));
    int nivelAtual = jogo->dificuldade;
    textoCentralizado("NOVO NÍVEL DESBLOQUEADO", 84 + deslocamento, 27, Fade(COR_DOURADO, entrada));
    textoCentralizado(TextFormat("NÍVEL %d  ·  %s", jogo->dificuldade, nomeDificuldade(jogo->dificuldade)),
                      132 + deslocamento, 32, Fade(COR_CLARO, entrada));

    // Os círculos dourados acompanham a dificuldade, inclusive o nível que acabou de abrir.
    DrawRectangle(224, 210, 352, 4, COR_FUNDO);
    DrawRectangle(224, 210, (nivelAtual - 1) * 88, 4, Fade(COR_DOURADO, entrada));
    for (int nivel = MUITO_FACIL; nivel <= IMPOSSIVEL; nivel++)
    {
        int x = 224 + (nivel - MUITO_FACIL) * 88;
        bool aberto = nivel <= nivelAtual;
        if (nivel == nivelAtual)
            DrawCircle(x, 212, 24, Fade(COR_DOURADO, brilho * entrada));
        DrawCircle(x, 212, 18, aberto ? Fade(COR_DOURADO, entrada) : COR_FUNDO);
        DrawCircleLines(x, 212, 18, Fade(COR_DOURADO, entrada));
        textoCentradoEm(TextFormat("%d", nivel), x, 203, 18, Fade(aberto ? COR_FUNDO : COR_CLARO, entrada));
    }

    DrawCircle(400, 299, 45, Fade(COR_DOURADO, brilho * entrada));
    DrawCircleLines(400, 299, 43, Fade(COR_DOURADO, entrada * 0.5f));
    desenharEscudo(400, 299, 25, Fade(COR_DOURADO, entrada));
    textoCentralizado("CHECKPOINT ALCANÇADO", 356, 21, Fade(COR_DOURADO, entrada));
    textoCentralizado(emReais(jogo->premioGarantido), 383, 34, Fade(COR_CLARO, entrada));
    textoCentralizado("Este prêmio está protegido em caso de derrota.", 433, 17, Fade(COR_CLARO, entrada));
    return botao("PROSSEGUIR", (Rectangle){276, 477, 248, 43}, true);
}

AcaoResultado desenharResultado(const Jogo *jogo)
{
    desenharFundo();
    bool venceu = jogo->estado == VITORIA;
    int acertos = venceu ? TOTAL_RODADAS : jogo->rodada;
    const Rectangle painel = {112, 73, 576, 454};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, COR_DOURADO);
    textoCentralizado(venceu ? "VITÓRIA" : "FIM DE JOGO", 100, 40, venceu ? COR_DOURADO : corErro);
    textoCentralizado(TextFormat("%02d/%02d rodadas concluídas", acertos, TOTAL_RODADAS),
                      155, 21, corLegenda);

    int premioExibido = jogo->premio;
    if (!venceu)
    {
        float entrada = animarEntrada(jogo->tempoEstado, 1.05f);
        int antesDoErro = jogo->premio + jogo->premioPerdido;
        premioExibido += (int)lroundf(jogo->premioPerdido * (1.0f - entrada));
        if (jogo->ultimoCheckpoint == 0 && jogo->premioPerdido > 0)
            textoCentralizado("VOCÊ PERDEU TODO O PRÊMIO", 187, 16, Fade(corErro, entrada));
        DrawLine(400, 217, 400, 270, Fade(COR_DOURADO, 0.4f));
        textoCentradoEm("CONQUISTADO ANTES DO ERRO", 260, 221, 14, corLegenda);
        textoCentradoEm(emReais(antesDoErro), 260, 245, 23, COR_CLARO);
        textoCentradoEm("VALOR PERDIDO", 540, 221, 14, corLegenda);
        textoCentradoEm(emReais(jogo->premioPerdido), 540, 245, 23, corErro);
        const char *checkpoint = jogo->ultimoCheckpoint > 0 ?
            TextFormat("Último checkpoint  ·  Pergunta %02d", jogo->ultimoCheckpoint) :
            "Último checkpoint  ·  Nenhum alcançado";
        textoCentralizado(checkpoint, 293, 18, COR_CLARO);
    }

    const Rectangle valor = {224, venceu ? 254 : 331, 352, 99};
    DrawRectangleRec(valor, COR_FUNDO);
    DrawRectangleLinesEx(valor, 1, COR_DOURADO);
    const char *rotuloPremio = "PREMIAÇÃO FINAL";
    if (!venceu)
        rotuloPremio = premioExibido > jogo->premio ? "PRÊMIO ATUAL" : "PRÊMIO FINAL GARANTIDO";
    textoCentralizado(rotuloPremio, (int)valor.y + 15, 17, corLegenda);
    textoCentralizado(emReais(premioExibido), (int)valor.y + 45, 34, COR_DOURADO);

    if (botao("JOGAR NOVAMENTE", (Rectangle){141, 461, 248, 43}, true))
        return RESULTADO_NOVA_PARTIDA;
    if (desenharBotao("MENU PRINCIPAL", (Rectangle){411, 461, 248, 43}))
        return RESULTADO_MENU;
    return RESULTADO_SEM_ACAO;
}

void desenharRegrasInstrucoes(void)
{
    DrawCircleLines(78, 203, 19, COR_DOURADO);
    textoCentradoEm("$", 78, 192, 22, COR_DOURADO);
    DrawText(TextFormat("Conquiste %s", emReais(valorRodada(TOTAL_RODADAS - 1))), 114, 185, 31, COR_DOURADO);
    DrawText(TextFormat("A partida tem %d perguntas.", TOTAL_RODADAS), 114, 225, 20, COR_CLARO);

    DrawLine(58, 261, 742, 261, Fade(COR_DOURADO, 0.4f));
    DrawText("A roleta sorteia uma das cinco categorias:", 58, 278, 20, COR_CLARO);
    DrawText(TextFormat("%s  ·  %s  ·  %s  ·  %s  ·  %s", nomeCategoria(GEEK), nomeCategoria(ESPORTES),
                        nomeCategoria(MATEMATICA), nomeCategoria(GEOGRAFIA), nomeCategoria(BIOLOGIA)),
             58, 312, 19, COR_DOURADO);
    DrawText("Cada pergunta tem quatro alternativas.", 58, 356, 20, COR_CLARO);
    DrawText("Escolha uma alternativa e confirme sua resposta.", 58, 384, 20, COR_CLARO);
    DrawText(TextFormat("A dificuldade aumenta a cada %d rodadas concluídas.", RODADAS_POR_NIVEL),
             58, 412, 20, COR_CLARO);
}

void desenharPremiosInstrucoes(void)
{
    DrawText("DIFICULDADE", 65, 165, 15, COR_DOURADO);
    DrawText("PERGUNTAS", 292, 165, 15, COR_DOURADO);
    textoADireita("CHECKPOINT OU PRÊMIO MÁXIMO", 735, 165, 15, COR_DOURADO);

    // A última rodada de cada nível determina o valor exibido.
    for (int nivel = MUITO_FACIL; nivel <= IMPOSSIVEL; nivel++)
    {
        int primeira = (nivel - MUITO_FACIL) * RODADAS_POR_NIVEL + 1;
        int ultima = primeira + RODADAS_POR_NIVEL - 1;
        int y = 190 + (nivel - MUITO_FACIL) * 36;
        bool checkpoint = rodadaTemCheckpoint(ultima - 1);
        DrawRectangleRec((Rectangle){55, y, 690, 31}, Fade(COR_FUNDO, 0.55f));
        DrawText(nomeDificuldade((Dificuldade)nivel), 65, y + 6, 18, COR_CLARO);
        DrawText(TextFormat("%02d a %02d", primeira, ultima), 292, y + 6, 18, COR_CLARO);
        textoADireita(TextFormat("%s %s", emReais(valorRodada(ultima - 1)),
                                 checkpoint ? "garantidos" : "(prêmio máximo)"), 735, y + 6, 18, COR_DOURADO);
    }

    desenharEscudo(69, 407, 10, COR_DOURADO);
    DrawText("No erro definitivo, você perde apenas o que ganhou após o último checkpoint.",
             90, 391, 16, COR_CLARO);
    DrawText(TextFormat("Sem checkpoint, o prêmio final é %s.", emReais(0)), 90, 418, 16, COR_CLARO);
}

void desenharAjudasInstrucoes(void)
{
    DrawText("Guitar Hero  ·  PULAR E GANHAR", 58, 162, 21, COR_DOURADO);
    DrawText("Acerte pelo menos 70% das notas para conquistar o prêmio da rodada", 58, 192, 17, COR_CLARO);
    DrawText("e avançar sem responder. Indisponível na pergunta do milhão.", 58, 214, 17, COR_CLARO);

    DrawLine(58, 240, 742, 240, Fade(COR_DOURADO, 0.4f));
    DrawText("Travessia  ·  SEGUNDA CHANCE", 58, 249, 21, COR_DOURADO);
    DrawText("Avance 25 pistas com 3 vidas para ganhar outra chance na pergunta atual.",
             58, 278, 17, COR_CLARO);

    DrawLine(58, 302, 742, 302, Fade(COR_DOURADO, 0.4f));
    DrawText("Tiro ao Alvo  ·  ELIMINAR DUAS", 58, 312, 21, COR_DOURADO);
    DrawText("Faça 7.500 pontos em 30 segundos para eliminar duas alternativas incorretas.",
             58, 341, 17, COR_CLARO);

    DrawRectangleRec((Rectangle){55, 372, 690, 65}, Fade(COR_FUNDO, 0.55f));
    DrawText("Cada minigame tem uma tentativa por partida, consumida ao iniciar o desafio.",
             66, 382, 16, COR_CLARO);
    DrawText("No treino, você pode jogar quantas vezes quiser sem interferir na partida.",
             66, 409, 16, COR_CLARO);
}

bool botaoNavegacao(const char *texto, Rectangle area, bool disponivel)
{
    if (disponivel)
        return desenharBotao(texto, area);

    DrawRectangleRec(area, COR_FUNDO);
    DrawRectangleLinesEx(area, 1, Fade(COR_DOURADO, 0.4f));
    textoNoRetangulo(texto, area, 22, corLegenda);
    return false;
}

bool desenharComoJogar(int *pagina)
{
    const char *const abas[] = {"O JOGO", "PREMIAÇÃO", "AJUDAS"};
    const int total = (int)(sizeof abas / sizeof abas[0]);
    int proxima = *pagina;

    desenharFundo();
    textoCentralizado("COMO JOGAR", 42, 34, COR_DOURADO);
    for (int i = 0; i < total; i++)
    {
        if (botao(abas[i], (Rectangle){43 + i * 240, 94, 234, 39}, i == *pagina))
            proxima = i;
    }

    const Rectangle painel = {36, 148, 728, 306};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 1, COR_DOURADO);
    switch (*pagina)
    {
    case 0: desenharRegrasInstrucoes(); break;
    case 1: desenharPremiosInstrucoes(); break;
    case 2: desenharAjudasInstrucoes(); break;
    }

    bool anterior = *pagina > 0;
    bool seguinte = *pagina < total - 1;
    if (botaoNavegacao("ANTERIOR", (Rectangle){44, 469, 174, 40}, anterior) ||
        (anterior && IsKeyPressed(KEY_LEFT)))
        proxima = *pagina - 1;
    if (botaoNavegacao("PRÓXIMO", (Rectangle){582, 469, 174, 40}, seguinte) ||
        (seguinte && IsKeyPressed(KEY_RIGHT)))
        proxima = *pagina + 1;
    textoCentralizado(TextFormat("PÁGINA %d / %d", *pagina + 1, total), 481, 17, corLegenda);

    // O clique define a página do próximo quadro, mantendo a aba alinhada ao conteúdo.
    *pagina = proxima;
    return desenharBotao("VOLTAR AO MENU PRINCIPAL", (Rectangle){208, 524, 384, 40}) ||
           IsKeyPressed(KEY_BACKSPACE);
}

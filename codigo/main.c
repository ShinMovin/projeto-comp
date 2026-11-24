/**
 * @file main.c
 * @author VGA
 * @brief Abre o menu gráfico ou o gerenciamento do banco no terminal.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "interface.h"
#include "interface_ajudas.h"
#include "interface_banco.h"
#include "jogo.h"
#include "premio.h"
#include "roleta.h"

// Dados da partida e das telas que a acompanham. O banco é compartilhado com o editor.
typedef struct
{
    Jogo jogo;
    Roleta roleta;
    const char *avisoRoleta;
    Desafios ajudas;
} PartidaGrafica;

void prepararRoleta(PartidaGrafica *partida, const BancoPerguntas *banco)
{
    iniciarRoleta(&partida->roleta, banco, partida->jogo.dificuldade, categoriaBloqueada(&partida->jogo));
    partida->avisoRoleta = NULL;
}

void iniciarPartida(PartidaGrafica *partida, BancoPerguntas *banco)
{
    iniciarJogo(&partida->jogo, banco);
    liberarDesafios(&partida->ajudas);
    prepararRoleta(partida, banco);
}

// Atualiza só o que está visível. Durante uma ajuda, o relógio da pergunta fica parado.
void atualizarPartida(PartidaGrafica *partida, const BancoPerguntas *banco, float segundos)
{
    Jogo *jogo = &partida->jogo;
    if (partida->ajudas.tela != DESAFIOS_FECHADOS)
    {
        const Pergunta *pergunta = &banco->perguntas[jogo->indicePergunta];
        atualizarDesafios(&partida->ajudas, jogo, pergunta, segundos);
        return;
    }

    atualizarJogo(jogo, banco, segundos);
    if (jogo->estado == SORTEA)
        atualizarRoleta(&partida->roleta, segundos);
}

// Desenha o estado atual e devolve a tela que o loop deve mostrar no próximo quadro.
Tela desenharPartida(PartidaGrafica *partida, BancoPerguntas *banco)
{
    Jogo *jogo = &partida->jogo;
    if (partida->ajudas.tela != DESAFIOS_FECHADOS)
    {
        if (desenharDesafios(&partida->ajudas, jogo))
        {
            liberarDesafios(&partida->ajudas);
            if (avancarRodada(jogo) && jogo->estado == SORTEA)
                prepararRoleta(partida, banco);
        }
        return TELA_JOGAR;
    }

    switch (jogo->estado)
    {
    case NOVO_NIVEL:
        if (desenharNovoNivel(jogo))
        {
            jogo->estado = SORTEA;
            jogo->tempoEstado = 0.0f;
            prepararRoleta(partida, banco);
        }
        return TELA_JOGAR;

    case SORTEA:
    {
        Roleta *roleta = &partida->roleta;
        AcaoRoleta acao = desenharRoleta(roleta, partida->avisoRoleta, jogo->dificuldade,
                                        podeGirarDificuldade(jogo));
        if (acao == ROLETA_VOLTAR || (!roleta->deDificuldade && IsKeyPressed(KEY_BACKSPACE)))
        {
            if (!roleta->deDificuldade)
                return MENU_PRINCIPAL;
            prepararRoleta(partida, banco);
        }
        else if (acao == ROLETA_DIFICULDADE)
            iniciarRoletaDificuldade(roleta, banco, jogo->rodada);
        else if (acao == ROLETA_GIRAR)
            girarRoleta(roleta);
        else if (acao == ROLETA_PROSSEGUIR)
        {
            if (roleta->deDificuldade)
            {
                aplicarDificuldade(jogo, roleta->resultado - DIF_MANTER);
                prepararRoleta(partida, banco);
            }
            else if (!selecionarPergunta(jogo, banco, (Categoria)(GEEK + roleta->resultado)))
                partida->avisoRoleta = "Sem perguntas inéditas no setor sorteado.";
        }
        return TELA_JOGAR;
    }

    case VITORIA:
    case DERROTA:
    {
        AcaoResultado acao = desenharResultado(jogo);
        if (acao == RESULTADO_MENU || IsKeyPressed(KEY_BACKSPACE))
            return MENU_PRINCIPAL;
        if (acao == RESULTADO_NOVA_PARTIDA)
            iniciarPartida(partida, banco);
        return TELA_JOGAR;
    }
    default:
        break; // Os demais estados da partida usam a mesma tela da pergunta.
    }

    const Pergunta *pergunta = &banco->perguntas[jogo->indicePergunta];
    AcaoPergunta acao = desenharTelaPergunta(jogo, pergunta);
    switch (acao.tipo)
    {
    case PERGUNTA_ESCOLHER:
        escolherResposta(jogo, acao.alternativa);
        break;
    case PERGUNTA_CANCELAR:
        cancelarResposta(jogo);
        break;
    case PERGUNTA_CONFIRMAR:
        confirmarResposta(jogo);
        break;
    case PERGUNTA_AJUDA:
        abrirDesafios(&partida->ajudas);
        break;
    case PERGUNTA_AVANCAR:
        if (avancarRodada(jogo) && jogo->estado == SORTEA)
            prepararRoleta(partida, banco);
        break;
    case PERGUNTA_SEM_ACAO:
        break;
    }
    return TELA_JOGAR;
}

// Quando chamado pelo terminal, o banco já está aberto e a trava continua com o terminal.
void menuGrafico(BancoPerguntas *banco, ArquivosBanco *arquivos, bool travaTerminal)
{
    InitWindow(800, 600, "Jogo do Milhão");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    Tela tela = MENU_PRINCIPAL;
    int paginaInstrucoes = 0;
    PartidaGrafica partida = {0};
    Desafios treino = {0};
    EditorBanco editor;
    iniciarEditorBanco(&editor);
    const char *erroMenu = NULL;
    bool fechamentoAnterior = false;

    while (tela != TELA_SAIR)
    {
        bool fechamento = WindowShouldClose();
        bool emDesafio = tela == TELA_MINIGAMES ||
                         (tela == TELA_JOGAR && partida.ajudas.tela != DESAFIOS_FECHADOS);
        bool fechouJanela = fechamento && !fechamentoAnterior;
        bool pediuParaSair = !emDesafio && IsKeyPressed(KEY_ESCAPE);
        if (fechouJanela || pediuParaSair)
        {
            if (tela != TELA_BANCO || !editorTemAlteracoes(&editor))
                break;
            editor.confirmacao = CONFIRMAR_FECHAR;
        }
        fechamentoAnterior = fechamento;

        float tempoQuadro = GetFrameTime();
        if (tela == TELA_JOGAR)
            atualizarPartida(&partida, banco, tempoQuadro);
        else if (tela == TELA_MINIGAMES)
            atualizarDesafios(&treino, NULL, NULL, tempoQuadro);

        Tela telaAnterior = tela;
        BeginDrawing();
        switch (tela)
        {
        case MENU_PRINCIPAL:
            tela = desenharMenu();
            if (tela == TELA_INSTRUCOES)
                paginaInstrucoes = 0;
            if (tela == TELA_MINIGAMES)
                abrirDesafios(&treino);
            if (tela == TELA_JOGAR || tela == TELA_BANCO)
            {
                erroMenu = travaTerminal ? NULL : abrirBancoArquivos(arquivos, banco);
                if (erroMenu)
                    tela = MENU_PRINCIPAL;
                else if (tela == TELA_JOGAR)
                    iniciarPartida(&partida, banco);
                else
                    iniciarEditorBanco(&editor);
            }
            if (erroMenu)
            {
                DrawRectangle(31, 545, 738, 29, COR_FUNDO);
                textoCentralizado(erroMenu, 551, 14, COR_CLARO);
            }
            break;

        case TELA_JOGAR:
            tela = desenharPartida(&partida, banco);
            break;

        case TELA_BANCO:
        {
            AcaoEditor acao = desenharEditorBanco(&editor, arquivos, banco);
            if (acao != EDITOR_CONTINUA)
                tela = acao == EDITOR_MENU ? MENU_PRINCIPAL : TELA_SAIR;
            break;
        }

        case TELA_MINIGAMES:
            if (desenharDesafios(&treino, NULL))
            {
                liberarDesafios(&treino);
                tela = MENU_PRINCIPAL;
            }
            break;

        case TELA_INSTRUCOES:
            if (desenharComoJogar(&paginaInstrucoes))
                tela = MENU_PRINCIPAL;
            break;

        case TELA_SAIR:
            break;
        }

        EndDrawing();
        // A sessão gráfica só tem a posse do banco durante a partida ou o editor.
        bool saiuPartida = telaAnterior == TELA_JOGAR && tela != TELA_JOGAR;
        bool saiuEditor = telaAnterior == TELA_BANCO && tela != TELA_BANCO;
        if (!travaTerminal && (saiuPartida || saiuEditor))
            fecharBancoArquivos(arquivos);
    }

    if (!travaTerminal)
        fecharBancoArquivos(arquivos);
    liberarDesafios(&partida.ajudas);
    liberarDesafios(&treino);
    if (IsAudioDeviceReady())
        CloseAudioDevice();
    CloseWindow();
}

bool salvarTerminal(ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    const char *erro = salvarBancoArquivos(arquivos, banco);
    if (erro)
    {
        printf("%s\n", erro);
        erro = recarregarBancoArquivos(arquivos, banco);
        if (erro)
        {
            printf("%s\n", erro);
            return false;
        }
        printf("A alteração não foi salva. O banco em disco foi recarregado.\n");
        return true;
    }
    printf("Banco salvo em perguntas.dat.\n");
    for (int nivel = MUITO_FACIL; nivel <= IMPOSSIVEL; nivel++)
    {
        if (contaPerguntasDistintas(banco, (Dificuldade)nivel) < RODADAS_POR_NIVEL)
            printf("Atenção: o nível %d tem menos de 3 perguntas distintas.\n", nivel);
    }
    return true;
}

void menuTerminal(BancoPerguntas *banco, ArquivosBanco *arquivos)
{
    if (arquivos->somenteLeitura)
        printf("Um arquivo está inválido ou ilegível. Banco aberto para consulta.\n");

    for (;;)
    {
        printf("======= MENU =======\n");
        printf("1 - Listar perguntas\n2 - Cadastrar pergunta\n3 - Pesquisar pergunta\n4 - Alterar pergunta\n5 - Excluir pergunta\n6 - Exportar perguntas para CSV\n7 - Jogar\n0 - Sair\n");
        printf("====================\n");

        int opcao = -1;

        printf("Opção :");
        if (scanf(" %d", &opcao) == EOF)
            return;
        printf("\n");

        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        bool mudou = false;
        if (arquivos->somenteLeitura && (opcao == 2 || opcao == 4 || opcao == 5))
        {
            printf("Corrija o arquivo inválido ou ilegível antes de editar.\n");
            continue;
        }

        switch (opcao)
        {
        case 1:
            listaPergunta(banco);
            break;

        case 2:
        {
            Pergunta nova;
            if (lerPergunta(&nova, false))
            {
                mudou = adicionaPerguntas(banco, &nova);
                if (!mudou)
                    printf("Não foi possível adicionar a pergunta.\n");
            }
            break;
        }

        case 3:
            pesquisaPergunta(banco);
            break;

        case 4:
            mudou = alteraPergunta(banco);
            break;

        case 5:
            mudou = excluiPergunta(banco);
            break;

        case 6:
            if (salvaPergunta(banco, TextFormat("%s../dados/perguntas_exportadas.csv", GetApplicationDirectory())))
            {
                printf("Perguntas exportadas para dados/perguntas_exportadas.csv\n");
            }
            else
                printf("Falha ao exportar as perguntas.\n");
            break;

        case 7:
            menuGrafico(banco, arquivos, true);
            break;

        case 0:
            printf("Saindo do menu... !\n");
            return;
        
        default:
            printf("Opção invalida !\n");
            break;
        }
        if (mudou && !salvarTerminal(arquivos, banco))
            return;
    }
}

int main(int argc, char **argv)
{
    BancoPerguntas banco = {0};
    ArquivosBanco arquivos;
    srand((unsigned int)time(NULL));
    if (!inicializarArquivosBanco(&arquivos, TextFormat("%s../dados", GetApplicationDirectory())))
        return 1;

    bool terminal = argc > 1 && strcmp(argv[1], "--banco") == 0;
    int resultado = 0;
    if (terminal)
    {
        const char *erro = abrirBancoArquivos(&arquivos, &banco);
        if (erro)
        {
            printf("%s\n", erro);
            resultado = 1;
        }
        else
            menuTerminal(&banco, &arquivos);
    }
    else
        menuGrafico(&banco, &arquivos, false);

    fecharBancoArquivos(&arquivos);
    liberaBancoPergunta(&banco);
    return resultado;
}

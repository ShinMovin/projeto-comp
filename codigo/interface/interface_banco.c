#include "interface_banco.h"
#include "interface.h"
#include "premio.h"
#include <ctype.h>
#include <string.h>

enum { LINHAS_POR_PAGINA = 5, CAMPOS_FORMULARIO = 8 };
const Color corErroBanco = {255, 150, 151, 255};
const Color corAvisoBanco = {244, 202, 111, 255};
const Color corSucessoBanco = {137, 222, 171, 255};
const Rectangle areasSeletores[] = {{35, 397, 165, 42}, {216, 397, 238, 42}, {470, 397, 296, 42}};

// A mesma área recebe o clique e desenha o campo do formulário.
Rectangle areaCampoTexto(int indice)
{
    return (Rectangle){175, 83 + indice * 56, 591, 42};
}

void iniciarEditorBanco(EditorBanco *editor)
{
    *editor = (EditorBanco){.selecionada = -1, .edicao = -1};
}

void mensagem(EditorBanco *editor, const char *texto, bool erro)
{
    editor->mensagem = texto;
    editor->erro = erro;
}

bool rascunhoAlterado(const EditorBanco *editor)
{
    return editor->formulario && !perguntasIguais(&editor->rascunho, &editor->original);
}

bool editorTemAlteracoes(const EditorBanco *editor)
{
    return editor->alterado || rascunhoAlterado(editor);
}

bool botaoEditor(const char *texto, Rectangle area, bool ativo)
{
    return desenharBotao(texto, area) && ativo;
}

// A fila da Raylib entrega pontos Unicode. Cada caractere entra inteiro no campo.
bool editarTexto(char *texto, int maximo)
{
    size_t tamanho = strlen(texto);
    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && tamanho > 0)
    {
        // Uma letra acentuada ocupa mais de um byte. A raylib diz quantos bytes tem a última
        // letra, e o Backspace apaga todos eles de uma vez.
        int bytes = 1;
        GetCodepointPrevious(texto + tamanho, &bytes);
        tamanho -= (size_t)bytes;
        texto[tamanho] = '\0';
    }

    bool atingiuLimite = false;
    int codigo;
    while ((codigo = GetCharPressed()) != 0)
    {
        if (codigo < 32 || codigo == 127)
            continue;
        int bytes;
        const char *utf8 = CodepointToUTF8(codigo, &bytes);
        if (tamanho + (size_t)bytes > (size_t)maximo)
        {
            atingiuLimite = true;
            continue;
        }
        for (int i = 0; i < bytes; i++)
            texto[tamanho++] = utf8[i];
        texto[tamanho] = '\0';
    }
    return atingiuLimite;
}

void desenharCampo(const char *texto, const char *vazio, Rectangle area, bool foco)
{
    DrawRectangleRec(area, COR_AZUL);
    DrawRectangleLinesEx(area, foco ? 2 : 1, foco ? COR_DOURADO : Fade(COR_DOURADO, 0.65f));
    const char *conteudo = texto[0] ? texto : vazio;
    const int tamanho = 18;
    int largura = MeasureText(conteudo, tamanho);
    int excesso = largura - ((int)area.width - 26);
    int deslocamento = foco && excesso > 0 ? excesso : 0;

    // O foco acompanha o fim da linha e a moldura do campo fica inteira.
    BeginScissorMode((int)area.x + 9, (int)area.y + 3, (int)area.width - 18, (int)area.height - 6);
    int x = (int)area.x + 10 - deslocamento;
    int y = (int)(area.y + (area.height - tamanho) / 2);
    DrawText(conteudo, x, y, tamanho, texto[0] ? COR_CLARO : Fade(COR_CLARO, 0.60f));
    if (foco && texto[0])
        DrawRectangle(x + largura + 2, y, 1, tamanho, COR_DOURADO);
    EndScissorMode();
}

void desenharAvisos(const EditorBanco *editor, const ArquivosBanco *arquivos, const BancoPerguntas *banco)
{
    int abaixo[5];
    int quantidade = 0;
    for (int nivel = MUITO_FACIL; nivel <= IMPOSSIVEL; nivel++)
    {
        if (contaPerguntasDistintas(banco, (Dificuldade)nivel) < RODADAS_POR_NIVEL)
            abaixo[quantidade++] = nivel;
    }
    int x = 35;
    if (quantidade)
    {
        const char *aviso = "Menos de 3 perguntas distintas nos níveis";
        DrawText(aviso, x, 485, 15, corAvisoBanco);
        x += MeasureText(aviso, 15) + 7;
        for (int i = 0; i < quantidade; i++)
        {
            DrawText(TextFormat("%d", abaixo[i]), x, 485, 15, corAvisoBanco);
            x += 20;
        }
    }

    if (editorTemAlteracoes(editor))
    {
        const char *aviso = "ALTERAÇÕES NÃO SALVAS";
        DrawText(aviso, 765 - MeasureText(aviso, 14), 486, 14, corAvisoBanco);
    }

    if (editor->mensagem)
        DrawText(editor->mensagem, 35, 508, 14, editor->erro ? corErroBanco : corSucessoBanco);
    else if (arquivos->somenteLeitura)
        DrawText("Um arquivo está inválido ou ilegível. Banco aberto para consulta.", 35, 508, 14, corAvisoBanco);
}

void abrirFormulario(EditorBanco *editor, const BancoPerguntas *banco, int indice)
{
    editor->edicao = indice;
    editor->rascunho = indice < 0 ? (Pergunta){.alternativa_correta = 'A',
                       .categorias = GEEK, .dificuldades = MUITO_FACIL} : banco->perguntas[indice];
    editor->original = editor->rascunho;
    editor->campo = 0;
    editor->formulario = true;
    editor->mensagem = NULL;
}

bool aplicarFormulario(EditorBanco *editor, BancoPerguntas *banco)
{
    const char *erro = validarPergunta(&editor->rascunho);
    if (erro)
    {
        mensagem(editor, erro, true);
        return false;
    }
    bool novo = editor->edicao < 0;
    if (novo || rascunhoAlterado(editor))
    {
        bool alterou = novo ? adicionaPerguntas(banco, &editor->rascunho) :
                       editarPerguntaPorIndice(banco, editor->edicao, &editor->rascunho);
        if (!alterou)
        {
            mensagem(editor, "Não foi possível alterar o registro. Confira o índice e a memória.", true);
            return false;
        }
        editor->selecionada = novo ? banco->quantidade - 1 : editor->edicao;
        if (novo)
        {
            editor->pesquisa[0] = '\0';
            editor->filtroNivel = 0;
            editor->filtroCategoria = 0;
            editor->pagina = editor->selecionada / LINHAS_POR_PAGINA;
        }
        editor->alterado = true;
        mensagem(editor, novo ? "Pergunta adicionada ao banco em edição." : "Pergunta atualizada no banco em edição.", false);
    }
    editor->formulario = false;
    return true;
}

bool salvarEditor(EditorBanco *editor, ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    if (rascunhoAlterado(editor) && !aplicarFormulario(editor, banco))
        return false;
    const char *erro = salvarBancoArquivos(arquivos, banco);
    mensagem(editor, erro ? erro : "Banco salvo em perguntas.dat.", erro != NULL);
    if (erro)
        return false;
    editor->alterado = false;
    return true;
}

void desenharDetalhe(const Pergunta *p)
{
    const Rectangle painel = {367, 138, 399, 302};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 1, Fade(COR_DOURADO, 0.65f));
    if (!p)
    {
        DrawText("Selecione uma pergunta da lista.", 382, 155, 17, COR_CLARO);
        return;
    }

    desenharTextoPergunta(p->conteudo_da_pergunta, (Rectangle){381, 149, 369, 61}, 20, false, COR_CLARO);
    for (int i = 0; i < 4; i++)
    {
        int y = 218 + 45 * i;
        Color cor = p->alternativa_correta == 'A' + i ? corSucessoBanco : COR_CLARO;
        DrawText(TextFormat("%c", 'A' + i), 382, y + 11, 17, cor);
        desenharTextoPergunta(p->alternativas[i], (Rectangle){408, y, 342, 42}, 17, false, cor);
    }
    DrawText(TextFormat("%s  /  Nível %d  /  Gabarito %c", nomeCategoria(p->categorias), p->dificuldades,
                        p->alternativa_correta), 381, 409, 16, COR_DOURADO);
}

bool passaNosFiltros(const EditorBanco *editor, const Pergunta *p)
{
    return (!editor->filtroCategoria || (int)p->categorias == editor->filtroCategoria) &&
           (!editor->filtroNivel || (int)p->dificuldades == editor->filtroNivel) &&
           strstr(p->conteudo_da_pergunta, editor->pesquisa) != NULL;
}

void desenharConsulta(EditorBanco *editor, const ArquivosBanco *arquivos, const BancoPerguntas *banco, bool ativo)
{
    DrawText("BANCO DE PERGUNTAS", 35, 34, 28, COR_DOURADO);
    const char *total = TextFormat("%d perguntas cadastradas", banco->quantidade);
    DrawText(total, 765 - MeasureText(total, 18), 42, 18, COR_CLARO);

    const Rectangle pesquisa = {35, 82, 310, 39};
    if (ativo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        editor->pesquisando = CheckCollisionPointRec(GetMousePosition(), pesquisa);
    if (ativo && IsKeyPressed(KEY_TAB))
        editor->pesquisando = true;
    if (ativo && editor->pesquisando)
    {
        char anterior[sizeof editor->pesquisa];
        strcpy(anterior, editor->pesquisa);
        editarTexto(editor->pesquisa, LIMITE_TEXTO_PERGUNTA);
        if (strcmp(anterior, editor->pesquisa))
            editor->pagina = 0;
    }

    desenharCampo(editor->pesquisa, "Buscar no enunciado", pesquisa, editor->pesquisando);
    const char *categoria = editor->filtroCategoria ? nomeCategoria((Categoria)editor->filtroCategoria) : "Categorias";
    if (botaoEditor(TextFormat("%s >", categoria), (Rectangle){358, 82, 205, 39}, ativo))
    {
        editor->filtroCategoria = (editor->filtroCategoria + 1) % 6;
        editor->pagina = 0;
    }
    const char *nivel = editor->filtroNivel ? TextFormat("Nível %d >", editor->filtroNivel) : "Todos os níveis";
    if (botaoEditor(nivel, (Rectangle){576, 82, 190, 39}, ativo))
    {
        editor->filtroNivel = (editor->filtroNivel + 1) % 6;
        editor->pagina = 0;
    }

    int quantidade = 0;
    for (int i = 0; i < banco->quantidade; i++)
        if (passaNosFiltros(editor, &banco->perguntas[i]))
            quantidade++;
    int ultima = quantidade > 0 ? (quantidade - 1) / LINHAS_POR_PAGINA : 0;
    if (editor->pagina > ultima)
        editor->pagina = ultima;

    if (editor->selecionada >= banco->quantidade ||
        (editor->selecionada >= 0 && !passaNosFiltros(editor, &banco->perguntas[editor->selecionada])))
        editor->selecionada = -1;
    int pular = editor->pagina * LINHAS_POR_PAGINA;
    int exibidas = 0;
    for (int indice = 0; indice < banco->quantidade && exibidas < LINHAS_POR_PAGINA; indice++)
    {
        if (!passaNosFiltros(editor, &banco->perguntas[indice]))
            continue;
        if (pular > 0)
        {
            pular--;
            continue;
        }
        Rectangle linha = {35, 138 + exibidas * 52, 315, 46};
        exibidas++;
        bool sobre = CheckCollisionPointRec(GetMousePosition(), linha);
        bool selecionada = editor->selecionada == indice;
        DrawRectangleRec(linha, sobre || selecionada ? ColorBrightness(COR_AZUL, 0.14f) : COR_AZUL);
        DrawRectangleLinesEx(linha, selecionada ? 2 : 1, Fade(COR_DOURADO, selecionada ? 1 : 0.55f));
        DrawText(TextFormat("%d", indice + 1), 44, (int)linha.y + 15, 15, COR_DOURADO);
        desenharTextoPergunta(banco->perguntas[indice].conteudo_da_pergunta,
                               (Rectangle){83, linha.y + 4, 255, 38}, 16, false, COR_CLARO);
        if (ativo && sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            editor->selecionada = indice;
    }
    if (quantidade == 0)
        DrawText("Nenhuma pergunta encontrada.", 39, 152, 16, COR_CLARO);

    if (editor->pagina > 0 && botaoEditor("<", (Rectangle){35, 407, 50, 34}, ativo))
        editor->pagina--;
    if (editor->pagina < ultima && botaoEditor(">", (Rectangle){300, 407, 50, 34}, ativo))
        editor->pagina++;
    DrawText(TextFormat("%d/%d  |  %d resultados", editor->pagina + 1, ultima + 1, quantidade), 104, 418, 14, COR_CLARO);

    const Pergunta *p = editor->selecionada < 0 ? NULL : &banco->perguntas[editor->selecionada];
    desenharDetalhe(p);
    if (p && !arquivos->somenteLeitura)
    {
        if (botaoEditor("EDITAR", (Rectangle){367, 443, 188, 35}, ativo))
            abrirFormulario(editor, banco, editor->selecionada);
        if (botaoEditor("EXCLUIR", (Rectangle){568, 443, 198, 35}, ativo))
            editor->confirmacao = CONFIRMAR_EXCLUSAO;
    }
}

void editarSeletores(EditorBanco *editor, bool ativo)
{
    const char *rotulos[] = {"Gabarito", "Dificuldade", "Categoria"};
    int valores[] = {editor->rascunho.alternativa_correta - 'A',
                     (int)editor->rascunho.dificuldades - 1, (int)editor->rascunho.categorias - 1};
    const int opcoes[] = {4, 5, 5};
    for (int i = 0; i < 3; i++)
    {
        bool clicou = ativo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), areasSeletores[i]);
        bool foco = editor->campo == 5 + i;
        if (ativo && foco)
        {
            int passo = 0;
            if (clicou || IsKeyPressed(KEY_RIGHT))
                passo = 1;
            else if (IsKeyPressed(KEY_LEFT))
                passo = -1;
            // Soma o total antes do resto para voltar da primeira opção à última.
            valores[i] = (valores[i] + opcoes[i] + passo) % opcoes[i];
            int codigo;
            while ((codigo = GetCharPressed()) != 0)
            {
                if (i == 0 && codigo < 128)
                {
                    int letra = toupper((unsigned char)codigo);
                    if (letra >= 'A' && letra <= 'D')
                        valores[i] = letra - 'A';
                }
                else if (i != 0 && codigo >= '1' && codigo <= '5')
                    valores[i] = codigo - '1';
            }
        }
        const char *valor;
        if (i == 0)
            valor = TextFormat("%c", 'A' + valores[i]);
        else if (i == 1)
            valor = TextFormat("Nível %d", valores[i] + 1);
        else
            valor = nomeCategoria((Categoria)(valores[i] + 1));
        DrawText(rotulos[i], (int)areasSeletores[i].x, 374, 17, COR_DOURADO);
        desenharCampo(valor, "", areasSeletores[i], foco);
        DrawText(">", (int)(areasSeletores[i].x + areasSeletores[i].width) - 23, 409, 17, COR_DOURADO);
    }
    editor->rascunho.alternativa_correta = (char)('A' + valores[0]);
    editor->rascunho.dificuldades = (Dificuldade)(valores[1] + 1);
    editor->rascunho.categorias = (Categoria)(valores[2] + 1);
}

void desenharFormulario(EditorBanco *editor, bool ativo)
{
    DrawText(editor->edicao < 0 ? "ADICIONAR PERGUNTA" : "EDITAR PERGUNTA", 35, 34, 28, COR_DOURADO);
    if (ativo && IsKeyPressed(KEY_TAB))
    {
        int passo = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? -1 : 1;
        editor->campo = (editor->campo + CAMPOS_FORMULARIO + passo) % CAMPOS_FORMULARIO;
    }

    char *campos[] = {editor->rascunho.conteudo_da_pergunta, editor->rascunho.alternativas[0],
                     editor->rascunho.alternativas[1], editor->rascunho.alternativas[2], editor->rascunho.alternativas[3]};
    // Resolve todo o foco antes de consumir os caracteres do quadro.
    if (ativo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        for (int i = 0; i < 5; i++)
        {
            if (CheckCollisionPointRec(GetMousePosition(), areaCampoTexto(i)))
                editor->campo = i;
        }
        for (int i = 0; i < 3; i++)
        {
            if (CheckCollisionPointRec(GetMousePosition(), areasSeletores[i]))
                editor->campo = 5 + i;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        Rectangle area = areaCampoTexto(i);
        bool foco = editor->campo == i;
        if (ativo && foco && editarTexto(campos[i], LIMITE_TEXTO_PERGUNTA))
            mensagem(editor, "Limite de 200 bytes neste campo.", true);
        DrawText(i == 0 ? "Enunciado" : TextFormat("Alternativa %c", 'A' + i - 1),
                 35, (int)area.y + 13, 17, COR_CLARO);
        desenharCampo(campos[i], "Digite o texto", area, foco);
    }
    editarSeletores(editor, ativo);
    DrawText("TAB muda o campo. Nas opções, use as setas, a letra ou o número.", 35, 452, 15, COR_CLARO);
}

void desenharModal(const char *titulo, const char *texto)
{
    DrawRectangle(0, 0, 800, 600, Fade(COR_FUNDO, 0.86f));
    const Rectangle painel = {90, 192, 620, 198};
    DrawRectangleRec(painel, COR_AZUL);
    DrawRectangleLinesEx(painel, 2, COR_DOURADO);
    textoCentralizado(titulo, 217, 25, COR_DOURADO);
    textoCentralizado(texto, 262, 17, COR_CLARO);
}

AcaoEditor confirmarAcao(EditorBanco *editor, ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    if (editor->confirmacao == CONFIRMAR_EXCLUSAO)
    {
        desenharModal("EXCLUIR PERGUNTA?", TextFormat("Confirme a exclusão da pergunta %d.", editor->selecionada + 1));
        if (desenharBotao("CANCELAR", (Rectangle){184, 318, 206, 42}))
            editor->confirmacao = SEM_CONFIRMACAO;
        if (desenharBotao("EXCLUIR", (Rectangle){410, 318, 206, 42}))
        {
            if (!arquivos->somenteLeitura && excluirPerguntaPorIndice(banco, editor->selecionada))
            {
                editor->selecionada = -1;
                editor->alterado = true;
                mensagem(editor, "Pergunta excluída do banco em edição.", false);
            }
            else
                mensagem(editor, "Não foi possível excluir a pergunta.", true);
            editor->confirmacao = SEM_CONFIRMACAO;
        }
        return EDITOR_CONTINUA;
    }

    desenharModal("ALTERAÇÕES NÃO SALVAS", "Salve o banco, descarte as alterações ou continue editando.");
    AcaoEditor saida = editor->confirmacao == CONFIRMAR_FECHAR ? EDITOR_FECHA : EDITOR_MENU;
    if (desenharBotao("SALVAR", (Rectangle){110, 318, 179, 42}))
    {
        editor->confirmacao = SEM_CONFIRMACAO;
        if (salvarEditor(editor, arquivos, banco))
            return saida;
    }
    if (desenharBotao("DESCARTAR", (Rectangle){310, 318, 179, 42}))
    {
        editor->confirmacao = SEM_CONFIRMACAO;
        const char *erro = editor->alterado ? recarregarBancoArquivos(arquivos, banco) : NULL;
        if (!erro)
        {
            iniciarEditorBanco(editor);
            return saida;
        }
        mensagem(editor, erro, true);
    }
    if (desenharBotao("CONTINUAR", (Rectangle){510, 318, 179, 42}))
        editor->confirmacao = SEM_CONFIRMACAO;
    return EDITOR_CONTINUA;
}

AcaoEditor desenharEditorBanco(EditorBanco *editor, ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    desenharFundo();
    bool ativo = editor->confirmacao == SEM_CONFIRMACAO;
    bool formulario = editor->formulario;
    if (formulario)
        desenharFormulario(editor, ativo);
    else
        desenharConsulta(editor, arquivos, banco, ativo);

    desenharAvisos(editor, arquivos, banco);
    if (formulario)
    {
        if (botaoEditor("CANCELAR", (Rectangle){35, 531, 194, 37}, ativo))
        {
            editor->formulario = false;
            editor->mensagem = NULL;
        }
        if (botaoEditor("APLICAR PERGUNTA", (Rectangle){515, 531, 251, 37}, ativo))
            aplicarFormulario(editor, banco);
    }
    else
    {
        if (botaoEditor("VOLTAR AO MENU", (Rectangle){35, 531, 218, 37}, ativo))
        {
            if (!editorTemAlteracoes(editor))
                return EDITOR_MENU;
            editor->confirmacao = CONFIRMAR_MENU;
        }
        if (!arquivos->somenteLeitura)
        {
            if (botaoEditor("ADICIONAR PERGUNTA", (Rectangle){265, 531, 276, 37}, ativo))
                abrirFormulario(editor, banco, -1);
            if (botaoEditor("SALVAR BANCO", (Rectangle){553, 531, 213, 37}, ativo))
                salvarEditor(editor, arquivos, banco);
        }
    }
    if (!ativo)
        return confirmarAcao(editor, arquivos, banco);
    return EDITOR_CONTINUA;
}

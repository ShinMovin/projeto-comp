#ifndef INTERFACE_BANCO_H
#define INTERFACE_BANCO_H

#include "banco_arquivos.h"

typedef enum { EDITOR_CONTINUA, EDITOR_MENU, EDITOR_FECHA } AcaoEditor;
typedef enum { SEM_CONFIRMACAO, CONFIRMAR_EXCLUSAO, CONFIRMAR_MENU, CONFIRMAR_FECHAR } ConfirmacaoBanco;

typedef struct
{
    char pesquisa[LIMITE_TEXTO_PERGUNTA + 1];
    int filtroCategoria;
    int filtroNivel;
    int pagina;
    int selecionada;
    int edicao;
    int campo;
    bool pesquisando;
    bool formulario;
    bool alterado;
    bool erro;
    ConfirmacaoBanco confirmacao;
    Pergunta rascunho;
    Pergunta original;
    const char *mensagem;
} EditorBanco;

void iniciarEditorBanco(EditorBanco *editor);
bool editorTemAlteracoes(const EditorBanco *editor);
AcaoEditor desenharEditorBanco(EditorBanco *editor, ArquivosBanco *arquivos, BancoPerguntas *banco);

#endif

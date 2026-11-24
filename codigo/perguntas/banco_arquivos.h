#ifndef BANCO_ARQUIVOS_H
#define BANCO_ARQUIVOS_H

#include "perguntas.h"

enum { CAMINHO_BANCO_MAXIMO = 4096 };

// Guarda a pasta e a trava do sistema enquanto uma sessão usa o banco.
typedef struct
{
    char pasta[CAMINHO_BANCO_MAXIMO];
    int trava;               // descritor do arquivo .banco-lock (flock)
    bool bloqueado;
    bool somenteLeitura;
} ArquivosBanco;

bool inicializarArquivosBanco(ArquivosBanco *arquivos, const char *pasta);
/** @brief Obtém a trava e carrega o banco. Retorna NULL ou uma mensagem de erro. */
const char *abrirBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco);
/** @brief Substitui os dados em memória pelo disco usando a trava já obtida. */
const char *recarregarBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco);
/** @brief Confere o DAT temporário antes de substituir o arquivo ativo. Retorna NULL ou erro. */
const char *salvarBancoArquivos(ArquivosBanco *arquivos, const BancoPerguntas *banco);
void fecharBancoArquivos(ArquivosBanco *arquivos);

#endif

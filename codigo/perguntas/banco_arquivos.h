#ifndef BANCO_ARQUIVOS_H
#define BANCO_ARQUIVOS_H

#include <stdint.h>
#include "perguntas.h"

enum { CAMINHO_BANCO_MAXIMO = 4096 };

typedef struct
{
    char pasta[CAMINHO_BANCO_MAXIMO];
    intptr_t trava;
    bool bloqueado;
    bool somenteLeitura;
    int somenteDat;
    int somenteCsv;
} ArquivosBanco;

bool inicializarArquivosBanco(ArquivosBanco *arquivos, const char *pasta);
const char *abrirBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco);
const char *recarregarBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco);
const char *salvarBancoArquivos(ArquivosBanco *arquivos, const BancoPerguntas *banco);
void fecharBancoArquivos(ArquivosBanco *arquivos);
void configurarTerminalUtf8(void);

#endif

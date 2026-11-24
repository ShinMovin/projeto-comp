#define _POSIX_C_SOURCE 200809L
#include "banco_arquivos.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

void montarCaminhoBanco(char caminho[CAMINHO_BANCO_MAXIMO], const ArquivosBanco *arquivos, const char *nome)
{
    // A inicialização reserva 32 bytes para a barra e o nome do arquivo.
    strcpy(caminho, arquivos->pasta);
    strcat(caminho, "/");
    strcat(caminho, nome);
}

bool inicializarArquivosBanco(ArquivosBanco *arquivos, const char *pasta)
{
    *arquivos = (ArquivosBanco){.trava = -1};
    if (strlen(pasta) >= sizeof arquivos->pasta - 32)
        return false;
    strcpy(arquivos->pasta, pasta);
    return true;
}

void fecharBancoArquivos(ArquivosBanco *arquivos)
{
    if (arquivos->trava >= 0)
    {
        close(arquivos->trava);
        arquivos->trava = -1;
    }
}

/* Devolve 1 para arquivo comum, 0 se não existe e -1 se há outro problema. */
int estadoArquivoBanco(const char *caminho)
{
    struct stat dados;
    if (stat(caminho, &dados) == 0)
        return S_ISREG(dados.st_mode) ? 1 : -1;
    return errno == ENOENT ? 0 : -1;
}

bool gravarDatTemporario(const BancoPerguntas *banco, const char *caminho)
{
    if (!salvaBinario(banco, caminho))
        return false;
    FILE *arquivo = fopen(caminho, "r+b");
    if (!arquivo)
        return false;

    // Confere o arquivo recém-gravado uma pergunta por vez.
    int quantidade = 0;
    bool ok = fsync(fileno(arquivo)) == 0 &&
              fread(&quantidade, sizeof quantidade, 1, arquivo) == 1 && quantidade == banco->quantidade;
    for (int i = 0; ok && i < quantidade; i++)
    {
        Pergunta pergunta;
        ok = fread(&pergunta, sizeof pergunta, 1, arquivo) == 1 &&
             !validarPergunta(&pergunta) && perguntasIguais(&pergunta, &banco->perguntas[i]);
    }
    ok = ok && fgetc(arquivo) == EOF && !ferror(arquivo);
    return fclose(arquivo) == 0 && ok;
}

bool recuperarGravacaoAntiga(const ArquivosBanco *arquivos)
{
    char marcador[CAMINHO_BANCO_MAXIMO];
    montarCaminhoBanco(marcador, arquivos, ".banco-pendente");
    int estado = estadoArquivoBanco(marcador);
    if (estado <= 0)
        return estado == 0;

    // O marcador antigo guardava dois caracteres, um para cada arquivo.
    FILE *arquivo = fopen(marcador, "rb");
    if (!arquivo)
        return false;
    int csv = fgetc(arquivo);
    int dat = fgetc(arquivo);
    bool valido = fgetc(arquivo) == EOF && !ferror(arquivo);
    if (fclose(arquivo) != 0 || !valido || (csv != '0' && csv != '1') || (dat != '0' && dat != '1'))
        return false;

    char destino[CAMINHO_BANCO_MAXIMO];
    montarCaminhoBanco(destino, arquivos, "perguntas.dat");
    if (dat == '1')
    {
        char backup[CAMINHO_BANCO_MAXIMO], temp[CAMINHO_BANCO_MAXIMO];
        montarCaminhoBanco(backup, arquivos, ".banco-dat.anterior");
        montarCaminhoBanco(temp, arquivos, ".banco-dat.novo");
        BancoPerguntas anterior = {0};
        bool restaurado = carregaBinario(&anterior, backup) && gravarDatTemporario(&anterior, temp) &&
                          rename(temp, destino) == 0;
        liberaBancoPergunta(&anterior);
        if (!restaurado)
            return false;
    }
    else if (remove(destino) != 0 && errno != ENOENT)
        return false;

    if (remove(marcador) != 0)
        return false;
    const char *restos[] = {".banco-csv.novo", ".banco-dat.novo", ".banco-csv.anterior",
                            ".banco-dat.anterior", ".banco-pendente.novo", ".banco-restauracao"};
    for (int i = 0; i < 6; i++)
    {
        montarCaminhoBanco(destino, arquivos, restos[i]);
        remove(destino);
    }
    return true;
}

const char *recarregarBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    if (arquivos->trava < 0 || !recuperarGravacaoAntiga(arquivos))
        return "Não foi possível recuperar os arquivos do banco.";

    char localCsv[CAMINHO_BANCO_MAXIMO], localDat[CAMINHO_BANCO_MAXIMO];
    montarCaminhoBanco(localDat, arquivos, "perguntas.dat");
    int existeDat = estadoArquivoBanco(localDat);
    if (existeDat == 1 && carregaBinario(banco, localDat))
    {
        arquivos->somenteLeitura = false;
        return NULL;
    }

    montarCaminhoBanco(localCsv, arquivos, "perguntas.csv");
    int existeCsv = estadoArquivoBanco(localCsv);
    arquivos->somenteLeitura = existeDat != 0;
    if (existeCsv == 1 && carregaPerguntas(banco, localCsv))
    {
        if (existeDat == 0)
            salvarBancoArquivos(arquivos, banco);
        return NULL;
    }
    if (existeDat != 0 || existeCsv != 0)
        return "O CSV e o DAT estão indisponíveis ou inválidos.";

    BancoPerguntas vazio;
    if (!inicializaBanco(&vazio))
        return "Memória insuficiente para abrir o banco.";
    liberaBancoPergunta(banco);
    *banco = vazio;
    return NULL;
}

const char *abrirBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    // A trava fica num arquivo separado porque o DAT muda de nome ao salvar.
    char caminho[CAMINHO_BANCO_MAXIMO];
    montarCaminhoBanco(caminho, arquivos, ".banco-lock");
    int trava = open(caminho, O_CREAT | O_RDWR, 0666);
    if (trava < 0)
        return "Banco em uso ou sem acesso. Feche a outra sessão e tente novamente.";
    if (flock(trava, LOCK_EX | LOCK_NB) != 0)
    {
        close(trava);
        return "Banco em uso ou sem acesso. Feche a outra sessão e tente novamente.";
    }
    arquivos->trava = trava;
    const char *erro = recarregarBancoArquivos(arquivos, banco);
    if (erro)
        fecharBancoArquivos(arquivos);
    return erro;
}

const char *salvarBancoArquivos(ArquivosBanco *arquivos, const BancoPerguntas *banco)
{
    if (arquivos->trava < 0 || !recuperarGravacaoAntiga(arquivos))
        return "Não foi possível recuperar os arquivos antes de salvar.";
    if (arquivos->somenteLeitura)
        return "Há um arquivo inválido ou ilegível. Os originais foram preservados.";

    char destino[CAMINHO_BANCO_MAXIMO], temp[CAMINHO_BANCO_MAXIMO];
    montarCaminhoBanco(destino, arquivos, "perguntas.dat");
    montarCaminhoBanco(temp, arquivos, ".banco-dat.novo");
    if (estadoArquivoBanco(destino) < 0)
        return "Não foi possível acessar o destino do banco.";
    if (!gravarDatTemporario(banco, temp))
        return "Falha ao gravar ou conferir o DAT temporário.";
    if (rename(temp, destino) != 0)
        return "Falha ao substituir o DAT. O banco anterior foi preservado.";
    return NULL;
}

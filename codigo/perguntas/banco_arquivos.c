// Libera as funções POSIX do Linux (fsync, fileno) mesmo compilando com -std=c99.
#define _POSIX_C_SOURCE 200809L
#include "banco_arquivos.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

static const char nomeCsv[] = "perguntas.csv";
static const char nomeDat[] = "perguntas.dat";
static const char nomeTemporario[] = ".banco-dat.novo";

static void caminho(char destino[CAMINHO_BANCO_MAXIMO], const ArquivosBanco *arquivos, const char *nome)
{
    // O comprimento da pasta é limitado na inicialização.
    strcpy(destino, arquivos->pasta);
    strcat(destino, "/");
    strcat(destino, nome);
}

bool inicializarArquivosBanco(ArquivosBanco *arquivos, const char *pasta)
{
    *arquivos = (ArquivosBanco){0};
    if (strlen(pasta) >= sizeof arquivos->pasta - 32)
        return false;
    strcpy(arquivos->pasta, pasta);
    return true;
}

// O sistema libera a trava ao fechar o descritor, inclusive ao encerrar o processo.
static bool tentarBloquear(ArquivosBanco *arquivos)
{
    char local[CAMINHO_BANCO_MAXIMO];
    caminho(local, arquivos, ".banco-lock");
    // O arquivo da trava é separado porque a gravação troca o arquivo DAT.
    int fd = open(local, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        return false;
    // LOCK_NB: se outro programa já tem a trava, falha na hora em vez de esperar.
    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
    {
        close(fd);
        return false;
    }
    arquivos->trava = fd;
    arquivos->bloqueado = true;
    return true;
}

void fecharBancoArquivos(ArquivosBanco *arquivos)
{
    if (!arquivos->bloqueado)
        return;
    close(arquivos->trava);
    arquivos->bloqueado = false;
}

static int estadoArquivo(const char *caminhoArquivo)
{
    struct stat atributos;
    if (stat(caminhoArquivo, &atributos) == 0)
        return S_ISREG(atributos.st_mode) ? 1 : -1;
    return errno == ENOENT ? 0 : -1;
}

// Troca o destino em uma única operação. Os dois caminhos ficam na mesma pasta.
static bool substituirArquivo(const char *origem, const char *destino)
{
    return rename(origem, destino) == 0;
}

static bool removerArquivo(const char *local)
{
    return remove(local) == 0 || errno == ENOENT;
}

// fflush envia a escrita de C ao sistema. A segunda chamada pede a gravação no dispositivo.
static bool sincronizar(FILE *arquivo)
{
    if (fflush(arquivo) != 0)
        return false;
    return fsync(fileno(arquivo)) == 0;
}

static bool sincronizarArquivo(const char *local)
{
    FILE *arquivo = fopen(local, "r+b");
    if (!arquivo)
        return false;
    bool ok = sincronizar(arquivo);
    return fclose(arquivo) == 0 && ok;
}

static bool copiarArquivo(const char *origem, const char *destino)
{
    FILE *entrada = fopen(origem, "rb");
    if (!entrada)
        return false;
    FILE *saida = fopen(destino, "wb");
    bool ok = saida != NULL;
    unsigned char bloco[4096];
    size_t lidos;
    while (ok && (lidos = fread(bloco, 1, sizeof bloco, entrada)) > 0)
        ok = fwrite(bloco, 1, lidos, saida) == lidos;
    ok = ok && !ferror(entrada);
    if (fclose(entrada) != 0)
        ok = false;
    if (saida)
    {
        ok = ok && sincronizar(saida);
        if (fclose(saida) != 0)
            ok = false;
    }
    return ok;
}

// Só a recuperação de versões antigas usa marcador e arquivos de backup.
static void limparTemporariosAntigos(const ArquivosBanco *arquivos)
{
    const char *const temporarios[] = {".banco-csv.novo", nomeTemporario, ".banco-csv.anterior",
        ".banco-dat.anterior", ".banco-pendente.novo", ".banco-restauracao"};
    char local[CAMINHO_BANCO_MAXIMO];
    for (size_t i = 0; i < sizeof temporarios / sizeof temporarios[0]; i++)
    {
        caminho(local, arquivos, temporarios[i]);
        removerArquivo(local);
    }
}

// Em instalações antigas, dois bytes diziam se o CSV e o DAT já existiam.
// Só recuperamos o DAT. O CSV original permanece como fonte inicial.
static bool recuperarGravacaoAntiga(const ArquivosBanco *arquivos)
{
    char pendente[CAMINHO_BANCO_MAXIMO];
    caminho(pendente, arquivos, ".banco-pendente");
    int estado = estadoArquivo(pendente);
    if (estado == 0)
        return true;
    if (estado < 0)
        return false;

    FILE *arquivo = fopen(pendente, "rb");
    if (!arquivo)
        return false;
    int existiaCsv = fgetc(arquivo);
    int existiaDat = fgetc(arquivo);
    bool valido = fgetc(arquivo) == EOF && !ferror(arquivo);
    if (fclose(arquivo) != 0 || !valido ||
        (existiaCsv != '0' && existiaCsv != '1') || (existiaDat != '0' && existiaDat != '1'))
        return false;

    char destino[CAMINHO_BANCO_MAXIMO];
    caminho(destino, arquivos, nomeDat);
    if (existiaDat == '1')
    {
        // Só apaga o backup ao terminar. Uma interrupção aqui permite nova tentativa.
        char backup[CAMINHO_BANCO_MAXIMO], temp[CAMINHO_BANCO_MAXIMO];
        caminho(backup, arquivos, ".banco-dat.anterior");
        caminho(temp, arquivos, ".banco-restauracao");
        if (!copiarArquivo(backup, temp) || !substituirArquivo(temp, destino))
            return false;
    }
    else if (!removerArquivo(destino))
        return false;
    if (!removerArquivo(pendente))
        return false;
    limparTemporariosAntigos(arquivos);
    return true;
}

const char *recarregarBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    if (!arquivos->bloqueado || !recuperarGravacaoAntiga(arquivos))
        return "Não foi possível recuperar os arquivos do banco.";

    char localCsv[CAMINHO_BANCO_MAXIMO], localDat[CAMINHO_BANCO_MAXIMO];
    caminho(localCsv, arquivos, nomeCsv);
    caminho(localDat, arquivos, nomeDat);
    int existeCsv = estadoArquivo(localCsv);
    int existeDat = estadoArquivo(localDat);
    // Cada carregador só troca o banco em memória quando termina a leitura com sucesso.
    if (existeDat == 1 && carregaBinario(banco, localDat))
    {
        arquivos->somenteLeitura = false;
        return NULL;
    }
    // Se o DAT existe mas falhou, o CSV é uma alternativa só para consulta.
    arquivos->somenteLeitura = existeDat != 0;
    if (existeCsv == 1 && carregaPerguntas(banco, localCsv))
    {
        // Quando ainda não há DAT, tenta criá-lo. Em caso de falha, usa o que está em memória.
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
    if (!tentarBloquear(arquivos))
        return "Banco em uso ou sem acesso. Feche a outra sessão e tente novamente.";
    const char *erro = recarregarBancoArquivos(arquivos, banco);
    if (erro)
        fecharBancoArquivos(arquivos);
    return erro;
}

// Reabre o temporário pelo leitor do jogo e verifica os campos de cada pergunta.
static bool conferirCopia(const BancoPerguntas *banco, const char *local)
{
    BancoPerguntas copia = {0};
    bool ok = carregaBinario(&copia, local);
    ok = ok && copia.quantidade == banco->quantidade;
    for (int i = 0; ok && i < banco->quantidade; i++)
        ok = perguntasIguais(&banco->perguntas[i], &copia.perguntas[i]);
    liberaBancoPergunta(&copia);
    return ok;
}

const char *salvarBancoArquivos(ArquivosBanco *arquivos, const BancoPerguntas *banco)
{
    if (!arquivos->bloqueado || !recuperarGravacaoAntiga(arquivos))
        return "Não foi possível recuperar os arquivos antes de salvar.";
    if (arquivos->somenteLeitura)
        return "Há um arquivo inválido ou ilegível. Os originais foram preservados.";

    char destino[CAMINHO_BANCO_MAXIMO], temp[CAMINHO_BANCO_MAXIMO];
    caminho(destino, arquivos, nomeDat);
    caminho(temp, arquivos, nomeTemporario);
    if (estadoArquivo(destino) < 0)
        return "Não foi possível acessar o destino do banco.";

    // O DAT em uso só muda depois de escrever, sincronizar e conferir o novo arquivo.
    if (!salvaBinario(banco, temp) || !sincronizarArquivo(temp) || !conferirCopia(banco, temp))
        return "Falha ao gravar ou conferir o DAT temporário.";
    if (!substituirArquivo(temp, destino))
        return "Falha ao substituir o DAT. O banco anterior foi preservado.";
    return NULL;
}

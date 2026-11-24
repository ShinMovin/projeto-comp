#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "banco_arquivos.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

static const char *const nomes[] = {"perguntas.csv", "perguntas.dat"};
static const char *const novos[] = {".banco-csv.novo", ".banco-dat.novo"};
static const char *const anteriores[] = {".banco-csv.anterior", ".banco-dat.anterior"};
static const char marca[] = ".banco-pendente";
static const char marcaNova[] = ".banco-pendente.novo";
static const char restauracao[] = ".banco-restauracao";

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

void configurarTerminalUtf8(void)
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

static bool tentarBloquear(ArquivosBanco *arquivos)
{
    char local[CAMINHO_BANCO_MAXIMO];
    caminho(local, arquivos, ".banco-lock");
    // A trava fica em um arquivo separado, pois CSV e DAT são substituídos.
#ifdef _WIN32
    HANDLE handle = CreateFileA(local, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return false;
    OVERLAPPED intervalo = {0};
    if (!LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &intervalo))
    {
        CloseHandle(handle);
        return false;
    }
    arquivos->trava = (intptr_t)handle;
#else
    int fd = open(local, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        return false;
    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
    {
        close(fd);
        return false;
    }
    arquivos->trava = fd;
#endif
    arquivos->bloqueado = true;
    return true;
}

void fecharBancoArquivos(ArquivosBanco *arquivos)
{
    if (!arquivos->bloqueado)
        return;
#ifdef _WIN32
    OVERLAPPED intervalo = {0};
    HANDLE handle = (HANDLE)arquivos->trava;
    UnlockFileEx(handle, 0, 1, 0, &intervalo);
    CloseHandle(handle);
#else
    close((int)arquivos->trava);
#endif
    arquivos->bloqueado = false;
}

static int estadoArquivo(const char *caminhoArquivo)
{
    struct stat atributos;
    if (stat(caminhoArquivo, &atributos) == 0)
        return S_ISREG(atributos.st_mode) ? 1 : -1;
    return errno == ENOENT ? 0 : -1;
}

static bool substituirArquivo(const char *origem, const char *destino)
{
#ifdef _WIN32
    return MoveFileExA(origem, destino, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(origem, destino) == 0;
#endif
}

static bool removerArquivo(const char *local)
{
    return remove(local) == 0 || errno == ENOENT;
}

static bool sincronizar(FILE *arquivo)
{
    if (fflush(arquivo) != 0)
        return false;
#ifdef _WIN32
    return _commit(_fileno(arquivo)) == 0;
#else
    return fsync(fileno(arquivo)) == 0;
#endif
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

static void limparTemporarios(const ArquivosBanco *arquivos)
{
    const char *const temporarios[] = {novos[0], novos[1], anteriores[0], anteriores[1], marcaNova, restauracao};
    char local[CAMINHO_BANCO_MAXIMO];
    for (size_t i = 0; i < sizeof temporarios / sizeof temporarios[0]; i++)
    {
        caminho(local, arquivos, temporarios[i]);
        removerArquivo(local);
    }
}

static bool recuperarGravacao(const ArquivosBanco *arquivos)
{
    char pendente[CAMINHO_BANCO_MAXIMO];
    caminho(pendente, arquivos, marca);
    int estado = estadoArquivo(pendente);
    if (estado == 0)
        return true;
    if (estado < 0)
        return false;

    FILE *arquivo = fopen(pendente, "rb");
    if (!arquivo)
        return false;
    int existiam[2] = {fgetc(arquivo), fgetc(arquivo)};
    bool valido = fgetc(arquivo) == EOF && !ferror(arquivo);
    if (fclose(arquivo) != 0 || !valido ||
        (existiam[0] != '0' && existiam[0] != '1') || (existiam[1] != '0' && existiam[1] != '1'))
        return false;

    // Mantém os backups até restaurar os dois destinos. Assim, a recuperação pode ser repetida.
    for (int i = 0; i < 2; i++)
    {
        char destino[CAMINHO_BANCO_MAXIMO], backup[CAMINHO_BANCO_MAXIMO], temp[CAMINHO_BANCO_MAXIMO];
        caminho(destino, arquivos, nomes[i]);
        caminho(backup, arquivos, anteriores[i]);
        caminho(temp, arquivos, restauracao);
        if (existiam[i] == '1')
        {
            if (!copiarArquivo(backup, temp) || !substituirArquivo(temp, destino))
                return false;
        }
        else if (!removerArquivo(destino))
            return false;
    }
    if (!removerArquivo(pendente))
        return false;
    limparTemporarios(arquivos);
    return true;
}

static bool reunirBancos(ArquivosBanco *arquivos, BancoPerguntas *dat, const BancoPerguntas *csv)
{
    int quantidadeDat = dat->quantidade;
    bool *comparadas = calloc((size_t)quantidadeDat + 1, sizeof *comparadas);
    if (!comparadas)
        return false;

    // Compara todos os campos e preserva também a quantidade de ocorrências de cada registro.
    arquivos->somenteDat = quantidadeDat;
    for (int i = 0; i < csv->quantidade; i++)
    {
        int j = 0;
        while (j < quantidadeDat && (comparadas[j] || !perguntasIguais(&dat->perguntas[j], &csv->perguntas[i])))
            j++;
        if (j < quantidadeDat)
        {
            comparadas[j] = true;
            arquivos->somenteDat--;
        }
        else
        {
            if (!adicionaPerguntas(dat, &csv->perguntas[i]))
            {
                free(comparadas);
                return false;
            }
            arquivos->somenteCsv++;
        }
    }
    free(comparadas);
    return true;
}

const char *recarregarBancoArquivos(ArquivosBanco *arquivos, BancoPerguntas *banco)
{
    if (!arquivos->bloqueado || !recuperarGravacao(arquivos))
        return "Não foi possível recuperar os arquivos do banco.";

    char localCsv[CAMINHO_BANCO_MAXIMO], localDat[CAMINHO_BANCO_MAXIMO];
    caminho(localCsv, arquivos, nomes[0]);
    caminho(localDat, arquivos, nomes[1]);
    int existeCsv = estadoArquivo(localCsv);
    int existeDat = estadoArquivo(localDat);
    BancoPerguntas csv = {0}, dat = {0};
    bool leuCsv = existeCsv == 1 && carregaPerguntas(&csv, localCsv);
    bool leuDat = existeDat == 1 && carregaBinario(&dat, localDat);

    arquivos->somenteDat = 0;
    arquivos->somenteCsv = 0;
    arquivos->somenteLeitura = (existeCsv != 0 && !leuCsv) || (existeDat != 0 && !leuDat);
    const char *erro = NULL;
    if (leuDat && leuCsv && !reunirBancos(arquivos, &dat, &csv))
        erro = "Memória insuficiente para comparar CSV e DAT.";
    else if (!leuDat && !leuCsv && (existeDat != 0 || existeCsv != 0))
        erro = "O CSV e o DAT estão indisponíveis ou inválidos.";
    else if (!leuDat && !leuCsv && !inicializaBanco(&dat))
        erro = "Memória insuficiente para abrir o banco.";

    if (!erro)
    {
        BancoPerguntas *origem = leuDat || !leuCsv ? &dat : &csv;
        liberaBancoPergunta(banco);
        *banco = *origem;
        *origem = (BancoPerguntas){0};
    }
    liberaBancoPergunta(&csv);
    liberaBancoPergunta(&dat);
    return erro;
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

static bool conferirCopia(const BancoPerguntas *banco, const char *local, int formato)
{
    BancoPerguntas copia = {0};
    bool ok = formato == 0 ? carregaPerguntas(&copia, local) : carregaBinario(&copia, local);
    ok = ok && copia.quantidade == banco->quantidade;
    for (int i = 0; ok && i < banco->quantidade; i++)
        ok = perguntasIguais(&banco->perguntas[i], &copia.perguntas[i]);
    liberaBancoPergunta(&copia);
    return ok;
}

const char *salvarBancoArquivos(ArquivosBanco *arquivos, const BancoPerguntas *banco)
{
    if (!arquivos->bloqueado || !recuperarGravacao(arquivos))
        return "Não foi possível recuperar os arquivos antes de salvar.";
    if (arquivos->somenteLeitura)
        return "Há um arquivo inválido ou ilegível. Os originais foram preservados.";

    char originais[2][CAMINHO_BANCO_MAXIMO], temp[2][CAMINHO_BANCO_MAXIMO];
    char pendente[CAMINHO_BANCO_MAXIMO], pendenteNovo[CAMINHO_BANCO_MAXIMO];
    char existiam[2];
    caminho(pendente, arquivos, marca);
    caminho(pendenteNovo, arquivos, marcaNova);
    for (int i = 0; i < 2; i++)
    {
        char backup[CAMINHO_BANCO_MAXIMO];
        caminho(originais[i], arquivos, nomes[i]);
        caminho(temp[i], arquivos, novos[i]);
        caminho(backup, arquivos, anteriores[i]);
        int estado = estadoArquivo(originais[i]);
        existiam[i] = estado == 1 ? '1' : '0';

        if (estado < 0 || (estado == 1 && !copiarArquivo(originais[i], backup)))
            return "Não foi possível preparar a cópia de segurança do banco.";
        bool gravou = i == 0 ? salvaPergunta(banco, temp[i]) : salvaBinario(banco, temp[i]);
        if (!gravou || !sincronizarArquivo(temp[i]) || !conferirCopia(banco, temp[i], i))
            return "Falha ao gravar ou conferir os arquivos temporários.";
    }

    // O marcador nasce antes da primeira troca e só desaparece ao confirmar o par.
    FILE *marcador = fopen(pendenteNovo, "wb");
    if (!marcador)
        return "Não foi possível preparar a confirmação do salvamento.";
    bool registrou = fwrite(existiam, 1, sizeof existiam, marcador) == sizeof existiam && sincronizar(marcador);
    if (fclose(marcador) != 0 || !registrou || !substituirArquivo(pendenteNovo, pendente))
        return "Não foi possível preparar a confirmação do salvamento.";

    if (!substituirArquivo(temp[0], originais[0]) || !substituirArquivo(temp[1], originais[1]) ||
        !removerArquivo(pendente))
    {
        if (!recuperarGravacao(arquivos))
            return "Falha no salvamento. A recuperação será tentada na próxima abertura.";
        return "Falha no salvamento. A versão anterior foi restaurada.";
    }
    limparTemporarios(arquivos);
    arquivos->somenteCsv = 0;
    arquivos->somenteDat = 0;
    return NULL;
}

/**
 * @file perguntas.c
 * @author VGA
 * @brief Banco de perguntas: CSV, binario, sorteio e CRUD.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "perguntas.h"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CAPACIDADE_INICIAL = 5 };

static bool redimensionarBanco(BancoPerguntas *banco, int capacidade)
{
    if (capacidade < banco->quantidade || (size_t)capacidade > SIZE_MAX / sizeof(Pergunta))
        return false;

    // Troca os dois vetores juntos. Em caso de falha, os originais continuam válidos.
    Pergunta *perguntas = malloc((size_t)capacidade * sizeof *perguntas);
    bool *usadas = calloc((size_t)capacidade, sizeof *usadas);
    if (!perguntas || !usadas)
    {
        free(perguntas);
        free(usadas);
        return false;
    }
    if (banco->quantidade > 0)
    {
        memcpy(perguntas, banco->perguntas, (size_t)banco->quantidade * sizeof *perguntas);
        memcpy(usadas, banco->usadas, (size_t)banco->quantidade * sizeof *usadas);
    }
    free(banco->perguntas);
    free(banco->usadas);
    banco->perguntas = perguntas;
    banco->usadas = usadas;
    banco->capacidade = capacidade;
    return true;
}

int inicializaBanco(BancoPerguntas *banco)
{
    // O banco começa vazio, com espaço para as primeiras cinco perguntas.
    banco->quantidade = 0;
    banco->capacidade = CAPACIDADE_INICIAL;
    banco->perguntas = calloc(CAPACIDADE_INICIAL, sizeof *banco->perguntas);
    banco->usadas = calloc(CAPACIDADE_INICIAL, sizeof *banco->usadas);
    if (banco->perguntas == NULL || banco->usadas == NULL)
    {
        liberaBancoPergunta(banco);
        return 0;
    }
    return 1;
}

void liberaBancoPergunta(BancoPerguntas *banco)
{
    free(banco->perguntas);
    free(banco->usadas);
    banco->perguntas = NULL;
    banco->usadas = NULL;
    banco->quantidade = 0;
    banco->capacidade = 0;
}

static bool textoValido(const char *texto)
{
    const unsigned char *p = (const unsigned char *)texto;
    const unsigned char *fim = memchr(p, '\0', LIMITE_TEXTO_PERGUNTA + 1);
    if (!fim)
        return false;

    bool temConteudo = false;
    while (p < fim)
    {
        unsigned char byte = *p++;
        if (byte < 0x80)
        {
            if (byte < 32 || byte == 127 || byte == '"')
                return false;
            temConteudo |= byte != ' ';
            continue;
        }

        // Valida as sequências UTF-8 sem ler além do campo.
        int restantes = byte >= 0xF0 ? 3 : byte >= 0xE0 ? 2 : 1;
        if (byte < 0xC2 || byte > 0xF4 || fim - p < restantes)
            return false;
        unsigned int ponto = byte & (0x7Fu >> (restantes + 1));
        const unsigned int minimo[] = {0, 0x80, 0x800, 0x10000};
        int extras = restantes;
        while (extras-- > 0)
        {
            unsigned char parte = *p++;
            if ((parte & 0xC0) != 0x80)
                return false;
            ponto = (ponto << 6) | (parte & 0x3F);
        }
        if (ponto < minimo[restantes] || ponto > 0x10FFFF || (ponto >= 0xD800 && ponto <= 0xDFFF))
            return false;
        temConteudo = true;
    }
    return temConteudo;
}

const char *validarPergunta(const Pergunta *pergunta)
{
    if (!textoValido(pergunta->conteudo_da_pergunta))
        return "Enunciado inválido. Use até 200 bytes, sem aspas ou quebras.";
    for (int i = 0; i < 4; i++)
    {
        if (!textoValido(pergunta->alternativas[i]))
            return "Preencha as quatro alternativas com até 200 bytes, sem aspas ou quebras.";
    }
    if (pergunta->alternativa_correta < 'A' || pergunta->alternativa_correta > 'D')
        return "Escolha o gabarito de A a D.";
    if (pergunta->categorias < GEEK || pergunta->categorias > BIOLOGIA)
        return "Escolha uma categoria de 1 a 5.";
    if (pergunta->dificuldades < MUITO_FACIL || pergunta->dificuldades > IMPOSSIVEL)
        return "Escolha uma dificuldade de 1 a 5.";
    return NULL;
}

bool perguntasIguais(const Pergunta *a, const Pergunta *b)
{
    if (a->alternativa_correta != b->alternativa_correta || a->dificuldades != b->dificuldades ||
        a->categorias != b->categorias || strcmp(a->conteudo_da_pergunta, b->conteudo_da_pergunta) != 0)
        return false;
    for (int i = 0; i < 4; i++)
    {
        if (strcmp(a->alternativas[i], b->alternativas[i]) != 0)
            return false;
    }
    return true;
}

bool perguntaContem(const Pergunta *pergunta, const char *trecho)
{
    return strstr(pergunta->conteudo_da_pergunta, trecho) != NULL;
}

static bool enunciadoJaContado(const BancoPerguntas *banco, int indice, bool apenasUsadas)
{
    const Pergunta *pergunta = &banco->perguntas[indice];
    int limite = apenasUsadas ? banco->quantidade : indice;
    for (int i = 0; i < limite; i++)
    {
        if ((apenasUsadas ? banco->usadas[i] : banco->perguntas[i].dificuldades == pergunta->dificuldades) &&
            strcmp(banco->perguntas[i].conteudo_da_pergunta, pergunta->conteudo_da_pergunta) == 0)
            return true;
    }
    return false;
}

int contaPerguntasDistintas(const BancoPerguntas *banco, Dificuldade dificuldade)
{
    int total = 0;
    for (int i = 0; i < banco->quantidade; i++)
    {
        if (banco->perguntas[i].dificuldades == dificuldade && !enunciadoJaContado(banco, i, false))
            total++;
    }
    return total;
}

int adicionaPerguntas(BancoPerguntas *banco, const Pergunta *pergunta)
{
    if (validarPergunta(pergunta))
        return 0;
    Pergunta copia = *pergunta;
    if (banco->quantidade == banco->capacidade)
    {
        if (banco->capacidade > INT_MAX / 2)
            return 0;
        int novaCapacidade = banco->capacidade > 0 ? banco->capacidade * 2 : CAPACIDADE_INICIAL;
        if (!redimensionarBanco(banco, novaCapacidade))
            return 0;
    }

    // A pergunta e a marca de uso ocupam a mesma posição nos dois vetores.
    banco->perguntas[banco->quantidade] = copia;
    banco->usadas[banco->quantidade] = false;
    banco->quantidade++;
    return 1;
}

static bool indiceValido(const BancoPerguntas *banco, int indice)
{
    return indice >= 0 && indice < banco->quantidade;
}

bool editarPerguntaPorIndice(BancoPerguntas *banco, int indice, const Pergunta *pergunta)
{
    if (!indiceValido(banco, indice) || validarPergunta(pergunta))
        return false;
    banco->perguntas[indice] = *pergunta;
    banco->usadas[indice] = false;
    return true;
}

bool excluirPerguntaPorIndice(BancoPerguntas *banco, int indice)
{
    if (!indiceValido(banco, indice))
        return false;

    for (int i = indice; i < banco->quantidade - 1; i++)
    {
        banco->perguntas[i] = banco->perguntas[i + 1];
        banco->usadas[i] = banco->usadas[i + 1];
    }
    banco->quantidade--;
    banco->usadas[banco->quantidade] = false;
    if (banco->capacidade > CAPACIDADE_INICIAL && banco->quantidade <= banco->capacidade / 4)
        redimensionarBanco(banco, banco->capacidade / 2);
    return true;
}

static bool perguntaDisponivel(const BancoPerguntas *banco, int indice,
                               Dificuldade dificuldade, Categoria categoria)
{
    return !banco->usadas[indice] && banco->perguntas[indice].dificuldades == dificuldade &&
           banco->perguntas[indice].categorias == categoria && !enunciadoJaContado(banco, indice, true);
}

int contaPerguntasDisponiveis(const BancoPerguntas *banco, Dificuldade dificuldade, Categoria categoria)
{
    int disponiveis = 0;
    for (int i = 0; i < banco->quantidade; i++)
    {
        if (perguntaDisponivel(banco, i, dificuldade, categoria))
            disponiveis++;
    }
    return disponiveis;
}

int sorteaPergunta(BancoPerguntas *banco, Dificuldade dificuldade, Categoria categoria)
{
    int disponiveis = contaPerguntasDisponiveis(banco, dificuldade, categoria);
    if (disponiveis == 0)
        return -1;

    // Sorteia entre as perguntas disponíveis e conta até alcançar a posição.
    int sorteada = rand() % disponiveis;
    for (int i = 0; i < banco->quantidade; i++)
    {
        if (!perguntaDisponivel(banco, i, dificuldade, categoria))
            continue;
        if (sorteada == 0)
        {
            banco->usadas[i] = true;
            return i;
        }
        sorteada--;
    }
    return -1;
}

static const char cabecalhoCsv[] =
    "enunciado,alternativa_A,alternativa_B,alternativa_C,alternativa_D,alternativa_correta,dificuldade,categoria";

// Lê um campo a partir da posição atual e deixa o cursor na vírgula seguinte.
static bool lerString(const char *texto, int *posicao, char *destino)
{
    int i = *posicao;
    int j = 0;
    if (texto[i] == '"')
    {
        i++;
        while (texto[i] != '"')
        {
            if (texto[i] == '\0' || texto[i] == '\r' || texto[i] == '\n' || j == LIMITE_TEXTO_PERGUNTA)
                return false;
            destino[j++] = texto[i++];
        }
        i++;
    }
    else
    {
        while (j < LIMITE_TEXTO_PERGUNTA && texto[i] != '\0' &&
               texto[i] != '\r' && texto[i] != '\n' && texto[i] != ',')
            destino[j++] = texto[i++];
    }

    if (j == 0 || texto[i] != ',')
        return false;
    destino[j] = '\0';
    *posicao = i;
    return true;
}

static bool lerRegistroCsv(const char *linha, Pergunta *pergunta)
{
    int posicao = 0;
    if (!lerString(linha, &posicao, pergunta->conteudo_da_pergunta))
        return false;
    for (int i = 0; i < 4; i++)
    {
        posicao++; // Pula a vírgula que separa a alternativa do campo anterior.
        if (!lerString(linha, &posicao, pergunta->alternativas[i]))
            return false;
    }

    posicao++;
    char correta = (char)toupper((unsigned char)linha[posicao]);
    if (correta < 'A' || correta > 'D' || linha[posicao + 1] != ',')
        return false;

    posicao += 2;
    char dificuldade = linha[posicao];
    if (dificuldade < '1' || dificuldade > '5' || linha[posicao + 1] != ',')
        return false;

    posicao += 2;
    char categoria = linha[posicao];
    if (categoria < '1' || categoria > '5')
        return false;

    posicao++;
    if (linha[posicao] == '\r')
        posicao++;
    if (linha[posicao] == '\n')
        posicao++;
    if (linha[posicao] != '\0')
        return false;

    pergunta->alternativa_correta = correta;
    pergunta->dificuldades = (Dificuldade)(dificuldade - '0');
    pergunta->categorias = (Categoria)(categoria - '0');
    return true;
}

static int concluirLeitura(BancoPerguntas *banco, BancoPerguntas *novo, bool ok)
{
    if (!ok)
    {
        liberaBancoPergunta(novo);
        return 0;
    }
    liberaBancoPergunta(banco);
    *banco = *novo;
    return 1;
}

static int lerLinhaCsv(FILE *arq, char *linha, size_t capacidade)
{
    size_t tamanho = 0;
    int c;
    while ((c = fgetc(arq)) != EOF)
    {
        if (c == '\0' || tamanho + 1 >= capacidade)
            return -1;
        linha[tamanho++] = (char)c;
        if (c == '\n')
            break;
    }
    linha[tamanho] = '\0';
    return ferror(arq) ? -1 : tamanho > 0;
}

int carregaPerguntas(BancoPerguntas *banco, const char *nome_arq)
{
    // Abre em modo binário para detectar bytes inválidos também no Windows.
    FILE *arq = fopen(nome_arq, "rb");
    if (!arq)
        return 0;
    BancoPerguntas novo;
    bool ok = inicializaBanco(&novo);
    char linha[1200] = {0};
    if (ok)
    {
        ok = lerLinhaCsv(arq, linha, sizeof linha) > 0;
        if (ok)
        {
            const char *inicio = linha;
            if ((unsigned char)linha[0] == 0xEF && (unsigned char)linha[1] == 0xBB &&
                (unsigned char)linha[2] == 0xBF)
                inicio += 3;
            linha[strcspn(linha, "\r\n")] = '\0';
            ok = strcmp(inicio, cabecalhoCsv) == 0;
        }
    }

    int lida = 0;
    while (ok && (lida = lerLinhaCsv(arq, linha, sizeof linha)) > 0)
    {
        Pergunta pergunta = {0};
        if (!lerRegistroCsv(linha, &pergunta) || !adicionaPerguntas(&novo, &pergunta))
        {
            ok = false;
            break;
        }
    }
    ok = ok && lida >= 0;
    if (fclose(arq) != 0)
        ok = false;
    return concluirLeitura(banco, &novo, ok);
}

int carregaBinario(BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq = fopen(nome_arq, "rb");
    if (!arq)
        return 0;
    BancoPerguntas novo;
    bool ok = inicializaBanco(&novo);
    int quantidade = -1;
    if (ok)
        ok = fread(&quantidade, sizeof quantidade, 1, arq) == 1 && quantidade >= 0 &&
             fseek(arq, 0, SEEK_END) == 0;
    if (ok)
    {
        long tamanho = ftell(arq);
        long dados = tamanho - (long)sizeof quantidade;
        // O arquivo legado tem um int e um vetor com o layout de Pergunta.
        ok = tamanho >= (long)sizeof quantidade && dados % (long)sizeof(Pergunta) == 0 &&
             dados / (long)sizeof(Pergunta) == quantidade &&
             fseek(arq, (long)sizeof quantidade, SEEK_SET) == 0;
    }
    if (ok && quantidade > novo.capacidade)
    {
        int novaCapacidade = novo.capacidade;
        // Cresce em blocos como no carregamento original, protegendo a multiplicação.
        while (novaCapacidade < quantidade && novaCapacidade <= INT_MAX / 2)
            novaCapacidade *= 2;
        if (novaCapacidade < quantidade)
            novaCapacidade = quantidade;
        ok = redimensionarBanco(&novo, novaCapacidade);
    }
    if (ok)
    {
        ok = fread(novo.perguntas, sizeof(Pergunta), (size_t)quantidade, arq) == (size_t)quantidade;
        for (int i = 0; ok && i < quantidade; i++)
            ok = !validarPergunta(&novo.perguntas[i]);
        novo.quantidade = quantidade;
    }
    if (fclose(arq) != 0)
        ok = false;
    return concluirLeitura(banco, &novo, ok);
}

int salvaPergunta(const BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq = fopen(nome_arq, "w");
    if (!arq)
        return 0;
    bool ok = fprintf(arq, "%s\n", cabecalhoCsv) >= 0;
    for (int i = 0; ok && i < banco->quantidade; i++)
    {
        const Pergunta *p = &banco->perguntas[i];
        ok = !validarPergunta(p) && fprintf(arq, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%c,%d,%d\n",
                p->conteudo_da_pergunta, p->alternativas[0], p->alternativas[1], p->alternativas[2],
                p->alternativas[3], p->alternativa_correta, p->dificuldades, p->categorias) >= 0;
    }
    return fclose(arq) == 0 && ok;
}

int salvaBinario(const BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq = fopen(nome_arq, "wb");
    if (!arq)
        return 0;
    size_t quantidade = (size_t)banco->quantidade;
    bool ok = fwrite(&banco->quantidade, sizeof banco->quantidade, 1, arq) == 1;
    if (ok && quantidade > 0)
        ok = fwrite(banco->perguntas, sizeof *banco->perguntas, quantidade, arq) == quantidade;
    return fclose(arq) == 0 && ok;
}

// A partir daqui ficam as operações que pedem dados ou imprimem no terminal.
static int lerLinhaTerminal(char *texto, size_t capacidade)
{
    if (!fgets(texto, (int)capacidade, stdin))
        return 0;
    size_t fim = strcspn(texto, "\r\n");
    if (texto[fim] == '\0' && !feof(stdin))
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        return -1;
    }
    texto[fim] = '\0';
    return 1;
}

static bool lerTextoTerminal(const char *rotulo, char *destino)
{
    char texto[LIMITE_TEXTO_PERGUNTA + 3];
    for (;;)
    {
        printf("%s: ", rotulo);
        int leitura = lerLinhaTerminal(texto, sizeof texto);
        if (leitura == 0)
            return false;
        if (leitura < 0 || strlen(texto) > LIMITE_TEXTO_PERGUNTA)
            puts("Texto muito grande. Use até 200 bytes.");
        else if (texto[0] == '\0')
            puts("O texto não pode ser vazio.");
        else if (strchr(texto, '"'))
            puts("Não use aspas no texto.");
        else if (textoValido(texto))
        {
            strcpy(destino, texto);
            return true;
        }
        else
            puts("Digite um texto válido em UTF-8.");
    }
}

static int lerNumeroTerminal(const char *rotulo, int minimo, int maximo, bool repetir)
{
    char linha[48];
    do
    {
        printf("%s: ", rotulo);
        int leitura = lerLinhaTerminal(linha, sizeof linha);
        if (leitura == 0)
            return -1;
        if (leitura > 0)
        {
            char *fim;
            long valor = strtol(linha, &fim, 10);
            while (isspace((unsigned char)*fim))
                fim++;
            if (fim != linha && *fim == '\0' && valor >= minimo && valor <= maximo)
                return (int)valor;
        }
        puts("Número inválido.");
    } while (repetir);
    return -1;
}

const char *nomeCategoria(Categoria categoria)
{
    static const char *const nomes[] = {"Geek", "Esportes", "Matemática", "Geografia", "Biologia"};
    return categoria >= GEEK && categoria <= BIOLOGIA ? nomes[categoria - GEEK] : "";
}

const char *nomeDificuldade(Dificuldade dificuldade)
{
    static const char *const nomes[] = {"Muito fácil", "Fácil", "Médio", "Difícil", "Impossível"};
    return dificuldade >= MUITO_FACIL && dificuldade <= IMPOSSIVEL ? nomes[dificuldade - MUITO_FACIL] : "";
}

static int lerCamposPergunta(Pergunta *pergunta, bool editando)
{
    Pergunta nova = {0};
    const char *enunciado = editando ? "Digite o novo enunciado da questão" : "Digite o enunciado da questão";
    if (!lerTextoTerminal(enunciado, nova.conteudo_da_pergunta))
        return 0;
    for (int i = 0; i < 4; i++)
    {
        char rotulo[32];
        snprintf(rotulo, sizeof rotulo, "Digite a %salternativa %c", editando ? "nova " : "", 'A' + i);
        if (!lerTextoTerminal(rotulo, nova.alternativas[i]))
            return 0;
    }

    char linha[16];
    for (;;)
    {
        printf("Digite a alternativa correta (A-D): ");
        int leitura = lerLinhaTerminal(linha, sizeof linha);
        if (leitura == 0)
            return 0;
        if (leitura > 0 && strlen(linha) == 1)
        {
            nova.alternativa_correta = (char)toupper((unsigned char)linha[0]);
            if (nova.alternativa_correta >= 'A' && nova.alternativa_correta <= 'D')
                break;
        }
        puts("Alternativa inválida.");
    }

    int dificuldade = lerNumeroTerminal(
        "Digite a dificuldade (1 - Muito fácil, 2 - Fácil, 3 - Médio, 4 - Difícil, 5 - Impossível)", 1, 5, true);
    if (dificuldade < 0)
        return 0;
    int categoria = lerNumeroTerminal(
        "Digite a categoria (1 - Geek, 2 - Esportes, 3 - Matemática, 4 - Geografia, 5 - Biologia)", 1, 5, true);
    if (categoria < 0)
        return 0;
    nova.dificuldades = (Dificuldade)dificuldade;
    nova.categorias = (Categoria)categoria;
    *pergunta = nova;
    return 1;
}

int lerPergunta(Pergunta *pergunta)
{
    return lerCamposPergunta(pergunta, false);
}

static void imprimirPergunta(const BancoPerguntas *banco, int indice)
{
    const Pergunta *p = &banco->perguntas[indice];
    printf("====== Pergunta %d ======\n%s\n", indice + 1, p->conteudo_da_pergunta);
    for (int i = 0; i < 4; i++)
        printf(" %c) %s\n", 'A' + i, p->alternativas[i]);
    printf("Alternativa correta : %c | Dificuldade : %s | Categoria : %s\n\n", p->alternativa_correta,
           nomeDificuldade(p->dificuldades), nomeCategoria(p->categorias));
}

int listaPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("Nenhuma pergunta cadastrada!");
        return 0;
    }
    for (int i = 0; i < banco->quantidade; i++)
        imprimirPergunta(banco, i);
    return 1;
}

int pesquisaPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("Ainda não existem perguntas no programa!");
        return 0;
    }

    char texto[LIMITE_TEXTO_PERGUNTA + 3];
    printf("Digite o enunciado ou um trecho da pergunta que deseja encontrar: ");
    if (lerLinhaTerminal(texto, sizeof texto) <= 0 || !texto[0])
    {
        puts("Busca inválida!");
        return 0;
    }
    int encontradas = 0;
    for (int i = 0; i < banco->quantidade; i++)
    {
        if (perguntaContem(&banco->perguntas[i], texto))
        {
            imprimirPergunta(banco, i);
            encontradas++;
        }
    }
    if (encontradas == 0)
        puts("Não foram encontradas perguntas relacionadas à sua busca!");
    return encontradas;
}

int excluiPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("O programa não possui perguntas!");
        return 0;
    }
    int numero = lerNumeroTerminal("Digite o índice da pergunta a ser removida (0 cancela)", 0, banco->quantidade, false);
    return numero > 0 && excluirPerguntaPorIndice(banco, numero - 1);
}

int alteraPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("O programa não possui perguntas para serem alteradas!");
        return 0;
    }
    int numero = lerNumeroTerminal("Digite o número da pergunta (0 cancela)", 0, banco->quantidade, false);
    Pergunta nova;
    return numero > 0 && lerCamposPergunta(&nova, true) && editarPerguntaPorIndice(banco, numero - 1, &nova);
}

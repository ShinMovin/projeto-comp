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
    *banco = (BancoPerguntas){0};
    return redimensionarBanco(banco, CAPACIDADE_INICIAL);
}

void liberaBancoPergunta(BancoPerguntas *banco)
{
    free(banco->perguntas);
    free(banco->usadas);
    *banco = (BancoPerguntas){0};
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
    if (banco->quantidade == banco->capacidade &&
        (banco->capacidade > INT_MAX / 2 || !redimensionarBanco(banco, banco->capacidade * 2)))
        return 0;

    banco->perguntas[banco->quantidade] = copia;
    banco->usadas[banco->quantidade++] = false;
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

    size_t restantes = (size_t)(banco->quantidade - indice - 1);
    memmove(banco->perguntas + indice, banco->perguntas + indice + 1, restantes * sizeof *banco->perguntas);
    memmove(banco->usadas + indice, banco->usadas + indice + 1, restantes * sizeof *banco->usadas);
    banco->usadas[--banco->quantidade] = false;
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

    int sorteada = rand() % disponiveis;
    for (int i = 0; i < banco->quantidade; i++)
    {
        if (perguntaDisponivel(banco, i, dificuldade, categoria) && sorteada-- == 0)
        {
            banco->usadas[i] = true;
            return i;
        }
    }
    return -1;
}

static const char cabecalhoCsv[] =
    "enunciado,alternativa_A,alternativa_B,alternativa_C,alternativa_D,alternativa_correta,dificuldade,categoria";

// Campos com vírgulas são delimitados por aspas, como no formato original.
static bool lerCampoCsv(const char **posicao, char *destino)
{
    const char *inicio = *posicao;
    bool entreAspas = *inicio == '"';
    inicio += entreAspas;
    const char *fim = inicio + strcspn(inicio, entreAspas ? "\"\r\n" : ",\r\n");
    size_t comprimento = (size_t)(fim - inicio);
    if (comprimento == 0 || comprimento > LIMITE_TEXTO_PERGUNTA)
        return false;
    if (entreAspas && *fim++ != '"')
        return false;
    if (*fim != ',')
        return false;
    memcpy(destino, inicio, comprimento);
    destino[comprimento] = '\0';
    *posicao = fim + 1;
    return true;
}

static bool lerRegistroCsv(const char *linha, Pergunta *pergunta)
{
    if (!lerCampoCsv(&linha, pergunta->conteudo_da_pergunta))
        return false;
    for (int i = 0; i < 4; i++)
    {
        if (!lerCampoCsv(&linha, pergunta->alternativas[i]))
            return false;
    }
    if (strcspn(linha, "\r\n") != 5 || linha[1] != ',' || linha[3] != ',' ||
        linha[2] < '1' || linha[2] > '5' || linha[4] < '1' || linha[4] > '5')
        return false;
    size_t fim = 5;
    if (linha[fim] == '\r')
        fim++;
    if (linha[fim] == '\n')
        fim++;
    if (linha[fim] != '\0')
        return false;

    pergunta->alternativa_correta = (char)toupper((unsigned char)linha[0]);
    pergunta->dificuldades = (Dificuldade)(linha[2] - '0');
    pergunta->categorias = (Categoria)(linha[4] - '0');
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
        ok = lerRegistroCsv(linha, &pergunta) && adicionaPerguntas(&novo, &pergunta);
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
        ok = redimensionarBanco(&novo, quantidade);
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
    bool ok = fwrite(&banco->quantidade, sizeof(int), 1, arq) == 1 &&
              fwrite(banco->perguntas, sizeof(Pergunta), (size_t)banco->quantidade, arq) ==
              (size_t)banco->quantidade;
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
        if (leitura > 0 && textoValido(texto))
        {
            strcpy(destino, texto);
            return true;
        }
        puts("Use até 200 bytes, com texto válido e sem aspas.");
    }
}

static int lerNumeroTerminal(const char *rotulo, int minimo, int maximo)
{
    char linha[48];
    for (;;)
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
    }
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

int lerPergunta(Pergunta *pergunta)
{
    Pergunta nova = {0};
    if (!lerTextoTerminal("Enunciado", nova.conteudo_da_pergunta))
        return 0;
    for (int i = 0; i < 4; i++)
    {
        char rotulo[] = "Alternativa A";
        rotulo[12] = (char)('A' + i);
        if (!lerTextoTerminal(rotulo, nova.alternativas[i]))
            return 0;
    }

    char linha[16];
    for (;;)
    {
        printf("Alternativa correta (A-D): ");
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

    int dificuldade = lerNumeroTerminal("Dificuldade (1 Muito fácil, 2 Fácil, 3 Médio, 4 Difícil, 5 Impossível)", 1, 5);
    if (dificuldade < 0)
        return 0;
    int categoria = lerNumeroTerminal("Categoria (1 Geek, 2 Esportes, 3 Matemática, 4 Geografia, 5 Biologia)", 1, 5);
    if (categoria < 0)
        return 0;
    nova.dificuldades = (Dificuldade)dificuldade;
    nova.categorias = (Categoria)categoria;
    *pergunta = nova;
    return 1;
}

static void imprimirPergunta(const BancoPerguntas *banco, int indice)
{
    const Pergunta *p = &banco->perguntas[indice];
    printf("====== Pergunta %d ======\n%s\n", indice + 1, p->conteudo_da_pergunta);
    for (int i = 0; i < 4; i++)
        printf(" %c) %s\n", 'A' + i, p->alternativas[i]);
    printf("Alternativa correta: %c | Dificuldade: %s | Categoria: %s\n\n", p->alternativa_correta,
           nomeDificuldade(p->dificuldades), nomeCategoria(p->categorias));
}

int listaPergunta(BancoPerguntas *banco)
{
    for (int i = 0; i < banco->quantidade; i++)
        imprimirPergunta(banco, i);
    if (banco->quantidade == 0)
        puts("Nenhuma pergunta cadastrada.");
    return banco->quantidade != 0;
}

int pesquisaPergunta(BancoPerguntas *banco)
{
    char texto[LIMITE_TEXTO_PERGUNTA + 2];
    printf("Digite o trecho do enunciado: ");
    if (lerLinhaTerminal(texto, sizeof texto) <= 0 || !texto[0])
        return 0;
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
        puts("Nenhuma pergunta encontrada.");
    return encontradas;
}

int excluiPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("Nenhuma pergunta cadastrada.");
        return 0;
    }
    int numero = lerNumeroTerminal("Número da pergunta a excluir (0 cancela)", 0, banco->quantidade);
    return numero > 0 && excluirPerguntaPorIndice(banco, numero - 1);
}

int alteraPergunta(BancoPerguntas *banco)
{
    if (banco->quantidade == 0)
    {
        puts("Nenhuma pergunta cadastrada.");
        return 0;
    }
    int numero = lerNumeroTerminal("Número da pergunta a alterar (0 cancela)", 0, banco->quantidade);
    Pergunta nova;
    return numero > 0 && lerPergunta(&nova) && editarPerguntaPorIndice(banco, numero - 1, &nova);
}

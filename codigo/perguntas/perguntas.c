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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define CAPACIDADE_INICIAL 5
#define MAXIMO_PERGUNTAS 100000 //limite de seguranca para um arquivo binario estragado

//joga fora o resto da linha digitada (inclusive o Enter)
static void limpaEntrada(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

//cria o banco vazio, alocando os vetores com a capacidade inicial
int inicializaBanco(BancoPerguntas *banco){
    (*banco).quantidade = 0;
    (*banco).capacidade = CAPACIDADE_INICIAL;
    (*banco).perguntas = calloc(CAPACIDADE_INICIAL, sizeof(Pergunta));
    (*banco).usadas = calloc(CAPACIDADE_INICIAL, sizeof(bool)); //bool serve para armazenar apenas 0 e 1 (falso/verdadeiro)

    //testando se deu erro de alocação
    if((*banco).perguntas == NULL || (*banco).usadas == NULL){
        //liberando memória
        free((*banco).perguntas);
        free((*banco).usadas);
        //limpando ponteiro
        ((*banco).perguntas) = NULL;
        ((*banco).usadas) = NULL;
        (*banco).capacidade = 0;

        return 0; //se deu erro retorna 0
    }

    return 1; //indica que deu certo
}

//libera a memoria do banco
void liberaBancoPergunta(BancoPerguntas *banco){
    //liberando memória
    free((*banco).perguntas);
    free((*banco).usadas);
    //limpando ponteiro
    (*banco).perguntas = NULL;
    (*banco).usadas = NULL;
    (*banco).quantidade = 0;
    (*banco).capacidade = 0;
}

// Um texto é válido se termina dentro do limite, não está vazio e não tem aspas
// (que quebrariam o CSV) nem caracteres de controle como Enter e Tab (códigos abaixo de 32).
// Letras acentuadas são aceitas como estão.
static bool textoValido(const char *texto)
{
    bool temConteudo = false;
    for (int i = 0; i <= LIMITE_TEXTO_PERGUNTA; i++)
    {
        unsigned char c = (unsigned char)texto[i];
        if (c == '\0')
            return temConteudo;
        if (c < 32 || c == 127 || c == '"')
            return false;
        if (c != ' ')
            temConteudo = true;
    }
    return false; // passou do limite sem encontrar o fim do texto
}

//retorna NULL se a pergunta esta certa, ou a mensagem do primeiro erro encontrado
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

//conta quantos enunciados diferentes existem em um nivel
int contaPerguntasDistintas(const BancoPerguntas *banco, Dificuldade dificuldade)
{
    int total = 0;
    for (int i = 0; i < banco->quantidade; i++)
    {
        const Pergunta *atual = &banco->perguntas[i];
        if (atual->dificuldades != dificuldade)
            continue;
        bool jaContada = false;
        // Só compara com registros anteriores do nível, para contar cada enunciado uma vez.
        for (int j = 0; j < i; j++)
        {
            const Pergunta *anterior = &banco->perguntas[j];
            if (anterior->dificuldades == dificuldade &&
                strcmp(anterior->conteudo_da_pergunta, atual->conteudo_da_pergunta) == 0)
            {
                jaContada = true;
                break;
            }
        }
        if (!jaContada)
            total++;
    }
    return total;
}

//adiciona uma pergunta no banco, aumentando o vetor se precisar
int adicionaPerguntas(BancoPerguntas *banco, const Pergunta *perguntas)
{
    if(validarPergunta(perguntas) != NULL)
    {
        return 0;
    }

    //verifica se o tamanho do vetor é suficiente para armazenar mais perguntas
    if((*banco).quantidade == (*banco).capacidade)
    {
        int novaCapacidade = (*banco).capacidade * 2;
        if(novaCapacidade == 0)
        {
            novaCapacidade = CAPACIDADE_INICIAL; //banco ainda sem memoria
        }

        Pergunta *temp = realloc((*banco).perguntas, novaCapacidade * sizeof(Pergunta));
        if(temp == NULL){
            printf("Erro ao aumentar memoria!\n");
            return 0;
        }

        (*banco).perguntas = temp;

        bool *tempUsadas = realloc((*banco).usadas, novaCapacidade * sizeof(bool));
        if(tempUsadas == NULL){
            printf("Erro ao aumentar memoria!\n");
            return 0;
        }

        (*banco).usadas = tempUsadas;

        for(int i = (*banco).capacidade; i < novaCapacidade; i++){
            (*banco).usadas[i] = false;
        }

        (*banco).capacidade = novaCapacidade;

    }

    //coloca a pergunta na primeira posicao livre
    banco->perguntas[banco->quantidade] = *perguntas;
    banco->usadas[banco->quantidade] = false;

    banco->quantidade++;

    return 1;
}

//troca a pergunta da posicao indice (0 e a primeira)
bool editarPerguntaPorIndice(BancoPerguntas *banco, int indice, const Pergunta *pergunta)
{
    if (indice < 0 || indice >= banco->quantidade || validarPergunta(pergunta) != NULL)
        return false;
    banco->perguntas[indice] = *pergunta;
    banco->usadas[indice] = false;
    return true;
}

//remove a pergunta da posicao indice (0 e a primeira), diminuindo o vetor se sobrar espaco
bool excluirPerguntaPorIndice(BancoPerguntas *banco, int indice)
{
    if(indice < 0 || indice >= banco->quantidade)
    {
        return false;
    }

    //puxa todas as perguntas seguintes uma posicao para tras
    for(int i = indice ; i < banco->quantidade - 1 ; i++)
    {
        banco->perguntas[i] = banco->perguntas[i + 1];
        banco->usadas[i] = banco->usadas[i + 1];
    }

    banco->quantidade--;

    //realocar memoria
    if(banco->quantidade <= banco->capacidade / 4 && banco->capacidade > CAPACIDADE_INICIAL)
    {
        int novaCapacidade = banco->capacidade / 2;

        Pergunta *temp = realloc(banco->perguntas, novaCapacidade * sizeof(Pergunta));

        bool *tempUsadas = realloc(banco->usadas, novaCapacidade * sizeof(bool));

        if(temp != NULL)
        {
            banco->perguntas = temp;
        }

        if(tempUsadas != NULL)
        {
            banco->usadas = tempUsadas;
        }

        if(temp != NULL && tempUsadas != NULL)
        {
            banco->capacidade = novaCapacidade;
        }
    }

    return true;
}

//diz se a pergunta do indice combina com o pedido e ainda nao saiu na partida
static bool perguntaDisponivel(const BancoPerguntas *banco, int indice,
                               Dificuldade dificuldade, Categoria categoria)
{
    const Pergunta *pergunta = &banco->perguntas[indice];
    if (banco->usadas[indice] || pergunta->dificuldades != dificuldade || pergunta->categorias != categoria)
        return false;
    // O enunciado fica bloqueado mesmo se houver uma cópia em outra categoria.
    for (int i = 0; i < banco->quantidade; i++)
        if (banco->usadas[i] &&
            strcmp(banco->perguntas[i].conteudo_da_pergunta, pergunta->conteudo_da_pergunta) == 0)
            return false;
    return true;
}

//conta as perguntas que ainda podem sair (a roleta usa para apagar categorias vazias)
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

//sorteia uma pergunta nao usada da dificuldade e categoria pedidas, retorna o indice ou -1
int sorteaPergunta(BancoPerguntas *banco, Dificuldade dificuldades, Categoria categorias)
{
    int disponiveis;
    int sorteadas;

    //contar quantas perguntas podem ser utilizadas
    disponiveis = contaPerguntasDisponiveis(banco, dificuldades, categorias);

    //nao possui perguntas disponiveis de determinada dificuldade e categoria
    if(disponiveis == 0)
    {
        return -1;
    }

    //sorteia das disponiveis
    sorteadas = rand() % disponiveis;

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        if(perguntaDisponivel(banco, i, dificuldades, categorias))
        {
            if(sorteadas == 0)
            {
                banco->usadas[i] = true;
                return i;//devolve indice desejado
            }
            sorteadas--;
        }

    }

    //retorna erro caso nao entre no if correto para retornar o i
    return -1;
}

//le um campo do texto a partir da posicao
static int lerString(const char *texto, int *posicao, char *destino)
{
    int i = *posicao;
    int j = 0;

    if(texto[i] == '"')
    {
        i++;
        while(texto[i] != '"')
        {
            if(texto[i] == '\n' || texto[i] == '\0' || texto[i] == '\r')
            {
                printf("Erro : aspas sem fechar !\n");
                return 0;
            }
            if(j == LIMITE_TEXTO_PERGUNTA)
            {
                printf("Erro : texto muito longo !\n");
                return 0;
            }
            destino[j] = texto[i];
            i++;
            j++;
        }

        i++;
    }
    else
    {
        while (texto[i] != '\n' && texto[i] != '\0' && texto[i] != ',' && texto[i] != '\r' && j < LIMITE_TEXTO_PERGUNTA)
        {
            destino[j] = texto[i];
            i++;
            j++;
        }
    }

    destino[j] = '\0';

    if(j== 0)
    {
        printf("Erro : campo vazio !\n");
        return 0;
    }
    if(texto[i] != ',')
    {
        printf("Erro : campo mal formado / muito longo\n");
        return 0;
    }

    *posicao = i;
    return 1;
}

//le as perguntas do arquivo csv e coloca no banco
int carregaPerguntas(BancoPerguntas *banco, const char *nome_arq){
    FILE *arq;

    arq = fopen(nome_arq, "r");
    if(arq == NULL){
        perror("Não foi possível abrir arquivo.");
        return 0;
    }//if

    char linha[1200];                  //armazena temporariamente uma linha de perguntas.csv
    //serve para ignorar o cabeçalho de perguntas.csv
    if(fgets(linha, sizeof(linha), arq) == NULL){
        printf("Erro: arquivo CSV vazio!\n");
        fclose(arq);
        return 0;
    }//if

    while(fgets(linha, sizeof(linha), arq) != NULL){
        Pergunta pergunta;
        int i = 0;

        if(lerString(linha, &i, pergunta.conteudo_da_pergunta) == 0)
        {
            fclose(arq);
            return 0;
        }

        for(int n = 0; n < 4; n++){
            //verificando se tem o separador
            if(linha[i] != ','){
                printf("Erro: linha do CSV incompleta!\n");
                fclose(arq);
                return 0;
            }//if

            i++;  // pula o ',' anterior
            if(lerString(linha, &i, pergunta.alternativas[n]) == 0)
            {
                fclose(arq);
                return 0;
            }//if
        }//for

        if(linha[i] != ','){
            printf("Erro: linha do CSV incompleta!\n");
            fclose(arq);
            return 0;
        }//if

        i++;  //pula o ',' depois da alternativa D
        //verificando se a alternativa tá no intervalo correto
        if(toupper(linha[i]) < 'A' || toupper(linha[i]) > 'D'){
            printf("Erro: alternativa correta fora do intervalo!\n");
            fclose(arq);
            return 0;
        }
        pergunta.alternativa_correta = toupper(linha[i]);  //passar a letra para maiuscula

        i ++;  //vai para ',' depois da letra correta
        //verificando se tem separador no lugar certo
        if(linha[i] != ','){
            printf("Erro: separador ausente!\n");
            fclose(arq);
            return 0;
        }

        i++; //pula o ','
        //verificando se a dificuldade foi digitada certa
        if(linha[i] < '1' || linha[i] > '5'){
            printf("Erro: dificuldade invalida!\n");
            fclose(arq);
            return 0;
        }
        pergunta.dificuldades = (Dificuldade)(linha[i] - '0');  //converte os caracteres ('1' a '5') no numero da dificuldade

        i ++;  //vai para ',' depois da dificulade
        //verificando se tem separador no lugar certo
        if(linha[i] != ','){
            printf("Erro: separador ausente!\n");
            fclose(arq);
            return 0;
        }

        i++; //pula o ','
        //verificando se a categoria foi digitada certa
        if(linha[i] < '1' || linha[i] > '5'){
            printf("Erro: categoria invalida!\n");
            fclose(arq);
            return 0;
        }
        pergunta.categorias = (Categoria)(linha[i] - '0');

        i++;
        //verificando se tem algo depois da categoria (\r aparece em arquivos salvos no Windows)
        if(linha[i] != '\n' && linha[i] != '\0' && linha[i] != '\r'){
            printf("Erro: caracteres extras no final da linha!\n");
            fclose(arq);
            return 0;
        }//if

        //armazenando a pergunta que foi pega de "perguntas.csv"
        if(adicionaPerguntas(banco, &pergunta) == 0){
            printf("Erro: pergunta invalida no CSV!\n");
            fclose(arq);
            return 0;
        }//if

    }//while

    fclose(arq);
    return 1; //indica que deu certo
}//carregaPerguntas

//capta as informacoes de uma pergunta (editando so muda o texto que aparece)
int lerPergunta(Pergunta *perguntas, bool editando)
{
    char texto[300];
    int dificuldade;
    int categoria;
    const char *novo = editando ? "novo " : "";
    const char *nova = editando ? "nova " : "";

    //ler enunciado
    do
    {
        printf("Digite o %senunciado da questão :", novo);
        if(fgets(texto, sizeof(texto), stdin) == NULL)
        {
            return 0; //acabou a entrada
        }
        texto[strcspn(texto, "\n")] = '\0';

        if (strlen(texto) > 200)
        {
            printf("Pergunta muito grande !\n");
        }
        else if(strlen(texto) == 0)
        {
            printf("O enunciado não pode ser vazio !\n");
        }
        else if(strchr(texto, '"') != NULL)
        {
            printf("Não use aspas no texto !\n");
        }

    } while (strlen(texto) > 200 || strlen(texto) == 0 || strchr(texto, '"') != NULL);

    strcpy(perguntas->conteudo_da_pergunta, texto);//copia a string para o lugar correto

    //le os enunciados das quatro alternativas
    for(int i = 0 ; i < 4 ; i++)
    {
        do
        {
            printf("Digite a %salternativa %c : ", nova, 'A'+i);
            if(fgets(texto, sizeof(texto), stdin) == NULL)
            {
                return 0;
            }
            texto[strcspn(texto, "\n")] = '\0';

            if (strlen(texto) > 200)
            {
                printf("Alternativa muito grande !\n");
            }
            else if(strlen(texto) == 0)
            {
                printf("A alternativa não pode ser vazia !\n");
            }
            else if(strchr(texto, '"') != NULL)
            {
                printf("Não use aspas no texto !\n");
            }

        } while (strlen(texto) > 200 || strlen(texto) == 0 || strchr(texto, '"') != NULL);

        strcpy(perguntas->alternativas[i], texto);
    }

    //le a alternativa certa
    do
    {
        printf("Digite a alternativa correta (A-D) : ");
        if(scanf(" %c", &perguntas->alternativa_correta) != 1)
        {
            return 0;
        }

        if(toupper(perguntas->alternativa_correta) < 'A' || toupper(perguntas->alternativa_correta) > 'D')
        {
            printf("Alternativa inválida !\n");
        }

        limpaEntrada();
    } while (toupper(perguntas->alternativa_correta) < 'A' || toupper(perguntas->alternativa_correta) > 'D');

    perguntas->alternativa_correta = toupper(perguntas->alternativa_correta);

    //le a dificuldade
    do
    {
        dificuldade = 0;

        printf("Digite a dificuldade (1 - Muito fácil, 2 - Fácil, 3 - Médio, 4 - Difícil, 5 - Impossível): ");
        if(scanf(" %d", &dificuldade) == EOF)
        {
            return 0;
        }

        if(dificuldade < 1 || dificuldade > 5)
        {
            printf("Dificuldade inválida !\n");
        }

        limpaEntrada();
    } while (dificuldade < 1 || dificuldade > 5);

    perguntas->dificuldades = (Dificuldade) dificuldade;

    //le a categoria
    do
    {
        categoria = 0;

        printf("Digite a categoria (1 - Geek, 2 - Esportes, 3 - Matematica, 4 - Geografia, 5 - Biologia): ");
        if(scanf(" %d", &categoria) == EOF)
        {
            return 0;
        }

        if(categoria < 1 || categoria > 5)
        {
            printf("Categoria inválida !\n");
        }

        limpaEntrada();
    } while (categoria < 1 || categoria > 5);

    perguntas->categorias = (Categoria) categoria;

    return 1;
}

//printa as perguntas no arquivo csv
int salvaPergunta(const BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq;

    arq = fopen(nome_arq, "w");

    if(arq == NULL)
    {
        perror("Erro ao abrir o arquivo.");
        return 0;
    }

    fprintf(arq, "enunciado,alternativa_A,alternativa_B,alternativa_C,alternativa_D,alternativa_correta,dificuldade,categoria\n");

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        fprintf(arq, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%c,%d,%d\n",
                banco->perguntas[i].conteudo_da_pergunta,
                banco->perguntas[i].alternativas[0],
                banco->perguntas[i].alternativas[1],
                banco->perguntas[i].alternativas[2],
                banco->perguntas[i].alternativas[3],
                banco->perguntas[i].alternativa_correta,
                banco->perguntas[i].dificuldades,
                banco->perguntas[i].categorias);
    }

    fclose(arq);

    return 1;
}

const char *nomeCategoria(Categoria categoria)
{
    switch (categoria)
    {
    case GEEK:       return "Geek";
    case ESPORTES:   return "Esportes";
    case MATEMATICA: return "Matemática";
    case GEOGRAFIA:  return "Geografia";
    case BIOLOGIA:   return "Biologia";
    }
    return "";
}

const char *nomeDificuldade(Dificuldade dificuldade)
{
    switch (dificuldade)
    {
    case MUITO_FACIL: return "Muito fácil";
    case FACIL:       return "Fácil";
    case MEDIO:       return "Médio";
    case DIFICIL:     return "Difícil";
    case IMPOSSIVEL:  return "Impossível";
    }
    return "";
}

//mostra uma pergunta na tela (usada pela listagem e pela pesquisa)
static void mostraPergunta(const BancoPerguntas *banco, int i)
{
    printf("====== Pergunta %d ======\n", i+1);
    printf("%s\n", banco->perguntas[i].conteudo_da_pergunta);
    printf(" A) %s\n", banco->perguntas[i].alternativas[0]);
    printf(" B) %s\n", banco->perguntas[i].alternativas[1]);
    printf(" C) %s\n", banco->perguntas[i].alternativas[2]);
    printf(" D) %s\n", banco->perguntas[i].alternativas[3]);
    printf("Alternativa correta : %c | Dificuldade : %s | Categoria : %s\n\n",
           banco->perguntas[i].alternativa_correta,
           nomeDificuldade(banco->perguntas[i].dificuldades),
           nomeCategoria(banco->perguntas[i].categorias));
}

//mostra todas as perguntas do banco
int listaPergunta (BancoPerguntas *banco)
{
    if(banco->quantidade == 0)
    {
        printf("Nenhuma pergunta cadastrada !\n");
        return 0;
    }

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        mostraPergunta(banco, i);
    }

    return 1;
}

//procura perguntas pelo texto do enunciado
int pesquisaPergunta(BancoPerguntas *banco)
{
    char texto[300];
    int encontradas = 0;

    if(banco->quantidade == 0)
    {
        printf("Ainda nao existem perguntas no programa !\n");
        return 0;
    }

    printf("Digite o enunciado da pergunta que deseja encontrar : ");
    if(fgets(texto, sizeof(texto), stdin) == NULL)
    {
        return 0;
    }
    texto[strcspn(texto, "\n")] = '\0';

    if(strlen(texto) == 0)
    {
        printf("Busca invalida !\n");
        return 0;
    }

    for(int i = 0 ; i < banco->quantidade ; i++)
    {
        if(strstr(banco->perguntas[i].conteudo_da_pergunta, texto) != NULL)
        {
            encontradas++;
            mostraPergunta(banco, i);
        }
    }

    if(encontradas == 0)
    {
        printf("Nao foram encontradas perguntas relacionadas a sua busca !\n");
        return 0;
    }

    return encontradas;
}

//pergunta pelo terminal qual pergunta remover
int excluiPergunta(BancoPerguntas *banco)
{

    int indice_removido = 0;

    if(banco->quantidade == 0)
    {
        printf("O programa nao possui perguntas !\n");
        return 0;
    }

    printf("Digite o indice da pergunta a ser removida : ");
    if(scanf(" %d", &indice_removido) == EOF)
    {
        return 0;
    }

    limpaEntrada();

    if(indice_removido < 1 || indice_removido > banco->quantidade)
    {
        printf("Indice invalido !\n");
        return 0;
    }

    //o usuario conta a partir de 1 e o vetor a partir de 0
    return excluirPerguntaPorIndice(banco, indice_removido - 1);
}

//pergunta pelo terminal qual pergunta alterar e le os novos dados
int alteraPergunta(BancoPerguntas *banco)
{
    int indice = 0;
    Pergunta nova;

    if(banco->quantidade == 0)
    {
        printf("O programa nao possui perguntas para serem alteradas !\n");
        return 0;
    }

    printf("Digite o numero da pergunta : ");
    if(scanf(" %d", &indice) == EOF)
    {
        return 0;
    }

    limpaEntrada();

    if(indice < 1 || indice > banco->quantidade)
    {
        printf("Numero invalido !\n");
        return 0;
    }

    //le tudo em uma copia, assim a pergunta antiga so muda se a leitura terminar
    if(lerPergunta(&nova, true) == 0)
    {
        return 0;
    }

    if(editarPerguntaPorIndice(banco, indice - 1, &nova) == false)
    {
        printf("%s\n", validarPergunta(&nova));
        return 0;
    }

    return 1;
}

//grava a quantidade e depois o vetor inteiro de perguntas
int salvaBinario(const BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq;

    arq = fopen(nome_arq, "wb");

    if(arq == NULL)
    {
        printf("Erro na abertura do arquivo %s\n", nome_arq);
        return 0;
    }

    int ok = 1;

    //fwrite devolve quantos itens conseguiu gravar
    if(fwrite(&banco->quantidade, sizeof(int), 1, arq) != 1)
    {
        ok = 0;
    }
    if(fwrite(banco->perguntas, sizeof(Pergunta), banco->quantidade, arq) != (size_t)banco->quantidade)
    {
        ok = 0;
    }
    if(fclose(arq) != 0)
    {
        ok = 0;
    }

    return ok;
}

//le a quantidade e depois o vetor, aumentando o vetor se precisar
int carregaBinario(BancoPerguntas *banco, const char *nome_arq)
{
    FILE *arq;

    arq = fopen(nome_arq, "rb");

    if(arq == NULL)
    {
        return 0;
    }

    int quantidade;

    if(fread(&quantidade, sizeof(int), 1, arq) != 1)
    {
        fclose(arq);
        return 0;
    }

    //um numero negativo ou absurdo so aparece se o arquivo estiver estragado
    if(quantidade < 0 || quantidade > MAXIMO_PERGUNTAS)
    {
        fclose(arq);
        return 0;
    }

    if(quantidade > banco->capacidade)
    {
        int novaCapacidade = banco->capacidade;
        if(novaCapacidade == 0)
        {
            novaCapacidade = CAPACIDADE_INICIAL; //banco ainda sem memoria
        }

        while(quantidade > novaCapacidade)
        {
            novaCapacidade = novaCapacidade * 2;
        }

        Pergunta *temp = realloc(banco->perguntas, novaCapacidade * sizeof(Pergunta));
        if(temp != NULL)
        {
            banco->perguntas = temp;
        }

        bool *tempUsadas = realloc(banco->usadas, novaCapacidade * sizeof(bool));
        if(tempUsadas != NULL)
        {
            banco->usadas = tempUsadas;
        }

        if(temp == NULL || tempUsadas == NULL)
        {
            fclose(arq);
            return 0;
        }

        banco->capacidade = novaCapacidade;
    }

    if(fread(banco->perguntas, sizeof(Pergunta), quantidade, arq) != (size_t)quantidade)
    {
        banco->quantidade = 0; //o que foi lido pela metade nao serve
        fclose(arq);
        return 0;
    }

    //confere se cada pergunta lida faz sentido
    for(int i = 0; i < quantidade; i++)
    {
        if(validarPergunta(&banco->perguntas[i]) != NULL)
        {
            banco->quantidade = 0;
            fclose(arq);
            return 0;
        }
    }

    banco->quantidade = quantidade;

    for(int i = 0; i < banco->capacidade ; i++)
    {
        banco->usadas[i] = false;
    }

    fclose(arq);
    return 1;
}

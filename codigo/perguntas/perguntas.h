/**
 * @file perguntas.h
 * @author VGA
 * @brief Tipos e funcoes do banco de perguntas.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef PERGUNTAS_H
#define PERGUNTAS_H
#include <stdbool.h>

typedef enum {GEEK = 1, ESPORTES, MATEMATICA, GEOGRAFIA, BIOLOGIA} Categoria;
typedef enum {MUITO_FACIL = 1, FACIL, MEDIO, DIFICIL, IMPOSSIVEL} Dificuldade;

const char *nomeCategoria(Categoria categoria);
const char *nomeDificuldade(Dificuldade dificuldade);

enum { LIMITE_TEXTO_PERGUNTA = 200 };

// Registro do banco. O DAT guarda estes campos na ordem em que estão na memória.
typedef struct {
    char conteudo_da_pergunta[LIMITE_TEXTO_PERGUNTA + 1];
    char alternativas[4][LIMITE_TEXTO_PERGUNTA + 1];
    char alternativa_correta;
    Dificuldade dificuldades;
    Categoria categorias;
} Pergunta;

// "quantidade" é o número de posições em uso e "capacidade" é o espaço alocado.
// "usadas[i]" acompanha "perguntas[i]" apenas durante a partida e não vai para o DAT.
typedef struct {
    int quantidade;
    int capacidade;
    bool *usadas;
    Pergunta *perguntas;
} BancoPerguntas;

/**
 * @brief Cria o banco vazio com a capacidade inicial.
 * @param banco banco a inicializar
 * @return 1 se deu certo, 0 se faltou memória
 */
int inicializaBanco(BancoPerguntas *banco);

/**
 * @brief Le as perguntas do arquivo CSV para o banco.
 * @param banco banco inicializado ou zerado, substituído apenas em caso de sucesso
 * @param nome_arq caminho do CSV
 * @return 1 se deu certo, 0 se deu erro
 */
int carregaPerguntas (BancoPerguntas *banco, const char *nome_arq);

/** @brief Conta perguntas ainda não usadas da categoria e dificuldade indicadas. */
int contaPerguntasDisponiveis(const BancoPerguntas *banco, Dificuldade dificuldade, Categoria categoria);

/**
 * @brief Sorteia uma pergunta nao usada e a marca como usada.
 * @param banco banco de perguntas
 * @param dificuldades dificuldade desejada
 * @param categorias categoria desejada
 * @return indice da pergunta, ou -1 se nao houver
 */
int sorteaPergunta (BancoPerguntas *banco, Dificuldade dificuldades, Categoria categorias);

/**
 * @brief Libera a memoria do banco.
 * @param banco banco a liberar
 */
void liberaBancoPergunta (BancoPerguntas *banco);

/**
 * @brief Le pelo teclado os dados de uma pergunta nova.
 * @param perguntas onde a pergunta e guardada
 * @return 1 quando termina
 */
int lerPergunta(Pergunta *perguntas);

/**
 * @brief Adiciona uma pergunta, aumentando o vetor se precisar.
 * @param banco banco de perguntas
 * @param perguntas pergunta a adicionar
 * @return 1 se deu certo, 0 se faltou memória ou a pergunta é inválida
 */
int adicionaPerguntas(BancoPerguntas *banco, const Pergunta *perguntas);

/** @brief Retorna NULL quando todos os campos são válidos. */
const char *validarPergunta(const Pergunta *pergunta);
bool perguntasIguais(const Pergunta *a, const Pergunta *b);
bool perguntaContem(const Pergunta *pergunta, const char *trecho);
int contaPerguntasDistintas(const BancoPerguntas *banco, Dificuldade dificuldade);
bool editarPerguntaPorIndice(BancoPerguntas *banco, int indice, const Pergunta *pergunta);
bool excluirPerguntaPorIndice(BancoPerguntas *banco, int indice);

/**
 * @brief Grava todas as perguntas em um arquivo CSV.
 * @param banco banco de perguntas
 * @param nome_arq caminho do CSV
 * @return 1 se deu certo, 0 se deu erro
 */
int salvaPergunta(const BancoPerguntas *banco, const char *nome_arq);

/**
 * @brief Mostra todas as perguntas do banco.
 * @param banco banco de perguntas
 * @return 1 se listou, 0 se o banco esta vazio
 */
int listaPergunta (BancoPerguntas *banco);

/**
 * @brief Procura perguntas por um trecho do enunciado.
 * @param banco banco de perguntas
 * @return quantidade de perguntas encontradas
 */
int pesquisaPergunta (BancoPerguntas *banco);

/**
 * @brief Substitui os dados de uma pergunta escolhida.
 * @param banco banco de perguntas
 * @return 1 se alterou, 0 se nao
 */
int alteraPergunta (BancoPerguntas *banco);

/**
 * @brief Exclui uma pergunta escolhida, diminuindo o vetor se sobrar espaco.
 * @param banco banco de perguntas
 * @return 1 se excluiu, 0 se nao
 */
int excluiPergunta (BancoPerguntas *banco);

/**
 * @brief Grava o banco em arquivo binario (quantidade + vetor).
 * @param banco banco de perguntas
 * @param nome_arq caminho do arquivo
 * @return 1 se deu certo, 0 se deu erro
 */
int salvaBinario(const BancoPerguntas *banco, const char *nome_arq);

/**
 * @brief Le o banco do arquivo binario, aumentando o vetor se precisar.
 * @param banco banco inicializado ou zerado, substituído apenas em caso de sucesso
 * @param nome_arq caminho do arquivo
 * @return 1 se deu certo, 0 se nao existe ou deu erro
 */
int carregaBinario(BancoPerguntas *banco, const char *nome_arq);

#endif

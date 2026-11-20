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

typedef struct {
    char conteudo_da_pergunta[201];
    char alternativas[4][201];
    char alternativa_correta;
    Dificuldade dificuldades;
    Categoria categorias;
} Pergunta;

typedef struct {
    int quantidade;
    int capacidade;
    bool *usadas;
    Pergunta *perguntas;
} BancoPerguntas;

/**
 * @brief Cria o banco vazio com a capacidade inicial.
 * @param banco banco a inicializar
 * @return 1 se deu certo, 0 se faltou memoria
 */
int inicializaBanco(BancoPerguntas *banco);

/**
 * @brief Le as perguntas do arquivo CSV para o banco.
 * @param banco banco de destino
 * @param nome_arq caminho do CSV
 * @return 1 se deu certo, 0 se deu erro
 */
int carregaPerguntas (BancoPerguntas *banco, const char *nome_arq);

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
 * @return 1 se deu certo, 0 se faltou memoria
 */
int adicionaPerguntas(BancoPerguntas *banco, Pergunta *perguntas);

/**
 * @brief Grava todas as perguntas em um arquivo CSV.
 * @param banco banco de perguntas
 * @param nome_arq caminho do CSV
 * @return 1 se deu certo, 0 se deu erro
 */
int salvaPergunta(BancoPerguntas *banco, char *nome_arq);

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
int salvaBinario(BancoPerguntas *banco, const char *nome_arq);

/**
 * @brief Le o banco do arquivo binario, aumentando o vetor se precisar.
 * @param banco banco de destino
 * @param nome_arq caminho do arquivo
 * @return 1 se deu certo, 0 se nao existe ou deu erro
 */
int carregaBinario(BancoPerguntas *banco, const char *nome_arq);

#endif 
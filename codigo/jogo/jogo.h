/**
 * @file jogo.h
 * @author VGA
 * @brief Estados e struct da partida.
 * @version 0.1
 * @date 2026-11-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef JOGO_H
#define JOGO_H
#include "perguntas.h"
#include "minigames/minigame.h"

typedef enum
{
    SORTEA, NOVO_NIVEL, PERGUNTA, CONFIRMANDO, SUSPENSE, ACERTOU, ERROU, VITORIA, DERROTA
} Estado;

// Uma instância representa a partida inteira. A rodada vale 0 a 14 internamente.
// Os índices e as letras apontam para a pergunta no banco e suas quatro alternativas.
typedef struct
{
    Estado estado;
    int rodada;
    Dificuldade dificuldade;
    Categoria categoria;
    int indicePergunta;
    int premio;
    int premioGarantido;
    int ultimoCheckpoint; // Número exibido da pergunta. Zero antes do primeiro checkpoint.
    int premioPerdido;
    int bonus;               // Soma dos bônus por acertar com a dificuldade aumentada.
    bool girouDificuldade;   // A roleta de dificuldade só pode ser usada uma vez por rodada.
    Categoria anteriores[2]; // Categorias das duas últimas rodadas ([0] é a mais recente; 0 = nenhuma).
    bool segundaChance;
    char respostaBloqueada;
    bool eliminadas[4];
    bool ajudasUsadas[QUANTIDADE_MINIGAMES];
    TipoMinigame ajudaAtiva;
    char resposta;
    float tempoEstado;
} Jogo;

/** @brief Inicia uma partida e libera as perguntas usadas na partida anterior. */
void iniciarJogo(Jogo *jogo, BancoPerguntas *banco);

/** @brief Carrega uma pergunta inédita da categoria e da dificuldade da rodada. */
bool selecionarPergunta(Jogo *jogo, BancoPerguntas *banco, Categoria categoria);

void escolherResposta(Jogo *jogo, char alternativa);
/** @brief Diz se a letra saiu do jogo por eliminação ou por uma segunda chance já usada. */
bool alternativaBloqueada(const Jogo *jogo, char alternativa);
void cancelarResposta(Jogo *jogo);
void confirmarResposta(Jogo *jogo);
/** @brief Usa o tempo do quadro em segundos para revelar respostas e animar transições. */
void atualizarJogo(Jogo *jogo, const BancoPerguntas *banco, float tempoQuadro);
/** @brief Avança após um acerto, reinicia os dados da rodada e retorna se houve avanço. */
bool avancarRodada(Jogo *jogo);
bool ajudaDisponivel(const Jogo *jogo, TipoMinigame tipo);
bool iniciarAjuda(Jogo *jogo, TipoMinigame tipo);
/** @brief Reage ao fim do desafio e retorna se sua recompensa foi aplicada à pergunta. */
bool concluirAjuda(Jogo *jogo, const Pergunta *pergunta, ResultadoMinigame resultado);

/** @brief Indica se o jogador ainda pode girar a roleta de dificuldade nesta rodada. */
bool podeGirarDificuldade(const Jogo *jogo);

/**
 * @brief Categoria que não pode sair na próxima roleta, para evitar 3 iguais seguidas.
 * @return a categoria das duas últimas rodadas, se foram iguais; senão 0 (nenhuma bloqueada)
 */
Categoria categoriaBloqueada(const Jogo *jogo);

/** @brief Aplica o resultado da roleta de dificuldade (-1, 0 ou +1 nível, limitado de 1 a 5). */
void aplicarDificuldade(Jogo *jogo, int variacao);

#endif

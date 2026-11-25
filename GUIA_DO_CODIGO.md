# Guia do código do Jogo do Milhão

O programa combina um cadastro de perguntas pelo terminal, uma partida de perguntas na janela e três minigames. A Raylib fornece a janela, o desenho, o teclado, o mouse e o áudio. Os módulos em C decidem o que cada ação faz.

## Ordem de estudo

1. Abra `perguntas/perguntas.h` e `jogo/jogo.h`. Identifique os dados de uma pergunta e os dados que mudam durante uma partida.
2. Leia `main.c` a partir de `main()`. Siga primeiro o menu do terminal (`menuTerminal()`) e depois a opção **7 - Jogar**, que chama `menuGrafico()`.
3. Leia `jogo/jogo.c` e `jogo/premio.c`. Acompanhe a resposta de uma pergunta até o avanço ou a derrota.
4. Veja `jogo/roleta.c` e as telas em `interface/`. Compare o sorteio com a animação e observe como um clique vira uma ação.
5. Leia `perguntas/perguntas.c`. Separe as operações em memória das operações que alteram o disco.
6. Leia `interface/interface_ajudas.c` e `minigames/minigame.c`. Entre em um minigame por suas funções de início, atualização, desenho, placar e encerramento.

## Onde fica cada parte

Os caminhos de fontes abaixo são relativos a `codigo/`. Em geral, o `.h` declara o que outros arquivos podem usar e o `.c` contém a implementação.

| Arquivo | Responsabilidade |
|---|---|
| `main.c` | Abre o banco, mostra o menu do terminal, abre a janela na opção 7, coordena as telas e libera os recursos ao sair. |
| `interface/interface.{c,h}` | Menu, pergunta, confirmação, novo nível, resultado e Como Jogar. Define os tipos de ações que as telas devolvem. |
| `interface/desenho.{c,h}` | Centralização de texto e cálculo de progresso de uma animação simples. |
| `interface/interface_ajudas.{c,h}` | Cartões, instruções, andamento e resultado dos desafios integrados. |
| `jogo/jogo.{c,h}` | Respostas, avanço de rodada, bloqueios, bônus, checkpoints e aplicação das recompensas. |
| `jogo/premio.{c,h}` | Tabela de valores, nível base por rodada, pesos da roleta de dificuldade e teto do prêmio. |
| `jogo/roleta.{c,h}` | Seleção das categorias e da variação de dificuldade, animação e desenho das duas roletas. |
| `perguntas/perguntas.{c,h}` | Tipos do banco, vetores, validação, CRUD, sorteio, leitura e gravação dos formatos. |
| `minigames/minigame.{c,h}` | Define os tipos, resultados e placares e escolhe com `switch` a função de cada minigame. |
| `minigames/guitarhero/guitarhero.{c,h}` | Relógio da faixa, notas simples e sustentadas, precisão, combo e sincronização de áudio. |
| `minigames/guitarhero/mapas.h` | Vetores de notas. Cada linha relevante contém tempo de início, duração e trilha. |
| `minigames/tiroalvo/tiroalvo.{c,h}` | Alvos, aparições, movimento, prioridade dos cliques, pontuação e áudio. |
| `minigames/travessia/travessia.{c,h}` | Grade, geração das pistas, movimento, colisões, vidas e modos com meta ou infinito. |
| `Makefile` na raiz | Lista fontes, bibliotecas e recursos e define `make` e `make run`. O projeto compila e roda no Linux. |

Na raiz, `dados/` guarda o CSV inicial e o DAT ativo. `compilado/` recebe os executáveis e as cópias dos áudios. O `README.md` mostra como compilar e jogar. O `.gitignore` lista arquivos gerados pelo programa e pelo compilador.

## Estruturas e enums principais

- **`Pergunta`** contém o enunciado, quatro alternativas, o gabarito, a categoria e a dificuldade. `Categoria` e `Dificuldade` são enums com valores de 1 a 5. O texto de cada campo tem até 200 bytes mais o terminador `'\0'`.
- **`BancoPerguntas`** guarda `quantidade`, `capacidade` e os endereços de dois vetores. `perguntas[i]` guarda o conteúdo. `usadas[i]` indica se aquela posição saiu na partida. A mesma posição identifica a pergunta nos dois vetores.
- **`Jogo`** guarda o estado da partida, a rodada de 0 a 14, o nível atual, o índice da pergunta, a letra escolhida, o prêmio, o bônus e o checkpoint. Os vetores de ajudas e alternativas usam o enum ou a letra como índice.
- **`Tela` e `Estado`** representam decisões diferentes. A tela principal pode ser o menu, a partida, o treino ou as instruções. Dentro da partida, o estado permite decidir entre sortear, mostrar pergunta, confirmar, revelar, avançar ou mostrar o resultado.
- **`Roleta`** guarda os tempos, os ângulos, os pesos e a posição sorteada. Quando `deDificuldade` é verdadeiro, a posição indica diminuir, manter ou aumentar o nível. No sorteio de categorias, ela indica uma das cinco categorias.
- **`Desafios`** guarda a etapa de uma ajuda ou treino, o tipo do minigame, a sessão e o placar final. `ResultadoMinigame` representa em curso, vitória, derrota ou abandono. O placar pode ser mostrado depois que a sessão foi liberada.

## Da entrada ao encerramento

1. **Entrada.** `main()` define a semente do sorteio, carrega o banco e abre `menuTerminal()`. O terminal imprime as opções e reage à opção digitada. A opção 7 chama `menuGrafico()`, que abre a janela e inicia o áudio; ao fechar a janela, o menu do terminal volta.
2. **Quadro.** O laço verifica saída, usa `GetFrameTime()` para atualizar a partida ou o treino e chama a tela correspondente entre `BeginDrawing()` e `EndDrawing()`. `atualizarPartida()` mantém o relógio da pergunta parado enquanto há uma ajuda em andamento.
3. **Início da partida.** `iniciarPartida()` chama `iniciarJogo()` para zerar o estado e limpar o uso das perguntas. `prepararRoleta()` monta a lista das categorias possíveis para o nível atual.
4. **Escolha e resposta.** Depois da roleta, `selecionarPergunta()` pede a `sorteaPergunta()` um índice válido. `desenharTelaPergunta()` devolve uma ação como escolher ou confirmar. `atualizarJogo()` julga a letra depois da espera definida para o suspense.
5. **Continuação.** `conquistarRodada()` atualiza bônus e prêmio. `avancarRodada()` prepara a próxima pergunta e decide se deve mostrar o novo nível. Um erro com segunda chance volta à pergunta. Um erro definitivo conserva o prêmio garantido.
6. **Encerramento.** Ao fechar a interface, os minigames liberam suas sessões e a Raylib fecha o áudio e a janela. Ao sair do programa, `liberaBancoPergunta()` libera os vetores do banco.

### Como um módulo chama outro

- **Clique em uma alternativa.** A tela descobre a letra e retorna uma `AcaoPergunta`. `main.c` passa a letra para `escolherResposta(&jogo, letra)`. A função modifica a instância recebida e muda o estado para confirmação.
- **Cadastrar pelo terminal.** `lerPergunta()` lê os campos e `adicionaPerguntas()` coloca a pergunta no vetor. Em seguida `salvarTerminal()` chama `salvaBinario()` para escrever o DAT.
- **Vencer um minigame.** `atualizarDesafios()` chama `atualizarMinigame(tipo, sessao, segundos)`. O `switch` seleciona a função do minigame. A interface copia o placar e chama `concluirAjuda()` se houver uma partida. O `switch` das recompensas aplica o efeito correspondente.

## Partes para explicar passo a passo

### Ponteiros e alocação

`&jogo` é o endereço da variável `jogo`. Uma função que recebe `Jogo *` pode alterar essa variável. Com `const Jogo *`, ela pode consultar os dados para desenhar a tela. Usa-se `->` para acessar o campo de uma struct por meio do ponteiro.

Os vetores do banco começam com espaço para cinco perguntas. Ao esgotar a capacidade, `adicionaPerguntas()` dobra os dois vetores (`perguntas` e `usadas`) com `realloc`. O resultado vai primeiro para um ponteiro temporário: se o `realloc` falhar, o vetor antigo continua utilizável. Ao excluir, `excluiPergunta()` puxa a pergunta e sua marca de uso uma posição para trás e, quando sobram três quartos livres, diminui os vetores pela metade.

As sessões de minigames são alocadas ao entrar e liberadas ao sair. Cada jogo conhece a própria struct. A interface guarda seu endereço em um `void *` junto do enum do tipo. Antes de acessar os dados, o `switch` entrega o endereço à função do tipo certo, como `AtualizarTiro()`.

### Sorteio, giro e dificuldade

1. `contaPerguntasDisponiveis()` verifica categoria, dificuldade e uso. Para impedir repetição, também compara o enunciado com os que já apareceram, mesmo se o banco contiver cópias em categorias diferentes.
2. `sorteaPergunta()` sorteia um número entre zero e a quantidade disponível menos um. Depois percorre o vetor, descontando um apenas nas posições válidas. Quando chega a zero, marca a posição e retorna seu índice.
3. A roleta de categorias usa cinco setores iguais. Ela só sorteia categorias que têm perguntas inéditas no nível atual e evita a terceira repetição consecutiva quando ainda há outra opção.
4. A roleta de dificuldade tem setores proporcionais aos pesos das opções possíveis. Por exemplo, com pesos 75 e 15, o total é 90. Cada unidade vale quatro graus. Tanto o desenho quanto o ângulo do ponteiro usam essa escala.
5. As duas roletas guardam o resultado antes de começar a girar. A atualização acumula o tempo do quadro e aproxima o ângulo do destino com uma curva que desacelera. A parte visual usa seno e cosseno para colocar os rótulos no disco.

A dificuldade base é `1 + rodada / 3`. Como a divisão é inteira e a rodada começa em zero, os três primeiros índices resultam no nível 1. A roleta opcional modifica o nível daquela pergunta. Ao avançar, o jogo volta a calcular o nível base da nova rodada.

### Prêmio, checkpoints e ajudas

`valorRodada()` consulta a tabela de 15 valores. `bonusDificuldade()` calcula metade do valor da rodada para cada nível acima da dificuldade base. O bônus é concedido no acerto e se acumula. `premioComBonus()` compara o bônus com o espaço restante até o teto antes de somar. Assim, a vitória vale até R$ 1 milhão.

Ao concluir as perguntas 3, 6, 9 e 12, `conquistarRodada()` copia o prêmio atual para `premioGarantido`. Um erro definitivo guarda a diferença em `premioPerdido` para a animação e entrega o valor garantido. A escada mostra os valores base da tabela. O cabeçalho mostra os valores reais da partida e a pergunta indica se há bônus disponível.

- **Guitar Hero.** Vencer com pelo menos 70% das notas aciona a mesma função que um acerto. A ajuda está indisponível na rodada final.
- **Travessia.** Chegar a 25 pistas na partida arma a segunda chance. No próximo erro, só a letra incorreta é bloqueada e o jogador pode responder novamente.
- **Tiro ao Alvo.** A vitória seleciona duas alternativas incorretas entre as que ainda podem ser escolhidas. A primeira sai da lista de candidatas antes do segundo sorteio.

A ajuda é marcada como usada quando uma sessão começa com sucesso. No treino, a interface passa `NULL` como partida, o que impede a aplicação de uma recompensa. O treino da Travessia tem mapa contínuo até acabarem as vidas.

### CSV e DAT

O CSV guarda um registro por linha. Campos de texto com vírgula são cercados por aspas. `lerString()` desloca um índice pela linha para separar os campos e `carregaPerguntas()` valida gabarito, dificuldade e categoria. O DAT começa com a quantidade em um `int`, seguida de um vetor de `Pergunta` escrito com `fwrite()`. `carregaBinario()` recusa quantidade negativa e confere cada pergunta lida com `validarPergunta()`.

Na abertura, `abrirBanco()` tenta ler o DAT primeiro. Sem DAT, usa o CSV para criar o binário. Se o DAT existir e a leitura falhar, as perguntas disponíveis no CSV ficam abertas para consulta.

`salvaBinario()` abre o DAT para escrita com `fopen()`, grava a quantidade e as perguntas com `fwrite()` e fecha com `fclose()`. O terminal salva após cada cadastro, alteração ou exclusão. Para gerar outro CSV, use a opção **6 - Exportar perguntas para CSV**.

### UTF-8, pistas e áudio

- **Texto.** Uma letra acentuada pode ocupar mais de um byte, então o limite de 200 é contado em bytes (`strlen`). A quebra de linhas da pergunta mede caracteres inteiros com a Raylib.
- **Travessia.** A pista de número `L` ocupa `L % BUFFER_PISTAS` no vetor. Quando linhas distantes chegam à câmera, o jogo reaproveita posições que já saíram da área visível. O campo `linha` confirma se a posição contém a pista pedida.
- **Guitar Hero.** A faixa é lida antes da partida. A contagem das notas começa junto com a reprodução. O fim do som no mixer pode preceder sua última amostra na saída de áudio, então o jogo espera mais 650 ms sem parar o laço da janela.

## Perguntas de apresentação

| Pergunta | Resposta |
|---|---|
| Por que a primeira pergunta está no índice zero? | Vetores em C começam no índice zero. A tela soma um para mostrar a numeração de 1 a 15. |
| Como o programa sabe se deve desenhar a roleta ou uma pergunta? | `desenharPartida()` consulta `Jogo.estado`. As etapas comuns da pergunta compartilham a mesma tela. |
| De onde vem o valor do checkpoint? | É uma cópia do prêmio atual no momento do acerto da rodada de checkpoint. |
| Como o cadastro aparece depois de reabrir o jogo? | O terminal grava a pergunta no DAT. Na próxima abertura, `carregaBinario()` monta o vetor a partir dele. |
| Como é garantido que uma resposta errada não encerre a partida com segunda chance ativa? | `julgarResposta()` consome a chance, guarda a letra bloqueada e retorna ao estado `PERGUNTA`. |
| Como o programa diferencia ajuda e treino? | A presença do ponteiro para `Jogo` indica que há uma partida em andamento. Treino tem sessão e placar próprios. |
| Qual é a diferença entre tempo de quadro e tempo total? | `GetFrameTime()` fornece segundos desde o quadro anterior. O código soma esse intervalo ao relógio que está ativo. |
| Como a troca para `switch` escolhe o minigame certo? | As funções em `minigame.c` recebem o enum do tipo e chamam a função correspondente com o endereço da sessão. |

## Exercícios para uma avaliação prática

1. Mude a cor do título do menu em `desenharMenu()`. Identifique quais outros textos continuam usando a paleta compartilhada.
2. Altere a duração de suspense em `jogo.c`. Teste uma resposta certa e uma errada para observar quando ocorre a revelação.
3. Ajuste o valor de uma rodada intermediária em `valoresRodadas`. Confira a escada, a tela de instruções e o prêmio recebido em uma rodada de checkpoint.
4. Adicione uma pergunta em uma cópia do banco pelo menu do terminal, pesquise um trecho do enunciado e confira a exportação CSV.
5. No treino da Travessia, mude a duração da contagem inicial e explique por que isso altera o momento da primeira jogada. Use os alvos independentes do Makefile para testar mudanças em um minigame.

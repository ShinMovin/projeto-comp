# projeto-comp

O [Guia do código](GUIA_DO_CODIGO.md) apresenta a estrutura do projeto e a ordem de estudo para a apresentação.

## Compilar e executar

Execute os comandos a partir da raiz do repositório. O Makefile detecta Windows ou Linux e grava o binário em `compilado/`. O jogo lê o banco em `dados/` a partir do diretório do executável, então mantenha essas pastas lado a lado.

### Windows com MSYS2 UCRT64

Com GCC, Make e Raylib para UCRT64 disponíveis, execute no PowerShell do VS Code:

```powershell
.\compilar.cmd
.\compilar.cmd jogar
```

O primeiro comando gera `compilado/jogo.exe`. O segundo também abre o jogo. Para usar `--banco`, execute `.\compilado\jogo.exe --banco` com `ucrt64\bin` do MSYS2 no `PATH`.

### Linux

Com GCC, Make e Raylib para Linux disponíveis, compile a partir da raiz do projeto.

```bash
make
```

Para abrir o jogo, use o comando abaixo. Ele também compila o necessário antes de executar.

```bash
make run
```

Para abrir o gerenciamento pelo terminal, execute `./compilado/jogo --banco`.

### Recursos e banco de perguntas

- O jogo carrega `dados/perguntas.dat` e usa `dados/perguntas.csv` como alternativa. O DAT legado depende do formato binário de `int`, `enum` e `Pergunta` utilizado pelo compilador. O CSV utiliza UTF-8 e aceita quebras LF ou CRLF.
- Os alvos opcionais `guitarhero`, `tiroalvo` e `travessia` compilam os minigames independentes. O Makefile copia os áudios para `compilado/guitarhero-musicas/` e `compilado/audio-tiroalvo/`. No Linux, execute `make guitarhero tiroalvo travessia` a partir da raiz.

## Partida

A partida tem 15 perguntas. A dificuldade base aumenta a cada três rodadas. Antes de sortear a categoria, o jogador pode girar uma roleta opcional para mudar a dificuldade. Acertar após um aumento dá bônus. O prêmio total, incluindo os bônus, é limitado a R$ 1 milhão. Os checkpoints das rodadas 3, 6, 9 e 12 protegem o prêmio atual daquele momento.

## Ajudas e treino

Use **PEDIR AJUDA** na pergunta ou **TESTAR MINIGAMES** no menu principal. Ambos abrem os mesmos desafios dos minigames na janela de 800 × 600. O treino permite repetir desafios e funciona independentemente da partida.

- **Guitar Hero** usa D, F, J e K. No modo de ajuda, acertar pelo menos 70% das notas da faixa concede o valor da rodada e libera o avanço pela roleta. A ajuda fica indisponível na pergunta do milhão.
- **Tiro ao Alvo** exige 7.500 pontos em 30 segundos. A vitória elimina duas alternativas incorretas sorteadas.
- **Travessia** usa as setas ou WASD. Avance 25 pistas com três vidas para ganhar uma segunda chance na pergunta atual. O treino continua até acabarem as vidas.

Cada tentativa de ajuda é consumida ao iniciar o desafio, inclusive se ele for abandonado. O Makefile copia as faixas e efeitos dos minigames ao compilar o jogo principal. Os alvos independentes continuam disponíveis para treino isolado.

## Padrão para cadastrar perguntas

Abra **BANCO DE PERGUNTAS** no menu principal para pesquisar, adicionar, editar ou excluir registros. Use **TAB** e **Shift+TAB** para mudar de campo, **Backspace** para apagar e **SALVAR BANCO** para gravar. A opção `--banco` oferece as mesmas operações pelo terminal, além da exportação para `dados/perguntas_exportadas.csv`.

O jogo usa um DAT válido como fonte principal. Quando ainda não existe DAT, tenta gerar um a partir do CSV. Se existir um DAT inválido e o CSV puder ser lido, o banco abre para consulta.

O editor e o modo `--banco` salvam as alterações no DAT. O jogo grava e confere um arquivo temporário antes de substituir o DAT ativo. O CSV original fica disponível para importação e o modo terminal pode gerar `dados/perguntas_exportadas.csv`. A trava do banco permanece ativa até o fim da sessão que está usando seus dados.

Cada campo de texto aceita até 200 bytes em UTF-8. O editor avisa se algum nível ficar com menos de três enunciados distintos, necessários para as 15 rodadas.

O arquivo `dados/perguntas.csv` usa a ordem abaixo.

enunciado,alternativa_A,alternativa_B,alternativa_C,alternativa_D,alternativa_correta,dificuldade,categoria

### Exemplo
Quanto é 2 + 2?,3,4,5,6,B,1,3
"Quanto é 1,5 + 2,5?",3,"3,5",4,"4,5",C,2,3

### Regras
- Separar os campos com vírgula (',').
- Alternativa correta vai de A até D.
- Dificuldade vai de 1 (muito fácil) a 5 (impossível).
- Não utilizar aspas (`"`) dentro do texto.
- Se um texto (enunciado ou alternativa) tiver vírgula, coloque o campo entre aspas: `"3,5"`.
- Categoria vai de 1 a 5: 1 = Geek, 2 = Esportes, 3 = Matemática, 4 = Geografia, 5 = Biologia.

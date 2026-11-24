# projeto-comp

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

Com GCC, Make e Raylib para Linux disponíveis:

```bash
make -C codigo
./compilado/jogo
```

Para abrir o gerenciamento pelo terminal, execute `./compilado/jogo --banco`.

### Recursos e banco de perguntas

- O jogo carrega `dados/perguntas.dat` e usa `dados/perguntas.csv` como alternativa. O DAT legado depende do formato binário de `int`, `enum` e `Pergunta` utilizado pelo compilador. O CSV utiliza UTF-8 e aceita quebras LF ou CRLF.
- Os alvos opcionais `guitarhero` e `tiroalvo` compilam os minigames existentes e copiam os áudios para `compilado/guitarhero-musicas/` e `compilado/audio-tiroalvo/`. Mantenha esses nomes e a caixa dos nomes dos arquivos. No Linux, por exemplo, execute `make -C codigo guitarhero tiroalvo` a partir da raiz.

## Ajudas e treino

Use **PEDIR AJUDA** na pergunta ou **TESTAR MINIGAMES** no menu principal. Ambos abrem os mesmos desafios dos minigames na janela de 800 × 600. O treino permite repetir desafios e funciona independentemente da partida.

- **Guitar Hero** usa D, F, J e K. No modo de ajuda, acertar pelo menos 70% das notas da faixa concede o valor da rodada e libera o avanço pela roleta. A ajuda fica indisponível na pergunta do milhão.
- **Tiro ao Alvo** exige 7.500 pontos em 30 segundos. A vitória elimina duas alternativas incorretas sorteadas.
- **Crossy Road** aparece como indisponível enquanto os arquivos-fonte estiverem vazios.

Cada tentativa de ajuda é consumida ao iniciar o desafio, inclusive se ele for abandonado. O Makefile copia as faixas e efeitos dos minigames ao compilar o jogo principal. Os alvos independentes continuam disponíveis para treino isolado.

## Padrão para cadastrar perguntas

Abra **BANCO DE PERGUNTAS** no menu principal para pesquisar, adicionar, editar ou excluir registros. Use **TAB** e **Shift+TAB** para mudar de campo, **Backspace** para apagar e **SALVAR BANCO** para gravar. A opção `--banco` oferece as mesmas operações pelo terminal, além da exportação para `dados/perguntas_exportadas.csv`.

O jogo usa o DAT válido como base e compara todo o conteúdo do CSV antes de salvar. Registros exclusivos de qualquer arquivo são preservados. Para editar ou excluir uma pergunta dos dois formatos, use o editor ou `--banco`. Se um arquivo existente estiver inválido ou ilegível, o banco disponível é aberto para consulta até que o arquivo com problema seja corrigido.

A gravação gera os dois arquivos a partir da mesma versão e conserva cópias de segurança até a confirmação do par. Após uma interrupção durante a troca, a próxima abertura recupera a versão anterior. Durante uma partida ou sessão do editor, outra instância pode abrir o banco assim que a sessão em uso terminar.

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

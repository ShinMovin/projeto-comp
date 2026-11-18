# Atalho: repassa os comandos para o Makefile da pasta codigo/
# (todas as regras de compilacao estao em codigo/Makefile)
#
#   make                -> compila o jogo e o Guitar Hero
#   make run            -> compila e executa o jogo
#   make guitarhero     -> compila so o Guitar Hero
#   make run-guitarhero -> compila e executa o Guitar Hero
#   make clean          -> apaga os executaveis
#
# Os executaveis ficam na pasta compilado/.

.PHONY: all run guitarhero run-guitarhero clean

all:
	$(MAKE) -C codigo all guitarhero

run:
	$(MAKE) -C codigo run

guitarhero:
	$(MAKE) -C codigo guitarhero

run-guitarhero:
	$(MAKE) -C codigo run-guitarhero

clean:
	$(MAKE) -C codigo clean

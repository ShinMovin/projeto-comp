# Atalhos para compilar e rodar a partir da raiz.
.PHONY: all run

all:
	$(MAKE) -C codigo all

run:
	$(MAKE) -C codigo run

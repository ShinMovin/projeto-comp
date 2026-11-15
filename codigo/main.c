#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "perguntas.h"
#include "raylib.h"

int main(){
    BancoPerguntas banco;

    if(inicializaBanco(&banco) == 0) {
        return 1;
    }

    //teste pra ver se ta funcionando a função
    if(carregaPerguntas(&banco, "../dados/perguntas.csv") == 0){
        liberaBancoPergunta(&banco);
        return 1;
    }

    liberaBancoPergunta(&banco);

    return 0;
}
# projeto-comp
## Padrão para cadastrar perguntas

As perguntas devem ser adicionadas ao arquivo
dados/perguntas.csv, seguindo esta ordem:

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
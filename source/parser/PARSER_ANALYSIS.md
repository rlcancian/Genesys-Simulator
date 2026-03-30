# Análise técnica do parser (Bison/Flex)

## Escopo
Arquivos avaliados:
- `source/parser/parserBisonFlex/bisonparser.yy`
- `source/parser/parserBisonFlex/lexerparser.ll`
- `source/parser/parserBisonFlex/bisonparser.report`

## Principais problemas encontrados

1. **Explosão de conflitos LR (gramática altamente ambígua)**
   - O relatório mostra centenas de conflitos, incluindo **shift/reduce** e **reduce/reduce**.
   - Causa raiz: regra `expression` referencia `arithmetic`, `logical`, `relacional`, e cada uma delas por sua vez usa `expression` nos dois lados, criando ambiguidade estrutural de precedência/associatividade.
   - Impacto: o parser aceita entradas de forma não determinística sem semântica robusta; mudanças pequenas na gramática podem alterar interpretações silenciosamente.

2. **Tokenização incorreta de `min`/`max` (invertidos)**
   - No Flex, `min` retorna token de `mathMAX` e `max` retorna `mathMIN`.
   - Impacto: `min(a,b)` computa máximo e `max(a,b)` computa mínimo (erro semântico crítico).

3. **Regex de `EntitiesWIP` incorreta (classe de caracteres)**
   - Regra usa `[EntitiesWIP]`, que em Flex significa “um caractere dentre E,n,t,i,...”, não a palavra completa.
   - Impacto: qualquer caractere isolado do conjunto pode virar token `simulEntitiesWIP`, desviando a análise léxica.

4. **Regra isolada `T` no lexer**
   - Existe uma linha `T` solta como regra léxica.
   - Impacto: entrada contendo `T` maiúsculo pode ser consumida por uma regra inesperada e mascarar identificadores/erros.

5. **Token `ILLEGAL` nunca consumido pela gramática efetiva**
   - Nonterminal `illegal` está declarado mas não é alcançável por `input -> expression`.
   - O relatório do Bison marca `illegal`/`ILLEGAL` como inúteis.
   - Impacto: tratamento de erro lexical planejado não executa; erros podem cair apenas no `error(...)` genérico.

6. **Conversão de hexadecimal conceitualmente incorreta**
   - Hex `0x...` é convertido via `atof(yytext)`.
   - `atof` não é apropriado para inteiros hexadecimais no formato esperado (`strtol` base 16 seria correto).
   - Impacto: valores hex podem ser interpretados como `0` ou incorretos.

7. **Ação vazia em não terminal tipado (`elementFunction`)**
   - O Bison sinaliza regra vazia para símbolo tipado sem ação.
   - Impacto: comportamento indefinido de valor semântico em alguns caminhos e warning estrutural.

8. **Precedência declarada sem efeito real em alguns tokens**
   - Bison sinaliza precedências inúteis (`oNOT`, funções matemáticas e `[`), indicando modelagem inconsistente de precedência.
   - Impacto: falsa sensação de controle da gramática; conflitos continuam altos.

9. **Operador de potência sem associatividade adequada**
   - `POWER` é tratado na mesma forma de binários genéricos à esquerda (via recursão com `expression`).
   - Em linguagens formais, `^` costuma ser associativo à direita (`a^b^c = a^(b^c)`).
   - Impacto: possível divergência semântica para expressões encadeadas.

10. **Semânticas parcialmente não implementadas retornando valores neutros**
    - Regras como `fDISC`, `fLASTINQ`, `fRESSEIZES`, avaliação de `FORM` possuem TODO com retorno fixo/zero.
    - Impacto: expressões sintaticamente válidas com resultado semântico errado.

11. **Risco de null-deref em ações semânticas**
    - Há acessos diretos encadeados (`getCurrentEvent()->getEntity()`) em atribuições sem checagem robusta.
    - Impacto: parser pode falhar em runtime dependendo do contexto de simulação.

## Recomendações de arquitetura (compiladores/linguagens formais)

1. **Refatorar gramática para níveis explícitos de precedência**:
   - `expr_or -> expr_and -> expr_rel -> expr_add -> expr_mul -> expr_unary -> expr_pow -> primary`.
   - Elimina a maior parte dos conflitos LR de forma canônica.

2. **Separar comandos de expressões**:
   - `if/for/assign` em não terminal de comando, não misturado ao núcleo aritmético/lógico.

3. **Corrigir o léxico antes de ajustar sintaxe**:
   - Consertar `min/max`, `EntitiesWIP`, remover regra `T`, e converter hex com `strtol(...,16)`.

4. **Conectar tratamento de erro léxico ao start symbol**:
   - Tornar `ILLEGAL` alcançável por regra de recuperação de erro (ou tratar no driver de forma unificada).

5. **Transformar TODOs semânticos em erro explícito**:
   - Melhor sinalizar “função não implementada” do que retornar 0 silenciosamente.


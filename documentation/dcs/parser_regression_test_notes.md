# Parser regression test notes

## Estrutura de testes

Os testes unitarios ficam em `source/tests/unit` e sao integrados ao CMake por `source/tests/unit/CMakeLists.txt`.

O helper local `genesys_add_unit_test` cria executaveis com GoogleTest, vincula `GTest::gtest_main` e registra os casos com `gtest_discover_tests(... LABELS "unit")`. O alvo `genesys_test_parser_expressions` compila `test_parser_expressions.cpp` e participa do alvo agregado `genesys_kernel_unit_tests`.

Os presets relevantes sao:

- `cmake --preset tests-unit`
- `cmake --build --preset tests-unit`
- `ctest --preset tests-unit`

## Cobertura adicionada para DCS

A cobertura de regressao do parser atual foi adicionada ao teste existente `genesys_test_parser_expressions` e cobre:

- expressoes aritmeticas simples: `1+2`, `2*3+4`, `(2+3)*4`;
- precedencia de multiplicacao sobre soma;
- associatividade esquerda de subtracao e divisao;
- associatividade direita de potencia;
- comportamento atual de unario antes de potencia em `-2^2`;
- funcoes matematicas reconhecidas pelo parser: `round`, `trunc`, `frac`, `sqrt`, `mod`, `exp`, `log`, `ln`, `min`, `max`, `sin`, `cos`.

## Limitacoes

Queue, Resource e Set nao foram cobertos nesta etapa. Esses casos dependem de objetos de modelo e plugins especificos e devem entrar como testes de integracao ou testes unitarios com montagem dedicada do modelo, para evitar que a primeira regressao do DCS misture avaliacao simples de expressoes com setup complexo de simulacao.

## Resultado de validacao local

O ambiente local nao tinha `cmake`, `ctest`, `ninja` ou Docker disponiveis no PATH. Para validar sem alterar o sistema, foi criado um virtualenv temporario em `/tmp/genesys-cmake-venv` com `cmake` e `ninja` instalados via `pip`.

Comandos executados:

- `PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-unit`
- `PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build build/tests-unit --target genesys_test_parser_expressions --parallel 2`
- `./build/tests-unit/source/tests/unit/genesys_test_parser_expressions --gtest_filter='ParserExpressionsTest.ArithmeticPrecedenceAndParentheses:ParserExpressionsTest.OperatorAssociativityMatchesCurrentGrammar:ParserExpressionsTest.MathFunctionsRoundTruncFracAndSqrt'`
- `PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest|ParserDriverThrowsFalseTest' --output-on-failure`

Resultados:

- Configuracao `tests-unit`: passou.
- Build de `genesys_test_parser_expressions`: passou.
- Testes novos/alterados filtrados: 3/3 passaram.
- CTest focado no parser: 12/15 passaram.

Falhas observadas no CTest focado:

- `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDReads`
- `ParserExpressionsTest.AttributeIndexesSupportLegacyAndNDReadsAndAssignmentsDuringEvent`
- `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDAssignments`

As falhas estao em testes preexistentes de indices multidimensionais de `Variable`/`Attribute`, nos quais expressoes ou leituras com chaves como `1,2`, `1,2,3` e `1,2,3,4,5` retornaram `0` em vez dos valores esperados. O parser/kernel nao foi alterado para corrigir essas falhas nesta etapa.

# Relatorio tecnico: parser extensivel por FunctionRegistry

## Contexto do Tema 12

O Tema 12 do DCS trata da extensibilidade do parser do GenESyS para permitir que novas funcoes sejam oferecidas por plugins ou componentes externos sem exigir, como primeira opcao, a edicao direta da gramatica para cada novo nome de funcao.

O trabalho foi conduzido de forma incremental:

1. registrar baseline do desenvolvimento;
2. inventariar o parser atual;
3. criar testes de regressao;
4. implementar `FunctionRegistry`;
5. integrar o registry ao driver do parser;
6. criar `SemanticResolver`;
7. adicionar chamada generica de funcao no parser;
8. demonstrar extensibilidade com `FakePlugin`.

## Problema original

O parser original reconhecia muitas funcoes por tokens especificos em Flex/Bison. Funcoes matematicas, probabilisticas, de kernel e de plugins conhecidos aparecem como regras lexicais e producoes dedicadas.

Exemplos de acoplamento encontrado:

- `nq`, `firstinq`, `mr`, `nr`, `numset` e outras funcoes sao regras lexicais explicitas em `lexerparser.ll`;
- `fNQ`, `fMR`, `fNR`, `fNUMSET`, `QUEUE`, `RESOURCE`, `SET`, `VARI` e `FORM` sao tokens especificos em `bisonparser.yy`;
- `pluginFunction`, `variable`, `formula` e outras producoes fazem casts diretos para classes de plugin;
- a avaliacao ocorre diretamente nas acoes semanticas do Bison por meio de `$$.valor`.

Consequencia: adicionar uma funcao nova exigia alterar lexer, tokens, producoes e, em alguns casos, includes e casts especificos. Isso tornava o parser sintaticamente acoplado a plugins conhecidos.

## Decisao arquitetural

A decisao adotada foi criar uma camada incremental de registro e resolucao de funcoes:

- `FunctionRegistry` armazena descritores e callbacks;
- `SemanticResolver` valida e executa chamadas por nome;
- `genesyspp_driver` referencia opcionalmente um registry externo, sem assumir ownership;
- o parser passou a aceitar uma chamada generica `IDENTIFIER(...)`;
- funcoes antigas continuam pelo caminho legado.

Essa abordagem separa, dentro do escopo possivel, reconhecimento sintatico e resolucao semantica. O parser ainda avalia argumentos diretamente, mas a funcao chamada passa a ser resolvida por nome e aridade fora de Flex/Bison.

## Por que nao iniciar por regeneracao dinamica completa

A regeneracao dinamica completa de Flex/Bison foi preservada como possibilidade futura, mas nao foi escolhida como primeira entrega.

Motivos tecnicos:

- `ParserManager::generateNewParser(...)` e `connectNewParser(...)` existem como interface, mas a implementacao atual ainda e minima;
- `ParserChangesInformation` armazena fragmentos textuais para o parser, o que exige sincronizar includes, tokens, tipos, regras lexicais e producoes;
- `GENESYS_PARSER_REGENERATE` existe no CMake, mas fica desligado por padrao;
- a gramatica atual mistura parsing, resolucao de simbolos e avaliacao em acoes C++;
- havia tokens reconhecidos sem producao semantica observada no inventario;
- a ativacao dinamica exigiria tratar toolchain, compilacao, link, rollback, erros e ciclo de vida em runtime.

A abordagem por registry reduz risco porque primeiro cria um contrato testavel para funcoes. A migracao de funcoes legadas pode ocorrer uma por vez.

## Arquitetura final

Fluxo atual da chamada generica:

```text
Expressao textual
  -> lexer retorna IDENTIFIER para literal nao reconhecido
  -> Bison reconhece IDENTIFIER '(' argumentList ')'
  -> Bison avalia argumentos como double
  -> SemanticResolver consulta FunctionRegistry
  -> callback registrado retorna double ou erro controlado
```

O parser nao conhece a origem concreta da funcao. No caso de demonstracao, `FakePlugin` registra `FakeAdd` no registry, mas o Bison/Flex nao possui token especifico para `FakeAdd`.

## Classes e estruturas criadas

- `FunctionDescriptor`: nome publico, aridade minima/maxima, origem/plugin, descricao e categoria.
- `FunctionRegistrationResult`: resultado de registro com erro textual.
- `FunctionCallResult`: resultado de execucao de callback com valor ou erro.
- `FunctionRegistry`: registro, lookup, validacao de conflito, validacao de aridade, execucao segura de callback e listagem.
- `FunctionCallRequest`: representacao minima de chamada por nome e argumentos.
- `SemanticResolverResult`: resultado semantico com sucesso, valor e mensagem.
- `SemanticResolver`: valida registry ausente, funcao inexistente, aridade incorreta, erro de callback e retorno nao finito.
- `FakePlugin`: classe de demonstracao em testes que registra `FakeAdd` e `FakeSquare`.

## Arquivos modificados ou criados

Infraestrutura do parser:

- `source/parser/FunctionRegistry.h`
- `source/parser/FunctionRegistry.cpp`
- `source/parser/SemanticResolver.h`
- `source/parser/SemanticResolver.cpp`
- `source/parser/Genesys++-driver.h`
- `source/parser/Genesys++-driver.cpp`
- `source/parser/parserBisonFlex/bisonparser.yy`
- `source/parser/parserBisonFlex/lexerparser.ll`
- `source/parser/GenesysParser.h`
- `source/parser/GenesysParser.cpp`
- `source/parser/Genesys++-scanner.cpp`

Ponte com o wrapper do parser:

- `source/kernel/simulator/ParserDefaultImpl2.h`
- `source/kernel/simulator/ParserDefaultImpl2.cpp`

Testes:

- `source/tests/unit/test_parser_function_registry.cpp`
- `source/tests/unit/test_parser_semantic_resolver.cpp`
- `source/tests/unit/test_parser_expressions.cpp`
- `source/tests/unit/test_parser_function_registry_demo.cpp`
- `source/tests/unit/fakes/FakePlugin.h`
- `source/tests/unit/CMakeLists.txt`

Documentacao:

- `documentation/dcs/development_baseline.md`
- `documentation/dcs/parser_dynamic_registry_inventory.md`
- `documentation/dcs/parser_regression_test_notes.md`
- `documentation/dcs/function_registry.md`
- `documentation/dcs/function_registry_demo.md`
- `documentation/dcs/parser_function_registry_report.md`

## Relacao com a infraestrutura original

### ParserChangesInformation

`ParserChangesInformation` foi preservado. Ele representa a infraestrutura historica para armazenar trechos textuais de parser, como includes, tokens, type objects, expressoes, assignments e producoes de funcao.

O DCS nao remove essa estrutura porque ela ainda pode ser util quando um plugin realmente precisar introduzir sintaxe nova. A proposta atual cobre funcoes numericas por chamada generica, nao todos os casos de extensao sintatica.

### ParserManager

`ParserManager` tambem foi preservado. O inventario mostrou que ele declara pontos para gerar e conectar novo parser, mas nao havia implementacao suficiente para depender desse fluxo como primeira entrega.

A solucao atual evita exigir geracao dinamica em runtime e permite validar comportamento com testes unitarios e de integracao.

### Marcadores em bisonparser.yy

Os marcadores existentes foram mantidos, incluindo:

- `begin_Includes_plugins`
- `begin_Tokens_plugins`
- `begin_TypeObj_plugins`
- `begin_Expression_plugins`
- `begin_ExpressionProdution_plugins`
- `begin_Assignment_plugins`
- `begin_FunctionProdution_plugins`

Eles continuam importantes para compatibilidade com o mecanismo historico de insercao textual. A chamada generica foi adicionada de forma localizada, sem substituir os blocos legados.

### Marcadores em lexerparser.ll

Os marcadores existentes no lexer tambem foram mantidos:

- `begin_Includes_plugins`
- `begin_Lexical_plugins`
- `begin_LexicalLiterals_plugins`

O lexer continua priorizando palavras reservadas, funcoes antigas e elementos conhecidos do modelo. O fallback de literal nao reconhecido passou a poder alimentar o token generico `IDENTIFIER`.

### Motivo da preservacao

Esses elementos foram preservados para manter compatibilidade e reduzir risco. O registry resolve o caso de funcoes numericas novas, mas nao substitui, nesta etapa, toda a infraestrutura de extensao sintatica do GenESyS.

## Compatibilidade

- Funcoes antigas continuam reconhecidas pelos tokens e producoes existentes.
- Funcoes de Queue, Resource, Set, Variable e Formula nao foram removidas nem migradas nesta etapa.
- Expressoes aritmeticas antigas continuam passando nos testes de regressao focados.
- `FakeAdd(2,3)` funciona pelo novo caminho generico quando registrado no `FunctionRegistry`.
- Antes do registro, `FakeAdd(2,3)` falha com erro controlado de funcao nao registrada.
- O registry permite extensoes futuras por nome, aridade, origem/plugin e callback, sem criar token especifico para cada funcao nova.

## Testes implementados

### Testes unitarios

`FunctionRegistryTest` cobre:

- registro e listagem;
- lookup case-insensitive;
- execucao de callbacks;
- funcao inexistente;
- conflito de nomes;
- entradas invalidas;
- aridade errada;
- excecao em callback;
- faixa de aridade.

`SemanticResolverTest` cobre:

- resolucao de funcoes registradas;
- registry ausente;
- funcao inexistente com lista de registradas;
- aridade incorreta com origem/plugin;
- erro de callback;
- retorno nao finito;
- troca da referencia de registry.

### Testes de regressao

`ParserExpressionsTest` cobre:

- `1+2`;
- `2*3+4`;
- `(2+3)*4`;
- precedencia e associatividade de operadores;
- funcoes matematicas legadas como `round`, `trunc`, `frac`, `sqrt`, `mod`, `exp`, `log`, `ln`, `min`, `max`, `sin` e `cos`;
- chamadas genericas registradas;
- manutencao de expressoes aritmeticas pelo caminho atual.

### Testes de integracao

`ParserFunctionRegistryDemoTest` cobre:

- `FakeAdd` indisponivel antes do registro;
- `FakeAdd(2,3)` retornando `5` depois de `FakePlugin::registerFunctions(...)`;
- erro controlado para `FakeAdd(1)`;
- erro controlado para `FuncaoInexistente(2)`.

## Validacao executada

Ambiente local: `cmake`, `ctest` e `ninja` foram usados pelo virtualenv temporario `/tmp/genesys-cmake-venv`.

Comando:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-unit
```

Resultado real:

- passou;
- saida: `ninja: no work to do.`

Comando:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest\.(ArithmeticPrecedenceAndParentheses|OperatorAssociativityMatchesCurrentGrammar|PowerIsRightAssociative|MathFunctionsRoundTruncFracAndSqrt|GenericFunctionCallsResolveThroughFunctionRegistry|GenericFunctionCallsReportSemanticErrors|GenericFunctionIntegrationKeepsLegacyArithmeticExpressions)|FunctionRegistryTest|ParserDriverFunctionRegistryTest|SemanticResolverTest|ParserFunctionRegistryDemoTest' --output-on-failure
```

Resultado real:

- 32/32 testes passaram;
- 0 falhas.

Comando:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest|ParserDriverThrowsFalseTest' --output-on-failure
```

Resultado real:

- 15/18 testes passaram;
- 3 falhas conhecidas permaneceram em testes de indices multidimensionais:
  - `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDReads`;
  - `ParserExpressionsTest.AttributeIndexesSupportLegacyAndNDReadsAndAssignmentsDuringEvent`;
  - `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDAssignments`.

Essas falhas retornam `0` para leituras ou atribuicoes com chaves como `1,2`, `1,2,3` e `1,2,3,4,5`. Elas nao foram corrigidas nesta etapa porque nao fazem parte da integracao do `FunctionRegistry`.

## Estabilizacao final antes da entrega

### Estado Git verificado

Comandos executados:

```bash
git branch --show-current
git status --short --branch
git log --oneline --decorate -n 10
git rev-parse --abbrev-ref --symbolic-full-name @{u}
git rev-list --left-right --count HEAD...origin/dcs-parser-function-registry
```

Resultados reais:

- branch atual: `dcs-parser-function-registry`;
- tracking: `origin/dcs-parser-function-registry`;
- relacao com origin: `0 0`, sem commits locais a frente ou atras;
- worktree limpo antes da atualizacao desta documentacao;
- ultimos commits DCS observados:
  - `1338c98d docs: documenta arquitetura e validacao do parser extensivel`;
  - `9ebb790a feat: adiciona FakePlugin para demonstrar extensibilidade`;
  - `00a520f7 feat: adiciona chamada generica de funcao ao parser`;
  - `79f4fee3 feat: implementa SemanticResolver`;
  - `5381d603 feat: integra FunctionRegistry ao parser driver`;
  - `fdaf1628 feat: adiciona FunctionRegistry`;
  - `0e142730 test: adiciona testes de regressao do parser`;
  - `97d5bfc8 docs: documenta inventario do parser atual`;
  - `097ee029 docs: registra baseline inicial do desenvolvimento`.

### Presets CMake encontrados

Presets de configuracao relevantes em `CMakePresets.json`:

- `tests-unit`;
- `tests-kernel-unit`;
- `terminal-app`;
- `terminal-smart`;
- `terminal-example`;
- `terminal-smart-hold-search-remove`;
- `web-app`;
- `genesys_web_app`;
- `gui-app`;
- `tests-smoke`.

Presets de build relevantes:

- `terminal-app`: build principal do terminal `genesys_terminal_application`;
- `tests-unit`: build dos testes unitarios agregados;
- `tests-smoke`: build dos smoke tests.

Presets de teste relevantes:

- `tests-unit`;
- `tests-kernel-unit`;
- `tests-smoke`.

Nao foi encontrado preset especifico para parser. A validacao de parser foi feita por filtro CTest sobre `ParserExpressionsTest`, `ParserDriverThrowsFalseTest` e os testes DCS.

### Comandos e resultados da estabilizacao

Build principal:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset terminal-app
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset terminal-app
```

Resultado real:

- configuracao passou;
- build passou;
- alvo terminal linkado: `source/applications/terminal/genesys_terminal_application`.

Build unitario:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-unit
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-unit
```

Resultado real:

- configuracao passou;
- build passou;
- saida do build: `ninja: no work to do.`

Teste unitario completo por preset:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --preset tests-unit
```

Resultado real:

- execucao interrompida manualmente com codigo 130 apos ficar sem progresso visivel por varios ciclos;
- antes da interrupcao foram observadas falhas fora do escopo direto do DCS:
  - `SimulatorRuntimeTest.EntityAttributeValuesRoundTripByNameAndIndex`, excecao `stoul`;
  - `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDReads`;
  - `ParserExpressionsTest.AttributeIndexesSupportLegacyAndNDReadsAndAssignmentsDuringEvent`;
  - `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDAssignments`;
  - `RuntimePluginManagerClassTest.DummyConnectorRegistersConcreteModelPlugins`, falha envolvendo `dummyelement.so`.

Testes focados do DCS:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest\.(ArithmeticPrecedenceAndParentheses|OperatorAssociativityMatchesCurrentGrammar|PowerIsRightAssociative|MathFunctionsRoundTruncFracAndSqrt|GenericFunctionCallsResolveThroughFunctionRegistry|GenericFunctionCallsReportSemanticErrors|GenericFunctionIntegrationKeepsLegacyArithmeticExpressions)|FunctionRegistryTest|ParserDriverFunctionRegistryTest|SemanticResolverTest|ParserFunctionRegistryDemoTest' --output-on-failure
```

Resultado real:

- 32/32 testes passaram;
- 0 falhas.

Regressao especifica do parser:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest|ParserDriverThrowsFalseTest' --output-on-failure
```

Resultado real:

- 15/18 testes passaram;
- 3 falhas remanescentes, todas ligadas aos testes de indices multidimensionais ja listados acima.

Smoke:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-smoke
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-smoke
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --preset tests-smoke
```

Resultado real:

- configuracao passou;
- build passou;
- `ctest --preset tests-smoke`: 1/1 teste passou (`smoke_simulator_start`).

Verificacao de arquivos temporarios e whitespace:

```bash
git ls-files -o --exclude-standard
git diff --check
```

Resultado real antes desta atualizacao documental:

- nenhum arquivo nao rastreado apareceu apos os builds;
- `git diff --check` passou.

### Como reproduzir a demonstracao FakeAdd

Executar o teste de demonstracao:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-unit
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-unit --target genesys_test_parser_function_registry_demo
./build/tests-unit/source/tests/unit/genesys_test_parser_function_registry_demo
```

Fluxo demonstrado pelo teste:

1. criar um `FunctionRegistry`;
2. tentar avaliar `FakeAdd(2,3)` antes do registro e receber erro controlado;
3. chamar `FakePlugin::registerFunctions(registry)`;
4. configurar o registry no `ParserDefaultImpl2`;
5. avaliar `FakeAdd(2,3)` e obter `5`.

O nome `FakeAdd` nao possui token especifico no lexer nem producao especifica no Bison. Ele entra pelo token generico `IDENTIFIER` e e resolvido pelo `SemanticResolver`.

### Checklist final para PR

- Build principal `terminal-app`: passou.
- Build `tests-unit`: passou.
- Build/teste `tests-smoke`: passou, 1/1.
- Testes novos e focados do DCS: passaram, 32/32.
- Regressao especifica do parser: 15/18, com 3 falhas ND remanescentes documentadas.
- Teste unitario completo `ctest --preset tests-unit`: interrompido manualmente; havia falhas fora do DCS antes da interrupcao.
- Documentacao DCS presente.
- Branch atualizada com `origin/dcs-parser-function-registry` no momento da verificacao.
- Sem arquivos temporarios detectados apos build/testes antes desta atualizacao documental.
- Sem arquivos de build versionados indevidamente detectados.
- Commits DCS observados e organizados por etapa.

## Limitacoes conhecidas

- Argumentos e retorno de funcoes registradas ainda sao `double`.
- O parser ainda avalia argumentos diretamente nas acoes semanticas do Bison.
- Nao ha AST explicita nem avaliacao tardia geral.
- `Model::parseExpression(...)` ainda nao expoe API publica para configurar `FunctionRegistry`; os testes de integracao usam `ParserDefaultImpl2` diretamente.
- `FakePlugin` e uma classe de demonstracao em testes, nao um plugin real carregado dinamicamente.
- Funcoes legadas tem precedencia lexical sobre chamadas genericas.
- Regeneracao dinamica completa de Flex/Bison nao foi implementada.
- Rollback e ciclo de vida de plugins dinamicos ainda nao foram tratados.

## Trabalho futuro

1. Migrar gradualmente funcoes legadas como `NQ`, `MR`, `NR` e `NUMSET` para o registry, mantendo testes de equivalencia com o caminho atual.
2. Integrar plugins reais ao `FunctionRegistry`, definindo quando e onde cada plugin registra suas funcoes.
3. Expor uma API de configuracao do registry em nivel apropriado do modelo ou simulador, se necessario.
4. Usar `ParserChangesInformation` apenas para casos que realmente exijam sintaxe nova, e nao para cada funcao numerica simples.
5. Avaliar regeneracao dinamica de Flex/Bison como etapa posterior, depois de estabilizar contrato, cobertura e politica de erro.
6. Definir rollback, descarregamento e substituicao de funcoes registradas quando houver carregamento dinamico real.
7. Avaliar suporte a tipos alem de `double`, contexto de simulacao e acesso controlado a elementos do modelo.

## Conclusao

A entrega implementa um caminho extensivel minimo e testado para chamadas de funcao por nome. O parser continua compativel com a gramatica legada, mas agora possui um ponto de extensao para funcoes novas sem token especifico.

O resultado nao substitui a infraestrutura historica de alteracao dinamica do parser. Ele cria uma camada mais simples e verificavel para o caso mais comum: registrar uma funcao, validar aridade e executar um callback a partir de uma expressao textual.

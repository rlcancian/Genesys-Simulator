# DCS: parser extensivel por FunctionRegistry

## Visao geral

Esta documentacao consolida a implementacao do DCS para tornar o parser do
GenESyS mais extensivel no suporte a funcoes usadas em expressoes textuais.

A entrega cria uma camada incremental de registro e resolucao de funcoes:

- `FunctionRegistry` armazena descritores e callbacks de funcoes;
- `SemanticResolver` valida e executa chamadas por nome;
- `genesyspp_driver` recebe uma referencia opcional para um registry externo;
- o parser aceita chamadas genericas no formato `IDENTIFIER(...)`;
- funcoes legadas continuam pelo caminho existente de Flex/Bison.

O objetivo nao foi substituir toda a infraestrutura historica de extensao do
parser, mas criar um caminho minimo, testavel e menos acoplado para funcoes
numericas novas.

## Objetivo da implementacao

O Tema 12 do DCS trata da extensibilidade do parser do GenESyS. O problema
inicial era que novas funcoes dependiam, em geral, de alteracoes diretas em
lexer, tokens, producoes Bison e acoes semanticas C++.

A implementacao busca permitir que funcoes numericas simples sejam registradas
por nome, aridade, origem e callback, sem criar um token especifico para cada
funcao nova.

A estrategia adotada foi incremental:

1. registrar um baseline de desenvolvimento;
2. inventariar o parser existente;
3. adicionar testes de regressao;
4. implementar `FunctionRegistry`;
5. integrar o registry ao driver do parser;
6. criar `SemanticResolver`;
7. adicionar chamada generica de funcao no parser;
8. demonstrar extensibilidade com `FakePlugin`.

## Contexto do DCS no GenESyS

O parser atual usa Flex/Bison com valores semanticos do tipo `obj_t`. O scanner
recebe o `genesyspp_driver`, reconhece palavras reservadas e literais, e retorna
tokens especificos para a gramatica. A gramatica avalia expressoes diretamente
nas acoes semanticas C++ por meio de `$$.valor`.

Nao foi identificada uma AST explicita para expressoes. A avaliacao ainda ocorre
durante o parsing, incluindo operadores, funcoes matematicas, funcoes
probabilisticas, elementos de kernel, variaveis, atributos, formulas e funcoes
de plugins conhecidos.

Antes desta implementacao, o padrao lexical `{L}` tentava classificar um literal
consultando o modelo e retornava tokens concretos como `ATRIB`, `CSTAT`,
`COUNTER`, `SIMRESP`, `SIMCTRL`, `VARI`, `FORM`, `QUEUE`, `RESOURCE` ou `SET`.
Quando nenhum caso era encontrado, o literal era tratado como ilegal. Com a nova
integracao, um literal nao reconhecido pode alimentar o token generico
`IDENTIFIER`, permitindo chamadas como `FakeAdd(2,3)` quando houver funcao
registrada.

## Problema original

O parser original reconhecia muitas funcoes por tokens especificos em Flex/Bison.
Funcoes matematicas, probabilisticas, de kernel e de plugins conhecidos
apareciam como regras lexicais e producoes dedicadas.

Exemplos de acoplamento encontrados:

- `nq`, `firstinq`, `mr`, `nr`, `numset` e outras funcoes eram regras lexicais
  explicitas em `lexerparser.ll`;
- `fNQ`, `fMR`, `fNR`, `fNUMSET`, `QUEUE`, `RESOURCE`, `SET`, `VARI` e `FORM`
  eram tokens especificos em `bisonparser.yy`;
- producoes como `pluginFunction`, `variable` e `formula` faziam casts diretos
  para classes de plugin;
- a avaliacao ocorria diretamente nas acoes semanticas do Bison.

Com isso, adicionar uma funcao nova exigia alterar lexer, tokens, producoes e,
em alguns casos, includes e casts especificos. O parser ficava sintaticamente
acoplado a plugins conhecidos.

## O que foi implementado

A implementacao adiciona:

- descritores de funcao com nome publico, aridade minima, aridade maxima,
  origem/plugin, descricao e categoria;
- registro, lookup, validacao de conflito, validacao de aridade, execucao de
  callback e listagem por meio de `FunctionRegistry`;
- resultados estruturados para registro e chamada de funcao;
- `FunctionCallRequest` como representacao minima de chamada por nome e
  argumentos numericos;
- `SemanticResolverResult` e `SemanticResolver` para validar registry ausente,
  funcao inexistente, aridade incorreta, erro de callback e retorno nao finito;
- referencia opcional de `FunctionRegistry` em `genesyspp_driver`;
- ponte em `ParserDefaultImpl2` para configurar o registry usado pelo parser;
- token generico `IDENTIFIER` e producoes para chamadas genericas de funcao;
- testes unitarios, regressivos e de demonstracao com `FakePlugin`.

O lookup do registry e case-insensitive, alinhado ao comportamento conceitual do
lexer atual, que reconhece nomes de funcoes em letras maiusculas ou minusculas.

## Arquitetura geral da solucao

A decisao arquitetural foi separar, dentro do escopo possivel, reconhecimento
sintatico e resolucao semantica de funcoes registradas. O parser ainda avalia os
argumentos diretamente como `double`, mas a funcao chamada pode ser resolvida
por nome e aridade fora de Flex/Bison.

Fluxo atual da chamada generica:

```text
Expressao textual
  -> lexer retorna IDENTIFIER para literal nao reconhecido
  -> Bison reconhece IDENTIFIER '(' argumentList ')'
  -> Bison avalia argumentos como double
  -> SemanticResolver consulta FunctionRegistry
  -> callback registrado retorna double ou erro controlado
```

O parser nao conhece a origem concreta da funcao. Na demonstracao, `FakePlugin`
registra `FakeAdd` e `FakeSquare`, mas Flex/Bison nao possuem tokens especificos
para esses nomes.

## Function Registry

O `FunctionRegistry` fica em `source/parser`. A escolha foi manter a primeira
versao proxima do parser/evaluator, sem transforma-la ainda em servico geral do
simulador nem acopla-la a plugins concretos.

A primeira versao registra funcoes numericas simples com:

- nome publico;
- aridade minima;
- aridade maxima;
- origem ou plugin;
- descricao curta;
- categoria opcional;
- callback `double(const std::vector<double>&)`.

Operacoes oferecidas:

- `registerFunction(...)`;
- `lookup(...)`;
- `hasFunction(...)`;
- `callFunction(...)`;
- `listFunctions()`.

O registry valida entradas invalidas, conflitos de nome, funcao inexistente,
aridade errada e excecoes de callback.

## Integracao com o parser

A integracao inicial adiciona chamadas genericas no formato:

```text
IDENTIFIER '(' argumentList ')'
IDENTIFIER '(' ')'
```

O lexer continua priorizando tokens legados, palavras reservadas e elementos
conhecidos do modelo. Apenas o fallback de literal nao reconhecido passa a poder
retornar `IDENTIFIER`.

Quando um identificador aparece sozinho, a gramatica preserva a mensagem legada
de literal nao encontrado. Quando aparece como chamada de funcao, o Bison delega
a resolucao para `SemanticResolver`, usando o `FunctionRegistry` configurado no
driver.

Funcoes antigas continuam reconhecidas por tokens e producoes existentes. Uma
funcao legada tem precedencia lexical sobre um identificador generico, mesmo que
exista uma funcao registrada com o mesmo nome.

Ao alterar `.yy` ou `.ll`, o build normal usa fontes gerados versionados; portanto
tambem e necessario regenerar `GenesysParser.*` e `Genesys++-scanner.cpp`.

## Integracao com o resolvedor semantico

O `SemanticResolver` fica entre a chamada reconhecida pelo parser e o
`FunctionRegistry`. Para uma chamada como `FakeAdd(2,3)`, ele:

1. verifica se ha registry configurado;
2. procura a funcao por nome;
3. valida a aridade recebida;
4. executa o callback registrado;
5. valida retorno numerico finito;
6. retorna o valor numerico ou uma mensagem de erro controlada.

As mensagens incluem o nome da funcao, a quantidade de argumentos, a aridade
esperada, a origem/plugin quando disponivel e a lista de funcoes registradas
quando a funcao nao existe.

## Ponte com o driver e o wrapper do parser

O `genesyspp_driver` possui uma referencia opcional para `FunctionRegistry`.
Essa referencia nao transfere ownership: o driver guarda apenas um ponteiro
externo, seguindo o estilo usado para `Model*` e `Sampler_if*`.

Metodos adicionados ao driver:

- `setFunctionRegistry(...)`;
- `getFunctionRegistry()`;
- `hasFunctionRegistry()`.

`ParserDefaultImpl2` apenas encaminha a referencia externa para o wrapper do
parser. O ciclo de vida do registry continua sendo responsabilidade do chamador.

## Demonstracao com FakePlugin

`FakePlugin` fica em `source/tests/unit/fakes/FakePlugin.h`. Ele nao e um plugin
real carregado pelo `PluginManager`; e uma classe de teste que valida o contrato
minimo esperado para um plugin futuro: receber um `FunctionRegistry` externo e
registrar funcoes publicas nele.

`FakePlugin::registerFunctions(FunctionRegistry&)` registra:

- `FakeAdd(a,b)`, que retorna `a+b`;
- `FakeSquare(x)`, que retorna `x*x`.

O teste de demonstracao mostra que `FakeAdd(2,3)` falha antes do registro e
passa depois que `FakePlugin::registerFunctions(...)` popula o registry. O nome
`FakeAdd` nao possui token especifico no lexer nem producao especifica no Bison:
ele entra pelo token generico `IDENTIFIER` e e resolvido pelo `SemanticResolver`.

## Relacao com a infraestrutura original

### ParserChangesInformation

`ParserChangesInformation` foi preservado. Ele representa a infraestrutura
historica para armazenar trechos textuais de parser, como includes, tokens, type
objects, expressoes, assignments e producoes de funcao.

O DCS nao remove essa estrutura porque ela ainda pode ser util quando um plugin
realmente precisar introduzir sintaxe nova. A proposta atual cobre funcoes
numericas por chamada generica, nao todos os casos de extensao sintatica.

### ParserManager

`ParserManager` tambem foi preservado. O inventario mostrou que ele declara
pontos para gerar e conectar novo parser, mas nao havia implementacao suficiente
para depender desse fluxo como primeira entrega.

A solucao atual evita exigir geracao dinamica em runtime e permite validar
comportamento com testes unitarios e de integracao.

### Marcadores em bisonparser.yy

Os marcadores existentes foram mantidos:

- `begin_Includes_plugins`;
- `begin_Tokens_plugins`;
- `begin_TypeObj_plugins`;
- `begin_Expression_plugins`;
- `begin_ExpressionProdution_plugins`;
- `begin_Assignment_plugins`;
- `begin_FunctionProdution_plugins`.

Eles continuam importantes para compatibilidade com o mecanismo historico de
insercao textual. A chamada generica foi adicionada de forma localizada, sem
substituir os blocos legados.

### Marcadores em lexerparser.ll

Os marcadores existentes no lexer tambem foram mantidos:

- `begin_Includes_plugins`;
- `begin_Lexical_plugins`;
- `begin_LexicalLiterals_plugins`.

O lexer continua priorizando palavras reservadas, funcoes antigas e elementos
conhecidos do modelo. O fallback de literal nao reconhecido passou a poder
alimentar o token generico `IDENTIFIER`.

## Por que nao iniciar por regeneracao dinamica completa

A regeneracao dinamica completa de Flex/Bison foi preservada como possibilidade
futura, mas nao foi escolhida como primeira entrega.

Motivos tecnicos documentados:

- `ParserManager::generateNewParser(...)` e `connectNewParser(...)` existem como
  interface, mas a implementacao atual ainda e minima;
- `ParserChangesInformation` armazena fragmentos textuais para o parser, o que
  exige sincronizar includes, tokens, tipos, regras lexicais e producoes;
- `GENESYS_PARSER_REGENERATE` existe no CMake, mas fica desligado por padrao;
- a gramatica atual mistura parsing, resolucao de simbolos e avaliacao em acoes
  C++;
- havia tokens reconhecidos sem producao semantica observada no inventario;
- a ativacao dinamica exigiria tratar toolchain, compilacao, link, rollback,
  erros e ciclo de vida em runtime.

A abordagem por registry reduz risco porque primeiro cria um contrato testavel
para funcoes. A migracao de funcoes legadas pode ocorrer uma por vez.

## Arquivos e classes principais envolvidos

Infraestrutura do parser:

- `source/parser/FunctionRegistry.h`;
- `source/parser/FunctionRegistry.cpp`;
- `source/parser/SemanticResolver.h`;
- `source/parser/SemanticResolver.cpp`;
- `source/parser/Genesys++-driver.h`;
- `source/parser/Genesys++-driver.cpp`;
- `source/parser/parserBisonFlex/bisonparser.yy`;
- `source/parser/parserBisonFlex/lexerparser.ll`;
- `source/parser/GenesysParser.h`;
- `source/parser/GenesysParser.cpp`;
- `source/parser/Genesys++-scanner.cpp`.

Ponte com o wrapper do parser:

- `source/kernel/simulator/ParserDefaultImpl2.h`;
- `source/kernel/simulator/ParserDefaultImpl2.cpp`.

Testes:

- `source/tests/unit/test_parser_function_registry.cpp`;
- `source/tests/unit/test_parser_semantic_resolver.cpp`;
- `source/tests/unit/test_parser_expressions.cpp`;
- `source/tests/unit/test_parser_function_registry_demo.cpp`;
- `source/tests/unit/fakes/FakePlugin.h`;
- `source/tests/unit/CMakeLists.txt`.

Principais estruturas criadas:

- `FunctionDescriptor`;
- `FunctionRegistrationResult`;
- `FunctionCallResult`;
- `FunctionRegistry`;
- `FunctionCallRequest`;
- `SemanticResolverResult`;
- `SemanticResolver`;
- `FakePlugin`.

## Inventario do parser legado

O inventario inicial analisou:

- `source/parser/parserBisonFlex/bisonparser.yy`;
- `source/parser/parserBisonFlex/lexerparser.ll`;
- `source/parser/Genesys++-driver.h`;
- `source/parser/Genesys++-driver.cpp`;
- `source/parser/obj_t.h`;
- `source/parser/obj_t.cpp`;
- `source/parser/CMakeLists.txt`;
- `source/kernel/simulator/ParserChangesInformation.h`;
- `source/kernel/simulator/ParserChangesInformation.cpp`;
- `source/kernel/simulator/ParserManager.h`;
- `source/kernel/simulator/ParserManager.cpp`;
- `source/kernel/simulator/ModelDataDefinition.h`.

Categorias observadas no parser legado:

- funcoes matematicas: `round`, `mod`, `trunc`, `frac`, `exp`, `sqrt`, `log`,
  `ln`, `min`, `max`, `sin`, `cos`;
- funcoes probabilisticas: `rnd`, `expo`, `norm`, `unif`, `weib`, `logn`,
  `gamm`, `erla`, `tria`, `beta`, `disc`;
- `SimulationResponse` e `SimulationControl`;
- `Entity` e `Attribute`;
- `Queue`;
- `Resource`;
- `Set`;
- `Variable`;
- `Formula`;
- funcoes de string reconhecidas como tokens, como `val`, `eval` e `leng`;
- comandos algoritmicos como `if`, `else`, `for`, `to` e `do`;
- elementos de kernel como `tavg` e `count`;
- tokens relacionados a `EntityGroup`, como `numgr` e `atrgr`;
- producao `userFunction` para `"USER" "(" expression ")"`;
- token `CTEZERO`.

Observacoes do inventario:

- algumas funcoes ou tokens eram reconhecidos no lexer e declarados no Bison,
  mas nao tinham producao semantica observada no arquivo analisado;
- `disc(listaparm)` retornava `sampleDiscrete(0, 0)` e estava marcado como nao
  implementado;
- a producao de `Formula` recuperava expressao textual, mas retornava `0.0`, com
  comentario sobre problema de reentrada;
- `lastinq(QUEUE)`, `resseizes(RESOURCE)`, `fENTATRANK`, `fRESUTIL`, `fVAL`,
  `fEVAL`, `fLENG`, `fNUMGR`, `fATRGR`, `USER` e `CTEZERO` tinham limitacoes ou
  conexoes incompletas conforme o inventario original.

Estruturas semelhantes a registry encontradas antes da implementacao:

- `ParserChangesInformation`, como armazenamento de fragmentos textuais;
- `ParserManager`, com pontos declarados para gerar e conectar parser;
- `ModelDataDefinition::_getParserChangesInformation()`, como hook virtual para
  plugins retornarem mudancas necessarias no parser;
- `genesyspp_driver::_referedDataElements`, para rastrear dependencias de
  elementos do modelo;
- `findSimulationResponse` e `findSimulationControl`, como helpers especificos.

Nao havia, no inventario inicial, `FunctionRegistry` para chamadas de funcao,
resolver generico por nome/assinatura, callback registrado por plugin, tabela de
simbolos generica para identificadores, AST explicita ou avaliacao tardia geral.

## Fluxo de funcionamento

Fluxo para uma funcao nova registrada:

1. um componente externo cria ou recebe um `FunctionRegistry`;
2. esse componente chama `registerFunction(...)` com descritor e callback;
3. o chamador configura o registry no parser por `ParserDefaultImpl2` ou pelo
   `genesyspp_driver`;
4. o lexer encontra um literal nao reconhecido e retorna `IDENTIFIER`;
5. o Bison reconhece a chamada `IDENTIFIER(...)`;
6. os argumentos sao avaliados como `double`;
7. o Bison chama o `SemanticResolver`;
8. o resolver consulta o registry e valida a chamada;
9. o callback retorna o valor numerico;
10. o valor volta para a expressao como resultado da chamada.

Fluxo demonstrado por `FakeAdd(2,3)`:

1. avaliar `FakeAdd(2,3)` antes do registro gera erro controlado;
2. `FakePlugin::registerFunctions(registry)` registra `FakeAdd` e `FakeSquare`;
3. o registry e configurado no `ParserDefaultImpl2`;
4. `FakeAdd(2,3)` retorna `5`.

## Compatibilidade

- Funcoes antigas continuam reconhecidas pelos tokens e producoes existentes.
- Funcoes de Queue, Resource, Set, Variable e Formula nao foram removidas nem
  migradas nesta etapa.
- Expressoes aritmeticas antigas continuam cobertas pelos testes de regressao
  focados.
- `FakeAdd(2,3)` funciona pelo novo caminho generico quando registrado no
  `FunctionRegistry`.
- Antes do registro, `FakeAdd(2,3)` falha com erro controlado de funcao nao
  registrada.
- O registry permite extensoes futuras por nome, aridade, origem/plugin e
  callback, sem criar token especifico para cada funcao nova.

## Como validar e testar

Os presets relevantes documentados sao:

- configuracao: `tests-unit`, `tests-kernel-unit`, `terminal-app`,
  `terminal-smart`, `terminal-example`, `terminal-smart-hold-search-remove`,
  `web-app`, `genesys_web_app`, `gui-app`, `tests-smoke`;
- build: `terminal-app`, `tests-unit`, `tests-smoke`;
- teste: `tests-unit`, `tests-kernel-unit`, `tests-smoke`.

Nao foi encontrado preset especifico para parser. A validacao de parser foi feita
por filtros CTest sobre `ParserExpressionsTest`, `ParserDriverThrowsFalseTest` e
os testes DCS.

Em ambiente local, os documentos originais registraram uso de um virtualenv
temporario em `/tmp/genesys-cmake-venv`, porque `cmake`, `ctest` e `ninja` nao
estavam disponiveis no `PATH` do sistema. Os comandos abaixo preservam esse
prefixo historico:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset terminal-app
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset terminal-app
```

Resultado documentado:

- configuracao passou;
- build passou;
- alvo terminal linkado: `source/applications/terminal/genesys_terminal_application`.

Build unitario:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-unit
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-unit
```

Resultado documentado:

- configuracao passou;
- build passou;
- saida do build: `ninja: no work to do.`

Testes focados do DCS:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest\.(ArithmeticPrecedenceAndParentheses|OperatorAssociativityMatchesCurrentGrammar|PowerIsRightAssociative|MathFunctionsRoundTruncFracAndSqrt|GenericFunctionCallsResolveThroughFunctionRegistry|GenericFunctionCallsReportSemanticErrors|GenericFunctionIntegrationKeepsLegacyArithmeticExpressions)|FunctionRegistryTest|ParserDriverFunctionRegistryTest|SemanticResolverTest|ParserFunctionRegistryDemoTest' --output-on-failure
```

Resultado documentado:

- 32/32 testes passaram;
- 0 falhas.

Regressao especifica do parser:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'ParserExpressionsTest|ParserDriverThrowsFalseTest' --output-on-failure
```

Resultado documentado:

- 15/18 testes passaram;
- 3 falhas remanescentes em testes de indices multidimensionais.

Smoke:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --preset tests-smoke
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build --preset tests-smoke
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --preset tests-smoke
```

Resultado documentado:

- configuracao passou;
- build passou;
- `ctest --preset tests-smoke`: 1/1 teste passou (`smoke_simulator_start`).

Verificacoes auxiliares registradas:

```bash
git ls-files -o --exclude-standard
git diff --check
```

Resultado documentado antes da consolidacao desta documentacao:

- nenhum arquivo nao rastreado apareceu apos os builds;
- `git diff --check` passou.

## Testes automatizados relacionados

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

`ParserDriverFunctionRegistryTest` cobre:

- driver iniciando sem registry;
- configuracao e exposicao de registry externo;
- limpeza da referencia;
- copia e movimento preservando referencia nao-dona.

`SemanticResolverTest` cobre:

- resolucao de funcoes registradas;
- registry ausente;
- funcao inexistente com lista de registradas;
- aridade incorreta com origem/plugin;
- erro de callback;
- retorno nao finito;
- troca da referencia de registry.

`ParserExpressionsTest` cobre:

- `1+2`;
- `2*3+4`;
- `(2+3)*4`;
- precedencia e associatividade de operadores;
- associatividade direita de potencia;
- comportamento atual de unario antes de potencia em `-2^2`;
- funcoes matematicas legadas como `round`, `trunc`, `frac`, `sqrt`, `mod`,
  `exp`, `log`, `ln`, `min`, `max`, `sin` e `cos`;
- chamadas genericas registradas;
- manutencao de expressoes aritmeticas pelo caminho atual.

`ParserFunctionRegistryDemoTest` cobre:

- `FakeAdd` indisponivel antes do registro;
- `FakeAdd(2,3)` retornando `5` depois de
  `FakePlugin::registerFunctions(...)`;
- erro controlado para `FakeAdd(1)`;
- erro controlado para `FuncaoInexistente(2)`.

## Cobertura de regressao adicionada para DCS

A cobertura de regressao do parser foi adicionada ao teste existente
`genesys_test_parser_expressions`, integrado em `source/tests/unit/CMakeLists.txt`.
O helper local `genesys_add_unit_test` cria executaveis com GoogleTest, vincula
`GTest::gtest_main` e registra os casos com `gtest_discover_tests(... LABELS
"unit")`. O alvo participa de `genesys_kernel_unit_tests`.

A cobertura adicionada inclui:

- expressoes aritmeticas simples: `1+2`, `2*3+4`, `(2+3)*4`;
- precedencia de multiplicacao sobre soma;
- associatividade esquerda de subtracao e divisao;
- associatividade direita de potencia;
- comportamento atual de unario antes de potencia em `-2^2`;
- funcoes matematicas reconhecidas pelo parser.

Queue, Resource e Set nao foram cobertos nessa primeira regressao especifica.
Esses casos dependem de objetos de modelo e plugins especificos e devem entrar
como testes de integracao ou testes unitarios com montagem dedicada do modelo.

## Limitacoes conhecidas

- Argumentos e retorno de funcoes registradas ainda sao `double`.
- O parser ainda avalia argumentos diretamente nas acoes semanticas do Bison.
- Nao ha AST explicita nem avaliacao tardia geral.
- `Model::parseExpression(...)` ainda nao expoe API publica para configurar
  `FunctionRegistry`; os testes de integracao usam `ParserDefaultImpl2`
  diretamente.
- `FakePlugin` e uma classe de demonstracao em testes, nao um plugin real
  carregado dinamicamente.
- Funcoes legadas tem precedencia lexical sobre chamadas genericas.
- Regeneracao dinamica completa de Flex/Bison nao foi implementada.
- Rollback e ciclo de vida de plugins dinamicos ainda nao foram tratados.
- Queue, Resource e Set nao foram cobertos na primeira regressao focada de
  expressoes aritmeticas e funcoes matematicas.
- O teste unitario completo `ctest --preset tests-unit` foi interrompido
  manualmente na validacao documentada original, apos falhas fora do escopo
  direto do DCS.

Falhas conhecidas documentadas na regressao especifica do parser:

- `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDReads`;
- `ParserExpressionsTest.AttributeIndexesSupportLegacyAndNDReadsAndAssignmentsDuringEvent`;
- `ParserExpressionsTest.VariableIndexesSupportScalarLegacyAndNDAssignments`.

Essas falhas retornavam `0` para leituras ou atribuicoes com chaves como `1,2`,
`1,2,3` e `1,2,3,4,5`. Elas nao foram corrigidas nesta etapa porque nao fazem
parte da integracao do `FunctionRegistry`.

Falhas fora do escopo direto do DCS observadas antes da interrupcao do
`ctest --preset tests-unit`:

- `SimulatorRuntimeTest.EntityAttributeValuesRoundTripByNameAndIndex`, com
  excecao `stoul`;
- os tres testes de indices multidimensionais listados acima;
- `RuntimePluginManagerClassTest.DummyConnectorRegistersConcreteModelPlugins`,
  envolvendo `dummyelement.so`.

## Pontos futuros

1. Migrar gradualmente funcoes legadas como `NQ`, `MR`, `NR` e `NUMSET` para o
   registry, mantendo testes de equivalencia com o caminho atual.
2. Integrar plugins reais ao `FunctionRegistry`, definindo quando e onde cada
   plugin registra suas funcoes.
3. Expor uma API de configuracao do registry em nivel apropriado do modelo ou
   simulador, se necessario.
4. Usar `ParserChangesInformation` apenas para casos que realmente exijam sintaxe
   nova, e nao para cada funcao numerica simples.
5. Avaliar regeneracao dinamica de Flex/Bison como etapa posterior, depois de
   estabilizar contrato, cobertura e politica de erro.
6. Definir rollback, descarregamento e substituicao de funcoes registradas quando
   houver carregamento dinamico real.
7. Avaliar suporte a tipos alem de `double`, contexto de simulacao e acesso
   controlado a elementos do modelo.

## Baseline historico do desenvolvimento

O baseline documental inicial registrou:

- branch atual no momento do registro: `dcs-parser-function-registry`;
- branch base assumida: `currentStable`;
- verificacao de ancestralidade: `currentStable` era ancestral de `HEAD`;
- remoto `origin`: `https://github.com/joaomdaltoe/Genesys-Simulator.git`;
- remoto `upstream`: `https://github.com/rlcancian/Genesys-Simulator`;
- estado local inicial sem alteracoes locais reportadas por
  `git status --short --branch`.

As restricoes iniciais daquela etapa eram nao modificar parser, kernel, CMake,
criar classes novas, executar refatoracao ou fazer commit automaticamente. Essas
restricoes pertenciam ao baseline inicial de levantamento; a implementacao final
foi conduzida depois em etapas incrementais documentadas.

## Checklist de PR documentado

Checklist registrado antes desta consolidacao documental:

- build principal `terminal-app`: passou;
- build `tests-unit`: passou;
- build/teste `tests-smoke`: passou, 1/1;
- testes novos e focados do DCS: passaram, 32/32;
- regressao especifica do parser: 15/18, com 3 falhas ND remanescentes
  documentadas;
- teste unitario completo `ctest --preset tests-unit`: interrompido manualmente,
  com falhas fora do DCS observadas antes da interrupcao;
- branch estava atualizada com `origin/dcs-parser-function-registry` no momento
  da verificacao;
- sem arquivos temporarios detectados apos build/testes antes da atualizacao
  documental;
- sem arquivos de build versionados indevidamente detectados.

## Conclusao

A entrega implementa um caminho extensivel minimo e testado para chamadas de
funcao por nome. O parser continua compativel com a gramatica legada, mas agora
possui um ponto de extensao para funcoes novas sem token especifico.

O resultado nao substitui a infraestrutura historica de alteracao dinamica do
parser. Ele cria uma camada mais simples e verificavel para o caso mais comum:
registrar uma funcao, validar aridade e executar um callback a partir de uma
expressao textual.

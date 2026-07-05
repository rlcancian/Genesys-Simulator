# DCS: parser extensivel por FunctionRegistry

## Visao geral

Esta documentacao consolida a implementacao do DCS - Tema 12.3.2 - para tornar o parser do
GenESyS mais extensivel no suporte a funcoes usadas em expressoes textuais.

A entrega cria uma camada incremental de registro e resolucao de funcoes:

- `FunctionRegistry` armazena descritores e callbacks de funcoes;
- `SemanticResolver` valida e executa chamadas por nome;
- `Simulator` e o dono do `FunctionRegistry` usado pelos modelos;
- `PluginInformation` permite que plugins declarem funcoes opcionais;
- `PluginManager` registra e remove funcoes de plugins no ciclo de insert/remove;
- `genesyspp_driver` recebe uma referencia nao-dona para o registry do simulador;
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
8. demonstrar extensibilidade com um plugin fixture carregado via `PluginManager`.

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

Exemplos de acoplamentos:

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
- ownership do registry em `Simulator`;
- ponte em `ParserDefaultImpl2` para usar o registry do simulador pai do modelo;
- declaracao opcional de funcoes em `PluginInformation`;
- registro e remocao dessas funcoes pelo `PluginManager`;
- token generico `IDENTIFIER` e producoes para chamadas genericas de funcao;
- testes unitarios, regressivos e de integracao atraves do `PluginManager`.

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

O parser nao conhece a origem concreta da funcao. Na demonstracao atual, um
plugin fixture declara `PluginAdd`, mas Flex/Bison nao possui token especifico
para esse nome.

## Function Registry e ownership

O `FunctionRegistry` fica em `source/parser`. A escolha foi manter a primeira
versao proxima do parser/evaluator, sem acopla-la a plugins concretos.

O ownership em runtime fica no `Simulator`. Cada simulador possui um registry
unico, exposto por `Simulator::getFunctionRegistry()`. O `PluginManager` do
simulador alimenta esse registry quando plugins sao inseridos, e os parsers dos
modelos recebem uma referencia nao-dona para o mesmo registry. Assim,
`Model::parseExpression(...)` usa as funcoes registradas pelos plugins
carregados no simulador, em vez de criar registries isolados por parser.

O registry registra funcoes numericas simples com:

- nome publico;
- aridade minima;
- aridade maxima;
- origem ou plugin;
- descricao curta;
- categoria opcional;
- callback `double(const std::vector<double>&)`.

Operacoes oferecidas:

- `registerFunction(...)`;
- `unregisterFunction(...)`;
- `unregisterFunctionsByOrigin(...)`;
- `lookup(...)`;
- `hasFunction(...)`;
- `callFunction(...)`;
- `listFunctions()`.

O registry valida entradas invalidas, conflitos de nome, funcao inexistente,
aridade errada e excecoes de callback. O lookup e o conflito de nomes sao
case-insensitive. Um novo registro conflitante e rejeitado; a funcao anterior
nao e sobrescrita.

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
parser. Quando construido para um `Model`, ele usa automaticamente
`model->getParentSimulator()->getFunctionRegistry()`. O driver e o parser nao
possuem ownership sobre o registry.

## Contrato de funcoes em plugins

Plugins declaram funcoes opcionais por `PluginInformation::insertParserFunction(...)`.
Cada declaracao contem `FunctionDescriptor` e `FunctionCallback`. Plugins que
nao chamam esse metodo continuam compativeis e nao alteram o parser.

No momento da insercao, `PluginManager::_insert(...)` valida dependencias
dinamicas e de sistema primeiro. Somente depois registra as funcoes declaradas
no registry do `Simulator`, preenchendo `originName` com o `pluginTypename`.
Se qualquer funcao falhar por conflito, callback vazio, aridade invalida ou
outro erro de registro, a insercao do plugin e rejeitada e as funcoes ja
registradas naquela tentativa sao removidas.

Na remocao por `PluginManager::remove(Plugin*)`, a desconexao precisa passar.
Depois disso, as funcoes daquele plugin sao removidas por origem antes de o
wrapper do plugin ser destruido. O overload
`PluginManager::remove(const std::string&)` recebe o mesmo identificador de
arquivo usado em `insert(...)`, por exemplo `delay.so`; o `PluginManager`
resolve esse arquivo para o `pluginTypename` via conector e remove o plugin
carregado correspondente. Chamadas posteriores, como `PluginAdd(2,3)`, passam a
falhar com erro semantico controlado de funcao nao registrada.

## Demonstracao com PluginManager

`ParserFunctionRegistryDemoTest` usa um `PluginConnector_if` fake de teste para
passar pelo fluxo real do `PluginManager`, sem depender de biblioteca dinamica
no ambiente unitario.

O plugin fixture declara em `PluginInformation`:

- `PluginAdd(a,b)`, que retorna `a+b`.

O teste mostra que `PluginAdd(2,3)` falha antes de
`PluginManager::insert("parser_function_plugin.so")` e passa depois da insercao.
Tambem cobre unload/remocao, conflito case-insensitive contra outro plugin e
aridade incorreta. O nome `PluginAdd` nao possui token especifico no lexer nem
producao especifica no Bison: ele entra pelo token generico `IDENTIFIER` e e
resolvido pelo `SemanticResolver`.

## Validacao com plugins reais

`PluginManagerRealComponentsTest` cobre a convivencia do novo registry com
plugins reais existentes do GenESyS carregados pelo fluxo normal de
`PluginManager` + `PluginConnectorDummyImpl1`. Essa validacao usa plugins reais
compilados e disponiveis pelo conector estatico/dummy do projeto; ela nao faz
regeneracao dinamica completa de Flex/Bison nem carregamento dinamico real de
bibliotecas compartilhadas.

Plugins reais carregados para convivencia, um por grupo solicitado:

- `delay.so` -> `Delay` (`DiscreteProcessing`);
- `dispose.so` -> `Dispose` (`Logic`);
- `separate.so` -> `Separate` (`Grouping`);
- `record.so` -> `Record` (`InputOutput`);
- `exit.so` -> `Exit` (`MaterialHandling`).

Esses plugins continuam sem declarar funcoes de parser em `PluginInformation`.
O teste valida que eles sao inseridos pelo `PluginManager`, que nao adicionam
entradas ao `FunctionRegistry`, que a remocao por ponteiro e por filename nao
deixa origem orfa no registry nem remove outros plugins por engano, e que
expressoes legadas continuam avaliando normalmente antes e depois do
carregamento/remocao, por exemplo `2+3`, `10/2` e `(2+3)*4`.

A demonstracao de funcao declarada por plugin permanece em
`ParserFunctionRegistryDemoTest`, com um `PluginConnector_if` fake que passa
pelo `PluginManager`. Isso mantém os plugins reais como validacao de
compatibilidade, sem alterar sua semantica. A migracao de funcoes legadas de
`Queue`, `Resource`, `Set`, `Variable` e `Formula` permanece como etapa futura.

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

- `source/kernel/simulator/Simulator.h`;
- `source/kernel/simulator/Simulator.cpp`;
- `source/kernel/simulator/PluginInformation.h`;
- `source/kernel/simulator/PluginInformation.cpp`;
- `source/kernel/simulator/PluginManager.h`;
- `source/kernel/simulator/PluginManager.cpp`;
- `source/kernel/simulator/ParserDefaultImpl2.h`;
- `source/kernel/simulator/ParserDefaultImpl2.cpp`.

Testes:

- `source/tests/unit/test_parser_function_registry.cpp`;
- `source/tests/unit/test_parser_semantic_resolver.cpp`;
- `source/tests/unit/test_parser_expressions.cpp`;
- `source/tests/unit/test_parser_function_registry_demo.cpp`;
- `source/tests/unit/test_plugin_manager_real_components.cpp`;
- `source/tests/unit/CMakeLists.txt`.

Principais estruturas criadas:

- `FunctionDescriptor`;
- `FunctionRegistrationResult`;
- `FunctionCallResult`;
- `FunctionRegistry`;
- `FunctionCallRequest`;
- `SemanticResolverResult`;
- `SemanticResolver`;
- `ParserFunctionDeclaration`.

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

Fluxo para uma funcao nova registrada por plugin:

1. o plugin declara uma ou mais funcoes em `PluginInformation`;
2. `PluginManager::insert(...)` valida o plugin e suas dependencias;
3. `PluginManager` registra as funcoes no `Simulator::getFunctionRegistry()`;
4. `ParserDefaultImpl2` usa o registry do simulador pai do modelo;
5. o lexer encontra um literal nao reconhecido e retorna `IDENTIFIER`;
6. o Bison reconhece a chamada `IDENTIFIER(...)`;
7. os argumentos sao avaliados como `double`;
8. o Bison chama o `SemanticResolver`;
9. o resolver consulta o registry e valida a chamada;
10. o callback retorna o valor numerico;
11. o valor volta para a expressao como resultado da chamada.

Fluxo demonstrado por `PluginAdd(2,3)`:

1. avaliar `PluginAdd(2,3)` antes da insercao do plugin gera erro controlado;
2. `PluginManager::insert("parser_function_plugin.so")` conecta o plugin
   fixture pelo `PluginConnector_if` de teste;
3. `PluginInformation` declara `PluginAdd`;
4. `PluginManager` registra `PluginAdd` no registry do `Simulator`;
5. `Model::parseExpression("PluginAdd(2,3)")` retorna `5`.

Se um modelo salvo contiver uma expressao com funcao de plugin ausente, o parser
nao cria token especial nem tenta carregar o plugin implicitamente. A expressao
falha semanticamente como funcao nao registrada ate que o plugin correspondente
seja inserido no `PluginManager` do simulador.

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
PATH=/tmp/genesys-cmake-venv/bin:$PATH cmake --build build/tests-unit --target \
  genesys_test_parser_function_registry \
  genesys_test_parser_semantic_resolver \
  genesys_test_parser_function_registry_demo \
  genesys_test_parser_expressions \
  genesys_test_plugin_manager_real_components -j2
```

Resultado documentado anteriormente para o build unitario amplo:

- configuracao passou;
- build passou;
- saida do build: `ninja: no work to do.`

Testes focados do DCS:

```bash
PATH=/tmp/genesys-cmake-venv/bin:$PATH ctest --test-dir build/tests-unit -R 'FunctionRegistryTest|ParserDriverFunctionRegistryTest|SemanticResolverTest|ParserFunctionRegistryDemoTest|ParserExpressionsTest|PluginManagerRealComponentsTest|ParserPluginManagerRealComponentsTest' --output-on-failure
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

- `PluginAdd` indisponivel antes da insercao do plugin;
- `PluginAdd(2,3)` retornando `5` depois de
  `PluginManager::insert("parser_function_plugin.so")`;
- `PluginAdd` indisponivel depois de `PluginManager::remove(...)`;
- unregister de `PluginAdd` tambem quando a remocao usa
  `PluginManager::remove("parser_function_plugin.so")`;
- rejeicao de conflito case-insensitive sem sobrescrever a funcao existente;
- erro controlado para `PluginAdd(1)`;
- erro controlado para `FuncaoInexistente(2)`.

`PluginManagerRealComponentsTest` cobre:

- plugins reais sem funcoes de parser nao alterando o registry;
- convivencia de `Delay`, `Dispose`, `Separate`, `Record` e `Exit` com parsing
  legado;
- remocao desses plugins reais sem deixar funcoes registradas por origem;
- remocao por filename usando o mesmo identificador de `insert(...)`, como
  `delay.so`;
- preservacao dos demais plugins carregados durante unload por filename;
- parser legado funcionando antes/depois do carregamento e depois do unload.

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

## Conclusao

A entrega implementa um caminho extensivel minimo e testado para chamadas de
funcao por nome. O parser continua compativel com a gramatica legada, mas agora
possui um ponto de extensao para funcoes novas sem token especifico.

O resultado nao substitui a infraestrutura historica de alteracao dinamica do
parser. Ele cria uma camada mais simples e verificavel para o caso mais comum:
registrar uma funcao, validar aridade e executar um callback a partir de uma
expressao textual.

Limitacoes atuais:

- as funcoes registradas aceitam e retornam apenas `double`;
- nao ha AST explicita nem avaliacao tardia geral;
- funcoes legadas de `Queue`, `Resource`, `Set`, `Variable` e `Formula`
  continuam no caminho historico de Flex/Bison;
- a migracao dessas funcoes legadas deve ser gradual, uma familia por vez,
  preservando tokens/producoes atuais ate haver cobertura especifica.

# Inventario do parser atual para DCS

## Escopo

Branch analisada: `dcs-parser-function-registry`.

Esta etapa e apenas documental. Nenhum arquivo de parser, kernel, CMake ou C++ foi alterado para produzir este inventario.

## Arquivos analisados

- `source/parser/parserBisonFlex/bisonparser.yy`
- `source/parser/parserBisonFlex/lexerparser.ll`
- `source/parser/Genesys++-driver.h`
- `source/parser/Genesys++-driver.cpp`
- `source/parser/obj_t.h`
- `source/parser/obj_t.cpp`
- `source/parser/CMakeLists.txt`
- `source/kernel/simulator/ParserChangesInformation.h`
- `source/kernel/simulator/ParserChangesInformation.cpp`
- `source/kernel/simulator/ParserManager.h`
- `source/kernel/simulator/ParserManager.cpp`
- `source/kernel/simulator/ModelDataDefinition.h`

## Visao geral do fluxo atual

O parser usa Bison/Flex com valores semanticos do tipo `obj_t`. O scanner recebe o `genesyspp_driver`, reconhece palavras reservadas e literais, e retorna tokens especificos para a gramatica. A gramatica avalia expressoes diretamente nas acoes semanticas C++ por meio de `$$.valor`.

Nao foi identificada uma AST explicita para expressoes. Tambem nao foi identificado token generico de identificador. O padrao lexical `{L}` existe em `lexerparser.ll`, mas ele tenta classificar o literal consultando o modelo e retorna tokens concretos como `ATRIB`, `CSTAT`, `COUNTER`, `SIMRESP`, `SIMCTRL`, `VARI`, `FORM`, `QUEUE`, `RESOURCE` ou `SET`; se nenhum caso for encontrado, retorna `ILLEGAL`.

O driver mantem uma estrutura chamada de "refered data elements" por meio de `getReferedDataElements()` e `addRefered()`. Essa estrutura registra dependencias por tipo/nome e nao resolve chamadas de funcao. As mencoes a "registry" em `Genesys++-driver.cpp` se referem a essa estrutura de referencias, nao a um `FunctionRegistry`.

## Tokens e producoes por categoria

### Funcoes matematicas

Tokens declarados em `bisonparser.yy`: `fROUND`, `fMOD`, `fTRUNC`, `fFRAC`, `fEXP`, `fSQRT`, `fLOG`, `fLN`, `mathMIN`, `mathMAX`, alem de `fSIN` e `fCOS`.

Lexemas reconhecidos em `lexerparser.ll`: `round`, `mod`, `trunc`, `frac`, `exp`, `sqrt`, `log`, `ln`, `min`, `max`, `sin`, `cos`.

Producoes:

- `trigonFunction`: `sin(expression)` e `cos(expression)`, avaliadas diretamente com `sin()` e `cos()`.
- `mathFunction`: `round(expression)`, `frac(expression)`, `trunc(expression)`, `exp(expression)`, `sqrt(expression)`, `log(expression)`, `ln(expression)`, `mod(expression, expression)`, `min(expression, expression)` e `max(expression, expression)`.

### Funcoes probabilisticas

Tokens declarados: `fRND1`, `fEXPO`, `fNORM`, `fUNIF`, `fWEIB`, `fLOGN`, `fGAMM`, `fERLA`, `fTRIA`, `fBETA`, `fDISC`.

Lexemas reconhecidos: `rnd`, `expo`, `norm`, `unif`, `weib`, `logn`, `gamm`, `erla`, `tria`, `beta`, `disc`.

Producoes:

- `rnd`: chama `driver.getSampler()->sampleUniform(0.0, 1.0)`.
- `expo(expression)`: chama `sampleExponential`.
- `norm(expression, expression)`: chama `sampleNormal`.
- `unif(expression, expression)`: chama `sampleUniform`.
- `weib(expression, expression)`: chama `sampleWeibull`.
- `logn(expression, expression)`: chama `sampleLogNormal`.
- `gamm(expression, expression)`: chama `sampleGamma`.
- `erla(expression, expression)`: chama `sampleErlang`.
- `tria(expression, expression, expression)`: chama `sampleTriangular`.
- `beta(expression, expression, expression, expression)`: chama `sampleBeta`.
- `disc(listaparm)`: retorna `sampleDiscrete(0, 0)` e esta marcado como nao implementado.

As funcoes probabilisticas capturam excecoes nas acoes semanticas, atualizam erro no driver e usam `YYERROR` quando necessario.

### SimulationResponse

Token: `SIMRESP`.

Nao ha palavra reservada fixa para cada response. O scanner consulta `driver.findSimulationResponse(yytext)` durante a classificacao do literal `{L}`. Quando encontra uma response do modelo, retorna `SIMRESP` com o nome textual em `obj_t::tipo`.

Producao:

- `simulationResponse: SIMRESP`, avaliada por `driver.getSimulationResponseValueAsDouble($1.tipo)`.

### SimulationControl

Token: `SIMCTRL`.

Assim como `SimulationResponse`, o scanner consulta `driver.findSimulationControl(yytext)` durante a classificacao do literal `{L}`.

Producao:

- `simulationControl: SIMCTRL`, avaliada por `driver.getSimulationControlValueAsDouble($1.tipo)`.

### Entity e Attribute

Tokens: `ATRIB`, `fIDENT`, `simulEntitiesWIP`.

Lexemas/fonte:

- `ATRIB` vem da classificacao de `{L}` quando `DataManager` encontra `Attribute`.
- `ident` retorna `fIDENT`.
- `entitieswip` retorna `simulEntitiesWIP`.

Producoes:

- `attribute: ATRIB`: le o valor do atributo da entidade do evento corrente.
- `attribute: ATRIB[indexList]`: le valor indexado pelo `SparseValueStore`.
- `fIDENT`: retorna o id da entidade do evento corrente.
- `entitieswip`: retorna `getNumberOfDataDefinitions(Util::TypeOf<Entity>())`.
- Assignments permitem `ATRIB = expression` e `ATRIB[indexList] = expression`.

### Queue

Tokens de plugin: `QUEUE`, `fNQ`, `fFIRSTINQ`, `fLASTINQ`, `fSAQUE`, `fAQUE`, `fENTATRANK`.

Lexemas reconhecidos: `nq`, `firstinq`, `lastinq`, `saque`, `aque`, `entatrank`. O token `QUEUE` vem da classificacao de `{L}` quando `DataManager` encontra `Queue`.

Producoes:

- `nq(QUEUE)`: retorna `Queue::size()`.
- `firstinq(QUEUE)`: retorna o id da primeira entidade na fila ou `0`.
- `lastinq(QUEUE)`: producao existe, mas a acao esta vazia.
- `saque(QUEUE, ATRIB)`: soma atributos das entidades aguardando na fila.
- `aque(QUEUE, NUMD, ATRIB)`: le atributo da entidade em um rank de espera.

Observacao: `fENTATRANK` e reconhecido pelo lexer e declarado no Bison, mas nao aparece em uma producao semantica no `.yy` analisado.

### Resource

Tokens de plugin: `RESOURCE`, `fNR`, `fMR`, `fIRF`, `fRESSEIZES`, `fSTATE`, `fSETSUM`, `fRESUTIL`.

Lexemas reconhecidos: `nr`, `mr`, `irf`, `state`, `setsum`, `resutil`, `resseizes`. O token `RESOURCE` vem da classificacao de `{L}` quando `DataManager` encontra `Resource`.

Constantes lexicais relacionadas a estado de recurso retornam `NUMD`: `idle_res = -1`, `busy_res = -2`, `inactive_res = -3`, `failed_res = -4`.

Producoes:

- `mr(RESOURCE)`: retorna capacidade do recurso.
- `nr(RESOURCE)`: retorna numero ocupado.
- `resseizes(RESOURCE)`: producao existe, mas esta marcada como TODO e nao atribui valor.
- `state(RESOURCE)`: retorna estado do recurso como inteiro.
- `irf(RESOURCE)`: retorna `1` se o recurso esta em `FAILED`, senao `0`.
- `setsum(SET)`: percorre elementos do `Set`, faz `dynamic_cast<Resource*>` e conta recursos em `BUSY`.

Observacao: `fRESUTIL` e reconhecido pelo lexer e declarado no Bison, mas nao aparece em uma producao semantica no `.yy` analisado.

### Set

Tokens de plugin: `SET`, `fNUMSET`.

Lexemas reconhecidos: `numset`. O token `SET` vem da classificacao de `{L}` quando `DataManager` encontra `Set`.

Producoes:

- `numset(SET)`: retorna o tamanho de `Set::getElementSet()`.
- `setsum(SET)`: aparece no bloco de Resource, mas recebe `SET` e opera sobre elementos Resource dentro do conjunto.

### Variable

Token de plugin: `VARI`.

O token vem da classificacao de `{L}` quando `DataManager` encontra `Variable`.

Producoes:

- `variable: VARI`: retorna `Variable::getValue()`.
- `variable: VARI[indexList]`: retorna `Variable::getValue(index)`.
- Assignments permitem `VARI = expression` e `VARI[indexList] = expression`.

### Formula

Token de plugin: `FORM`.

O token vem da classificacao de `{L}` quando `DataManager` encontra `Formula`.

Producoes:

- `formula: FORM`
- `formula: FORM[expression]`
- `formula: FORM[expression, expression]`
- `formula: FORM[expression, expression, expression]`

As producoes recuperam a expressao textual da Formula, mas retornam `0.0`. O proprio comentario no arquivo registra problema de reentrada: a intencao seria reavaliar a expressao da Formula pelo parser, mas isso esta marcado como problematico.

### Outras categorias encontradas

Funcoes de string: tokens `fVAL`, `fEVAL`, `fLENG`; lexemas `val`, `eval`, `leng`. Estes tokens sao reconhecidos no lexer e declarados no Bison, mas nao foram encontradas producoes semanticas correspondentes.

Funcoes/controles algoritmicos: tokens `cIF`, `cELSE`, `cFOR`, `cTO`, `cDO`; lexemas `if`, `else`, `for`, `to`, `do`. Producoes existentes: `if(expression, expression, expression)`, `if(expression, expression)` e `for variable = expression to expression do assigment`, alem de variante com `attribute`. `cELSE` e reconhecido, mas nao aparece em producao semantica no `.yy` analisado.

Elementos de kernel: tokens `CSTAT`, `COUNTER`, `fTAVG`, `fCOUNT`. Producoes existentes: `tavg(CSTAT)` e `count(COUNTER)`.

EntityGroup: tokens `fNUMGR` e `fATRGR`; lexemas `numgr` e `atrgr`. Eles sao reconhecidos no lexer e declarados no Bison, mas nao foram encontradas producoes semanticas correspondentes.

Funcao de usuario: a producao `userFunction: "USER" "(" expression ")"` existe e apenas retorna o valor da expressao. Nao foi encontrado padrao no scanner que retorne explicitamente o literal `"USER"`, portanto essa producao parece desconectada do lexer atual.

Token `CTEZERO`: usado em `pluginFunction` para retornar `0`, mas nao foi encontrado padrao lexical correspondente nos arquivos analisados.

## Marcadores de extensao dinamica

Marcadores encontrados em `bisonparser.yy`:

- `begin_Includes_plugins`
- `begin_Tokens_plugins`
- `begin_TypeObj_plugins`
- `begin_Expression_plugins`
- `begin_ExpressionProdution_plugins`
- `begin_Assignment_plugins`
- `begin_FunctionProdution_plugins`

Marcadores encontrados em `lexerparser.ll`:

- `begin_Includes_plugins`
- `begin_Lexical_plugins`
- `begin_LexicalLiterals_plugins`

Os marcadores contem blocos especificos para tipos conhecidos, como `Variable`, `Queue`, `Formula`, `Resource`, `Set` e `EntityGroup`. Isso evidencia que a extensao historica esperada e textual: inserir includes, tokens, producoes Bison e regras Flex em pontos predefinidos.

## Onde a avaliacao ocorre diretamente no Bison

O Bison avalia expressoes diretamente nas acoes semanticas:

- Operadores logicos, relacionais, aritmeticos, potencia e unarios calculam `$$.valor` diretamente.
- `commandIF` escolhe o resultado com operador ternario em C++.
- `kernelFunction` consulta diretamente `Simulation`, `CurrentEvent`, `Entity` e `DataManager`.
- `mathFunction` e `trigonFunction` chamam funcoes da biblioteca C++ diretamente.
- `probFunction` chama diretamente o `Sampler_if` via `driver.getSampler()`.
- `attribute`, `simulationResponse`, `simulationControl`, `variable`, `formula`, assignments e `pluginFunction` tambem executam a semantica diretamente.

Consequencia: a gramatica atual mistura parsing, resolucao de simbolos do modelo e avaliacao. Nao ha fronteira clara entre reconhecer a chamada e executa-la.

## Acoplamentos atuais entre parser e plugins

O acoplamento aparece em quatro camadas:

1. Includes diretos em `.yy` e `.ll` para `Variable`, `Queue`, `Formula`, `Resource` e `Set`.
2. Tokens especificos por plugin, como `QUEUE`, `RESOURCE`, `SET`, `VARI`, `FORM`, `fNQ`, `fMR`, `fNUMSET`.
3. Regras lexicais especificas para nomes de funcoes de plugin, como `nq`, `firstinq`, `mr`, `state`, `numset`.
4. Acoes semanticas com casts diretos para classes de plugin, como `Queue*`, `Resource*`, `Set*`, `Variable*` e `Formula*`.

O scanner tambem acopla nomes de objetos do modelo a tokens concretos, consultando `DataManager` para tipos conhecidos. Isso significa que adicionar uma nova categoria de objeto ou funcao nao passa por um identificador generico: exige novo reconhecimento lexical e nova producao ou extensao textual equivalente.

## Estruturas semelhantes a registry, resolver, callback, tabela de simbolos, AST ou avaliacao tardia

Encontrado:

- `ParserChangesInformation`: armazena strings para includes, tokens, type objects, expressoes, producoes, assignments e function productions.
- `ParserManager`: declara `generateNewParser(ParserChangesInformation*)` e `connectNewParser(NewParser)`, mas a implementacao atual so contem o construtor.
- `ModelDataDefinition::_getParserChangesInformation()`: hook virtual descrito como mecanismo para plugins retornarem mudancas necessarias no parser.
- `genesyspp_driver::_referedDataElements`: mapa de tipo para nomes referenciados, usado para rastrear dependencias de elementos do modelo.
- `findSimulationResponse` e `findSimulationControl`: resolvem nomes de responses/controls no modelo, mas sao helpers especificos, nao um resolver geral de funcoes.

Nao encontrado:

- `FunctionRegistry` para chamadas de funcao.
- Resolver generico de funcoes por nome/assinatura.
- Callback registrado por plugin para avaliacao de funcao.
- Tabela de simbolos generica para identificadores.
- AST explicita para expressoes.
- Avaliacao tardia geral. O caso de `Formula` sugere uma tentativa futura de reavaliacao textual, mas atualmente retorna `0.0`.

## Evidencias de acoplamento sintatico a plugins conhecidos

- O Bison declara tokens de plugin dentro de `begin_Tokens_plugins`, incluindo `RESOURCE`, `QUEUE`, `SET`, `VARI` e `FORM`.
- O Flex declara includes dos mesmos plugins e regras lexicais especificas para funcoes desses plugins.
- A producao `pluginFunction` enumera chamadas conhecidas e executa casts diretos para `Queue`, `Resource` e `Set`.
- As producoes `variable` e `formula` existem como nao terminais especificos de tipos de plugin.
- `ParserChangesInformation` guarda trechos textuais de parser e `ModelDataDefinition` descreve a intencao historica de alterar codigo-fonte do parser, recompilar e linkar dinamicamente.

Esses pontos mostram que o parser atual nao esta apenas semanticamente acoplado ao runtime de plugins; ele tambem esta sintaticamente acoplado a nomes, tokens e producoes de plugins conhecidos.

## Riscos de iniciar por regeneracao dinamica completa de Flex/Bison

- A opcao CMake `GENESYS_PARSER_REGENERATE` existe, mas fica desligada por padrao.
- `ParserManager::generateNewParser` e `connectNewParser` estao declarados, mas nao implementados no `.cpp`.
- A abordagem textual depende de inserir fragmentos em multiplos pontos coordenados do `.yy` e `.ll`, o que aumenta risco de inconsistencia entre token, regra lexical, producao e include.
- O parser mistura reconhecimento e avaliacao. Regenerar a gramatica sem antes separar a avaliacao de funcoes manteria o mesmo acoplamento em uma superficie mais dificil de testar.
- Alguns tokens ja existem sem producao semantica observada, como `fRESUTIL`, `fENTATRANK`, `fNUMGR`, `fATRGR`, `fVAL`, `fEVAL` e `fLENG`. Isso indica que a sincronizacao lexical/sintatica ja tem lacunas.
- O caso de `Formula` documenta problema de reentrada ao tentar reavaliar expressoes via parser.
- Um fluxo completo de geracao, compilacao e ativacao dinamica teria impacto em toolchain, artefatos gerados, carregamento de codigo, tratamento de erro e testes, alem de exigir alteracoes em CMake ou infraestrutura de runtime.

## Justificativa para abordagem incremental com FunctionRegistry

Uma abordagem incremental por `FunctionRegistry` permite introduzir uma fronteira de resolucao de funcoes antes de tentar regenerar Flex/Bison dinamicamente.

O primeiro ganho esperado e separar "nome da funcao e argumentos" de "como a funcao e avaliada". Mesmo que o parser continue estatico inicialmente, o registry pode consolidar assinaturas, validacao de aridade, chamada de callbacks e tratamento de erro em uma camada testavel.

Essa abordagem reduz o risco porque preserva o comportamento atual da gramatica enquanto cria um caminho para migrar funcoes uma categoria por vez. Tambem permite escrever testes comparando o resultado antigo e o resultado via registry antes de substituir producoes existentes.

## Proximos passos

1. Definir o contrato minimo do `FunctionRegistry`: nome canonico, aridade, tipos de argumentos, contexto de avaliacao e politica de erro.
2. Escolher uma categoria pequena para prova de conceito, preferencialmente funcoes matematicas puras ou funcoes de kernel sem dependencia de plugins.
3. Escrever testes de caracterizacao para as funcoes existentes antes de mover qualquer avaliacao para registry.
4. Preservar o scanner e a gramatica atuais na primeira etapa de integracao, usando o registry apenas por baixo de uma producao existente.
5. So depois avaliar se um token generico de chamada de funcao ou identificador e necessario.
6. Manter a regeneracao completa de Flex/Bison fora do caminho critico ate existir cobertura de comportamento e contrato estavel para funcoes dinamicas.

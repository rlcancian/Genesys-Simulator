# FunctionRegistry inicial

## Localizacao

O `FunctionRegistry` inicial foi colocado em `source/parser`.

A escolha e intencional: nesta fase o registry representa infraestrutura de avaliacao de chamadas de funcao para o parser/evaluator, mas ainda nao e um servico do simulador nem depende de plugins concretos. Manter a primeira versao em `source/parser` reduz o escopo e evita acoplar o kernel a uma API que ainda esta sendo caracterizada pelo DCS.

## Escopo da primeira versao

A primeira versao registra funcoes numericas simples:

- nome publico;
- aridade minima;
- aridade maxima;
- origem ou plugin;
- descricao curta;
- categoria opcional;
- callback `double(const std::vector<double>&)`.

O registry oferece:

- `registerFunction(...)`;
- `lookup(...)`;
- `hasFunction(...)`;
- `callFunction(...)`;
- `listFunctions()`.

O lookup e case-insensitive para preservar compatibilidade conceitual com o lexer atual, que reconhece nomes de funcoes em letras maiusculas ou minusculas.

## Fora do escopo

Esta etapa nao altera:

- `source/parser/parserBisonFlex/bisonparser.yy`;
- `source/parser/parserBisonFlex/lexerparser.ll`;
- funcoes legadas reconhecidas pelo parser;
- classes de plugin;
- avaliacao atual de expressoes pelo Bison.

## Ponte com Genesys++-driver

O `genesyspp_driver` possui agora uma referencia opcional para `FunctionRegistry`.

O ownership e nao-dono: o driver guarda apenas um ponteiro externo, alinhado ao estilo ja usado para `Model*` e `Sampler_if*`. O ciclo de vida do registry continua responsabilidade do chamador. Por isso, o driver oferece `setFunctionRegistry(...)`, `getFunctionRegistry()` e `hasFunctionRegistry()`, mas nao cria nem destroi o registry.

## SemanticResolver

O `SemanticResolver` foi adicionado em `source/parser` como camada isolada entre reconhecimento sintatico e avaliacao de funcoes registradas.

A estrutura minima `FunctionCallRequest` representa uma chamada por nome e argumentos numericos. O resolver recebe um `FunctionRegistry` externo, valida registry ausente, funcao inexistente, aridade incorreta, erros do callback e retorno numerico nao finito. As mensagens incluem o nome da funcao, quantidade de argumentos, aridade esperada, origem/plugin quando disponivel e lista de funcoes registradas quando a funcao nao existe.

Na etapa de criacao do resolver, ele ainda nao estava conectado ao Bison/Flex. A integracao inicial abaixo registra a primeira mudanca localizada na gramatica e no lexer.

## Integracao inicial no parser

A primeira integracao sintatica adiciona chamada generica de funcao no formato `IDENTIFIER(...)`.

O lexer continua priorizando tokens legados, palavras reservadas e elementos conhecidos do modelo. Apenas o fallback de literal nao reconhecido passa a retornar `IDENTIFIER`; quando esse identificador aparece sozinho, a gramatica preserva a mensagem legada de literal nao encontrado. Quando aparece como chamada de funcao, o Bison delega a resolucao para `SemanticResolver` usando o `FunctionRegistry` configurado no driver.

Limitacoes desta etapa:

- argumentos e retorno continuam restritos a `double`;
- a avaliacao ainda acontece diretamente nas acoes semanticas do Bison, sem AST;
- funcoes que ja possuem token legado continuam seguindo o caminho legado, mesmo que exista funcao registrada com mesmo nome;
- `Model::parseExpression(...)` ainda nao expoe API publica para configurar registry; testes de integracao usam `ParserDefaultImpl2` diretamente;
- o build normal usa fontes gerados versionados; ao alterar `.yy` ou `.ll`, e necessario regenerar `GenesysParser.*` e `Genesys++-scanner.cpp`.

## Proximos passos

1. Adicionar testes de caracterizacao para uma categoria real de funcoes antes de migrar avaliacao.
2. Integrar uma producao existente ao registry por baixo, preservando o token e a sintaxe atuais.
3. Avaliar se o registry deve migrar para `source/kernel/simulator` somente quando existir necessidade concreta de compartilhamento como servico do simulador.

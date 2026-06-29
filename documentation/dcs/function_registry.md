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

## Proximos passos

1. Adicionar testes de caracterizacao para uma categoria real de funcoes antes de migrar avaliacao.
2. Integrar uma producao existente ao registry por baixo, preservando o token e a sintaxe atuais.
3. Avaliar se o registry deve migrar para `source/kernel/simulator` somente quando existir necessidade concreta de compartilhamento como servico do simulador.

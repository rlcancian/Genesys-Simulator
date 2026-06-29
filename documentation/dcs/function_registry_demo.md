# Demonstracao do FunctionRegistry com FakePlugin

## Objetivo

Esta demonstracao mostra que uma funcao nova pode ser usada em expressao textual sem criar token especifico no Flex/Bison.

O `FakePlugin` usado nos testes fica em `source/tests/unit/fakes/FakePlugin.h`. Ele nao e um plugin real carregado pelo `PluginManager`; e uma classe de demonstracao para validar o contrato minimo esperado de um plugin futuro: receber um `FunctionRegistry` externo e registrar funcoes publicas nele.

## Registro de funcoes

`FakePlugin::registerFunctions(FunctionRegistry&)` registra:

- `FakeAdd(a,b)`, que retorna `a+b`;
- `FakeSquare(x)`, que retorna `x*x`.

As funcoes sao registradas por descritor e callback no `FunctionRegistry`. O parser nao conhece `FakePlugin`, nao inclui seu header e nao cria caminho especial para essas funcoes.

## Por que o lexer nao precisou conhecer FakeAdd

O lexer continua reconhecendo primeiro palavras reservadas, funcoes legadas e elementos conhecidos do modelo. Quando um literal nao reconhecido aparece, ele pode ser entregue ao parser como `IDENTIFIER`.

Isso significa que `FakeAdd` nao precisa de regra lexical propria. O nome e carregado no valor semantico do token generico e sera resolvido depois.

## Como o parser reconhece a chamada

A gramatica aceita chamadas genericas no formato:

```text
IDENTIFIER '(' argumentList ')'
IDENTIFIER '(' ')'
```

Os argumentos ainda sao avaliados imediatamente como `double`, seguindo o estilo atual do parser legado. A chamada generica entao delega para o `SemanticResolver`.

## Como o SemanticResolver usa o registry

O `SemanticResolver` recebe o `FunctionRegistry` configurado no driver do parser. Para uma chamada como `FakeAdd(2,3)`, ele:

1. verifica se ha registry configurado;
2. procura `FakeAdd`;
3. valida a aridade recebida;
4. executa o callback registrado;
5. retorna o valor numerico ou uma mensagem de erro controlada.

O teste `ParserFunctionRegistryDemoTest` demonstra que `FakeAdd(2,3)` falha antes do registro e passa depois que `FakePlugin::registerFunctions(...)` popula o registry.

## Limitacoes atuais

- A demonstracao usa uma classe de teste, nao um plugin real carregado dinamicamente.
- O parser ainda avalia argumentos diretamente nas acoes semanticas do Bison.
- Argumentos e retorno sao numericos (`double`).
- Funcoes legadas continuam no caminho antigo e tem precedencia lexical sobre identificadores genericos.
- `Model::parseExpression(...)` ainda nao expoe uma API publica para configurar o registry; os testes usam `ParserDefaultImpl2` diretamente.

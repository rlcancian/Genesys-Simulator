# Diagrama de Sequencia - Lazy Reload do Parser

Este diagrama apresenta o caso de uso principal: uma extensao contribui mudancas ao parser, o modelo fica marcado como stale e a primeira chamada a `parseExpression()` regenera e conecta o parser automaticamente.

```mermaid
sequenceDiagram
    autonumber
    participant Ext as Extensao / ParserExtensionTestPlugin
    participant Dm as ModelDataManager
    participant M as Model
    participant Pm as ParserManager
    participant Bf as Bison/Flex + Compilador
    participant So as genesys_parser_dynamic.so
    participant P as ParserDefaultImpl2

    Ext->>Dm: insert(extensao)
    Dm->>Ext: _getParserChangesInformation()
    Ext-->>Dm: ParserChangesInformation com tokens e regras
    Dm->>M: setParserIsStale(true)

    Note over M: Usuario ou simulacao avalia uma expressao
    M->>M: parseExpression("lazytest(5)")

    alt parser esta stale
        M->>Pm: aggregateChanges()
        Pm->>Ext: _getParserChangesInformation()
        Ext-->>Pm: ParserChangesInformation
        Pm-->>M: lista de mudancas

        M->>Pm: generateNewParser(mudancas combinadas)
        Pm->>Bf: injeta em bisonparser.yy e lexerparser.ll
        Bf->>Bf: bison, flex e compilacao C++
        Bf-->>Pm: genesys_parser_dynamic.so
        Pm-->>M: NewParser com caminho da biblioteca

        M->>Pm: connectNewParser(newParser)
        Pm->>So: dlopen() e dlsym("genesys_createParser")
        So-->>Pm: factory C ABI
        Pm->>So: genesys_createParser(model, sampler)
        So-->>Pm: nova instancia ParserDefaultImpl2
        Pm->>M: setParser(novoParser)
    end

    M->>P: parse("lazytest(5)")
    P-->>M: 7.0
    M-->>Ext: expressao avaliada com parser atualizado
```

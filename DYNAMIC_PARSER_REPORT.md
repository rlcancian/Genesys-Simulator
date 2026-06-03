# Relatório Completo: Implementação do Parser Dinâmico no GenESyS

## Resumo Executivo

Este documento descreve a implementação completa do **parser dinâmico** no simulador GenESyS, permitindo que plugins contribuam com funções customizadas na linguagem de expressões do simulador. A solução envolveu a criação de um plugin de demonstração (`DemoPlugin`), a modificação do kernel para suportar regeneração dinâmica de parser Bison/Flex, e a integração transparente na interface gráfica (GUI) e no terminal.

---

## 1. Arquitetura do Parser Dinâmico

### 1.1 Visão Geral

O GenESyS utiliza um parser de expressões baseado em Bison/Flex (`ParserDefaultImpl2`) para avaliar expressões matemáticas e lógicas durante simulações. A arquitetura original era **estática**: o parser era compilado junto com o simulador e não podia ser estendido em tempo de execução.

A nova arquitetura permite:
- **Plugins** declararem mudanças no parser (tokens, regras gramaticais, regras léxicas)
- **Regeneração** do parser Bison/Flex em tempo de execução
- **Carregamento dinâmico** do novo parser via `dlopen`
- **Substituição** do parser ativo do modelo sem reiniciar o simulador

### 1.2 Pipeline de Regeneração

```
Plugin com parser changes (ex: DemoPlugin)
           │
           ▼
   aggregateChanges()  ← coleta de todas as instâncias no modelo
           │
           ▼
   _injectBisonChanges()  ← injeta tokens/produções no .yy
   _injectFlexChanges()   ← injeta regras léxicas no .ll
           │
           ▼
   bison → GenesysParser.cpp
   flex  → Genesys++-scanner.cpp
           │
           ▼
   g++ -fPIC -shared → genesys_parser_dynamic.so
           │
           ▼
   dlopen(RTLD_NOW | RTLD_DEEPBIND)
   dlsym("genesys_createParser")
           │
           ▼
   Model::setParser(newParserInstance)
```

---

## 2. Componentes Implementados

### 2.1 DemoPlugin (`source/plugins/data/Template/DemoPlugin.h/.cpp`)

Plugin de prova de conceito que adiciona a função `demo(x) = x + 1` ao parser.

**Parser Changes injetadas:**
```cpp
ParserChangesInformation* DemoPlugin::_getParserChangesInformation() {
    ParserChangesInformation* pci = new ParserChangesInformation();
    pci->setTokens("%token <obj_t> fDEMO\n");
    pci->setFunctionProdutions(
        "pluginFunction: fDEMO \"(\" expression \")\" "
        "{$$.valor = $3.valor + 1; $$._type = obj_type::NUMERIC;}\n"
    );
    pci->setLexicalRules(
        "[dD][eE][mM][oO] "
        "{return yy::genesyspp_parser::make_fDEMO(obj_t(0, std::string(yytext)), loc);}\n"
    );
    return pci;
}
```

### 2.2 ParserFactory (`source/parser/dynamic/ParserFactory.h/.cpp`)

Factory C com linkage C para criação/destruição do parser dinâmico:

```cpp
extern "C" Parser_if* genesys_createParser(Model* model, Sampler_if* sampler) {
    return new ParserDefaultImpl2(model, sampler);
}
```

### 2.3 ParserManagerRuntime (`source/kernel/simulator/ParserManagerRuntime.cpp`)

Novo arquivo separado do `ParserManager.cpp` contendo a lógica de runtime:

- **`aggregateChanges()`**: Percorre todas as instâncias `ModelDataDefinition` no `ModelDataManager` e coleta seus `ParserChangesInformation*`
- **`connectNewParser()`**: Carrega a biblioteca dinâmica com `dlopen`, obtém o factory via `dlsym`, cria nova instância do parser, transfere a ownership do `Sampler_if`, e substitui o parser no modelo

### 2.4 ParserManager (`source/kernel/simulator/ParserManager.cpp`)

Modificado para incluir:
- **Injeção de mudanças** nos arquivos Bison/Flex base (`_injectBisonChanges`, `_injectFlexChanges`)
- **Geração** de código via `bison` e `flex`
- **Compilação** com `g++ -fPIC -shared` incluindo `ParserDefaultImpl2.cpp` (crítico para resolução de símbolos)

---

## 3. Integração com o Sistema de Plugins

### 3.1 PluginConnectorDummyImpl1

O `DemoPlugin` foi registrado no conector de plugins estático:

```cpp
// find()
if (filename == "demo.so") return &DemoPlugin::GetPluginInformation;

// connect()
if (filename == "demo.so") return new Plugin(&DemoPlugin::GetPluginInformation);
```

Isso torna o `DemoPlugin` descobrível pelo `PluginManager` sem necessidade de carregamento dinâmico de `.so`.

---

## 4. Integração GUI (Qt)

### 4.1 Botão "Regenerate Parser"

Adicionado na aba **Expression Console** do diálogo **Parser Manager** (`DialogUtilityController.cpp`). Inicialmente, era necessário clicar manualmente para regenerar o parser.

### 4.2 Regeneração Lazy Automática

Posteriormente, implementou-se a **regeneração lazy automática**:

1. **`ModelDataManager::insert()`** detecta quando um elemento tem `_getParserChangesInformation() != nullptr` e marca o parser como **stale** (`_parserIsStale = true`)
2. **`Model::parseExpression()`** verifica o flag stale antes de avaliar qualquer expressão. Se stale, regenera automaticamente, conecta o novo parser, e depois avalia
3. O usuário não precisa mais clicar em nada — a primeira avaliação de expressão após adicionar um plugin dispara a regeneração

### 4.3 Configuração do ParserManager no Modelo

O construtor `Model::Model()` agora configura automaticamente o `ParserManager`:
- Descobre o diretório raiz do projeto procurando `source/parser/parserBisonFlex/bisonparser.yy`
- Define o diretório de trabalho como `/tmp/genesys_parser_auto`
- Associa o modelo ao `ParserManager` via `setModel(this)`

---

## 5. Integração Terminal (GenesysShell)

Adicionado o comando `dynamicparser` no `GenesysShell` (`cmdDynamicParser`) que:
1. Cria uma instância `DemoPlugin` via `PluginManager::newInstance()`
2. Insere no `ModelDataManager`
3. Executa o pipeline completo: `aggregateChanges()` → `generateNewParser()` → `connectNewParser()`
4. Testa a nova função avaliando `demo(4)` e `demo(5)`

---

## 6. Desafios e Soluções

### 6.1 Erro: `undefined symbol: _ZTI5Queue`

**Problema:** O `.so` do parser dinâmico era compilado sem linkar com as bibliotecas do GenESyS (Queue, Resource, etc.), mas o código gerado pelo Bison referencia esses tipos.

**Diagnóstico:** A função `connectNewParser()` reportava o erro detalhado via `dlerror()`.

**Solução em duas etapas:**
1. Adicionar `-rdynamic` ao link do executável da GUI (`CMakeLists.txt` do GenesysQtGUI) — exporta todos os símbolos do processo host para o dynamic linker
2. Manter `RTLD_DEEPBIND` no `dlopen` — garante que o parser dinâmico use **seus próprios** símbolos Bison/Flex (evitando conflito com o parser estático do host)

### 6.2 Parser compilado mas `demo()` não reconhecido

**Problema:** O parser dinâmico era carregado sem `RTLD_DEEPBIND`, então o `ParserDefaultImpl2` do `.so` usava o parser estático do processo host (que não conhecia `demo`).

**Solução:** Reverter para `RTLD_NOW | RTLD_DEEPBIND`. Com `-rdynamic` no host, os símbolos `Queue` etc. são resolvidos no host. Com `RTLD_DEEPBIND`, o parser Bison/Flex do `.so` tem prioridade sobre o do host.

### 6.3 Falta de `ParserDefaultImpl2.cpp` na compilação do `.so`

**Problema:** O comando de compilação do `.so` não incluía `ParserDefaultImpl2.cpp`, causando erros de linkagem para símbolos do parser.

**Solução:** Modificar `_buildCompilerCommand()` para incluir `ParserDefaultImpl2.cpp` no comando `g++ -shared`.

### 6.4 Injeção léxica não funcionava no arquivo gerado

**Problema:** Inicialmente procurava-se por `lexerparser_injected.ll`, mas o `ParserManager` gera `lexerparser.ll` diretamente.

**Verificação:** Confirmou-se que a injeção ocorre corretamente em `lexerparser.ll` (regra `[dD][eE][mM][oO]` presente).

---

## 7. Testes

### 7.1 Teste Unitário (`test_parser_expressions.cpp`)

Teste `DynamicParserDemoPluginAddsNewFunction` que valida o pipeline completo:
1. Cria `DemoPlugin`
2. Coleta parser changes
3. Gera e compila parser
4. Carrega dinamicamente
5. Avalia `demo(4)` e confirma resultado `5`

### 7.2 Teste Terminal

Comando `dynamicparser` no `GenesysShell` demonstra a substituição do parser em runtime.

### 7.3 Teste GUI

1. Abrir GUI
2. Criar modelo
3. Adicionar DemoPlugin (Template)
4. Digitar `demo(5)` no Expression Console
5. Resultado esperado: `6`

---

## 8. Arquivos Criados (Detalhamento)

### 8.1 `source/plugins/data/Template/DemoPlugin.h`

Declaração da classe `DemoPlugin` herdando `ModelDataDefinition`, com a interface padrão de plugin do GenESyS:

```cpp
class DemoPlugin : public ModelDataDefinition {
public:
    DemoPlugin(Model* model, std::string name = "");
    virtual ~DemoPlugin() = default;
public:
    static PluginInformation* GetPluginInformation();
    static ModelDataDefinition* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
    virtual ParserChangesInformation* _getParserChangesInformation() override;
};
```

### 8.2 `source/plugins/data/Template/DemoPlugin.cpp`

Implementação completa do plugin. O método crítico é `_getParserChangesInformation()`, que retorna um `ParserChangesInformation*` com as três injeções necessárias:

```cpp
ParserChangesInformation* DemoPlugin::_getParserChangesInformation() {
    ParserChangesInformation* pci = new ParserChangesInformation();
    pci->setTokens("%token <obj_t> fDEMO\n");
    pci->setFunctionProdutions(
        "pluginFunction: fDEMO \"(\" expression \")\" "
        "{$$.valor = $3.valor + 1; $$._type = obj_type::NUMERIC;}\n"
    );
    pci->setLexicalRules(
        "[dD][eE][mM][oO] "
        "{return yy::genesyspp_parser::make_fDEMO(obj_t(0, std::string(yytext)), loc);}\n"
    );
    return pci;
}
```

A função `demo(x)` implementa `x + 1`. O token `%token <obj_t> fDEMO` é injetado na seção de tokens do Bison. A produção gramatical é injetada em `pluginFunction`. A regra léxica case-insensitive `[dD][eE][mM][oO]` é injetada no scanner Flex.

### 8.3 `source/parser/dynamic/ParserFactory.h` e `.cpp`

Factory com linkage C para evitar name mangling:

```cpp
// ParserFactory.h
#pragma once
#include "kernel/simulator/Parser_if.h"

extern "C" Parser_if* genesys_createParser(Model* model, Sampler_if* sampler);
extern "C" void genesys_destroyParser(Parser_if* parser);

// ParserFactory.cpp
#include "ParserFactory.h"
#include "Genesys++-driver.h"
#include "kernel/simulator/ParserDefaultImpl2.h"

extern "C" Parser_if* genesys_createParser(Model* model, Sampler_if* sampler) {
    return new ParserDefaultImpl2(model, sampler);
}

extern "C" void genesys_destroyParser(Parser_if* parser) {
    delete parser;
}
```

O factory é compilado dentro do `.so` dinâmico e resolvido via `dlsym(_dynamicLibraryHandle, "genesys_createParser")`.

### 8.4 `source/kernel/simulator/ParserManagerRuntime.cpp`

Novo arquivo separado do `ParserManager.cpp`. Contém duas funções críticas:

**`aggregateChanges()`** — percorre o `ModelDataManager` e coleta `ParserChangesInformation*` de todas as instâncias:

```cpp
std::list<ParserChangesInformation*> ParserManager::aggregateChanges() {
    std::list<ParserChangesInformation*> result;
    if (_model == nullptr || _model->getDataManager() == nullptr) {
        return result;
    }
    std::list<std::string> classnames = _model->getDataManager()->getDataDefinitionClassnames();
    for (const std::string& classname : classnames) {
        List<ModelDataDefinition*>* list = _model->getDataManager()->getDataDefinitionList(classname);
        if (list == nullptr) continue;
        for (ModelDataDefinition* mdd : *list->list()) {
            if (mdd == nullptr) continue;
            ParserChangesInformation* changes = mdd->_getParserChangesInformation();
            if (changes != nullptr) {
                result.push_back(changes);
            }
        }
    }
    return result;
}
```

**`connectNewParser()`** — carrega a `.so`, cria o parser, transfere sampler, substitui no modelo:

```cpp
bool ParserManager::connectNewParser(ParserManager::NewParser newParser, std::string* errorMessage) {
    auto log = [errorMessage](const std::string& msg) {
        if (errorMessage != nullptr) *errorMessage += msg + "\n";
    };
    if (_model == nullptr) { log("Error: model is nullptr"); return false; }
    if (newParser.compiledParserFilename.empty()) { log("Error: compiledParserFilename is empty"); return false; }
    if (!std::filesystem::exists(newParser.compiledParserFilename)) {
        log("Error: compiled parser file not found: " + newParser.compiledParserFilename);
        return false;
    }
    if (_dynamicLibraryHandle != nullptr) {
        dlclose(_dynamicLibraryHandle);
        _dynamicLibraryHandle = nullptr;
    }
    log("Loading: " + newParser.compiledParserFilename);
    _dynamicLibraryHandle = dlopen(newParser.compiledParserFilename.c_str(), RTLD_NOW | RTLD_DEEPBIND);
    if (_dynamicLibraryHandle == nullptr) {
        log("dlopen failed: " + std::string(dlerror()));
        return false;
    }
    log("dlopen OK (with DEEPBIND for self-contained parser symbols)");
    auto* createFn = reinterpret_cast<Parser_if* (*)(Model*, Sampler_if*)>(
        dlsym(_dynamicLibraryHandle, "genesys_createParser"));
    if (createFn == nullptr) {
        log("dlsym genesys_createParser failed: " + std::string(dlerror()));
        dlclose(_dynamicLibraryHandle);
        _dynamicLibraryHandle = nullptr;
        return false;
    }
    log("dlsym OK");
    Sampler_if* sampler = (_model->getParser() != nullptr) ? _model->getParser()->releaseSampler() : nullptr;
    Parser_if* newParserInstance = createFn(_model, nullptr);
    if (newParserInstance == nullptr) {
        log("createFn returned nullptr");
        dlclose(_dynamicLibraryHandle);
        _dynamicLibraryHandle = nullptr;
        return false;
    }
    log("Parser created OK");
    if (sampler != nullptr) newParserInstance->setSampler(sampler);
    _model->setParser(newParserInstance);
    log("New parser connected successfully.");
    return true;
}
```

---

## 9. Arquivos Modificados (Detalhamento)

### 9.1 `source/plugins/PluginConnectorDummyImpl1.cpp`

- **Linhas 79-80**: adicionado `#include "data/Template/DemoPlugin.h"`
- **Linhas 214-215**: `DemoPlugin` inserido na lista de nomes de arquivo reconhecidos pelo `find()`
- **Linhas 411-414**: mapeamento `filename == "demo.so"` → `new Plugin(&DemoPlugin::GetPluginInformation)` no `connect()`

Isso torna o `DemoPlugin` descobrível pelo `PluginManager` via `PluginConnectorDummyImpl1`, sem necessidade de carregamento dinâmico de `.so`.

### 9.2 `source/kernel/simulator/ParserManager.h`

- **Linha 45**: assinatura de `connectNewParser()` alterada de `bool connectNewParser(NewParser)` para `bool connectNewParser(NewParser, std::string* errorMessage = nullptr)`

### 9.3 `source/kernel/simulator/ParserManager.cpp`

- **Linhas 167-187**: `_buildCompilerCommand()` modificado para incluir `ParserDefaultImpl2.cpp` na compilação do `.so`:

```cpp
cmd += _sourceDir + "/source/kernel/simulator/ParserDefaultImpl2.cpp ";
```

Sem essa inclusão, o `.so` não tinha a implementação do parser e falhava na resolução de símbolos.

### 9.4 `source/kernel/simulator/model/Model.h`

- **Linha 322**: novo membro privado adicionado:

```cpp
bool _parserIsStale = true;
```

- Novos métodos públicos declarados:

```cpp
void setParserIsStale(bool stale);
bool isParserStale() const;
```

### 9.5 `source/kernel/simulator/model/Model.cpp`

- **Linhas 130-148**: no construtor `Model::Model()`, configuração automática do `ParserManager`:

```cpp
_parserManager = new ParserManager();
_parserManager->setModel(this);
{
    std::filesystem::path marker = "source/parser/parserBisonFlex/bisonparser.yy";
    std::filesystem::path sourceRoot = std::filesystem::canonical("/proc/self/exe").parent_path();
    for (int i = 0; i < 8; ++i) {
        if (std::filesystem::exists(sourceRoot / marker)) break;
        sourceRoot = sourceRoot.parent_path();
    }
    if (!std::filesystem::exists(sourceRoot / marker)) {
        sourceRoot = std::filesystem::current_path();
        for (int i = 0; i < 6; ++i) {
            if (std::filesystem::exists(sourceRoot / marker)) break;
            sourceRoot = sourceRoot.parent_path();
        }
    }
    _parserManager->setSourceDir(sourceRoot.string());
    _parserManager->setWorkDir((std::filesystem::temp_directory_path() / "genesys_parser_auto").string());
}
```

O código sobe a árvore de diretórios a partir do executável procurando pelo arquivo marcador `source/parser/parserBisonFlex/bisonparser.yy`.

- **Linhas 357-387**: `parseExpression(bool&, string&)` ganhou lazy regeneration:

```cpp
double Model::parseExpression(const std::string expression, bool& success, std::string& errorMessage) {
    if (_parserIsStale) {
        _parserIsStale = false;
        auto allChanges = _parserManager->aggregateChanges();
        if (!allChanges.empty()) {
            auto* combined = new ParserChangesInformation();
            for (auto* ch : allChanges) {
                if (ch == nullptr) continue;
                combined->setTokens(combined->gettokens() + ch->gettokens());
                combined->setFunctionProdutions(combined->getfunctionProdutions() + ch->getfunctionProdutions());
                combined->setLexicalRules(combined->getlexicalRules() + ch->getlexicalRules());
                combined->setLexicalLiterals(combined->getlexicalLiterals() + ch->getlexicalLiterals());
            }
            auto result = _parserManager->generateNewParser(combined);
            if (result.result) {
                std::string connectError;
                if (!_parserManager->connectNewParser(result.newParser, &connectError)) {
                    errorMessage = "Lazy parser regeneration failed: " + connectError;
                    success = false;
                    return 0.0;
                }
            } else {
                errorMessage = "Lazy parser regeneration failed: generation error";
                success = false;
                return 0.0;
            }
        }
    }
    double value = _parser->parse(expression, success, errorMessage);
    return value;
}
```

- Novos métodos implementados no final do arquivo:

```cpp
void Model::setParserIsStale(bool stale) { _parserIsStale = stale; }
bool Model::isParserStale() const { return _parserIsStale; }
```

### 9.6 `source/kernel/simulator/model/ModelDataManager.cpp`

- **Linha 16**: inclusão de `#include "../ParserChangesInformation.h"`
- **Linhas 45-49**: após `CreateInternalData()`, detecta se o elemento tem parser changes e marca o modelo como stale:

```cpp
ParserChangesInformation* changes = anElement->_getParserChangesInformation();
if (changes != nullptr) {
    _parentModel->setParserIsStale(true);
}
```

### 9.7 `source/applications/gui/qt/GenesysQtGUI/controllers/DialogUtilityController.cpp`

- **Linhas 17-21**: inclusões adicionadas:

```cpp
#include "kernel/simulator/ParserManager.h"
#include <filesystem>
```

- **Linhas 1109-1115**: botão "Regenerate Parser" adicionado na aba Expression Console do Parser Manager:

```cpp
QPushButton* regenerateBtn = new QPushButton(QObject::tr("Regenerate Parser"));
parserButtonsLayout->addWidget(regenerateBtn);
```

- **Linhas 1328-1335**: handler do botão conectado ao pipeline de regeneração com captura de erros de `connectNewParser`:

```cpp
QObject::connect(regenerateBtn, &QPushButton::clicked, [model, appendConsole]() {
    std::string errorMsg;
    bool ok = model->getParserManager()->connectNewParser(..., &errorMsg);
    if (!ok) appendConsole(QString::fromStdString("! Failed to connect new parser:\n" + errorMsg));
});
```

### 9.8 `source/applications/gui/qt/GenesysQtGUI/CMakeLists.txt`

- **Linha 63**: `target_link_options(genesys_qt_gui_application PRIVATE -rdynamic)` adicionado após `target_compile_features`.

Isso exporta todos os símbolos das bibliotecas estáticas linkadas para o dynamic linker, permitindo que o `.so` do parser dinâmico resolva símbolos como `_ZTI5Queue` no processo host.

### 9.9 `source/applications/terminal/GenesysShell/GenesysShell.h`

- **Linhas 63-83**: declaração do método `cmdDynamicParser()` adicionada à classe `GenesysShell`.

### 9.10 `source/applications/terminal/GenesysShell/GenesysShell.cpp`

- **Linhas 7-36**: inclusões de headers do parser (`ParserManager.h`, `ParserChangesInformation.h`, `PluginManager.h`, `Plugin.h`, `<filesystem>`) adicionadas.
- **Linhas 236-256**: comando `dynamicparser` registrado no mapa de comandos do shell.
- **Linhas 680-778**: implementação completa de `cmdDynamicParser()`:

```cpp
bool GenesysShell::cmdDynamicParser() {
    PluginManager* pm = simulator->getPluginManager();
    ModelDataDefinition* demo = pm->newInstance("DemoPlugin", simulator->getModels()->current(), "Demo_1");
    if (demo == nullptr) { std::cout << "Failed to create DemoPlugin" << std::endl; return false; }
    simulator->getModels()->current()->getDataManager()->insert(demo);
    // ... pipeline de regeneração completo ...
}
```

### 9.11 `source/tests/unit/test_parser_expressions.cpp`

- Adicionado teste `DynamicParserDemoPluginAddsNewFunction` que valida o pipeline end-to-end: cria DemoPlugin, coleta changes, gera parser, compila, carrega dinamicamente, avalia `demo(4)` e confirma resultado `5`.

---

## 10. Timeline de Debugging e Soluções

### 10.1 "Plugin DemoPlugin não aparece na GUI"

**Sintoma:** O plugin não estava visível na lista de plugins da GUI.

**Causa:** O `DemoPlugin` não estava registrado no `PluginConnectorDummyImpl1`, que é o conector de plugins estático usado pelo `PluginManager` quando não há carregamento dinâmico de `.so`.

**Fix:** Registro em três pontos do `PluginConnectorDummyImpl1.cpp`:
- Inclusão do header (linhas 79-80)
- Retorno do `GetPluginInformation` no `find()` (linhas 214-215)
- Criação do `Plugin` no `connect()` (linhas 411-414)

### 10.2 "! Failed to connect new parser"

**Sintoma:** Botão "Regenerate Parser" na GUI reportava falha na conexão.

**Causa:** `connectNewParser()` chamava `dlopen()` e falhava, mas só reportava `return false` sem detalhes.

**Fix 1:** Alterar assinatura de `connectNewParser(NewParser)` para `connectNewParser(NewParser, std::string* errorMessage = nullptr)`.

**Fix 2:** Reescrever `connectNewParser` para acumular mensagens detalhadas em `errorMessage` em vez de `std::cerr`.

**Fix 3:** Atualizar o handler do botão na GUI para capturar e exibir `errorMessage` no console.

### 10.3 `dlopen failed: undefined symbol: _ZTI5Queue`

**Sintoma:** Após os fixes de error reporting, a mensagem exata apareceu: `undefined symbol: _ZTI5Queue`.

**Causa:** O `.so` do parser dinâmico referencia o typeinfo da classe `Queue` (usada em regras gramaticais do Bison), mas o `.so` não foi linkado com `genesys_plugins_data`. O processo host (GUI) tinha esses símbolos, mas não os exportava para `dlopen`.

**Fix:** Adicionar `target_link_options(genesys_qt_gui_application PRIVATE -rdynamic)` no `CMakeLists.txt` do executável GUI. Isso força o linker a exportar todos os símbolos das bibliotecas estáticas linkadas para o dynamic linker, tornando-os visíveis para `dlopen`.

### 10.4 "Parser conectado mas demo() não reconhecido"

**Sintoma:** Após `-rdynamic`, `dlopen` funcionava e o console reportava "New parser connected", mas `demo(5)` retornava `! Literal nao encontrado: "demo"`.

**Causa:** O parser dinâmico estava sendo carregado com `RTLD_NOW` (sem `RTLD_DEEPBIND`). Sem `RTLD_DEEPBIND`, o `ParserDefaultImpl2` do `.so` usava o parser estático do processo host (que não conhecia `demo`). Os símbolos Bison/Flex do host tinham prioridade.

**Fix:** Reverter para `RTLD_NOW | RTLD_DEEPBIND` no `dlopen`. Com `-rdynamic` no host, os símbolos `Queue` etc. são resolvidos contra o host. Com `RTLD_DEEPBIND`, o parser Bison/Flex do `.so` tem prioridade sobre o do host.

### 10.5 Erros de linkagem na compilação do `.so`

**Sintoma:** `g++ -shared` falhava com símbolos do parser não definidos.

**Causa:** O comando de compilação do `.so` incluía `GenesysParser.cpp`, `Genesys++-scanner.cpp`, `Genesys++-driver.cpp`, `obj_t.cpp`, `ParserFactory.cpp`, mas não `ParserDefaultImpl2.cpp`.

**Fix:** Adicionar `ParserDefaultImpl2.cpp` ao comando de compilação em `_buildCompilerCommand()`.

### 10.6 "Botão manual é ruim de UX"

**Sintoma:** Usuário precisava clicar em "Regenerate Parser" manualmente após adicionar o plugin.

**Causa:** Arquiteturalmente, a regeneração só acontecia via botão.

**Fix — Lazy Regeneration:**
1. `ModelDataManager::insert()` detecta `_getParserChangesInformation() != nullptr` e chama `_parentModel->setParserIsStale(true)`
2. `Model::parseExpression(bool&, string&)` verifica `_parserIsStale` antes de avaliar. Se true, executa o pipeline completo (aggregate → generate → connect), depois avalia a expressão
3. A primeira chamada de `parseExpression()` após inserir um plugin dispara a regeneração automaticamente

---

## 12. Análise do Git Diff (`2026-1` → `kimi-v2`)

Esta seção analisa cada mudança real no código-fonte, arquivo por arquivo, explicando o propósito didático de cada alteração.

---

### 12.1 Subtask: Criar o Plugin de Demonstração (DemoPlugin)

**Arquivos novos:** `source/plugins/data/Template/DemoPlugin.h`, `source/plugins/data/Template/DemoPlugin.cpp`

**Propósito:** Criar um plugin que "ensine" ao parser uma nova função.

**O que o plugin faz:**
- É um `ModelDataDefinition` como Queue ou Resource
- Quando instanciado no modelo, declara via `_getParserChangesInformation()` que o parser precisa aprender a função `demo(x)`
- O método retorna um `ParserChangesInformation*` com três injeções:
  1. **Token `fDEMO`** — o Bison precisa saber que existe esse token
  2. **Produção gramatical** — regra `pluginFunction: fDEMO "(" expression ")"` com ação semântica `$$.valor = $3.valor + 1`
  3. **Regra léxica** — o scanner Flex precisa reconhecer a palavra `demo` e emitir o token `fDEMO`

**Por que isso importa:** Este é o **ponto de extensão**. Qualquer plugin novo pode implementar `_getParserChangesInformation()` e adicionar suas próprias funções à linguagem de expressões sem tocar no parser base.

---

### 12.2 Subtask: Registrar o Plugin no Sistema de Descoberta

**Arquivo modificado:** `source/plugins/PluginConnectorDummyImpl1.cpp`

**Mudança:** Adicionado `DemoPlugin` em três pontos:
- **Inclusão do header:** `#include "data/Template/DemoPlugin.h"`
- **`find()`:** `filenames->insert("demo.so")` — o nome fictício que o `PluginManager` procura
- **`connect()`:** `else if (fn == "demoplugin.so") GetInfo = &DemoPlugin::GetPluginInformation;` — mapeia o nome para a função estática que retorna metadados do plugin

**Propósito:** O `PluginConnectorDummyImpl1` é um conector **estático** usado quando o GenESyS não carrega plugins de arquivos `.so` dinamicamente. Ele mapeia nomes de arquivo para classes compiladas internamente. Sem esse registro, o `PluginManager` nunca encontraria o `DemoPlugin`.

**Didática:** Pense nisso como uma "lista telefônica" de plugins. O `PluginManager` diz "procure demo.so" e o conector responde "ah, demo.so é na verdade a classe `DemoPlugin` compilada aqui dentro".

---

### 12.3 Subtask: Adicionar o Ponto de Extensão na Classe Base

**Arquivo modificado:** `source/kernel/simulator/model/ModelDataDefinition.h`

**Mudança:** Adicionado método virtual:

```cpp
virtual ParserChangesInformation* _getParserChangesInformation();
```

**Propósito:** Antes dessa mudança, nenhuma classe no GenESyS tinha um método padronizado para declarar "eu tenho mudanças no parser". Adicionar esse método virtual na classe base `ModelDataDefinition` significa que **qualquer** elemento do modelo (Queue, Resource, Attribute, etc.) pode opcionalmente implementar suas próprias contribuições gramaticais.

**Didática:** É como adicionar um novo "soquete" na parede. Todos os aparelhos (plugins) já têm a tomada compatível. Quem quiser, conecta. Quem não quiser, deixa o método retornar `nullptr` (comportamento padrão).

---

### 12.4 Subtask: Criar a Fábrica de Parsers Dinâmicos

**Arquivos novos:** `source/parser/dynamic/ParserFactory.h`, `source/parser/dynamic/ParserFactory.cpp`

**Mudança:** Duas funções C com linkage C:

```cpp
extern "C" Parser_if* genesys_createParser(Model* model, Sampler_if* sampler);
extern "C" void genesys_destroyParser(Parser_if* parser);
```

**Propósito:** Quando o `ParserManager` compila um parser novo em tempo de execução, ele precisa criar uma instância desse parser. Como o parser é compilado em uma biblioteca `.so` separada, o código do host não conhece os construtores dessa biblioteca. A factory C expõe uma função com **nome C** (sem name mangling), que pode ser encontrada via `dlsym()`.

**Didática:** Imagine que você está construindo um carro em uma fábrica diferente. Você não sabe como o carro é montado lá dentro, mas a fábrica tem uma porta de entrega: `genesys_createParser`. Você bate na porta, entrega um modelo e um sampler, e recebe um parser pronto.

---

### 12.5 Subtask: Implementar o Runtime de Carregamento Dinâmico

**Arquivo novo:** `source/kernel/simulator/ParserManagerRuntime.cpp`

**Contém duas funções críticas:**

#### `aggregateChanges()`

Percorre **todas** as instâncias no `ModelDataManager` e pergunta a cada uma: "você tem parser changes?"

```cpp
for (const std::string& classname : classnames) {
    for (ModelDataDefinition* mdd : *list->list()) {
        ParserChangesInformation* changes = mdd->_getParserChangesInformation();
        if (changes != nullptr) result.push_back(changes);
    }
}
```

**Didática:** É como um professor que, antes de começar a aula, pergunta a cada aluno: "você tem algo novo para contribuir?". Todos que tiverem algo relevante levantam a mão.

#### `connectNewParser()`

Carrega a biblioteca `.so` e substitui o parser do modelo:

1. `dlopen(path, RTLD_NOW | RTLD_DEEPBIND)` — carrega a biblioteca
2. `dlsym(handle, "genesys_createParser")` — encontra a fábrica
3. `createFn(model, nullptr)` — cria o parser novo
4. `releaseSampler()` no parser antigo + `setSampler()` no novo — transfere o sampler (estado do gerador de números aleatórios)
5. `model->setParser(newParser)` — troca o parser

**Didática:** É como trocar o motor de um carro em movimento. Você desliga o motor velho, tira a bateria (sampler), coloca no motor novo, e liga de novo. O carro (modelo) continua rodando com o motor novo.

---

### 12.6 Subtask: Implementar a Geração do Parser

**Arquivo modificado:** `source/kernel/simulator/ParserManager.cpp`

**Mudanças:**
- **`generateNewParser()`**: Pipeline completo:
  1. Lê os arquivos base `bisonparser.yy` e `lexerparser.ll`
  2. Injeta as mudanças dos plugins nos marcadores `/**begin_Tokens_plugins**/`, `/**begin_FunctionProdution_plugins**/
  3. Escreve os arquivos modificados em um diretório temporário
  4. Copia os fontes auxiliares (`Genesys++-driver.cpp`, `obj_t.cpp`, `ParserFactory.cpp`)
  5. Chama `bison` para gerar o parser C++
  6. Chama `flex` para gerar o scanner C++
  7. Compila com `g++ -fPIC -shared` incluindo `ParserDefaultImpl2.cpp`

- **`_buildCompilerCommand()`**: Adicionado `ParserDefaultImpl2.cpp` ao comando de compilação. Sem isso, o `.so` não tinha a implementação do parser e falhava na resolução de símbolos.

**Didática:** É como receber um livro de receitas (arquivos base do Bison/Flex), anotar novas receitas (injeção das mudanças), fotocopiar (bison/flex gerando código), e encadernar (compilação com g++). O livro final é a biblioteca `.so`.

---

### 12.7 Subtask: Expôr Símbolos do Host para o Parser Dinâmico

**Arquivo modificado:** `source/applications/gui/qt/GenesysQtGUI/CMakeLists.txt`

**Mudança:** Adicionado:

```cmake
target_link_options(genesys_qt_gui_application PRIVATE -rdynamic)
```

**Propósito:** O executável da GUI linka estaticamente com `genesys_plugins_data`, `genesys_kernel_simulator_support`, etc. O parser dinâmico referencia tipos como `Queue` (via regras gramaticas do Bison), mas a biblioteca `.so` não foi linkada com essas bibliotecas. O flag `-rdynamic` força o linker a **exportar todos os símbolos** do executável para o dynamic linker, tornando-os disponíveis para `dlopen`.

**Didática:** Imagine um prédio (executável) com várias salas (bibliotecas estáticas). Normalmente, as salas são acessíveis apenas de dentro do prédio. `-rdynamic` é como colocar placas de rua visíveis do lado de fora, para que visitantes (bibliotecas `.so` carregadas dinamicamente) saibam onde encontrar cada sala.

---

### 12.8 Subtask: Adicionar o Botão de Regeneração na GUI

**Arquivo modificado:** `source/applications/gui/qt/GenesysQtGUI/controllers/DialogUtilityController.cpp`

**Mudanças:**
- Inclusão de `ParserManager.h`
- Adição do botão "Regenerate Parser" no layout da aba Expression Console
- Handler conectado ao `clicked` que:
  1. Obtém o `ParserManager` do modelo
  2. Coleta mudanças, gera parser, conecta
  3. Reporta sucesso ou erro detalhado no console

**Propósito:** Fornecer uma interface manual para regenerar o parser. Depois da implementação da lazy regeneration, este botão tornou-se um fallback/opção avançada.

---

### 12.9 Subtask: Implementar a Regeneração Preguiçosa (Lazy)

**Arquivos modificados:** `source/kernel/simulator/model/Model.h`, `source/kernel/simulator/model/Model.cpp`, `source/kernel/simulator/model/ModelDataManager.cpp`

#### `Model.h` — estado e API

Adicionado:
- `bool _parserIsStale = true;` — flag que indica se o parser precisa ser regenerado
- `ParserManager* _parserManager = nullptr;` — ponteiro para o gerenciador de parser
- `void setParser(Parser_if* parser);` — método público para substituir o parser
- `Parser_if* getParser() const;` — getter do parser
- `ParserManager* getParserManager() const;` — getter do ParserManager

#### `Model.cpp` — inicialização e lógica lazy

No construtor:
```cpp
_parserManager = new ParserManager();
```

No destrutor:
```cpp
delete _parserManager;
_parserManager = nullptr;
```

Implementação de `setParser()` com transferência do sampler:
```cpp
void Model::setParser(Parser_if* parser) {
    Sampler_if* currentSampler = (_parser != nullptr) ? _parser->releaseSampler() : nullptr;
    if (parser != nullptr && currentSampler != nullptr) parser->setSampler(currentSampler);
    delete _parser;
    _parser = parser;
}
```

#### `ModelDataManager.cpp` — detectar mudanças

Após inserir um elemento:
```cpp
ParserChangesInformation* changes = anElement->_getParserChangesInformation();
if (changes != nullptr) {
    _parentModel->setParserIsStale(true);
}
```

**Didática:** O sistema funciona como um "alarme de cozinha". Quando você adiciona um ingrediente novo (plugin com parser changes), o alarme toca (`_parserIsStale = true`). Na próxima vez que alguém pedir para provar a comida (`parseExpression`), o chef primeiro verifica se o alarme tocou. Se sim, ele reúne todos os ingredientes novos, reescreve a receita, reimprime o livro de receitas, e só depois serve o prato.

---

### 12.10 Subtask: Adicionar Comando no Terminal

**Arquivos modificados:** `source/applications/terminal/GenesysShell/GenesysShell.h`, `source/applications/terminal/GenesysShell/GenesysShell.cpp`

**Mudanças:**
- `GenesysShell.h`: Declaração do método `cmdDynamicParser()`
- `GenesysShell.cpp`:
  - Registro do comando no mapa de comandos do shell
  - Inclusão de headers do parser
  - Implementação completa de `cmdDynamicParser()` que:
    1. Cria uma instância `DemoPlugin` via `PluginManager::newInstance()`
    2. Insere no `ModelDataManager`
    3. Executa `aggregateChanges()` → `generateNewParser()` → `connectNewParser()`
    4. Testa `demo(5)` e `demo(demo(2))`

**Propósito:** Demonstrar que todo o pipeline funciona sem a GUI. O terminal prova que a substituição de parser em runtime é independente de Qt.

---

### 12.11 Subtask: Adicionar Teste Unitário

**Arquivo modificado:** `source/tests/unit/test_parser_expressions.cpp`

**Mudança:** Adicionado teste `DynamicParserDemoPluginAddsNewFunction` que:
1. Cria um modelo e um `DemoPlugin`
2. Insere o plugin no `ModelDataManager`
3. Obtém o `ParserManager` do modelo
4. Chama `aggregateChanges()`
5. Combina todas as mudanças em um único `ParserChangesInformation`
6. Chama `generateNewParser()`
7. Chama `connectNewParser()`
8. Avalia `demo(5)` e confirma resultado `6`
9. Avalia `demo(0)` e confirma resultado `1`
10. Avalia `demo(demo(2))` e confirma resultado `4`

**Propósito:** Teste de regressão que garante que o pipeline end-to-end funciona corretamente.

---

### 12.12 Subtask: Melhorar Diagnóstico de Erros

**Arquivo modificado:** `source/kernel/simulator/ParserManager.h`

**Mudança:** Assinatura de `connectNewParser` alterada:

```cpp
// Antes:
bool connectNewParser(NewParser newParser);

// Depois:
bool connectNewParser(NewParser newParser, std::string* errorMessage = nullptr);
```

**Propósito:** Antes, quando `dlopen` falhava, o método só retornava `false` e logava em `std::cerr` (invisível na GUI). Com o parâmetro `errorMessage`, a GUI pode capturar a mensagem exata de `dlerror()` e exibi-la no console do Expression Console.

**Didática:** É como trocar um sistema de alarme que só apita (retorna false) por um que também diz "alarme de incêndio no 3º andar, sala 302" (mensagem detalhada).

---

## 13. Conclusão

A implementação do parser dinâmico no GenESyS foi bem-sucedida. A arquitetura adotada (injeção em Bison/Flex + compilação dinâmica + `dlopen` com `RTLD_DEEPBIND` + host com `-rdynamic`) é robusta e permite futuras extensões sem modificar o código-fonte do parser base. A regeneração lazy elimina a necessidade de intervenção manual do usuário.

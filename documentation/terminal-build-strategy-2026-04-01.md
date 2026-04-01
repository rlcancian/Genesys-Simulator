# Estratégia de build para `source/applications/terminal`

Data: 2026-04-01

## 1) Como está hoje (resumo técnico)

- O `main.cpp` instancia `TraitsTerminalApp<GenesysApplication_if>::Application` e chama `main(...)` da classe selecionada.
- A seleção da aplicação está centralizada em `TraitsTerminalApp.h` via `typedef ... Application;`.
- Existem dezenas de aplicações em `examples/` (smarts, arenaSmarts, arenaExamples, teaching, book).
- Muitas dessas aplicações têm dependências cruzadas com `kernel` e `plugins/components`, e algumas têm código legado/experimental que pode quebrar quando se tenta compilar “tudo de uma vez”.

## 2) Problema arquitetural principal

A escolha da aplicação por `typedef` é simples, mas **não escala** para múltiplos perfis de build porque:

1. mistura seleção de app com composição do link;
2. dificulta compilar “qualquer app” sem editar header;
3. incentiva estratégia de incluir muitos `.cpp` no mesmo executável, aumentando tempo de build e chance de quebra.

## 3) Recomendação de estratégia (baixa complexidade)

## 3.1. Separar em 3 alvos CMake

1. `genesys_terminal_core` (STATIC)
   - `main.cpp`, `GenesysShell`, `BaseGenesysTerminalApplication` e infraestrutura comum.
2. `genesys_terminal_examples` (STATIC)
   - exemplos (idealmente por grupos).
3. `genesys_terminal_application` (EXECUTABLE)
   - linka `core` + o subconjunto de exemplos selecionado.

Benefício: o executável final continua único, mas a organização de fontes fica previsível.

## 3.2. Seleção da app por variável CMake (sem editar header)

Introduzir variável de cache, por exemplo:

- `GENESYS_TERMINAL_APP_CLASS` (string), exemplos:
  - `Smart_SeizeDelayReleaseMany`
  - `GenesysShell`
  - `Smart_BasicModeling`

E gerar um header em configure-time (`configured_terminal_app.h`) com:

```cpp
using SelectedTerminalApplication = <valor da variável>;
```

Então o `main.cpp` passa a incluir esse header gerado.

Benefício: trocar aplicação vira **comando de build**, não edição de código.

## 3.3. Seleção de grupos de exemplos via opções

Sugestão de flags (booleans):

- `GENESYS_TERMINAL_BUILD_SMARTS`
- `GENESYS_TERMINAL_BUILD_ARENA_SMARTS`
- `GENESYS_TERMINAL_BUILD_ARENA_EXAMPLES`
- `GENESYS_TERMINAL_BUILD_TEACHING`
- `GENESYS_TERMINAL_BUILD_BOOK`

Começar com default conservador (`OFF`) e habilitar apenas o grupo necessário.

Benefício: evita build “gigante” e reduz regressões.

## 3.4. Catálogo explícito de apps suportadas

Criar uma tabela (CMake list) `nome_da_app -> arquivo.cpp` para as aplicações oficialmente suportadas no build do terminal.

- evita depender de `GLOB_RECURSE` para tudo;
- permite mensagem de erro amigável se `GENESYS_TERMINAL_APP_CLASS` inválida.

## 4) O que evitar

- Evitar compilar recursivamente todos os `.cpp` de `examples/**` por padrão.
- Evitar fallback silencioso para app “dummy” no build oficial (pode mascarar erro de configuração).
- Evitar acoplamento entre “seleção da app” e “seleção de plugins/componentes”.

## 5) Plano incremental recomendado

Fase A (rápida)
1. Introduzir `GENESYS_TERMINAL_APP_CLASS`.
2. Gerar header `configured_terminal_app.h`.
3. Validar nome da app e falhar com erro claro quando inválido.

Fase B (estabilização)
1. Criar `genesys_terminal_core`.
2. Quebrar exemplos por grupos com opções ON/OFF.
3. Manter preset `terminal-app` mínimo e criar presets por família (`terminal-smarts`, etc.).

Fase C (escalabilidade)
1. Migrar gradualmente para catálogo explícito de apps.
2. Opcional: um executável por app (`genesys_terminal_<app>`) para CI granular.

## 6) Comandos de uso sugeridos (alvo único, app configurável)

Exemplo de experiência desejada:

```bash
cmake --preset terminal-app -DGENESYS_TERMINAL_APP_CLASS=Smart_SeizeDelayReleaseMany
cmake --build --preset terminal-app
```

Ou:

```bash
cmake --preset terminal-app -DGENESYS_TERMINAL_APP_CLASS=GenesysShell
cmake --build --preset terminal-app
```

Isso resolve a necessidade de compilar “qualquer aplicação terminal” com complexidade controlada.

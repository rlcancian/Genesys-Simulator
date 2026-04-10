# Estratégia de build para `source/applications/terminal`

Data: 2026-04-01

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: reauditoria do roadmap de build terminal contra `CMakeLists.txt`, `main.cpp`, `TraitsTerminalApp.h` e presets
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`
- Data desta reauditoria documental: `2026-04-10`

## 1) Como está hoje (revalidado)

- Existe `source/applications/terminal/CMakeLists.txt` com alvo executável `genesys_terminal_application`.
- `main.cpp` define `SelectedTerminalApplication` e usa `TraitsTerminalApp<GenesysApplication_if>::Application` quando `GENESYS_TERMINAL_USE_TRAITS_APP=1`.
- Há opção CMake `GENESYS_TERMINAL_BUILD_ALL_SOURCES` (global) para compilar tudo recursivamente.
- Preset `terminal-app` existe em `CMakePresets.json` e habilita `GENESYS_TERMINAL_APPLICATION=ON`.

### Audit status
`PARTIAL`

### Evidence
- Itens principais do fluxo terminal já estão implementados e funcionais.
- A seleção de app segue centrada em `TraitsTerminalApp.h` (edição de header).

### Remaining gaps
- Estratégia proposta no documento original foi implementada apenas em parte e com diferenças de desenho.

## 2) Reclassificação do plano original

### 2.1 Separar em 3 alvos (`core`, `examples`, `application`)
### Audit status
`SUPERSEDED`

### Evidence
- Solução atual funciona com executável único `genesys_terminal_application` e link explícito de libs do kernel/parser/plugins.
- Não há sinais de `genesys_terminal_core`/`genesys_terminal_examples` como no plano original.

### Remaining gaps
- Se desejado, pode virar melhoria de organização/tempo de build, mas não bloqueia operação atual.

### 2.2 Seleção da app por variável CMake (`GENESYS_TERMINAL_APP_CLASS`) + header gerado
### Audit status
`OPEN`

### Evidence
- Não há variável `GENESYS_TERMINAL_APP_CLASS` no CMake atual.
- Não há header gerado em configure-time para seleção da app.

### Remaining gaps
- Implementar seleção por cache variable para remover necessidade de editar `TraitsTerminalApp.h`.

### 2.3 Flags granulares por grupos (`SMARTS`, `ARENA_SMARTS`, `TEACHING`, `BOOK`...)
### Audit status
`OPEN`

### Evidence
- O build expõe apenas o switch global `GENESYS_TERMINAL_BUILD_ALL_SOURCES`.

### Remaining gaps
- Introduzir flags por família de exemplos para controlar escopo de compilação.

### 2.4 Catálogo explícito de apps suportadas
### Audit status
`PARTIAL`

### Evidence
- Existe lista explícita de alguns `.cpp` no modo default do CMake atual (sem glob total).
- Porém não há validação declarativa do nome de app escolhido via variável dedicada.

### Remaining gaps
- Falta mapear formalmente “nome de app -> fonte” com validação de entrada de configuração.

## 3) O que foi entregue de forma diferente da proposta antiga

### Audit status
`DONE`

### Evidence
- Foi adotado um caminho pragmático: alvo único + seleção via traits + opção global de build total.
- Isso resolve parte do objetivo de operabilidade com complexidade menor que o plano em 3 alvos.

### Remaining gaps
- Para escalabilidade de CI e reprodutibilidade de seleção de app, o backlog arquitetural segue aberto.

## 4) Conclusão da reauditoria
O roadmap de 2026-04-01 está parcialmente superado: o núcleo da aplicação terminal já existe e está integrado a presets e build padrão, mas a parte de parametrização forte por CMake (classe selecionada, header gerado e flags granulares por grupo) não foi implementada até o estado auditado.

## Remaining Work
- `OPEN` — Criar `GENESYS_TERMINAL_APP_CLASS` e validar valor no configure.
- `OPEN` — Gerar header de seleção de app em configure-time para desacoplar de `TraitsTerminalApp.h`.
- `OPEN` — Adicionar flags granulares por famílias de exemplos (SMARTS/ARENA_SMARTS/TEACHING/BOOK...).
- `PARTIAL` — Evoluir de lista estática parcial para catálogo explícito e validado de aplicações suportadas.

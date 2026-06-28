# DCS development baseline

## Estado inicial

- Branch atual: `dcs-parser-function-registry`
- Branch base assumida: `currentStable`
- Verificacao de ancestralidade: `currentStable` e ancestral de `HEAD`
- Remotos observados:
  - `origin`: `https://github.com/joaomdaltoe/Genesys-Simulator.git`
  - `upstream`: `https://github.com/rlcancian/Genesys-Simulator`
- Estado local antes deste registro: sem alteracoes locais reportadas por `git status --short --branch`

## Objetivo do DCS

O objetivo inicial do DCS nesta branch e preparar uma evolucao controlada do suporte a funcoes dinamicas no simulador, mantendo o comportamento atual como referencia. A primeira fronteira tecnica deve permitir registrar e resolver funcoes por meio de um `FunctionRegistry`, criando uma base explicita para extensao sem acoplar a etapa inicial a mudancas amplas no parser, no kernel ou no sistema de build.

## Decisao inicial

A abordagem inicial sera incremental por `FunctionRegistry`.

Esta decisao evita, nesta fase, uma regeneracao dinamica completa de Flex/Bison. O parser existente continua sendo tratado como a fronteira estavel do sistema enquanto o desenho do DCS e consolidado em passos menores, rastreaveis e testaveis.

## Diretorios relevantes

- `source/kernel/simulator`: nucleo do simulador, incluindo modelos, componentes, persistencia, plugins, parser manager e controle de simulacao.
- `source/parser`: implementacao atual do parser, arquivos gerados e fontes Flex/Bison em `parserBisonFlex/`.
- `source/plugins`: conectores de plugins, componentes e definicoes de dados usadas pelo simulador.
- `source/tests`: testes existentes de smoke, unidade, parser, runtime e suporte ao kernel.
- `CMakeLists.txt`: configuracao CMake de topo do projeto.
- `CMakePresets.json`: presets de configuracao e build do CMake.

## Restricoes desta etapa

- Nao modificar parser.
- Nao modificar kernel.
- Nao modificar CMake.
- Nao criar classes novas.
- Nao executar refatoracao.
- Nao fazer commit automaticamente.

## Proximos passos tecnicos

1. Mapear os pontos atuais onde funcoes sao reconhecidas, avaliadas ou encaminhadas pelo parser.
2. Definir o contrato minimo do `FunctionRegistry`, incluindo nome da funcao, aridade, tipos aceitos e comportamento de erro.
3. Planejar testes focados para preservar o comportamento atual antes de integrar qualquer registro de funcoes.
4. Identificar a menor superficie de integracao entre parser e registry, mantendo Flex/Bison estatico nesta fase.
5. Registrar explicitamente qualquer futura alteracao em parser, kernel ou CMake em etapas separadas.

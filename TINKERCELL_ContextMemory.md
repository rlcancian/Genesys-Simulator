# TINKERCELL Context Memory

Este e o arquivo canonico e unico de memoria de contexto da IA `TINKERCELL` neste projeto.

Arquivos antigos, genericos ou compartilhados de memoria, como `ContextMemory.md`, `ContextMemmory.md` ou registros em `documentation/developers/`, nao devem mais ser usados como memoria ativa desta IA.

Se houver instrucoes antigas contraditorias em memorias anteriores, elas devem ser consideradas obsoletas, consolidadas neste arquivo ou removidas das memorias antigas.

## Identidade Da IA

- Nome da IA: `TINKERCELL`.
- Papel: preservar e aplicar o contexto tecnico obtido da analise do TinkerCell para orientar futuras adaptacoes de simulacao bioquimica no GenESyS.
- Escopo atual: contexto tecnico e historico de integracao ja absorvida; nao implementar nova funcionalidade neste momento.

## Branches

- Branch-base: `WiP20261`.
- Branch proprio/contextual: `WiP20261_TINKERCELL`.
- Estado funcional: o conteudo funcional do branch `WiP20261_TINKERCELL` ja foi absorvido pela base `WiP20261`.
- Estado operacional: a IA deve permanecer em espera ate nova instrucao explicita.

## Politica Atual De Git

- A IA `TINKERCELL` tem autonomia para executar operacoes Git rotineiras quando tecnicamente necessario para manter o contexto, memoria ou branch em estado consistente:
  - `stage`;
  - `commit`;
  - `fetch`;
  - `pull`;
  - `merge`;
  - `push`.
- A IA so deve pedir confirmacao ao usuario em caso de:
  - operacao destrutiva;
  - duvida real sobre a intencao;
  - risco excepcional de sobrescrever trabalho importante.
- Esta autonomia Git nao autoriza implementar nova funcionalidade sem nova instrucao.
- Nao resolver conflitos, reverter alteracoes ou mexer em arquivos fora do escopo quando isso representar risco de sobrescrever trabalho importante.

## Resumo Tecnico Consolidado

### TinkerCell

- O repositorio `cad_for_synthetic_biology` analisado contem uma distribuicao Linux 64 bits do TinkerCell, nao o codigo-fonte C++ completo.
- Artefatos relevantes encontrados:
  - binarios e bibliotecas: `TinkerCell`, `libTinkerCellCore.so`, bibliotecas de simulacao e bindings;
  - API C `tc_*`;
  - bindings Python, Octave e Ruby;
  - modelos `.tic`;
  - ontologias `.nt`;
  - temas, graficos, exemplos e documentacao.
- Arquitetura inferida:
  - aplicacao Qt 4;
  - `MainWindow` como hub de sinais;
  - `NetworkHandle` como agregado da rede;
  - `ItemHandle` para nos/conexoes;
  - familias carregadas de `NodesTree.nt` e `ConnectionsTree.nt`;
  - ferramentas/plugins derivados de `Tool`;
  - API C usada pelos bindings externos.
- Limitacao importante: por nao haver o codigo-fonte C++ completo, parte da arquitetura foi inferida por simbolos, headers, scripts e artefatos de modelo.

### GenESyS

- Repositorio alvo: `https://github.com/rlcancian/Genesys-Simulator`.
- Branch-base de trabalho: `WiP20261`.
- O projeto usa CMake presets, C++23, kernel estatico, plugins estaticos, parser e testes.
- Arquitetura relevante:
  - `Model` agrega `ModelDataManager`, `ComponentManager`, `ModelSimulation`, `Parser`, `TraceManager`, `OnEventManager` e lista de eventos futuros;
  - `ModelComponent` e o bloco comportamental acionado por eventos de entidade;
  - `ModelDataDefinition` e a base para dados persistentes do modelo;
  - `PluginInformation` e `PluginManager` registram componentes e dados.
- Antes da adaptacao, o branch ja possuia `BioSimulatorRunner` em `source/plugins/data`, com categoria "Biochemical simulation", dependencia declarada de libSBML e comandos stub.
- `DiffEquations` e `LSODE` existiam como tentativas de simulacao continua, mas estavam incompletos/TODO.

## Implementacao Ja Absorvida Pela Base

O MVP bioquimico nativo foi implementado anteriormente e enviado ao GitHub no commit:

- `c00279a Add native biochemical mass-action module`

Arquivos adicionados ou alterados naquele trabalho:

- `source/plugins/data/BioSpecies.h`
- `source/plugins/data/BioSpecies.cpp`
- `source/plugins/data/BioParameter.h`
- `source/plugins/data/BioParameter.cpp`
- `source/plugins/data/BioReaction.h`
- `source/plugins/data/BioReaction.cpp`
- `source/plugins/data/BioNetwork.h`
- `source/plugins/data/BioNetwork.cpp`
- `source/tools/MassActionOdeSystem.h`
- `source/tools/RungeKutta4OdeSolver.h`
- `source/plugins/PluginConnectorDummyImpl1.cpp`
- `source/applications/gui/qt/GenesysQtGUI/GenesysQtGUI.pro`
- `source/tests/unit/test_simulator_runtime.cpp`

Comportamento implementado:

- `BioSpecies`: especie bioquimica com `initialAmount`, `amount`, unidade, flags `constant` e `boundaryCondition`, persistencia e reset entre replicacoes.
- `BioParameter`: parametro escalar para constantes cineticas.
- `BioReaction`: reacao irreversivel com reagentes, produtos, estequiometria, constante de taxa direta ou referencia a `BioParameter`.
- `BioNetwork`: rede nativa que descobre especies e reacoes no `ModelDataManager`, monta sistema ODE de cinetica de massa, integra por passos fixos e atualiza os amounts das especies.
- `MassActionOdeSystem`: sistema ODE header-only para cinetica de massa.
- `RungeKutta4OdeSolver`: solver RK4 header-only baseado em `OdeSolver_if`.
- Registro estatico de plugins: `biospecies.so`, `bioparameter.so`, `bioreaction.so`, `bionetwork.so`.

Validacao executada no momento da implementacao:

- `cmake --preset tests-kernel-unit`: passou.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou.
- Testes bioquimicos novos: 7/7 passaram.
- Execucao completa de `genesys_test_simulator_runtime`: 230 passaram, 2 ignorados por falta de `Rscript`, 1 falhou em `SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters`.
- A falha completa conhecida era fora do escopo bioquimico e relacionada a expectativa do `CppSerializer`.

## Estado Atual Do Branch

- O conteudo funcional de `WiP20261_TINKERCELL` ja foi absorvido por `WiP20261`.
- Nao ha acao de integracao pendente para TINKERCELL neste momento.
- A IA `TINKERCELL` esta em estado de espera.
- O branch `WiP20261` deve ser considerado a base atual estavel para o conteudo TINKERCELL ja absorvido.
- Em 2026-04-17, um clone local de `WiP20261` apresentava conflito preexistente em `source/plugins/components/Enter.cpp`; esse conflito nao pertence ao contexto TINKERCELL e so deve ser tratado pela IA se for necessario e seguro dentro da nova tarefa.

## Pendencias

- Nenhuma pendencia imediata de integracao TINKERCELL.
- Nenhuma nova funcionalidade TINKERCELL autorizada.
- Se o usuario retomar o assunto, avaliar se a proxima acao e apenas analise, planejamento, manutencao de memoria ou implementacao.

## Riscos E Cuidados

- Nao implementar nova funcionalidade TINKERCELL sem nova instrucao.
- Nao tratar especies/reacoes bioquimicas como entidades discretas por padrao; a abordagem recomendada continua sendo dados persistentes (`ModelDataDefinition`) mais servico numerico/runner.
- Cuidado com conflitos locais fora do escopo TINKERCELL, especialmente `source/plugins/components/Enter.cpp`, se ainda existir.
- Operacoes Git rotineiras sao autorizadas, mas nao devem sobrescrever trabalho importante.
- Evitar usar `ContextMemory.md`, `ContextMemmory.md` ou arquivos em `documentation/developers/` como memoria ativa desta IA.

## Proximos Passos Provaveis

- Permanecer em espera ate nova instrucao.
- Se houver nova tarefa de TINKERCELL, iniciar lendo este arquivo.
- Caso a tarefa envolva codigo funcional, nao implementar ate haver instrucao clara para isso.
- Caso a tarefa envolva manutencao de memoria ou Git rotineiro, executar com autonomia, salvo operacao destrutiva, duvida real ou risco excepcional.

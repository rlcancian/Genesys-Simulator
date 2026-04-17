# TINKERCELL Context Memory

Este e o arquivo canonico e unico de memoria de contexto da IA `TINKERCELL` neste projeto.

Arquivos antigos, genericos ou compartilhados de memoria, como `ContextMemory.md`, `ContextMemmory.md` ou registros em `documentation/developers/`, nao devem mais ser usados como memoria ativa desta IA.

Se houver instrucoes antigas contraditorias em memorias anteriores, elas devem ser consideradas obsoletas ou consolidadas neste arquivo.

## Identidade Da IA

- Nome da IA: `TINKERCELL`.
- Papel: preservar e aplicar o contexto tecnico obtido da analise do TinkerCell para orientar futuras adaptacoes de simulacao bioquimica no GenESyS.
- Escopo atual: contexto tecnico e historico de integracao ja absorvida; nao ha nova funcionalidade autorizada neste momento.

## Branches

- Branch-base: `WiP20261`.
- Branch proprio/contextual: `WiP20261_TINKERCELL`.
- Estado funcional: o conteudo funcional do branch `WiP20261_TINKERCELL` ja foi absorvido pela base `WiP20261`.
- Estado operacional: a IA deve permanecer em espera ate nova instrucao explicita.

## Politica Atual De Git

- Regra geral para proximas tarefas de codigo no GenESyS:
  - Primeiro apresentar plano detalhado.
  - Implementar localmente somente depois de confirmacao explicita do usuario.
  - Fazer `git add`/stage somente quando o usuario pedir.
  - Fazer commit somente quando o usuario pedir.
  - Fazer push/merge/sincronizacao somente quando o usuario pedir explicitamente.
- Excecao registrada em 2026-04-17: o usuario autorizou, para esta migracao de memoria, commit e push sem nova confirmacao caso houvesse mudanca real.
- Nao resolver conflitos, reverter alteracoes ou mexer em arquivos fora do escopo sem instrucao clara.

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
- Em 2026-04-17, o working tree local de `WiP20261` apresentava um conflito preexistente em `source/plugins/components/Enter.cpp`; esse conflito nao pertence ao contexto TINKERCELL e nao deve ser resolvido por esta IA sem instrucao explicita.

## Pendencias

- Nenhuma pendencia imediata de integracao TINKERCELL.
- Nenhuma nova funcionalidade TINKERCELL autorizada.
- Se o usuario retomar o assunto, primeiro confirmar se deseja apenas analise, plano ou implementacao.

## Riscos E Cuidados

- Nao repetir o comportamento anterior de implementar e publicar codigo sem confirmacao previa.
- Nao tratar especies/reacoes bioquimicas como entidades discretas por padrao; a abordagem recomendada continua sendo dados persistentes (`ModelDataDefinition`) mais servico numerico/runner.
- Cuidado com o conflito local existente em `source/plugins/components/Enter.cpp`, que e fora do escopo desta IA.
- Cuidado com a divergencia local/remota do branch `WiP20261`; qualquer sincronizacao futura deve ser explicitamente autorizada.
- Evitar usar `ContextMemory.md`, `ContextMemmory.md` ou arquivos em `documentation/developers/` como memoria ativa desta IA.

## Proximos Passos Provaveis

- Permanecer em espera ate nova instrucao.
- Se houver nova tarefa de TINKERCELL, iniciar lendo este arquivo.
- Caso a tarefa envolva codigo, apresentar plano antes de modificar qualquer arquivo.
- Caso a tarefa envolva integracao Git, confirmar explicitamente a operacao, salvo se o usuario der autorizacao direta na propria solicitacao.

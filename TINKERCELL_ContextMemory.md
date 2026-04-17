# TINKERCELL Context Memory

Este e o arquivo canonico e unico de memoria de contexto da IA `TINKERCELL` neste projeto.

Arquivos antigos, genericos ou compartilhados de memoria, como `ContextMemory.md`, `ContextMemmory.md` ou registros em `documentation/developers/`, nao devem mais ser usados como memoria ativa desta IA.

Se houver instrucoes antigas contraditorias em memorias anteriores, elas devem ser consideradas obsoletas, consolidadas neste arquivo ou removidas das memorias antigas.

## Identidade Da IA

- Nome da IA: `TINKERCELL`.
- Papel: preservar e aplicar o contexto tecnico obtido da analise do TinkerCell para orientar futuras adaptacoes de simulacao bioquimica no GenESyS.
- Escopo atual: retomar a integracao bioquimica de forma faseada sobre `WiP20261`, sem reorganizar diretorios de plugins.
- Responsabilidade principal: evoluir o biossimulador TinkerCell-inspired no GenESyS, incluindo especies, parametros, reacoes, redes bioquimicas, leis cineticas, ODEs, persistencia, testes, futura GUI e interoperabilidade, sempre preservando a arquitetura do simulador.
- Antes de qualquer acao tecnica, TINKERCELL deve reanalisar o estado real do repositorio e preservar continuidade com o plano registrado nesta memoria.
- Conversas e relatorios ao usuario devem ser em portugues; codigo, identificadores, comentarios tecnicos e documentacao tecnica no repositorio devem permanecer em ingles.

## Formato Obrigatorio De Resposta

- Toda resposta da IA `TINKERCELL` deve seguir exatamente este protocolo:
  - primeira linha contendo somente `TINKERCELL`;
  - corpo tecnico depois da primeira linha;
  - ultima linha contendo somente `----------`.
- Este formato vale para respostas finais e tambem para atualizacoes intermediarias relevantes.

## Politica De Trabalho Por Fases

- TINKERCELL deve trabalhar fase a fase.
- Ao iniciar ou retomar trabalho tecnico:
  - ler esta memoria;
  - recuperar o plano atual;
  - verificar o estado real do repositorio;
  - identificar a proxima fase logica;
  - ajustar o plano somente se houver motivo tecnico claro, registrando o motivo.
- Executar somente a proxima fase aprovada ou explicitamente confirmada pelo usuario.
- Ao concluir uma fase:
  - parar;
  - relatar o que foi feito;
  - registrar o estado resultante;
  - atualizar esta memoria;
  - pedir confirmacao explicita antes de avancar para outra fase.
- TINKERCELL nao deve continuar automaticamente para a fase seguinte.

## Politica De Memoria De Contexto

- Ao final de cada fase concluida, atualizar `TINKERCELL_ContextMemory.md` com:
  - o que foi feito;
  - estado atual do trabalho;
  - validacoes executadas;
  - commits relevantes, se houver;
  - limitacoes remanescentes;
  - proxima fase sugerida.
- A memoria deve ser operacional, clara e estavel, nao um historico gigante de conversa.
- Se houver divergencia entre memorias antigas e este arquivo, este arquivo prevalece.

## Branches

- Branch-base: `WiP20261`.
- Branch proprio/contextual: `WiP20261_TINKERCELL`.
- Estado funcional: o conteudo funcional do branch `WiP20261_TINKERCELL` ja foi absorvido pela base consolidada atual `WiP20261`.
- Estado operacional: trabalhar a partir da base atual `WiP20261` quando houver instrucao explicita do usuario.
- Regra de retomada: qualquer trabalho futuro deve partir da base atualizada `WiP20261`, e nao de estado antigo, divergente ou conflitado de clone local.

## Politica Atual De Git

- A IA `TINKERCELL` tem autonomia para executar operacoes Git rotineiras quando tecnicamente necessario para manter o contexto, memoria ou branch em estado consistente:
  - `stage`;
  - `commit`;
  - `fetch`;
  - `pull`;
  - `merge`;
  - `push`, apenas quando explicitamente pedido ou quando o fluxo operacional aprovado determinar publicacao.
- A IA so deve pedir confirmacao ao usuario em caso de:
  - operacao destrutiva;
  - duvida real sobre a intencao;
  - risco excepcional de sobrescrever trabalho importante.
- Esta autonomia Git nao autoriza implementar nova funcionalidade sem nova instrucao.
- Nao resolver conflitos, reverter alteracoes ou mexer em arquivos fora do escopo quando isso representar risco de sobrescrever trabalho importante.

### Stage

- Usar `stage` de forma intencional e organizada.
- Agrupar em stage apenas mudancas coerentes entre si.
- Nao misturar alteracoes independentes no mesmo stage.
- Se muitos arquivos pertencem a mesma unidade logica, agrupa-los consistentemente.
- Evitar terminar uma fase com alteracoes importantes soltas e sem rastreabilidade, salvo razao tecnica explicita.

### Commit

- Fazer commits claros, auditaveis e representativos de uma unidade logica concluida.
- Preferir um commit por fase concluida, ou por subunidade logica validada dentro da fase.
- Nao fazer commits que misturem mudancas independentes.
- Nao deixar fase concluida sem commit, salvo razao tecnica explicita registrada na resposta.
- Mensagens de commit devem ser objetivas e tecnicamente representativas.
- Ao responder, informar quais commits foram criados.

### Push E Publicacao

- `push` e publicacao remota sao assuntos separados de `stage` e `commit`.
- Nao fazer push automaticamente apenas porque houve commit.
- Publicar no remoto somente quando explicitamente pedido pelo usuario ou quando o fluxo operacional aprovado determinar isso.
- Ao responder, distinguir claramente:
  - mudancas apenas locais;
  - commits locais;
  - commits publicados no remoto.

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
- A reorganizacao fisica de plugins por categoria ja foi tratada por outra frente; TINKERCELL nao deve mover plugins nem abrir nova reorganizacao estrutural.

## Implementacao Ja Absorvida Pela Base

O MVP bioquimico nativo foi implementado anteriormente e enviado ao GitHub no commit:

- `c00279a Add native biochemical mass-action module`

Arquivos adicionados ou alterados naquele trabalho:

- `source/plugins/data/BiochemicalSimulation/BioSpecies.h`
- `source/plugins/data/BiochemicalSimulation/BioSpecies.cpp`
- `source/plugins/data/BiochemicalSimulation/BioParameter.h`
- `source/plugins/data/BiochemicalSimulation/BioParameter.cpp`
- `source/plugins/data/BiochemicalSimulation/BioReaction.h`
- `source/plugins/data/BiochemicalSimulation/BioReaction.cpp`
- `source/plugins/data/BiochemicalSimulation/BioNetwork.h`
- `source/plugins/data/BiochemicalSimulation/BioNetwork.cpp`
- `source/tools/MassActionOdeSystem.h`
- `source/tools/RungeKutta4OdeSolver.h`
- `source/plugins/PluginConnectorDummyImpl1.cpp`
- `source/applications/gui/qt/GenesysQtGUI/GenesysQtGUI.pro`
- `source/tests/unit/test_simulator_runtime.cpp`

Comportamento implementado:

- `BioSpecies`: especie bioquimica com `initialAmount`, `amount`, unidade, flags `constant` e `boundaryCondition`, persistencia e reset entre replicacoes.
- `BioParameter`: parametro escalar para constantes cineticas.
- `BioReaction`: reacao irreversivel com reagentes, produtos, estequiometria, constante de taxa direta ou referencia a `BioParameter`.
- `BioNetwork`: rede nativa que pode usar pertencimento explicito por nomes de especies/reacoes; quando as listas explicitas estao vazias, preserva fallback compativel por descoberta global no `ModelDataManager`.
- `MassActionOdeSystem`: sistema ODE header-only para cinetica de massa.
- `BioKineticLawExpression`: avaliador header-only especifico para leis cineticas bioquimicas.
- `RungeKutta4OdeSolver`: solver RK4 header-only baseado em `OdeSolver_if`.
- Registro estatico de plugins: `biospecies.so`, `bioparameter.so`, `bioreaction.so`, `bionetwork.so`.

Validacao executada no momento da implementacao:

- `cmake --preset tests-kernel-unit`: passou.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou.
- Testes bioquimicos novos: 7/7 passaram.
- Execucao completa de `genesys_test_simulator_runtime`: 230 passaram, 2 ignorados por falta de `Rscript`, 1 falhou em `SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters`.
- A falha completa conhecida era fora do escopo bioquimico e relacionada a expectativa do `CppSerializer`.

## Fase Atual Ja Executada Em 2026-04-17

- O usuario autorizou iniciar pelo pertencimento explicito de especies/reacoes em `BioNetwork`.
- `BioNetwork` recebeu listas persistentes opcionais de nomes de `BioSpecies` e `BioReaction`.
- APIs adicionadas:
  - `addSpecies`;
  - `addReaction`;
  - `clearSpecies`;
  - `clearReactions`;
  - `getSpeciesNames`;
  - `getReactionNames`.
- Semantica adotada:
  - lista explicita preenchida: resolve membros por nome no `ModelDataManager`;
  - lista explicita vazia: preserva descoberta global antiga para compatibilidade;
  - referencias explicitas vazias ou ausentes falham na validacao/simulacao com mensagem objetiva.
- Testes adicionados em `source/tests/unit/test_simulator_runtime.cpp`:
  - `BioNetworkUsesExplicitSpeciesAndReactionMembership`;
  - `BioNetworkPersistencePreservesExplicitMembership`;
  - `BioNetworkRejectsMissingExplicitMembers`.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 26 testes passaram.
- Proxima frente proposta pelo usuario: parser especifico para leis cineticas, se ainda for a melhor decisao, somente apos confirmacao explicita.

## Fase De Leis Cineticas Executada Em 2026-04-17

- O usuario confirmou seguir para a fase posterior ao pertencimento explicito.
- Decisao tecnica: nao alterar o parser global gerado do GenESyS nesta fase, porque ele nao expoe `BioSpecies` e `BioParameter` como simbolos cineticos e uma mudanca nele teria impacto estrutural maior.
- Foi criado `source/tools/BioKineticLawExpression.h`, um avaliador especifico e estreito para leis cineticas bioquimicas.
- Sintaxe suportada nesta fase:
  - literais numericos;
  - identificadores de especies/parametros;
  - operadores `+`, `-`, `*`, `/`, `^`;
  - parenteses;
  - funcoes `abs`, `exp`, `log`, `sqrt`, `min`, `max`, `pow`.
- `BioReaction` recebeu `kineticLawExpression` opcional, com getter/setter, controle, persistencia, `show()` e validacao.
- Se `kineticLawExpression` estiver vazia, `BioNetwork` preserva a cinetica de massa antiga por `rateConstant` ou `rateConstantParameterName`.
- Se `kineticLawExpression` estiver preenchida, `MassActionOdeSystem` usa a expressao para calcular a taxa da reacao durante cada avaliacao ODE.
- `BioNetwork` valida a expressao no escopo da rede: especies citadas na lei precisam pertencer ao `BioNetwork`; parametros sao resolvidos por `BioParameter` no modelo.
- Testes adicionados:
  - persistencia de `kineticLawExpression`;
  - simulacao com expressao `k * pow(A, 2)`;
  - rejeicao de especie usada na lei cinetica mas fora do pertencimento explicito da rede.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 29 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `git diff --check`: sem problemas.

## Fase De Modificadores Em BioReaction Executada Em 2026-04-17

- O usuario escolheu seguir a recomendacao de tratar participantes nao consumidos antes de abrir nova frente de GUI/editor ou interoperabilidade.
- `BioReaction` passou a armazenar modificadores por nome de `BioSpecies`, separados de reagentes e produtos.
- APIs adicionadas em `BioReaction`:
  - `addModifier`;
  - `clearModifiers`;
  - `getModifiers`.
- Semantica adotada:
  - modificadores sao participantes formais da reacao;
  - modificadores precisam existir como `BioSpecies`;
  - modificadores precisam pertencer ao `BioNetwork` quando a rede usa pertencimento explicito;
  - modificadores podem ser usados por `kineticLawExpression`;
  - modificadores nao sao consumidos nem produzidos no sistema ODE.
- Persistencia de `BioReaction` agora salva e carrega a lista de modificadores.
- `BioReaction::show()` passou a expor `modifiers={...}` para observabilidade.
- Testes adicionados ou ajustados:
  - persistencia de modificadores junto com `kineticLawExpression`;
  - simulacao com lei cinetica dependente de modificador, mantendo o modificador inalterado;
  - rejeicao de modificador ausente no modelo;
  - rejeicao de modificador fora do pertencimento explicito da rede.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 31 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `git diff --check`: sem problemas.
- Commit local da fase: `Add biochemical reaction modifiers`; nao fazer push sem pedido explicito.

## Fase De Escopo Formal De Leis Cineticas Executada Em 2026-04-17

- O usuario confirmou seguir para a fase sugerida de endurecer a semantica das leis cineticas.
- Regra implementada: toda `BioSpecies` usada em `kineticLawExpression` deve ser participante formal da propria `BioReaction`.
- Participantes formais aceitos:
  - reagentes;
  - produtos;
  - modificadores.
- `BioParameter` continua podendo ser usado na expressao sem precisar ser participante da reacao.
- `BioReaction::_check()` agora rejeita expressoes que citam uma `BioSpecies` existente no modelo mas ausente de reagentes, produtos e modificadores.
- `BioNetwork::buildSystem()` tambem rejeita o mesmo caso antes de montar o ODE, mesmo quando a especie pertence ao `BioNetwork`.
- Essa fase diferencia dois escopos:
  - escopo da rede: a especie precisa pertencer ao `BioNetwork`;
  - escopo formal da reacao: a especie precisa ter papel explicito na `BioReaction`.
- Testes adicionados:
  - `BioReactionRejectsKineticLawSpeciesOutsideFormalParticipants`;
  - `BioNetworkRejectsKineticLawSpeciesOutsideReactionParticipants`.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 33 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `git diff --check`: sem problemas.
- Commit local da fase: `Constrain biochemical kinetic-law species`; nao fazer push sem pedido explicito.

## Estado Atual Do Branch

- `WiP20261` e a base consolidada atual para TINKERCELL.
- O conteudo relevante de `WiP20261_TINKERCELL` ja foi absorvido por `WiP20261`.
- As fases de pertencimento explicito em `BioNetwork`, leis cineticas especificas, modificadores em `BioReaction` e escopo formal de leis cineticas foram implementadas localmente e validadas.
- A IA `TINKERCELL` deve aguardar confirmacao explicita antes de iniciar a proxima fase.
- Qualquer trabalho futuro deve partir da base atualizada `WiP20261`, e nao de estado antigo local.
- Em 2026-04-17, um clone local de `WiP20261` apresentava conflito preexistente em `source/plugins/components/Enter.cpp`; esse conflito nao pertence ao contexto TINKERCELL e so deve ser tratado pela IA se for necessario e seguro dentro da nova tarefa.

## Pendencias

- Confirmar com o usuario antes de iniciar a proxima fase.
- Proxima fase candidata: definir o proximo eixo apos escopo formal de leis cineticas, possivelmente GUI/editor, importacao/exportacao SBML/TinkerCell, ou semantica de reacoes reversiveis.
- Ainda falta fluxo GUI/editor para editar listas explicitas de membros em `BioNetwork`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar `kineticLawExpression`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar modificadores de `BioReaction`; a fase atual cobriu API, persistencia, validacao e runtime.

## Riscos E Cuidados

- Nao implementar nova funcionalidade TINKERCELL sem nova instrucao.
- Nao tentar novo merge TINKERCELL neste estado; a base `WiP20261` ja absorveu o conteudo relevante.
- Nao partir de clone local antigo, divergente ou conflitado para trabalho futuro; atualizar para a base consolidada `WiP20261` antes de qualquer retomada tecnica.
- Nao tratar especies/reacoes bioquimicas como entidades discretas por padrao; a abordagem recomendada continua sendo dados persistentes (`ModelDataDefinition`) mais servico numerico/runner.
- Cuidado com conflitos locais fora do escopo TINKERCELL, especialmente `source/plugins/components/Enter.cpp`, se ainda existir.
- Operacoes Git rotineiras sao autorizadas, mas nao devem sobrescrever trabalho importante.
- Evitar usar `ContextMemory.md`, `ContextMemmory.md` ou arquivos em `documentation/developers/` como memoria ativa desta IA.

## Proximos Passos Provaveis

- Aguardar confirmacao explicita do usuario para escolher o proximo eixo.
- Manter a implementacao faseada e nao reorganizar diretorios de plugins.
- Caso a tarefa envolva manutencao de memoria ou Git rotineiro, executar com autonomia, salvo operacao destrutiva, duvida real ou risco excepcional.

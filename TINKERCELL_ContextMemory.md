# TINKERCELL Context Memory

Este e o arquivo canonico e unico de memoria de contexto da IA `TINKERCELL` neste projeto.

Arquivos antigos, genericos ou compartilhados de memoria, como `ContextMemory.md`, `ContextMemmory.md` ou registros em `documentation/developers/`, nao devem mais ser usados como memoria ativa desta IA.

Se houver instrucoes antigas contraditorias em memorias anteriores, elas devem ser consideradas obsoletas, consolidadas neste arquivo ou removidas das memorias antigas.

## Identidade Da IA

- Nome da IA: `TINKERCELL`.
- Papel: preservar e aplicar o contexto tecnico obtido da analise do TinkerCell para orientar futuras adaptacoes de simulacao bioquimica no GenESyS.
- Escopo atual: retomar a integracao bioquimica de forma faseada sobre `WiP20261`, sem reorganizar diretorios de plugins, e preparar uma arquitetura generica de plugins graficos para extensao futura da GUI.
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
  - ler esta memoria na raiz do repositorio/branch `WiP20261`;
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

- Ao final de cada fase concluida, atualizar este arquivo `TINKERCELL_ContextMemory.md`, localizado na raiz do checkout `WiP20261/Genesys-Simulator`, com:
  - o que foi feito;
  - estado atual do trabalho;
  - validacoes executadas;
  - commits relevantes, se houver;
  - limitacoes remanescentes;
  - proxima fase sugerida.
- A memoria deve ser operacional, clara e estavel, nao um historico gigante de conversa.
- Se houver divergencia entre memorias antigas, arquivos em `documentation/developers/`, ou este arquivo, este arquivo prevalece.
- `documentation/developers/TINKERCELL_context.md` pode existir como documentacao historica/auxiliar, mas nao e mais a memoria ativa canonica.

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
- Quando o ambiente de execucao impor sandbox/permissoes, obedecer as restricoes do sistema mesmo que o usuario conceda autonomia geral.
- Se uma edicao fora do sandbox for tecnicamente necessaria e o sistema exigir aprovacao, solicitar a aprovacao pela ferramenta apropriada.

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

## Fase De Sintese E Degradacao Executada Em 2026-04-17

- O usuario confirmou prosseguir para permitir reacoes de sintese e degradacao e pediu checkpoint com stage/commit ao chegar em base estavel.
- `BioReaction` agora permite:
  - reacoes de sintese sem reagentes e com pelo menos um produto;
  - reacoes de degradacao com pelo menos um reagente e sem produtos.
- `BioReaction` continua rejeitando reacoes sem nenhum reagente nem produto, pois elas nao tem efeito estequiometrico.
- O contrato documentado de `BioReaction` foi atualizado para deixar essa semantica explicita.
- A execucao ODE ja suportava essa semantica:
  - sintese sem reagentes produz taxa de ordem zero quando usa `rateConstant`;
  - degradacao sem produtos remove massa dos reagentes;
  - `kineticLawExpression` segue disponivel, respeitando a regra de participantes formais.
- Testes adicionados:
  - `BioReactionRejectsEmptyStoichiometricEffect`;
  - `BioNetworkSimulatesZeroOrderSynthesisReaction`;
  - `BioNetworkSimulatesDegradationReactionWithoutProducts`.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 36 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `git diff --check`: sem problemas.
- Commit local da fase: `Allow biochemical synthesis and degradation`; nao fazer push sem pedido explicito.

## Fase De Reacoes Reversiveis Mass-Action Executada Em 2026-04-17

- O usuario autorizou avancar ate o proximo checkpoint em poucas fases.
- Decisao tecnica: implementar reversibilidade mass-action basica, sem abrir GUI, SBML/TinkerCell ou leis reversas customizadas nesta fase.
- `BioReaction` deixou de rejeitar `reversible=true`.
- `BioReaction` recebeu cinetica reversa explicita:
  - `reverseRateConstant`;
  - `reverseRateConstantParameterName`;
  - getters, setters e `resolveReverseRateConstant`.
- A persistencia de `BioReaction` salva e carrega os campos reversos.
- `BioReaction::show()` passou a expor os campos reversos.
- `BioNetwork::buildSystem()` agora monta reacoes reversiveis para o ODE, validando constantes reversas nao negativas e parametros reversos existentes.
- `MassActionOdeSystem` passou a calcular fluxo liquido:
  - fluxo direto por lei cinetica direta ou mass-action dos reagentes;
  - fluxo reverso mass-action dos produtos quando `reversible=true`;
  - derivadas aplicadas como `reverse - forward` nos reagentes e `forward - reverse` nos produtos.
- Escopo mantido intencionalmente fora desta fase:
  - expressao cinetica reversa customizada;
  - GUI/editor para campos reversos;
  - importacao/exportacao SBML/TinkerCell.
- Testes adicionados ou ajustados:
  - simulacao de `A <-> B` com solucao analitica esperada;
  - rejeicao de parametro reverso ausente;
  - persistencia de `reverseRateConstant` e `reversible`.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 37 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters'`: 1 teste passou;
  - `git diff --check`: sem problemas.
- Commit local da fase: `Add reversible biochemical mass-action kinetics`; nao fazer push sem pedido explicito.

## Fase De Leis Cineticas Reversas Customizadas Executada Em 2026-04-17

- O usuario pediu prosseguir ate o proximo checkpoint estavel.
- Decisao tecnica: completar a reversibilidade ja existente com uma expressao cinetica reversa opcional, sem abrir GUI/editor, SBML/TinkerCell ou reorganizacao estrutural de plugins.
- `BioReaction` recebeu `reverseKineticLawExpression` opcional, com getter/setter, `SimulationControl`, persistencia, `show()` e validacao.
- Semantica adotada:
  - `reverseKineticLawExpression` so afeta a execucao quando `reversible=true`;
  - se `reverseKineticLawExpression` estiver vazia, o fluxo reverso continua usando mass-action com `reverseRateConstant` ou `reverseRateConstantParameterName`;
  - se `reverseKineticLawExpression` estiver preenchida, ela substitui a cinetica reversa mass-action durante a montagem e avaliacao ODE;
  - especies usadas na lei reversa precisam pertencer ao `BioNetwork` e ser participantes formais da `BioReaction` como reagentes, produtos ou modificadores;
  - parametros usados na lei reversa continuam sendo resolvidos por `BioParameter`.
- `BioNetwork::buildSystem()` agora valida leis cineticas direta e reversa por uma rotina comum de escopo de rede e participantes formais.
- `MassActionOdeSystem` agora avalia taxa direta e taxa reversa pela mesma rotina, alternando entre expressao cinetica e mass-action conforme o campo preenchido.
- Testes adicionados ou ajustados:
  - simulacao de `A <-> B` usando `kf * A` e `kr * B`;
  - persistencia de `reverseKineticLawExpression`;
  - rejeicao de especie usada na lei cinetica reversa mas fora dos participantes formais em `BioReaction`;
  - rejeicao do mesmo caso em `BioNetwork`, mesmo quando a especie pertence a rede.
- Validacao executada:
  - `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime`: passou;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.Bio*'`: 40 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.MassActionOdeSystem*:SimulatorRuntimeTest.RungeKutta4OdeSolver*'`: 2 testes passaram;
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_simulator_runtime --gtest_filter='SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters'`: 1 teste passou;
  - `git diff --check`: sem problemas.
- Commit local da fase: `Add reversible biochemical kinetic laws`; nao fazer push sem pedido explicito.

## Fase De Exemplos Bioquimicos Executaveis Executada Em 2026-04-17

- O usuario mudou o foco da fase: nao expandir o biossimulador, mas demonstrar por exemplos executaveis o que ja foi integrado pela linha TINKERCELL.
- Foram usados como referencia de estilo os exemplos em `source/applications/terminal/examples/smarts`, especialmente:
  - `Smart_ODE`, pela execucao terminal de um modelo continuo;
  - `Smart_Process` e `Smart_Dummy`, pela estrutura simples de aplicacao terminal que instancia `Simulator`, carrega plugins, monta modelo, salva `.gen` e executa.
- Exemplos criados:
  - `Smart_BioReversibleMassAction`;
  - `Smart_BioKineticLawRegulation`.
- `Smart_BioReversibleMassAction` demonstra:
  - `BioSpecies`;
  - `BioParameter`;
  - `BioReaction` reversivel;
  - constantes cineticas direta e reversa via nomes de `BioParameter`;
  - pertencimento explicito em `BioNetwork`;
  - simulacao deterministica por `BioNetwork::simulate`;
  - payload final com quantidades de especies.
- `Smart_BioKineticLawRegulation` demonstra:
  - sintese sem reagentes;
  - degradacao sem produtos;
  - modificador formal de reacao;
  - especie constante;
  - `kineticLawExpression` customizada com parametro e especie modificadora;
  - pertencimento explicito em `BioNetwork`;
  - simulacao deterministica por `BioNetwork::simulate`.
- Validacao executada:
  - configuracao CMake em `/tmp/genesys-terminal-smart-bio-reversible` com `GENESYS_TERMINAL_EXAMPLE=smarts/Smart_BioReversibleMassAction.cpp`: passou;
  - build de `genesys_terminal_application` para `Smart_BioReversibleMassAction`: passou;
  - execucao de `Smart_BioReversibleMassAction`: passou, salvou `./models/Smart_BioReversibleMassAction.gen`, status `Completed`, payload final com `A=4.82086773432292` e `B=5.17913226567708`;
  - configuracao CMake em `/tmp/genesys-terminal-smart-bio-kinetic-law` com `GENESYS_TERMINAL_EXAMPLE=smarts/Smart_BioKineticLawRegulation.cpp`: passou;
  - build de `genesys_terminal_application` para `Smart_BioKineticLawRegulation`: passou;
  - execucao de `Smart_BioKineticLawRegulation`: passou, salvou `./models/Smart_BioKineticLawRegulation.gen`, status `Completed`, payload final com `Protein=3.24249268786054` e `Activator=3`.
- Observacao de execucao: os executaveis emitiram aviso de `autoloadplugins.txt` ausente no diretorio de build em `/tmp`, mas a insercao estatica dos plugins prosseguiu e os exemplos executaram com sucesso.
- Commit da fase: `24a8435d Add biochemical terminal smart examples`; publicacao remota autorizada pelo usuario em 2026-04-17.

## Fase De Exemplo Com Lei Cinetica Reversa Executada Em 2026-04-17

- O usuario pediu mais um exemplo demonstrando explicitamente lei cinetica reversa.
- Foi criado `Smart_BioReversibleKineticLaw`.
- O exemplo demonstra:
  - `BioSpecies` para `Substrate`, `Product` e `Enzyme`;
  - `BioParameter` para `kForward` e `kReverse`;
  - `BioReaction` reversivel com modificador formal `Enzyme`;
  - `kineticLawExpression="kForward * Enzyme * Substrate"`;
  - `reverseKineticLawExpression="kReverse * Enzyme * Product"`;
  - `BioNetwork` com pertencimento explicito de especies e reacao;
  - simulacao deterministica por `BioNetwork::simulate`.
- Validacao executada:
  - configuracao CMake em `/tmp/genesys-terminal-smart-bio-reversible-kinetic-law` com `GENESYS_TERMINAL_EXAMPLE=smarts/Smart_BioReversibleKineticLaw.cpp`: passou;
  - build de `genesys_terminal_application` para `Smart_BioReversibleKineticLaw`: passou;
  - execucao de `Smart_BioReversibleKineticLaw`: passou, salvou `./models/Smart_BioReversibleKineticLaw.gen`, status `Completed`, payload final com `Substrate=3.61517214395755`, `Product=6.38482785604245` e `Enzyme=2`;
  - o `.gen` salvo expos `reverseKineticLawExpression` no `BioReaction`.
- Observacao de execucao: o executavel em `/tmp` emitiu aviso de `autoloadplugins.txt` ausente, mas a insercao estatica dos plugins prosseguiu e o exemplo executou com sucesso.
- Commit da fase: `7c0efb9e Add reversible kinetic law smart example`; publicacao remota autorizada pelo usuario em 2026-04-17.

## Encerramento Do Checkpoint Em 2026-04-17

- O usuario pediu push dos checkpoints locais e atualizacao final desta memoria antes de encerrar o dia.
- Branch local usada: `WiP20261`.
- Branch remota de destino: `origin/WiP20261`.
- Commits de exemplo a publicar:
  - `24a8435d Add biochemical terminal smart examples`;
  - `7c0efb9e Add reversible kinetic law smart example`.
- Esta atualizacao de memoria deve ser commitada junto ao encerramento antes do push.

## Estado Atual Do Branch

- `WiP20261` e a base consolidada atual para TINKERCELL.
- O conteudo relevante de `WiP20261_TINKERCELL` ja foi absorvido por `WiP20261`.
- As fases de pertencimento explicito em `BioNetwork`, leis cineticas especificas, modificadores em `BioReaction`, escopo formal de leis cineticas, sintese/degradacao, reversibilidade mass-action, leis cineticas reversas customizadas, exemplos bioquimicos executaveis, exemplo com lei cinetica reversa, resultado temporal estruturado e analise bioquimica nao-GUI foram implementadas e validadas.
- Em 2026-04-21, `BioSimulationResult` foi adicionado como contrato nativo de series temporais bioquimicas e integrado ao `SimulationResultsDataset`.
- Em 2026-04-21, `BioNetwork` passou a registrar amostra inicial e todas as amostras apos passos ODE em `BioSimulationResult`.
- Em 2026-04-21, `BioSimulationAnalysis` foi adicionado como camada nao-GUI reutilizavel com matriz estequiometrica, series de taxas de reacao, checagem de estado estacionario e varredura local de sensibilidade de parametros por diferencas finitas.
- Em 2026-04-21, `MassActionOdeSystem` passou a expor avaliadores de taxa direta, reversa e liquida por reacao.
- Em 2026-04-21, `BioNetwork` passou a expor `buildOdeSystemForAnalysis`, `getStoichiometryMatrix`, `getReactionRateTimeCourse`, `checkLastSampleSteadyState` e `scanLocalParameterSensitivity`.
- Em 2026-04-21, os testes de categoria de `RSimulator` foram alinhados ao valor corrente `ExternalIntegration`.
- Validacao mais recente: `cmake --build --preset tests-kernel-unit-run` passou com a suite completa do preset.
- Proximo foco aprovado pelo usuario: antes de desenvolver telas especificas da GUI, planejar uma arquitetura generica de plugins graficos para expansao da GUI; depois criar plugins graficos especificos de biossimulacao sobre essa arquitetura.
- Em 2026-04-21, as etapas 1 e 2 do plano de plugins graficos foram iniciadas:
  - etapa 1: auditoria tecnica da GUI atual registrada em `documentation/developers/GUI_GRAPHICAL_PLUGIN_AUDIT.md`;
  - etapa 2: contrato generico de extensao grafica criado em `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionContracts.h`.
- O contrato introduz `GuiExtensionRuntimeContext`, contribuicoes de `action/dock/window`, `GuiExtensionRegistry` e interface `GuiExtensionPlugin`.
- Em 2026-04-21, a etapa 3 foi implementada de forma incremental e sem mudar o comportamento funcional atual da GUI:
  - adicionado `GuiExtensionManager` em `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionManager.{h,cpp}`;
  - `MainWindow` passou a criar e acionar o manager dentro de `_rebuildViewDependentControllers`, com `GuiExtensionRuntimeContext`;
  - nesta iteracao, `setPlugins({})` mantem carga vazia por design, preparando o ciclo de vida sem ativar contribuicoes reais.
  - o manager agora recebe snapshot real dos plugins de modelo carregados via `PluginManager` e aplica filtro por `requiredModelPlugins` antes de registrar contribuicoes graficas.
- Validacao da etapa 3:
  - `cmake --build --preset gui-app`: passou, incluindo compilacao e link de `genesys_qt_gui_application`.
- Em 2026-04-21, o carregamento de plugins graficos foi conectado a um catalogo/fabrica real:
  - adicionados `GuiExtensionPluginCatalog.{h,cpp}` com auto-registro estatico de `GuiExtensionPlugin`;
  - adicionado plugin base `CoreGuiExtensionPlugin` (sem contribuicoes visuais) para validar pipeline de descoberta;
  - `MainWindow` passou a carregar plugins do catalogo em `_rebuildViewDependentControllers`, substituindo `setPlugins({})`.
- Validacao do catalogo/fabrica:
  - `cmake --build --preset gui-app`: passou apos inclusao de novos arquivos e link final da GUI.
- Em 2026-04-21, foram implementadas as duas etapas seguintes do plano:
  - plugin grafico funcional generico `ModelPluginInspectorGuiExtensionPlugin` com item de menu `Tools/Extensions/Loaded Model Plugins` que lista plugins de modelo carregados;
  - plugin grafico condicional `BioNetworkAwareGuiExtensionPlugin` com dependencia declarada `requiredModelPlugins={"bionetwork"}`, contribuindo item `Tools/Extensions/BioNetwork Extension Status` apenas quando a dependencia estiver carregada.
- Validacao das duas etapas:
  - `cmake --build --preset gui-app`: passou apos compilacao dos plugins graficos novos e link da GUI.
- Em 2026-04-21, para facilitar testes do usuario no terminal:
  - adicionado `Smart_BioNetworkAnalysis` em `source/applications/terminal/examples/smarts/Smart_BioNetworkAnalysis.{h,cpp}`;
  - o smart novo exercita simulacao + analise: `BioSimulationResult`, dataset temporal por especie, matriz estequiometrica, serie temporal de taxas, check de estado estacionario e varredura de sensibilidade local.
  - `Smart_BioReversibleMassAction`, `Smart_BioKineticLawRegulation` e `Smart_BioReversibleKineticLaw` foram atualizados para usar `plugins->autoInsertPlugins()` (sem dependencia de `autoloadplugins.txt` local no diretorio de build).
- Em 2026-04-21, a infraestrutura de plugins graficos foi ajustada para ciclo de vida dinamico sem acoplamento de dominio:
  - `MainWindow` ganhou `_refreshGuiExtensions()` para centralizar rebuild das contribuicoes graficas;
  - o callback de refresh apos `DialogPluginManager` agora recarrega catalogo de plugins de modelo e tambem reaplica contribuicoes de GUI;
  - `GuiExtensionManager` passou a rastrear e remover menus criados dinamicamente (`_createdMenuActions`) durante `clear()`, evitando sobras visuais quando dependencias deixam de estar carregadas.
- Validacao da iteracao dinamica:
  - `cmake --build --preset gui-app`: passou, com link final de `genesys_qt_gui_application`.
- Em 2026-04-21, foi adicionada resolucao declarativa de plugins graficos no catalogo:
  - `GuiExtensionPluginCatalog::resolvedPlugins()` agora resolve lista final com base em JSON opcional (`enabled`, `disabled`, `order`);
  - caminho padrao do JSON: arquivo `gui_extensions.json` no mesmo diretorio de `preferences.json` (`SystemPreferences::configFilePath()`);
  - override por ambiente: `GENESYS_GUI_EXTENSIONS_CONFIG`;
  - fallback seguro: se o arquivo nao existir ou estiver invalido, usa ordem de registro estatico.
- Em 2026-04-21, a camada declarativa foi estendida com metadados de apresentacao:
  - novo bloco opcional `presentation` no JSON de extensoes (`category`, `group`, `priority` por `extensionId`);
  - ordenacao aplicada a plugins nao fixados em `order`: `priority` decrescente, depois `category`, `group` e `displayName`;
  - `order` continua com precedencia e mantem itens explicitamente fixados sem reordenacao por metadados.
- Em 2026-04-21, foi implementada uma GUI de gerenciamento declarativo das extensoes graficas:
  - menu: `Tools/Extensions/Manage Graphical Extensions...`;
  - dialogo novo `GuiExtensionConfigurationDialog` com tabela editavel de `enabled`, `order`, `category`, `group`, `priority`;
  - persistencia em `gui_extensions.json` (mesmo caminho base do `preferences.json`, com override por `GENESYS_GUI_EXTENSIONS_CONFIG`);
  - o manager permite `Reload` e `Save` e gera `enabled/disabled/order/presentation`;
  - apos `Save`, a GUI agora aplica refresh imediato das contribuicoes graficas via callback para `MainWindow::refreshGuiExtensions()`.
- Em 2026-04-21, o callback de `Save` do manager passou a atualizar tambem o catalogo visual de plugins (`MainWindow::refreshPluginCatalog()`), mantendo sincronia imediata entre arvore de catalogo e contribuicoes dinamicas.
- Em 2026-04-21, foram adicionados os primeiros plugins graficos de biossimulacao sobre a infraestrutura generica:
  - `BioNetworkEditorGuiExtensionPlugin`: `Tools/Extensions/Biochemical/Edit BioNetwork Membership...` para editar membros de especies/reacoes e controles de tempo (`start/stop/step`) de um `BioNetwork`;
  - `BioReactionKineticsGuiExtensionPlugin`: `Tools/Extensions/Biochemical/Edit BioReaction Kinetics...` para editar `reversible`, constantes direta/reversa, parametros direta/reversa, leis cineticas direta/reversa e lista de modificadores de um `BioReaction`.
- Gating de dependencias aplicado:
  - editor de `BioNetwork` exige plugin de modelo `bionetwork`;
  - editor de `BioReaction` exige plugin de modelo `bioreaction`.
- A `MainWindow` passou a usar `GuiExtensionPluginCatalog::resolvedPlugins()` ao reconstruir contribuicoes graficas.
- Documentacao tecnica da configuracao declarativa:
  - `documentation/developers/GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md`.
- Validacao da etapa declarativa:
  - `cmake --build --preset gui-app`: passou (rebuild + link de `genesys_qt_gui_application`).
- Validacao terminal mais recente:
  - `Smart_BioNetworkAnalysis`: `status=Completed`, `samples=302`, matriz estequiometrica 3x1, `steady=false` para `tolerance=0.05`, sensitividade com 2 entradas (`kForward`, `kReverse`);
  - `Smart_BioReversibleMassAction`: `status=Completed`, payload final `A=4.82086773432292`, `B=5.17913226567708`;
  - `Smart_BioKineticLawRegulation`: `status=Completed`, payload final `Protein=3.24249268786054`, `Activator=3`;
  - `Smart_BioReversibleKineticLaw`: `status=Completed`, payload final `Substrate=3.61517214395755`, `Product=6.38482785604245`, `Enzyme=2`.
- A IA `TINKERCELL` pode executar stage e commit quando pedido ou quando a politica operacional permitir, mas push continua separado salvo pedido explicito.
- Qualquer trabalho futuro deve partir da base atualizada `WiP20261`, e nao de estado antigo local.
- Em 2026-04-17, um clone local de `WiP20261` apresentava conflito preexistente em `source/plugins/components/Enter.cpp`; esse conflito nao pertence ao contexto TINKERCELL e so deve ser tratado pela IA se for necessario e seguro dentro da nova tarefa.

## Pendencias

- Reapresentar e amadurecer o plano de plugins graficos genericos para a GUI do GenESyS.
- Definir contrato de plugin grafico sem acoplamento a biossimulacao: menus, acoes, docks, janelas, editores, ferramentas de cena, inspectors, providers de propriedades e comandos.
- Definir como plugins graficos descobrem dependencias de plugins de modelo/dados e como ficam inativos quando essas dependencias nao estao carregadas.
- Depois da arquitetura generica, criar plugins graficos especificos de biossimulacao como consumidores dessa infraestrutura.
- Validar fluxo funcional GUI ponta a ponta com execucao manual: carregar/descarregar plugins bio e confirmar aparecimento/ocultacao dos menus `Tools/Extensions/Biochemical`.
- Definir escopo de importacao/exportacao SBML/TinkerCell.
- Ainda falta fluxo GUI/editor para editar listas explicitas de membros em `BioNetwork`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar `kineticLawExpression`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar `reverseKineticLawExpression`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar modificadores de `BioReaction`; a fase atual cobriu API, persistencia, validacao e runtime.
- Ainda falta fluxo GUI/editor para editar campos reversos de `BioReaction`; a fase atual cobriu API, persistencia, validacao e runtime.

## Riscos E Cuidados

- Nao implementar nova funcionalidade TINKERCELL sem nova instrucao.
- Nao tentar novo merge TINKERCELL neste estado; a base `WiP20261` ja absorveu o conteudo relevante.
- Nao partir de clone local antigo, divergente ou conflitado para trabalho futuro; atualizar para a base consolidada `WiP20261` antes de qualquer retomada tecnica.
- Nao tratar especies/reacoes bioquimicas como entidades discretas por padrao; a abordagem recomendada continua sendo dados persistentes (`ModelDataDefinition`) mais servico numerico/runner.
- Cuidado com conflitos locais fora do escopo TINKERCELL, especialmente `source/plugins/components/Enter.cpp`, se ainda existir.
- Operacoes Git rotineiras sao autorizadas, mas nao devem sobrescrever trabalho importante.
- Evitar usar `ContextMemory.md`, `ContextMemmory.md` ou arquivos em `documentation/developers/` como memoria ativa desta IA.

## Proximos Passos Provaveis

- Focar no plano e, depois, na implementacao da infraestrutura generica de plugins graficos.
- Nao iniciar ferramentas graficas de biossimulacao antes de existir contrato grafico generico.
- Manter a implementacao faseada e nao reorganizar diretorios de plugins sem motivo tecnico claro.
- Caso a tarefa envolva manutencao de memoria ou Git rotineiro, executar com autonomia, salvo operacao destrutiva, duvida real, restricao de sandbox ou risco excepcional.

# PythonForG — Cobertura de `SimulatorFacade`

Estado da fase 4 da integração Python do GenESyS.

## Resumo

- O script recebe `simulator` diretamente.
- O fluxo de componente mantém a analogia com `CppForG`: `init` e `on_dispatch`.
- A cobertura atual privilegia métodos públicos de consulta e mutação controlada de `SimulatorFacade`.
- Os casos que exigem callbacks C++, `void*`, templates ou interfaces abstratas abertas permanecem explicitamente excluídos nesta fase.

## Superfície exposta em Python

### `simulator`

**Direto**
- `getVersion`, `getVersionNumber`, `getName`
- `currentModel`, `newModel`, `modelCount`, `modelAt`, `models`, `setCurrentModel`
- `simGetSimulatedTime`
- `modelCheck`, `modelShowLanguage`
- `infoGetName`, `infoSetName`, `infoGetAnalystName`, `infoSetAnalystName`
- `infoGetDescription`, `infoSetDescription`, `infoGetProjectTitle`, `infoSetProjectTitle`
- `infoGetVersion`, `infoSetVersion`, `infoShow`, `infoHasChanged`, `infoSetHasChanged`
- `simSetNumberOfReplications`, `simGetNumberOfReplications`
- `simSetReplicationLength`, `simGetReplicationLength`
- `simSetReplicationLengthTimeUnit`, `simGetReplicationLengthTimeUnit`
- `simSetReplicationReportBaseTimeUnit`, `simGetReplicationBaseTimeUnit`
- `simSetWarmUpPeriod`, `simGetWarmUpPeriod`
- `simSetWarmUpPeriodTimeUnit`, `simGetWarmUpPeriodTimeUnit`
- `simSetTerminatingCondition`, `simGetTerminatingCondition`
- `simShow`
- `simSetPauseOnEvent`, `simIsPauseOnEvent`
- `simSetStepByStep`, `simIsStepByStep`
- `simSetInitializeStatistics`, `simIsInitializeStatistics`
- `simSetInitializeSystem`, `simIsInitializeSystem`
- `simSetPauseOnReplication`, `simIsPauseOnReplication`
- `simSetShowReportsAfterReplication`, `simIsShowReportsAfterReplication`
- `simSetShowReportsAfterSimulation`, `simIsShowReportsAfterSimulation`
- `simSetShowSimulationResponsesInReport`, `simIsShowSimulationResponsesInReport`
- `simSetShowSimulationControlsInReport`, `simIsShowSimulationControlsInReport`
- `simIsRunning`, `simIsPaused`, `simGetCurrentReplicationNumber`
- `simGetBreakpointsOnTime`, `simGetBreakpointsOnEntity`, `simGetBreakpointsOnComponent`
- `simGetSimulationStatisticsAggregates`, `simGetCurrentEvent`
- `modelGetId`, `modelHasChanged`, `modelSetHasChanged`, `modelGetLevel`
- `modelShow`, `modelClear`, `modelSave`, `modelLoad`
- `modelInsert`, `modelRemove`, `modelCollectDataDefinitionsRemovedWith`
- `modelGetFutureEvents`, `modelGetControls`, `modelGetResponses`
- `dataInsert`, `dataRemove`, `dataClear`, `dataHasChanged`, `dataSetHasChanged`
- `dataGetDataDefinitionClassnames`, `dataGetDataDefinitionList`, `dataGetRankOf`
- `componentInsert`, `componentRemove`, `componentClear`
- `componentHasChanged`, `componentSetHasChanged`
- `componentGetNumberOfComponents`, `componentGetSourceComponents`, `componentGetTransferInComponents`, `componentGetAllComponents`
- `trace`, `traceReport`, `traceError`, `traceGetErrorMessages`

**Wrappers auxiliares**
- `genesys.Context`
- `genesys.Model`
- `genesys.Entity`
- `genesys.Component`
- `genesys.ModelDataDefinition`
- `genesys.Event`

**Model wrapper**
- `getId`, `hasChanged`, `setHasChanged`, `check`, `clear`, `save`, `load`
- `showLanguage`, `parseExpression`, `parseExpressionDetailed`
- `createEntity`, `componentFind`, `dataGetDataDefinition`
- `collectDataDefinitionsRemovedWith`, `getFutureEvents`
- `getComponentCount`, `getDataDefinitionCount`

## Exclusões explícitas

### `LicenceManager`
- Não exposto nesta fase. Os métodos são simples, mas não eram essenciais para a integração de execução do `PythonForG`.

### `PluginManager`
- Não exposto nesta fase.
- Motivos: `Plugin*`, `PluginLoadIssue`, `PluginInsertionOptions`, descoberta e auto-inserção de plugins exigem uma camada própria de wrappers e política de lifetime.

### `ParserManager`
- Não exposto nesta fase.
- Motivos: callbacks dinâmicos e `ParserChangesInformation` precisam de contrato adicional de integração.

### `ExperimentManager`
- Não exposto nesta fase.
- Motivos: `SimulationExperiment*` e listas associadas exigem wrappers próprios.

### `ModelSimulation`
- Controles de execução (`simStart`, `simPause`, `simStep`, `simStop`) ficam fora do caminho automático do componente Python.
- `simSetReporter` / `simGetReporter` permanecem fora por dependerem de `SimulationReporter_if`.
- `simLoadInstance` / `simSaveInstance` ficam fora nesta fase.

### `ModelInfo`, `ModelDataManager`, `ModelComponentManager`
- Parte da API foi exposta.
- Permanecem fora os caminhos que exigem `PersistenceRecord`, ponteiros crus de propriedade externa ou iteradores internos.

### `OnEventManager`
- Não exposto.
- Motivos: APIs de callback de alto acoplamento, com ponteiros para função e objetos, exigem camada específica para Python e política de GIL/reentrância.

### Tipos não expostos diretamente
- `newInstance<T>`
- `componentBegin` / `componentEnd`
- `traceSimulation(void*, ...)`
- `addTrace*Handler(...)`
- `eventAddOn*Handler(...)`
- `eventNotify*Handlers(...)`

## Observação de segurança

- A integração é experimental e executa código confiável do modelo no mesmo processo do simulador.
- Não existe sandbox de segurança real nesta fase.
- Exceções Python são capturadas e convertidas em erro de runtime do GenESyS.

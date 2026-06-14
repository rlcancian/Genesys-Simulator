# Análise de sincronização temporal discreta-contínua no GenESyS

**Data:** 2026-06-03  
**Autora (análise):** Maia (Claude Code Sonnet)  
**Branch:** WiP20261  
**Repositório:** Genesys-Simulator

---

## 1. Escopo e restrições

Esta análise cobre o repositório `WiP20261/Genesys-Simulator`, branch `WiP20261`. Toda leitura foi feita com ferramentas somente-leitura. Nenhum arquivo foi modificado. As buscas cobriram:

- `source/kernel/simulator/` — kernel de simulação discreta
- `source/plugins/components/` — componentes de simulação
- `source/plugins/data/` — definições de dados
- `source/tools/Continuous/` e `source/tools/Biochemical/` — solvers e ferramentas matemáticas

**Restrição**: Este documento é exclusivamente analítico. Propostas são conceituais. Nenhuma implementação foi feita.

---

## 2. Resumo executivo

O GenESyS possui um kernel de simulação discreta orientada a eventos bem consolidado. O tempo simulado global é controlado exclusivamente por `ModelSimulation::_simulatedTime` e avança apenas quando eventos são retirados do calendário `_futureEvents`.

O repositório já contém um conjunto significativo de plugins contínuos e bioquímicos. Esses plugins se dividem em três categorias de acoplamento temporal:

1. **Temporalmente desacoplados** (execução bulk interna): `BioSimulate`/`BioNetwork` em modo síncrono, `LSODE` — executam todos os passos internos dentro de uma única chamada de `_onDispatchEvent()`, sem criar eventos intermediários. O kernel permanece parado enquanto o solver avança.

2. **Parcialmente acoplados via parâmetro fixo** (`deltaT`): `CellGrowthComponent`, `MetabolicSubmodelComponent`, `FtsZPolymerizationComponent` — executam um único passo por evento, mas o tamanho do passo é um parâmetro estático `_deltaT`, não derivado do tempo simulado global entre despachos consecutivos.

3. **Já integrado ao calendário (microeventos)**: `BioNetwork` com `autoSchedule = true` — insere `InternalEvent` no calendário a cada passo, realizando a abordagem de microeventos periódicos já implementada parcialmente.

Os solvers matemáticos (`RungeKutta4OdeSolver`, `SolverDefaultImpl1`, `MassActionOdeSystem`) são ferramentas numéricas puras sem qualquer referência ao tempo simulado global — o que é correto e não deve ser alterado.

O problema arquitetural central é: **não existe um contrato formal entre componentes que avançam em passos temporais e o tempo simulado global**. Componentes com `_deltaT` assumem implicitamente que `deltaT` corresponde ao intervalo real entre despachos, mas nada no kernel garante essa correspondência.

---

## 3. Como o tempo discreto avança hoje no kernel

### 3.1 Classes e arquivos relevantes

| Classe | Arquivo | Papel |
|---|---|---|
| `ModelSimulation` | `source/kernel/simulator/model/ModelSimulation.{h,cpp}` | Controlador de simulação; detém `_simulatedTime` |
| `Model` | `source/kernel/simulator/model/Model.{h,cpp}` | Detém `_futureEvents`; fornece `sendEntityToComponent()` |
| `Event` | `source/kernel/simulator/Event.h` | Evento normal: `(time, Entity*, ModelComponent*)` |
| `InternalEvent` | `source/kernel/simulator/Event.h` | Evento interno sem entidade; usa `InternalEventHandler` |
| `ModelComponent` | `source/kernel/simulator/model/ModelComponent.{h,cpp}` | Classe base de todos os componentes; define `_onDispatchEvent()` |

### 3.2 Fluxo de execução

```
ModelSimulation::start()
  └─ loop de réplicas
      └─ _initReplication()               ← zera _simulatedTime = 0.0, esvazia _futureEvents
          └─ ModelComponent::InitBetweenReplications() para cada componente
              └─ SourceModelComponent insere primeiros eventos
      └─ while (!_isReplicationEndCondition())
          └─ _stepSimulation()             ← processa um único evento
```

### 3.3 Inserção, retirada e despacho de eventos

**Inserção**: `Model::sendEntityToComponent()` (`Model.cpp:247–249`):
```cpp
Event* newEvent = new Event(
    this->getSimulation()->getSimulatedTime() + timeDelay,
    entity, component, componentinputPortNumber);
this->getFutureEvents()->insert(newEvent);
```
O método `List<Event*>::insert()` mantém a lista ordenada cronologicamente.

**Retirada**: `ModelSimulation::_stepSimulation()` (`ModelSimulation.cpp:484`):
```cpp
Event* nextEvent = _model->getFutureEvents()->front();
// ... verificações ...
_model->getFutureEvents()->pop_front();
```

**Despacho**: `ModelSimulation::_dispatchEvent()` (`ModelSimulation.cpp:520–537`):
```cpp
InternalEvent* intEvent = dynamic_cast<InternalEvent*>(event);
if (intEvent == nullptr) {
    ModelComponent::DispatchEvent(event); // → _onDispatchEvent(entity, port)
} else {
    intEvent->dispatchEvent(); // → handler registrado via setEventHandler()
}
```

### 3.4 Atualização do tempo simulado

`ModelSimulation.cpp:491`:
```cpp
if (nextEvent->getTime() >= _simulatedTime) {
    _simulatedTime = nextEvent->getTime();
}
```

O tempo avança de forma monotônica. Se um evento futuro tem tempo menor que o atual (o que não deveria ocorrer), o kernel adota a política filosófica de tratar como se ocorresse agora.

**Eventos simultâneos**: Não há tie-break explícito. A ordem de processamento de eventos com mesmo tempo depende da ordem de inserção na lista. Não há prioridade formal.

**Limites de simulação**:
- Por tempo: `_replicationLength * _replicationTimeScaleFactorToBase` (`ModelSimulation.cpp:478`)
- Por fila vazia: `_model->getFutureEvents()->size() == 0` (`ModelSimulation.cpp:114`)
- Por condição terminante: `parseExpression(_terminatingCondition) != 0.0`

**Fluxo de entidades**: `Model::sendEntityToComponent()` cria evento futuro para o componente destino com `timeDelay` opcional. O componente executa quando o evento é despachado.

---

## 4. Inventário de plugins/tools contínuos ou potencialmente contínuos

---

### 4.1 `LSODE` — `source/plugins/components/Continuous/LSODE.cpp`

- **Classe**: `LSODE : ModelComponent`
- **Tipo**: Solver ODE numérico inline (RK4 manual)
- **Usa tempo local**: Sim — variável `Variable* _timeVariable` com valor `time`
- **Usa laço interno**: Sim — `while (_doStep())` em `_onDispatchEvent()` (`LSODE.cpp:194`)
- **Consulta `ModelSimulation`**: Sim — `tnow = _parentModel->getSimulation()->getSimulatedTime()` (`LSODE.cpp:133`); usa como alvo para o laço
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Alto — moderado**. O laço avança `_timeVariable` do valor atual até `tnow` (tempo simulado global), mas todos os passos ocorrem dentro de uma única chamada de `_onDispatchEvent()`. Os estados intermediários são invisíveis ao kernel. Não cria eventos intermediários.
- **Observação**: `_doStep()` implementa RK4 inline com VLA (`double k1[numEqs]`). Partes do código têm expressões comentadas (`"TODO: List<T> metaprogramming issue"`), deixando `expression = ""` e invalidando os cálculos RK4 atuais.

---

### 4.2 `DiffEquations` — `source/plugins/components/Continuous/DiffEquations.cpp`

- **Classe**: `DiffEquations : ModelComponent`
- **Tipo**: Solver ODE via biblioteca compartilhada gerada dinamicamente
- **Usa tempo local**: Desconhecido (depende do código em `.so` gerado pelo usuário)
- **Usa laço interno**: Desconhecido
- **Consulta `ModelSimulation`**: Desconhecido; o código gerado pode ou não chamar `Model::getSimulation()->getSimulatedTime()`
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Indeterminado** — completamente dependente do código C++ gerado pelo usuário e compilado em runtime
- **Observação**: O stub gerado em `_check()` cria `SolverDefaultImpl1` mas está incompleto (`delete solver` sem equações). Arquitetura de plugin dinâmico (`.so`) delega a integração temporal ao usuário.

---

### 4.3 `BioSimulate` — `source/plugins/components/BiochemicalSimulation/BioSimulate.cpp`

- **Classe**: `BioSimulate : ModelComponent`
- **Tipo**: Gatilho de simulação contínua bioquímica
- **Usa tempo local**: Indireto — via `BioNetwork::_currentTime`
- **Usa laço interno**: Não diretamente, mas `_bioNetwork->simulate()` executa loop bulk
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Alto**. `_onDispatchEvent()` chama `_bioNetwork->simulate(startTime, stopTime, stepSize)` ou `_bioNetwork->simulate()`. Ambas as variantes executam `while (_currentTime < _stopTime) advanceOneStep()` internamente, resolvendo toda a trajetória `[t0, tf]` em uma única chamada. O tempo simulado global permanece congelado durante toda a integração.

---

### 4.4 `BioNetwork` — `source/plugins/data/BiochemicalSimulation/BioNetwork.cpp`

- **Classe**: `BioNetwork : ModelDataDefinition`
- **Tipo**: Rede bioquímica com solver RK4 nativo
- **Usa tempo local**: Sim — `_currentTime` (double), avança de `_startTime` a `_stopTime`
- **Usa laço interno**: Sim no modo síncrono (`simulate()`): `while (_currentTime < _stopTime)` (`BioNetwork.cpp:331`)
- **Consulta `ModelSimulation`**: Não diretamente
- **Agenda eventos**: **Sim, no modo `autoSchedule`** — `scheduleNextInternalEvent()` insere `InternalEvent` em `_parentModel->getFutureEvents()` (`BioNetwork.cpp:597–605`)
- **Risco de dessincronização**: **Dois perfis**:
  - Modo síncrono (`autoSchedule=false`): Alto — loop bulk interno, `_currentTime` desacoplado de `ModelSimulation::_simulatedTime`
  - Modo `autoSchedule=true`: Baixo para a contagem de passos — um passo por `InternalEvent`. Porém `_currentTime` ainda inicia em `_startTime` (e.g. `0.0`), independente de onde `ModelSimulation::_simulatedTime` está.
- **Observação**: `_autoSchedule` é **a única implementação existente de microeventos periódicos** no repositório. É um protótipo funcional da Abordagem A, mas sem alinhar `_currentTime` com o tempo global.

---

### 4.5 `BioSteadyState` — `source/plugins/components/BiochemicalSimulation/BioSteadyState.cpp`

- **Classe**: `BioSteadyState : ModelComponent`
- **Tipo**: Verificação de estado estacionário (atemporal)
- **Usa tempo local**: Não
- **Usa laço interno**: Não
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Nulo** — operação analítica sobre último resultado de `BioNetwork`. Não avança tempo.

---

### 4.6 `MetabolicFluxBalance` — `source/plugins/components/BiochemicalSimulation/MetabolicFluxBalance.cpp`

- **Classe**: `MetabolicFluxBalance : ModelComponent`
- **Tipo**: FBA — Flux Balance Analysis, LP solver
- **Usa tempo local**: Não
- **Usa laço interno**: Não (delegado a GLPK ou `MetabolicFluxBalanceSolver::solve()`)
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Nulo** — FBA é uma operação de otimização linear instantânea, sem dimensão temporal. Correto como está.

---

### 4.7 `GeneticExpressionStep` — `source/plugins/components/BiochemicalSimulation/GeneticExpressionStep.cpp`

- **Classe**: `GeneticExpressionStep : ModelComponent`
- **Tipo**: Passo de expressão genética (cinética de Hill)
- **Usa tempo local**: Parâmetro `_timeStep` (double)
- **Usa laço interno**: Não — passo único
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Médio** — `_timeStep` é parâmetro fixo; semanticamente deveria corresponder ao intervalo entre despachos consecutivos, mas nada no kernel garante isso.

---

### 4.8 `CellGrowthComponent` — `source/plugins/components/WholeCellModeling/CellGrowthComponent.cpp`

- **Classe**: `CellGrowthComponent : ModelComponent`
- **Tipo**: Crescimento exponencial de massa celular
- **Usa tempo local**: `_deltaT` (parâmetro fixo)
- **Usa laço interno**: Não — fórmula direta: `mass += mass * _growthRate * _deltaT` (`CellGrowthComponent.cpp:141`)
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Médio** — o cálculo assume que `_deltaT` é o intervalo real entre dois despachos. Se o modelo agenda este componente a cada `_deltaT` segundos simulados (por exemplo, via `sendEntityToComponent(entity, conn, _deltaT)`), há consistência. Caso o intervalo entre eventos seja outro, a integração está errada.

---

### 4.9 `MetabolicSubmodelComponent` — `source/plugins/components/WholeCellModeling/MetabolicSubmodelComponent.cpp`

- **Classe**: `MetabolicSubmodelComponent : ModelComponent`
- **Tipo**: Submodelo metabólico simplificado (ATP/ADP)
- **Usa tempo local**: `_deltaT` (parâmetro fixo)
- **Usa laço interno**: Não
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Médio** — idêntico ao padrão de `CellGrowthComponent`.

---

### 4.10 `FtsZPolymerizationComponent` — `source/plugins/components/WholeCellModeling/FtsZPolymerizationComponent.cpp`

- **Classe**: `FtsZPolymerizationComponent : ModelComponent`
- **Tipo**: Cinética de polimerização de FtsZ (divisão celular)
- **Usa tempo local**: `_deltaT` (parâmetro fixo)
- **Usa laço interno**: Não — equação diferencial discreta: `df = (kPoly*phi*(1-f) - kDepoly*f) * _deltaT`
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Médio** — mesma situação dos demais WholeCellModeling com `_deltaT`.

---

### 4.11 `StochasticReactionComponent` — `source/plugins/components/WholeCellModeling/StochasticReactionComponent.cpp`

- **Classe**: `StochasticReactionComponent : ModelComponent`
- **Tipo**: Gillespie SSA (Direct Method)
- **Usa tempo local**: Sim — `state.simulatedTime` (local), `_lastSimulatedTime`, `_wholeCellState->setCurrentTime()` (`StochasticReactionComponent.cpp:162`)
- **Usa laço interno**: Sim — `while (state.simulatedTime < timeWindow)` (`StochasticReactionComponent.cpp:187`)
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Alto** — o laço Gillespie avança um tempo estocástico interno de `0` até `_timeWindow`. Múltiplas reações disparam internamente dentro de uma única chamada de evento. `_wholeCellState->setCurrentTime(current + _timeWindow)` mantém relógio interno separado de `ModelSimulation::_simulatedTime`.

---

### 4.12 `CellDivisionEvent` — `source/plugins/components/WholeCellModeling/CellDivisionEvent.cpp`

- **Classe**: `CellDivisionEvent : ModelComponent`
- **Tipo**: Evento discreto de divisão celular
- **Usa tempo local**: Não — só verifica limiares de massa e FtsZ
- **Usa laço interno**: Não
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não (só encaminha entidade)
- **Risco de dessincronização**: **Baixo** — é corretamente discreto. O contador `_wholeCellState->setStepCount(0)` não é temporal.

---

### 4.13 `CellularAutomataComp` — `source/plugins/components/ModalModel/CellularAutomataComp.cpp`

- **Classe**: `CellularAutomataComp : ModelComponent`
- **Tipo**: Autômato celular (uma geração por evento)
- **Usa tempo local**: Não (geração discreta)
- **Usa laço interno**: Não
- **Consulta `ModelSimulation`**: Não
- **Agenda eventos**: Não
- **Risco de dessincronização**: **Baixo** — um `step()` por evento. Associação temporal implícita depende de como entidades são recirculadas. Não há unidade de tempo formal.

---

### 4.14 `MarkovChain` — `source/plugins/components/AnalyticalModeling/MarkovChain.cpp`

- **Classe**: `MarkovChain : ModelComponent`
- **Tipo**: Análise analítica de Markov (transições de estado)
- **Usa tempo local**: Não
- **Usa laço interno**: Para cálculo de estado estacionário, não para avanço temporal
- **Consulta `ModelSimulation`**: Não
- **Risco de dessincronização**: **Nulo** — ferramenta analítica; semântica de tempo não se aplica.

---

### 4.15 `RungeKutta4OdeSolver` — `source/tools/Continuous/RungeKutta4OdeSolver.h`

- **Classe**: `RungeKutta4OdeSolver : OdeSolver_if`
- **Tipo**: Solver matemático puro
- **Usa tempo local**: Recebe `t0` e `dt` como parâmetros, não possui estado temporal
- **Usa laço interno**: Não — único passo RK4
- **Consulta `ModelSimulation`**: Não
- **Risco de dessincronização**: **Nulo** — é ferramenta matemática corretamente projetada.

---

### 4.16 `SolverDefaultImpl1` — `source/tools/Continuous/SolverDefaultImpl1.{h,cpp}`

- **Classe**: `SolverDefaultImpl1 : Solver_if`
- **Tipo**: Quadratura numérica (Simpson 1/3) + derivação numérica parcial
- **Usa tempo local**: Não — iteração interna por índice `i`; variável `x` representa ponto de avaliação, não tempo de simulação
- **Usa laço interno**: Sim — `for (i = 0; i <= steps; i++)` — loop numérico de integração
- **Consulta `ModelSimulation`**: Não
- **Risco de dessincronização**: **Nulo** — ferramenta matemática. O loop é numérico de quadratura, não iteração temporal de simulação.

---

## 5. Padrões encontrados de dessincronização temporal

### Padrão 1 — Loop bulk dentro de único evento (CRÍTICO)

**Localização**: `BioNetwork::simulate()` (`BioNetwork.cpp:331–337`), `LSODE::_onDispatchEvent()` (`LSODE.cpp:194`), `StochasticReactionComponent::_runGillespie()` (`StochasticReactionComponent.cpp:187`).

**Descrição**: O componente executa `N` passos temporais dentro de uma única chamada de `_onDispatchEvent()`. Durante toda a integração, `ModelSimulation::_simulatedTime` está congelado no instante do evento que acionou o componente. Se a simulação tivesse eventos discretos agendados para instantes intermediários (e.g., `t=2`, `t=3` enquanto o solver integra de `t=0` a `t=10`), esses eventos seriam processados **depois** que toda a integração já ocorreu — não em paralelo cronológico.

```
ModelSimulation::_simulatedTime = T_dispatch  ← congelado durante todo o loop
    └─ BioNetwork::simulate(t0=0, tf=100, dt=0.1)
         while (_currentTime < 100.0) {
             advanceOneStep();  // 1000 passos internos
         }
    // Retorno ao kernel: _simulatedTime ainda = T_dispatch
```

### Padrão 2 — Parâmetro `_deltaT` fixo sem garantia de correspondência (MODERADO)

**Localização**: `CellGrowthComponent` (`_deltaT`), `MetabolicSubmodelComponent` (`_deltaT`), `FtsZPolymerizationComponent` (`_deltaT`), `GeneticExpressionStep` (`_timeStep`).

**Descrição**: O componente aplica uma fórmula de passo único usando `_deltaT` como parâmetro. O projeto implicitamente assume que o componente será invocado a cada `_deltaT` de tempo simulado (via `sendEntityToComponent(entity, conn, _deltaT)`). Mas se o modelo não garante esse intervalo — por exemplo, se outros eventos atrasam a entidade — o `_deltaT` utilizado no cálculo não corresponde ao tempo real transcorrido entre dois despachos consecutivos.

### Padrão 3 — Relógio interno autônomo (MODERADO)

**Localização**: `BioNetwork::_currentTime`, `StochasticReactionComponent::_lastSimulatedTime`, `LSODE::_timeVariable`, `WholeCellState::getCurrentTime()`.

**Descrição**: Cada componente mantém seu próprio relógio interno iniciando de um valor definido pelo usuário (frequentemente zero). Esses relógios avançam independentemente de `ModelSimulation::_simulatedTime`, criando múltiplos "tempos" coexistentes sem coordenação formal.

### Padrão 4 — Sincronização parcial via leitura de `getSimulatedTime()` sem eventos intermediários

**Localização**: `LSODE::_doStep()` (`LSODE.cpp:133`): `tnow = _parentModel->getSimulation()->getSimulatedTime()`.

**Descrição**: `LSODE` lê o tempo simulado global como alvo (`tnow`) e avança passos até atingi-lo. Isso evita que `_timeVariable` ultrapasse `ModelSimulation::_simulatedTime`, mas todos os passos ainda ocorrem dentro de um único despacho. É uma sincronização de **destino**, não de **intercalação**.

### Padrão 5 — Relógio WholeCellState desacoplado

**Localização**: `StochasticReactionComponent::_onDispatchEvent()` (`StochasticReactionComponent.cpp:162`): `_wholeCellState->setCurrentTime(_wholeCellState->getCurrentTime() + _timeWindow)`.

**Descrição**: O estado celular mantém um contador de tempo próprio incrementado por `_timeWindow` a cada despacho. Não há verificação de que `_timeWindow` corresponde ao intervalo entre eventos no calendário.

---

## 6. Casos em que solver matemático não deve ser sincronizado ao tempo simulado

Os seguintes casos representam usos **corretos e intencionais** de ferramentas matemáticas **fora** do contexto de avanço temporal da simulação:

| Ferramenta | Arquivo | Motivo para não sincronizar |
|---|---|---|
| `RungeKutta4OdeSolver` | `tools/Continuous/RungeKutta4OdeSolver.h` | Recebe `t0` e `dt` explicitamente; é um método numérico puro, sem estado próprio |
| `SolverDefaultImpl1` (quadratura) | `tools/Continuous/SolverDefaultImpl1.cpp` | Integração de função matemática arbitrária num intervalo; o "tempo" das variáveis `x` é coordenada matemática, não tempo de simulação |
| `MassActionOdeSystem::evaluate()` | `tools/Biochemical/MassActionOdeSystem.h` | Avalia derivadas `dydt` em ponto específico; ferramenta matemática sem estado |
| `MetabolicFluxBalanceSolver::solve()` | `tools/Biochemical/MetabolicFluxBalanceSolver.h` | LP solver; calcula otimização estática; sem dimensão temporal |
| `GlpkFluxBalanceSolver::solve()` | `tools/Biochemical/GlpkFluxBalanceSolver.h` | Idem; wrapper GLPK |
| `MarkovChain` | `source/plugins/components/AnalyticalModeling/MarkovChain.cpp` | Análise estacionária de Markov; não representa avanço temporal |

**Regra geral**: Um solver deve ser integrado ao calendário de eventos somente se o problema que ele resolve é intrinsecamente temporal **dentro do modelo simulado** e se o estado produzido por ele deve ser visível a outros componentes **em instantes intermediários**. Se o resultado é uma resposta matemática pontual (integral, FBA, Markov estacionário), o solver não deve ser acoplado ao `ModelSimulation`.

---

## 7. Problemas arquiteturais atuais

### 7.1 Ausência de contrato temporal formal para componentes de passo único

`CellGrowthComponent`, `MetabolicSubmodelComponent` e `FtsZPolymerizationComponent` usam `_deltaT` como parâmetro mas não têm mecanismo para verificar ou garantir que o tempo simulado avançou exatamente `_deltaT` entre dois despachos consecutivos. O modelo do usuário precisa implementar essa garantia externamente (agendando eventos com `timeDelay = _deltaT`), sem nenhum suporte do kernel.

### 7.2 Dois modos incompatíveis em `BioNetwork`

`BioNetwork` oferece `autoSchedule` (microeventos) e modo síncrono bulk. No modo bulk, `BioSimulate` pode chamar `simulate(t0, tf, dt)` com qualquer janela temporal, completamente desacoplada. No modo `autoSchedule`, a rede avança um passo por evento, mas o tempo interno `_currentTime` começa em `_startTime` independentemente do `ModelSimulation::_simulatedTime`. Não há mecanismo para alinhar `_startTime` com `ModelSimulation::_simulatedTime` automaticamente na inicialização de réplica.

### 7.3 Múltiplos relógios internos sem hierarquia

O modelo WholeCellModeling acumula: `ModelSimulation::_simulatedTime`, `WholeCellState::getCurrentTime()`, `BioNetwork::_currentTime`, `StochasticReactionComponent::_lastSimulatedTime`, e contadores de passo (`_wholeCellState->incrementStep()`). Não há hierarquia formal nem protocolo de sincronização entre esses relógios.

### 7.4 `LSODE` parcialmente correto mas sem cobertura de intercalação

`LSODE` é o único componente que lê `getSimulatedTime()` como referência. Mas o ponto de sincronização só ocorre no início de cada `_onDispatchEvent()`: ele avança até o tempo atual, não além. Eventos discretos agendados para `t < tnow` que chegaram à fila enquanto `LSODE` ainda não havia sido despachado serão processados apenas depois.

### 7.5 `DiffEquations` com comportamento temporal indeterminado

O plugin `DiffEquations` gera e compila código C++ em runtime. A semântica temporal do código gerado depende inteiramente do usuário, sem nenhuma checagem ou interface definida pelo kernel.

---

## 8. Abordagens possíveis para semântica híbrida

### 8.1 Abordagem A — Microeventos periódicos

**Conceito**: O componente contínuo executa exatamente um passo por `_onDispatchEvent()` e insere um novo evento para si mesmo em `currentTime + dt`.

**Status no repositório**: **Já implementado parcialmente** em `BioNetwork` com `autoSchedule = true` (`BioNetwork.cpp:590–605`):

```cpp
void BioNetwork::scheduleNextInternalEvent() {
    const double eventTime = std::min(_currentTime + _stepSize, _stopTime);
    auto* event = new InternalEvent(eventTime, "BioNetworkStep");
    event->setEventHandler(this, &BioNetwork::handleInternalEvent, nullptr);
    _parentModel->getFutureEvents()->insert(event);
}
```

**Extensão necessária para alinhamento completo**: O componente precisaria usar `ModelSimulation::getSimulatedTime()` como ponto de partida do passo, em vez de seu próprio `_currentTime`. Isso exigiria que na `_initBetweenReplications()`, `_currentTime = _parentModel->getSimulation()->getSimulatedTime()` em vez de `_startTime`.

**Vantagens**:
- Reutiliza totalmente o calendário de eventos existente
- Permite intercalação cronológica de eventos discretos e contínuos
- Implementado parcialmente (prova de conceito existente)
- Não exige modificação de solvers matemáticos
- Preserva arquitetura orientada a eventos

**Desvantagens**:
- Pode gerar volume alto de eventos (e.g., 10.000 passos = 10.000 `InternalEvent`s)
- Overhead de inserção/remoção na lista para cada passo
- Cada componente com `_deltaT` precisaria de sua própria política de auto-reagendamento
- Eventos simultâneos de múltiplos componentes contínuos precisam de ordem de processamento definida
- Política de parada precisa ser explícita (não apenas `_currentTime < _stopTime`)

---

### 8.2 Abordagem B — Macrointegração com pontos de interrupção

**Conceito**: O componente agenda uma janela `[t, t+H]`, mas consulta o próximo evento discreto na fila antes de integrar. Se houver evento em `t_disc < t+H`, integra apenas até `t_disc`, processa o evento discreto e depois continua.

**Requerimentos no kernel**: O componente precisaria ter acesso ao próximo evento futuro na fila (`_model->getFutureEvents()->front()->getTime()`), que é público via `Model::getFutureEvents()`. Isso já existe.

**Vantagens**:
- Menor overhead que microeventos
- Permite passos adaptativos dentro de janelas sem eventos discretos
- Adequado para ODEs com stiffness moderada

**Desvantagens**:
- Exige que o componente consulte e interprete a fila de eventos do kernel
- Aumenta acoplamento entre plugin e kernel
- Rollback de estado é complexo se o evento discreto modificar condições do ODE
- Não existe infraestrutura para isso no repositório atual

---

### 8.3 Abordagem C — Coordenador híbrido no kernel

**Conceito**: Criar em `ModelSimulation` uma política de passo híbrido que distingue explicitamente eventos discretos e "eventos de passo contínuo". O kernel alternaria entre processar eventos discretos e avançar componentes contínuos registrados.

**Requerimentos no kernel**: Nova interface em `ModelSimulation` para componentes registrarem-se como "temporizados" com um método `continuousStep(double currentTime, double dt)` e `nextInternalEventTime()`.

**Vantagens**:
- Semântica formal mais clara
- Extensível para modelos DEVS (Discrete Event System Specification)
- Permite sincronização perfeita entre subsistemas

**Desvantagens**:
- Maior risco de regressão no kernel
- Requer nova API que todos os componentes contínuos precisariam implementar
- Complexidade de implementação alta
- Não há precedente no repositório

---

### 8.4 Abordagem D — Wrappers de solver como componentes GenESyS

**Conceito**: Manter `RungeKutta4OdeSolver`, `SolverDefaultImpl1` intactos. Criar componentes GenESyS que encapsulam um solver e se sincronizam com `ModelSimulation` via `getSimulatedTime()`.

**Status**: Parcialmente existente. `LSODE` é um wrapper que lê `getSimulatedTime()` como alvo. `BioNetwork::advanceOneStep()` usa `RungeKutta4OdeSolver::advance()` como ferramenta interna.

**Vantagens**:
- Preserva todos os solvers existentes
- Separa método numérico de semântica temporal
- `BioNetwork` com `autoSchedule` já é um protótipo funcional desta abordagem
- Nenhuma modificação em solvers

**Desvantagens**:
- Cada wrapper precisa de política de parada, alinhamento de tempo inicial, e estratégia de auto-reagendamento
- Se implementado de formas diferentes por diferentes plugins, gera inconsistências

---

### 8.5 Abordagem E — Subsimulação contínua explicitamente desacoplada

**Conceito**: Declarar formalmente que alguns solvers (`SolverDefaultImpl1`, `MassActionOdeSystem` standalone) executam em "tempo matemático local", sem pretensão de representar tempo simulado global.

**Status**: Este é o comportamento atual de `MetabolicFluxBalance` (FBA), `MarkovChain`, `BioSteadyState` e dos solvers matemáticos puros.

**Vantagens**:
- Nenhuma mudança necessária
- Correto para ferramentas analíticas sem dimensão temporal
- Zero impacto no kernel

**Desvantagens**:
- Não serve para componentes que precisam avançar tempo genuinamente
- Se aplicada a `CellGrowth`, `MetabolicSubmodel`, etc., semanticamente incorreta para modelos de células com ciclo de vida temporal
- Pode induzir confusão entre "tempo de ODE local" e "tempo simulado global"

---

## 9. Recomendação incremental

A rota mais segura dado o estado atual do repositório é:

**Passo 1 — Catalogar (sem código)**: Documentar formalmente quais componentes são "atemporais" (FBA, Markov, SteadyState) e quais são "temporizados" (LSODE, BioNetwork, WholeCellModeling com `_deltaT`). Esta distinção não existe explicitamente hoje.

**Passo 2 — Padronizar `_deltaT` via agendamento controlado**: Para `CellGrowthComponent`, `MetabolicSubmodelComponent`, `FtsZPolymerizationComponent` — documentar que o modelo do usuário deve agendar a entidade para retornar ao componente com `timeDelay = _deltaT`. Considerar futuramente um campo `_autoReschedule` que chame `sendEntityToComponent(entity, getFrontConnection(), _deltaT)` ao final de `_onDispatchEvent()`, em vez de depender do circuito externo.

**Passo 3 — Completar e estabilizar `BioNetwork::autoSchedule`**: O protótipo existente é a demonstração mais clara da Abordagem A. Alinhar `_currentTime` com `ModelSimulation::_simulatedTime` na `_initBetweenReplications()` seria uma mudança localizada e de baixo risco. Nenhum solver precisa ser modificado.

**Passo 4 — Definir interface `TimedModelComponent` (futuramente)**: Após validar o padrão com `BioNetwork`, definir uma interface opcional que componentes temporizados possam implementar: `nextInternalEventTime()` e `advanceOneStep(double t, double dt)`. Isso seria a base para a Abordagem D formalizada.

**Passo 5 — Não modificar solvers matemáticos**: `RungeKutta4OdeSolver`, `SolverDefaultImpl1`, `MassActionOdeSystem` estão corretamente projetados como ferramentas matemáticas puras. Acoplá-los ao `ModelSimulation` seria erro arquitetural.

**Passo 6 — Avaliar Abordagem C somente se overhead de microeventos for inaceitável**: Para modelos com passos muito pequenos (e.g., dt=0.001 em simulação de 1000s = 10⁶ eventos), o overhead de 10⁶ `InternalEvent`s pode ser inaceitável. Só nesse caso justifica-se avaliar um coordenador híbrido no kernel.

---

## 10. Riscos, trade-offs e critérios de decisão

### Quando a dessincronização é aceitável

- Quando o componente é ferramenta analítica atemporal (FBA, Markov, SteadyState).
- Quando o modelo não tem eventos discretos que precisem intercalar com a trajetória contínua.
- Quando o resultado do solver é apenas pós-processamento ou saída (não altera estado que outros componentes discretos consultam durante a integração).
- Quando `_deltaT` é pequeno o suficiente que o erro numérico de não sincronizar é menor que a tolerância do modelo.

### Quando a dessincronização é problemática

- Quando um componente discreto (e.g., `CellDivisionEvent`) verifica condições que deveriam ser avaliadas no instante exato em que uma concentração atinge um limiar.
- Quando múltiplos subsistemas (ex: Gillespie SSA + cinética determinística) precisam de estado consistente em cada passo.
- Quando o modelo tem lógica condicional baseada em tempo ("se concentração X > limiar em t=500s, disparar evento Y").
- Quando `_deltaT` de diferentes componentes divergem e o modelo espera que todos estejam sincronizados.

### Trade-off principal

**Microeventos (Abordagem A)** têm overhead de calendário mas preservam toda a semântica discreta do kernel. **Bulk synchronous (modo atual)** é eficiente numericamente mas bloqueia o calendário. Para modelos biológicos com ciclos de horas/dias de tempo simulado e passos de segundos, o volume de microeventos pode ser da ordem de 10³–10⁶, que é gerenciável em hardware moderno.

---

## 11. Arquivos que precisariam ser estudados em uma próxima etapa

| Arquivo | Razão |
|---|---|
| `source/plugins/data/WholeCellModeling/WholeCellState.{h,cpp}` | Entender a semântica de `getCurrentTime()`, `getStepCount()`, `getMoleculeCounts()` — interface central para todos os componentes WCM |
| `source/plugins/components/WholeCellModeling/StochasticTranscription.cpp` | Verificar se há loop interno + interação com `WholeCellState::getResourceBudget()` |
| `source/plugins/components/WholeCellModeling/StochasticTranslation.cpp` | Idem |
| `source/kernel/simulator/model/Model.cpp` (completo) | Entender `_createInternalDataDefinitions()` e ciclo de vida de entidades |
| `source/plugins/data/BiochemicalSimulation/BioSimulatorRunner.{h,cpp}` | Verificar se existe lógica de execução assíncrona ou de scheduling externo |
| `source/plugins/components/BiochemicalSimulation/BacteriaColony.cpp` | Colônia de bactérias pode envolver iteração temporal complexa |
| `source/plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.cpp` | Verificar padrão de execução (bulk vs passo) |
| `source/plugins/data/BiochemicalSimulation/GroProgramRuntime.cpp` | Runtime do GRO program — pode ter loop temporal |
| `source/kernel/simulator/model/Model.cpp` linha 610+ | Inicialização de componentes e ordem de criação de eventos iniciais |
| `source/kernel/util/List.{h,cpp}` | Entender política de inserção ordenada de `List<Event*>` — se usa `std::list::insert()` ou outra estrutura |

---

## 12. Conclusão

O kernel de simulação discreta do GenESyS é tecnicamente correto e bem estruturado. O mecanismo de calendário de eventos futuro, o avanço de `_simulatedTime` por retirada do front da fila e o despacho via `_onDispatchEvent()` são implementações clássicas de DES sem falhas identificadas.

O problema de dessincronização não está no kernel — está na lacuna entre o kernel e os plugins contínuos/bioquímicos. Esses plugins possuem tempos internos próprios que avançam dentro de uma única chamada de evento, sem criar eventos intermediários no calendário global.

A descoberta mais importante desta análise é que **a Abordagem A (microeventos periódicos) já existe como protótipo funcional** em `BioNetwork::autoSchedule`. Isso significa que o caminho para integração temporal híbrida já foi validado conceitual e praticamente no repositório — falta apenas formalizar o alinhamento entre `_currentTime` de cada componente e `ModelSimulation::_simulatedTime`, e estender o padrão para os demais componentes temporizados.

A recomendação é não modificar nenhum solver matemático, formalizar a distinção entre componentes atemporais e temporizados, e usar `BioNetwork::autoSchedule` como referência de implementação para os demais.

---

## Estado do working tree

Resultado de `git status --short` **antes** da análise:
```
(sem output — working tree limpo)
```

Resultado de `git status --short` **depois** da análise:
```
(sem output — working tree limpo)
```

**Nenhuma alteração foi realizada no código.** Todos os comandos executados foram exclusivamente de leitura. O repositório permanece em estado idêntico ao início da sessão.

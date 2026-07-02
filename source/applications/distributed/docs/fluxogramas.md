# Parte 7 — Diagramas do funcionamento da camada distribuída

Quatro diagramas Mermaid complementares: **(1)** arquitetura de componentes, **(2)** fluxo
ponta-a-ponta do orquestrador, **(3)** ciclo HTTP master↔worker (sequência) e **(4)** a lógica de
execução com failover. Renderizam no GitHub, no VS Code (extensão Mermaid) e em <https://mermaid.live>.

---

## 7.1. Arquitetura de componentes

Quem fala com quem. O **master** (`genesys_distributed_app`) orquestra; os **workers**
(`genesys_web_app`) executam lotes via HTTP; o **executor local** roda um `Simulator` no próprio
processo. Há **dois** clientes HTTP com timeouts distintos (descoberta rápida vs. execução longa).

As setas **sólidas** são relações de uso/composição (quem cria/chama quem); as **pontilhadas** são
uso do cliente HTTP, chamadas HTTP aos workers e a execução in-process. A ordem temporal está em 7.2.

```mermaid
flowchart TB
    subgraph MASTER["MASTER — genesys_distributed_app (um processo)"]
        direction TB
        CLI["app/main.cpp<br/>lê modelo + config"]
        COORD["CoordinatorApplication<br/>execute · renderSummary · renderJson"]
        MGR["DistributedSimulationManager.run()<br/>orquestra o pipeline"]
        DISC["WorkerDiscoveryService"]
        REG[("WorkerRegistry<br/>estado · latência · falhas")]
        SCHED["DistributedScheduler<br/>particiona N (seed = baseSeed + i)"]
        EXEC["DistributedExecutionManager<br/>fase paralela + failover"]
        AGG["DistributedResultsAggregator<br/>pooled exato"]
        RE["RemoteSimulationExecutor (×N)"]
        LE["LocalSimulationExecutor"]
        KER["Kernel Simulator<br/>in-process"]
        HC1["WorkerHttpClient — descoberta<br/>connect/recv 5s"]
        HC2["WorkerHttpClient — execução<br/>connect 5s · recv 300s"]
    end

    subgraph WORKERS["Workers remotos — genesys_web_app (outros processos/máquinas)"]
        direction TB
        W1["Worker :8101<br/>sessão + Kernel Simulator"]
        W2["Worker :8102<br/>sessão + Kernel Simulator"]
    end

    CLI --> COORD --> MGR
    MGR --> DISC
    MGR --> SCHED
    MGR --> EXEC
    MGR --> AGG
    DISC -->|preenche| REG
    EXEC -->|consulta e marca| REG
    EXEC --> RE
    EXEC --> LE
    LE --> KER

    DISC -. usa .-> HC1
    RE -. usa .-> HC2
    HC1 -. HTTP .-> W1
    HC1 -. HTTP .-> W2
    HC2 -. HTTP .-> W1
    HC2 -. HTTP .-> W2
```

---

## 7.2. Fluxo ponta-a-ponta (orquestrador)

Os 5 passos de `DistributedSimulationManager.run()`, com os dois caminhos de borda (nenhum target
disponível; lotes perdidos não impedem a agregação dos demais).

```mermaid
flowchart TD
    S(["Início: genesys_distributed_app"]) --> CFG["Lê config (--config JSON ou flags da CLI)<br/>e lê o arquivo do modelo"]
    CFG --> DISC["1. Descoberta: valida cada worker da lista<br/>via GET /info + /capabilities (timeout 5s)"]
    DISC --> AVAIL{"Há algum target?<br/>(worker available ou --local)"}
    AVAIL -->|não| FAIL["AggregatedResult com falha:<br/>'no available workers and local execution disabled'<br/>(preserva totalReplicationsRequested)"]
    AVAIL -->|sim| PART["2-3. Particiona N replicações em lotes<br/>soma dos lotes = N · seed = baseSeed + índice"]
    PART --> EXEC["4. Executa os lotes<br/>Fase 1 paralela + Fase 2 failover (ver 7.4)"]
    EXEC --> AGG["5. Agrega parciais (pooled exato)<br/>+ relatório por worker (estado/latência/replicações)"]
    AGG --> OUT["renderSummary no stdout<br/>+ renderJson se --output"]
    FAIL --> OUT
    OUT --> E(["Fim"])
```

---

## 7.3. Ciclo HTTP master ↔ worker (sequência)

O ciclo de vida de um job. A descoberta usa o cliente curto; cada lote remoto usa o cliente longo
(o `POST /run` bloqueia até a simulação terminar). O lote local segue o **mesmíssimo** mecanismo de
seed, mas in-process.

```mermaid
sequenceDiagram
    autonumber
    participant M as Master (run)
    participant D as WorkerDiscoveryService
    participant R as RemoteSimulationExecutor
    participant W as Worker (genesys_web_app)
    participant L as LocalSimulationExecutor

    Note over M,W: Descoberta — cliente curto (timeout 5s)
    M->>D: discover(endpoints)
    D->>W: GET /api/v1/worker/info
    W-->>D: 200 {role, simulator...}
    D->>W: GET /api/v1/worker/capabilities
    W-->>D: 200 {supportsDistributedJobs...}
    D-->>M: WorkerRegistry (available)

    Note over M: Particiona N em lotes (seed = baseSeed + i)

    Note over M,W: Lote REMOTO — cliente longo (recv 300s)
    M->>R: execute(job)
    R->>W: POST /api/v1/auth/session
    W-->>R: 201 {accessToken}
    R->>W: POST /api/v1/worker/models/import-language (modelText)
    W-->>R: 200 {componentCount}
    R->>W: POST /api/v1/worker/jobs {numberOfReplications, seed}
    W-->>R: 201 {jobId}
    R->>W: POST /api/v1/worker/jobs/{id}/run
    Note over W: aplica seed + replicações<br/>start() — bloqueia até terminar
    W-->>R: 200 (ok)
    R->>W: GET /api/v1/worker/jobs/{id}/result
    W-->>R: 200 {statistics[], counters[]}
    R-->>M: BatchResult

    Note over M,L: Lote LOCAL — sem rede
    M->>L: execute(job)
    Note over L: autoInsertPlugins → createFromLanguage<br/>applySeed (muta param in-place) → start → captura
    L-->>M: BatchResult

    Note over M: Agrega (pooled) → renderSummary + renderJson
```

---

## 7.4. Execução com retry e failover (AC-06)

Duas fases no `DistributedExecutionManager`: tentativa inicial **paralela**, depois **failover
sequencial** dos lotes que falharam. Garantia central: **um lote bem-sucedido conta uma única vez**;
lotes sem alternativa viram `lost` e não corrompem a agregação.

```mermaid
flowchart TD
    A["Lotes particionados<br/>(um destino designado por lote)"] --> B["Fase 1 — paralela<br/>std::async por lote no destino designado"]
    B --> C{"Resultado de cada lote"}
    C -->|sucesso| D["grava result + ranOn<br/>(conta UMA vez)"]
    C -->|falha| E["registra em failedTargets<br/>_markFailure: worker → Unavailable (só remoto)"]

    E --> F["Fase 2 — failover sequencial<br/>(para cada lote ainda sem sucesso)"]
    F --> G{"Há destino elegível?<br/>não-tentado · no mapa · Available se remoto<br/>(local é sempre elegível)"}
    G -->|sim| H["reatribui o lote · executa SÍNCRONO"]
    H --> I{"Sucesso?"}
    I -->|sim| D
    I -->|não| J{"ainda há tentativa (maxRetries)<br/>e alternativa?"}
    J -->|sim| G
    J -->|não| K["lote LOST<br/>(reportado em failures)"]
    G -->|não| K

    D --> Z["DistributedResultsAggregator<br/>pooled exato · lotes LOST não contam"]
    K --> Z
    Z --> R["AggregatedResult"]
```

---

## 7.5. Diagrama único — visão geral completa

Tudo num só fluxograma: da CLI à descoberta, particionamento, dispatch para os dois tipos de
executor (remoto via HTTP e local in-process), failover e agregação final. As setas pontilhadas são
chamadas/retornos entre o orquestrador e os executores.

```mermaid
flowchart TB
    START(["genesys_distributed_app — model, replications N, workers, local"])

    subgraph MASTER["MASTER — orquestrador (genesys_distributed_app)"]
        direction TB
        M1["main.cpp: lê config + arquivo do modelo"]
        M2["CoordinatorApplication.execute<br/>→ DistributedSimulationManager.run()"]

        subgraph DISCOVERY["1 · Descoberta (cliente HTTP 5s)"]
            direction TB
            D1["WorkerDiscoveryService.discover(lista)"]
            D2{"compatível?<br/>role=worker + capabilities"}
            D3[("WorkerRegistry — Available")]
            D4["marca Unavailable · ignora"]
        end

        subgraph PARTBLK["2-3 · Particionamento"]
            direction TB
            P1["DistributedScheduler.partition(N, targets, baseSeed)"]
            P2["lotes: soma = N · seed_i = baseSeed + i"]
        end

        subgraph EXECMGR["4 · DistributedExecutionManager"]
            direction TB
            E1["dispatch do lote ao destino designado"]
            E2{"lote ok?"}
            E3["Fase 2 — failover sequencial"]
            E4{"destino elegível<br/>e ainda há tentativa?"}
            E5["lote LOST · reportado em failures"]
        end

        subgraph AGGBLK["5 · Agregação + saída"]
            direction TB
            A1["DistributedResultsAggregator — pooled exato"]
            A2["AggregatedResult + WorkerReport por target"]
            A3["renderSummary (stdout) + renderJson (output)"]
        end
    end

    subgraph REMOTE["RemoteSimulationExecutor → Worker remoto (genesys_web_app · cliente HTTP 300s)"]
        direction TB
        R1["POST /api/v1/auth/session → 201 + accessToken"]
        R2["POST /api/v1/worker/models/import-language (modelText)"]
        R3["POST /api/v1/worker/jobs (numberOfReplications, seed) → 201 + jobId"]
        R4["POST /api/v1/worker/jobs/id/run<br/>worker: aplica seed → start() → bloqueia"]
        R5["GET /api/v1/worker/jobs/id/result → statistics + counters"]
    end

    subgraph LOCAL["LocalSimulationExecutor — in-process"]
        direction TB
        L1["autoInsertPlugins + createFromLanguage"]
        L2["applySeed (muta param in-place) + setNumberOfReplications"]
        L3["Simulator.start() → captura stats"]
    end

    START --> M1 --> M2 --> D1
    D1 --> D2
    D2 -->|sim| D3
    D2 -->|não| D4
    D3 --> P1 --> P2 --> E1

    E1 -. lote remoto .-> R1
    R1 --> R2 --> R3 --> R4 --> R5
    R5 -. BatchResult .-> E2

    E1 -. lote local .-> L1
    L1 --> L2 --> L3
    L3 -. BatchResult .-> E2

    E2 -->|sucesso| A1
    E2 -->|falha| E3 --> E4
    E4 -->|sim| E1
    E4 -->|não| E5 --> A1

    A1 --> A2 --> A3 --> ENDN(["resultado unificado (equivalente ao monolítico)"])
```

---
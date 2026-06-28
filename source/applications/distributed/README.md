# Genesys Distributed Simulation Layer

Intermediate orchestration layer that runs GenESyS simulations across several machines, using
existing **web worker** instances (`genesys_web_app`) as remote simulation workers. It discovers
workers, partitions the replications, executes them (locally and remotely, with failover) and
aggregates the partial results into a single unified result — as if the whole simulation had run
locally.

This layer is **not** part of the kernel and **not** the web application itself; it is a reusable
library (`genesys_distributed_core`) plus a standalone orchestrator app (`genesys_distributed_app`).

## Architecture

- `WorkerDescriptor` / `WorkerRegistry` — known workers, their capabilities, state and failure history.
- `WorkerDiscoveryService` — validates a static list of endpoints via the worker API.
- `WorkerHttpClient` — minimal outbound HTTP/1.1 client (POSIX sockets, no extra dependency).
- `DistributedScheduler` — partitions N replications into batches (one per target) with distinct seeds.
- `RemoteSimulationExecutor` / `LocalSimulationExecutor` — run a batch on a worker or in-process.
- `DistributedExecutionManager` — runs batches in parallel, with timeout/failure handling and failover.
- `DistributedResultsAggregator` — merges partial results with exact pooled statistics.
- `DistributedSimulationManager` — high-level facade used by applications.

## Build

The layer is gated by a CMake option and is **off by default** (so it never affects other apps):

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON \
    -DGENESYS_BUILD_WEB_APPLICATION=ON
cmake --build build/distributed --target genesys_distributed_app genesys_web_app
```

Binaries:
- orchestrator: `build/distributed/source/applications/distributed/genesys_distributed_app`
- worker:       `build/distributed/source/applications/web/genesys_web_app`

## Running the orchestrator

The binary is **not installed on `PATH`** — run it from its build location. The examples below run
from the project root and use a shell variable for brevity:

```bash
APP=build/distributed/source/applications/distributed/genesys_distributed_app
```

Via command-line arguments (no config file needed):

```bash
$APP --model <file> --replications <N> [--local] \
    [--worker host:port]... [--output <file.json>] \
    [--max-retries <N>] [--base-seed <N>] [--timeout <seconds>]
```

Or via a JSON config. Copy the example and edit it (the `modelFile` path and worker ports must
match your setup); the path is relative to your current directory:

```bash
cp source/applications/distributed/config/workers.example.json workers.json
# edit workers.json, then:
$APP --config workers.json
```

Running `$APP` with no arguments prints the usage. The app prints a human-readable summary to
stdout and, when `--output` (or `outputFile` in the config) is given, writes the aggregated result
as structured JSON.

## Reproducing the distributed speedup

The primary use case of this layer is running a **large number of replications** faster by spreading
them across workers. The steps below show a measurable wall-clock reduction.

### 1. Create a model file `model.txt`

The model **must be multi-line** — the `.gen` parser reads one record per line. A single-line model is
parsed as a single `ModelInfo` record (zero components, empty statistics).

```
# Genesys Simulation Model
0   ModelInfo  "DistributedDemo" version="1.0" projectTitle="" description="" analystName=""
0   ModelSimulation "" traceLevel=0 replicationLength=10.000000 numberOfReplications=20000
62  EntityType "Part"
61  Create     "Create_1" entityType="Part" nextId=64 timeBetweenCreations="norm(1.5,0.5)"
64  Delay      "Delay_1" delayExpression="norm(1.0,0.2)" nextId=63
63  Dispose    "Dispose_1" nexts=0
```

This model does real component work (entities flow Create → Delay → Dispose), so it produces real
statistics (`Delay_1.DelayTime`, `Part.TotalTimeInSystem`, `Part.WaitTime`) and counters. Because each
replication now does real work, **far fewer replications** are needed for a measurable run than with a
trivial model (and a worker batch can take several seconds — see the `--timeout` note below).

### 2. Start two workers on free ports

```bash
WEB=build/distributed/source/applications/web/genesys_web_app
pkill -f genesys_web_app           # make sure no stale worker holds the ports
$WEB --port 8101 --max-requests 200 &
$WEB --port 8102 --max-requests 200 &
sleep 2
```

### Demonstrating the required behaviors (Tema 13, section 14)

`demonstrate-requirements.sh` walks through every item the assignment's tests must show
(discovery of multiple workers, remote job submission, replication distribution, aggregation,
timeout/failure handling, failover re-routing, and distributed-vs-local coherence), printing a
PASS/FAIL line for each. It mixes live orchestrator+worker runs with the targeted unit tests.

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON -DGENESYS_BUILD_TESTS=ON
cmake --build build/distributed --target \
    genesys_distributed_app genesys_web_app genesys_kernel_unit_tests
source/applications/distributed/demonstrate-requirements.sh
```

The orchestrator output includes a per-worker report (discovery state, observed latency, failure
count and replications completed per target), so the distribution and failure handling are visible
directly in the summary / JSON.

### Automated benchmark

The helper script runs both versions and prints a filtered comparison (no startup banner). From the
project root:

```bash
source/applications/distributed/run-distributed-demo.sh            # default 20000 replications
source/applications/distributed/run-distributed-demo.sh 50000      # custom replication count
BUILD_DIR=build/distributed WORKER_PORTS="8101 8102 8103" \
    source/applications/distributed/run-distributed-demo.sh        # override build dir / workers
```

It builds nothing (build first, see above), starts/stops the workers itself, and prints `local`,
`distributed` and `speedup`. It passes a generous `--timeout` (env `TIMEOUT`, default 60s) so heavy
worker batches are not aborted. The manual steps below do the same by hand.

### 3. Measure baseline (local only) vs distributed

```bash
APP=build/distributed/source/applications/distributed/genesys_distributed_app

# Baseline: a single local engine.
time $APP --model model.txt --replications 20000 --timeout 60 --local

# Distributed: two remote workers plus the local engine (3 engines in parallel).
time $APP --model model.txt --replications 20000 --timeout 60 \
     --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local

pkill -f genesys_web_app           # clean up the workers
```

Representative result (4-core machine, model above):

| Execution                         | Wall-clock |
|-----------------------------------|------------|
| Local only (1 engine)             | ~7.5 s     |
| 2 workers + local (3 engines)     | ~3.0 s     |

Both runs complete `20000 / 20000` replications. Increase `--replications` or add more `--worker`
endpoints for a larger, clearer speedup.

### Tips and caveats

- **Raise `--timeout` for real models.** The orchestrator's default per-request timeout is **5s**. A
  worker batch that runs a non-trivial model for many replications can take longer than that; if it
  does, the batch is treated as a worker failure (timed out → marked unavailable → failover, or the
  batch is reported `lost`). Pass `--timeout <seconds>` comfortably above the per-batch time. The demo
  scripts already do this (`TIMEOUT`, default 60s).
- **Use free ports.** A worker that fails to bind is marked unavailable during discovery, and its
  share of the load falls back to the remaining targets (or local) — which hides the speedup.
- The orchestrator runs the initial attempts in parallel; failover reassignment is sequential.
- Failover during execution (a worker dying mid-job) is covered by the unit tests; pointing
  `--worker` at a dead port instead exercises the "ignore unavailable worker" path at discovery.

## Component statistics: how it works and current limits

Imported language models now instantiate their components and produce **real component statistics**
(Cstats and Counters), both locally and on remote workers, and the layer aggregates them correctly.

This relies on registering the built-in component plugins before parsing: the worker
(`genesys_web_app`) and the local executor call `autoInsertPlugins()` when creating their `Simulator`.
The built-in plugin connector maps component type names to compiled-in classes, so this works in the
static build **without loading any `.so` file** (no dynamic plugin system needed).

Two limits remain:

- **Model format must be multi-line** — one record per line, like the `.gen` files under `models/`.
  A model squeezed onto a single line is parsed as a single record (only its `ModelInfo`), yielding
  zero components. (The replication count still works because it comes from `--replications` / the job
  config, not from the parsed model.)
- **Complex `.gen` models still fail to import** (`INVALID_MODEL_SPECIFICATION`) on this branch. This
  is a separate pre-existing limitation of the parser/serializer, unrelated to plugin registration.
  Simple and moderate models run end-to-end with real statistics.

Example (Create→Dispose, multi-line, run on a remote worker) returns non-empty results:
`Create_1.CountNumberOut`, `Dispose_1.CountNumberIn` and the `Part.TotalTimeInSystem` Cstat. The
exactness of the pooled aggregation is additionally proven by `genesys_test_distributed_aggregator`
(pooled moments equal the statistics computed over the combined values).

## Tests

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON -DGENESYS_BUILD_TESTS=ON
cmake --build build/distributed --target genesys_kernel_unit_tests
ctest --test-dir build/distributed -L unit -R distributed --output-on-failure
```

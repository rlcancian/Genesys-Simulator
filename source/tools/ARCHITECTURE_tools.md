# Architecture notes for `source/tools`

## 1. Architectural overview

The `tools` package provides domain-support services for statistical analysis and numerical procedures. It exposes interfaces used by higher-level workflows and retains legacy implementations for compatibility.

## 2. Current package responsibilities

- Dataset-oriented analysis orchestration (`DataAnalyser_if`).
- Simulation result dataset loading, including Genesys `Record` text output with replication metadata.
- Distribution fitting contracts (`Fitter_if`).
- Parametric hypothesis testing (`HypothesisTester_if`, default impl 1).
- Distribution mathematical utilities and quantile inversion.
- Numerical integration and derivation utilities via legacy solver abstractions.
- Binding of abstractions to implementations through traits.

## 3. Approved target decomposition

### Data analysis
- Keep façade role in `DataAnalyser_if`.
- Clarify ownership and lifecycle of returned collaborating services.

### Fitting
- Preserve legacy fitting API.
- Transition from dummy implementation to a concrete default implementation in phased manner (FITTER-1 delivered baseline behavior, FITTER-2 added functional Beta and Weibull fitting in `FitterDefaultImpl`, and FITTER-3 promotes `TraitsTools<Fitter_if>` to `FitterDefaultImpl`).

### Hypothesis testing
- Maintain current contract and refine implementation completeness, especially two-population methods.
- HYPTEST-1 alignment keeps the current pooled-vs-Welch heuristic, but documents it as a TODO while consolidating one-population p-value coherence and the two-proportion-difference confidence interval formula.
- HYPTEST-2 final alignment updates one-population proportion confidence intervals to the normal-approximation formulation (including finite-population correction when `N` is provided).

### Distribution abstractions
- Introduce `Distribution_if` and specialized continuous/discrete interfaces.
- Keep legacy static façade while migration is in progress.

### Numerical tools
- Split legacy solver responsibilities into dedicated interfaces:
  - `Quadrature_if`
  - `RootFinder_if`
  - `OdeSystem_if`
  - `OdeSolver_if`

### Traits/binding
- Keep current traits stable.
- Expand only when safe concrete implementations exist.

## 4. Legacy compatibility strategy

- Do not break existing public signatures in this phase.
- Preserve compatibility by keeping legacy classes available while promoting only validated trait selections.
- Add new abstractions as header-only contracts until implementations are mature.

## 5. Planned migration order

1. Stabilize documentation and explicit contracts for existing headers.
2. Introduce new interface headers without forcing behavior changes.
3. Add concrete implementations incrementally behind existing traits.
4. Migrate trait bindings once implementations are production-safe.
5. Deprecate legacy conflated abstractions only after full replacement is validated.

## 6. AI Assistant component

### Role
`AIAssistant_if` is a new high-level tool abstraction that will allow users to
describe a simulation system in natural language and have the assistant construct,
configure and run a GenESyS model automatically.

### Stage 1 (complete) — Interface and implementation scaffold
- `AIAssistant_if`: abstract interface with enumerations (`AIProvider`,
  `ExecutionState`, `ReasoningEffort`), data structures (`AIAssistantConfiguration`,
  `AIAssistantRequest`, `AIAssistantResponse`, `GenesysKnowledgeItem`,
  `GenesysKnowledgeBaseSummary`) and method groups for configuration, security,
  SimulatorFacade integration, knowledge base, provider client, and workflow.
- `AIAssistantDefaultImpl`: concrete class implementing all interface methods.
- Security model: API key stored in memory only; no public getter; clearApiKey()
  overwrites the buffer before clearing; getMaskedApiKey() exposes only the last
  four characters. Permission flags (allowNetworkAccess, allowModelMutation,
  allowSimulationExecution, allowFileSystemWrites) default to false (deny-by-default).
- `TraitsTools<AIAssistant_if>` bound to `AIAssistantDefaultImpl`.

### Stage 2 (complete) — Plugin introspection
- `refreshKnowledgeBaseFromSimulator()` populates the knowledge base with:
  - A static catalog of 15 well-known GenESyS building blocks (Create, Dispose,
    Queue, Seize, Release, Process, Delay, Decide, Assign, Record, Resource,
    Entity, Statistics, Variable).
  - Dynamic introspection via `SimulatorFacade::firstPlugin()` / `nextPlugin()`:
    for each loaded `Plugin`, reads `PluginInformation` fields (typename, isComponent,
    isSource/isSink, input/output topology, descriptionHelp, languageTemplate).
  - Duplicate entries between the static catalog and dynamic plugins are silently skipped.
- Returns `true` even when no facade is attached (static catalog is still useful).

### Stage 3 (complete) — AI provider HTTP clients
- `AIProviderClient_if`: abstract interface for a single chat-completion HTTP call.
  Structures: `ChatMessage`, `ProviderRequest`, `ProviderResponse`.
- `HttpProviderClientBase`: abstract base implementing popen+curl transport.
  Security: API key written to a 0600 temp file (`mkstemp`), never on the command
  line. Shared JSON utilities: `_escapeJsonString`, `_extractJsonString`,
  `_extractJsonInt`, `_buildMessagesArray`, `_extractFirstArrayElement`.
- `OpenAIProviderClientImpl`: OpenAI chat completions; supports o-series reasoning
  models via `highReasoningMode`; optional Organization and Project headers.
- `AnthropicProviderClientImpl`: Anthropic Messages API; separates system message
  from conversation turns; extended-thinking budget for `highReasoningMode`.
- `LocalProviderClientImpl`: Ollama/LM Studio/llama.cpp; OpenAI-compatible format;
  no API key required.
- `AIAssistant_if` extended with `setProviderClient`, `getProviderClient`,
  `hasProviderClient`. `AIAssistantDefaultImpl` also exposes
  `createAndAttachProviderClient()` which creates the right client from configuration
  and takes ownership.

### Stage 4 (complete) — Prompt analysis and model plan generation
- `analyzePrompt()`: builds a system prompt injecting the GenESyS knowledge base,
  calls the provider client, and returns the LLM's structured analysis.
- `createModelPlan()`: uses a structured system prompt requesting a Markdown model
  plan (entities, resources, queues, arrival, processing steps, statistics,
  simulation parameters) and returns it in `AIAssistantResponse::generatedPlan`.
- Both methods enforce `allowNetworkAccess == true` and check `checkReady()`.
- Diagnostics include provider name, model, input/output token counts, and HTTP status.

### Stage 5 (complete) — Model construction
- `buildModel()`: makes an LLM call requesting a GenESyS `.gen` model language file.
  The system prompt includes the `.gen` format specification, a complete example, and
  the knowledge base. If `AIAssistantRequest::modelPlan` is non-empty it is included
  as the model specification. The generated `.gen` content is written to a `0600` temp
  file (`mkstemp`), loaded via `SimulatorFacade::loadModel()`, and then deleted.
  `AIAssistantResponse::generatedModelLanguage` holds the generated file text.
- `configureSimulation()`: after a model is loaded, makes a small deterministic
  (temperature=0) LLM call to extract simulation parameters as JSON from
  `AIAssistantRequest::experimentDescription`. Applies non-zero values to the current
  model via `simSetNumberOfReplications`, `simSetReplicationLength`, `simSetWarmUpPeriod`.
  Falls back gracefully when no provider is available (`.gen` parameters remain active).
- `HttpProviderClientBase` JSON utilities promoted to `public static` so that
  `AIAssistantDefaultImpl` can call `_extractJsonInt`, `_extractJsonDouble`, and
  `_extractJsonString` directly; `_extractJsonDouble` added for floating-point fields.
- `AIAssistantRequest::modelPlan` field added; `execute()` populates it from
  `createModelPlan()` output and passes the enriched request to `buildModel()`.

### Stage 6 (complete) — Simulation execution and result collection
- `runSimulation()`: verifies `allowSimulationExecution` and non-null SimulatorFacade,
  checks not already running, then calls `SimulatorFacade::simStart()` (synchronous).
  Reports number of completed replications in the response.
- `collectResults()`: iterates `SimulatorFacade::modelGetResponses()` using
  `List<SimulationResponse*>::front()`/`next()` and renders a Markdown table of
  metric name → value.
- `execute()` updated to call steps in the correct order with proper plan propagation:
  analyzePrompt → createModelPlan → buildModel (with enriched plan) →
  configureSimulation → runSimulation → collectResults.

### Planned stages
- **Stage 7** — GUI integration: prompt panel, plan review, graphical model creation.
- **Stage 8** — Security and governance: OS secret store, sandbox, dry-run mode, audit log.

### Dependency constraints
- `AIAssistant_if.h` uses only a forward declaration of `SimulatorFacade` to avoid
  pulling the full kernel chain into every including translation unit.
- `AIAssistantDefaultImpl.cpp` includes `kernel/simulator/SimulatorFacade.h` directly.
- No external libraries are introduced in Stage 1.

## 7. Stability notes and non-goals for this phase

- FITTER-2 consolidated additional fitting algorithms in `FitterDefaultImpl` (beta and weibull) on top of FITTER-1 families; FITTER-3 now switches the default trait binding so `TraitsTools<Fitter_if>` points to `FitterDefaultImpl`.
- `FitterDummyImpl` is intentionally preserved as a legacy/documental placeholder and is no longer the default fitter binding.
- No full completion of hypothesis-testing theory coverage.
- No numerical refactor of legacy solver internals.
- No cross-package behavior changes outside `source/tools`.

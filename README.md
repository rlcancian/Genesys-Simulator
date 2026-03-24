# GenESyS — Generic and Expansible System Simulator

GenESyS (**Ge**neric and E**xp**ansible **S**ystem Simulator) is an open-source simulation platform centered on **modeling and simulation as software architecture**, not just as a fixed end-user tool.

This repository contains the source code of a simulator designed to support **multiple modeling paradigms**, **plugin-based extensibility**, and **research/teaching use cases** in simulation, systems engineering, computer systems, cyber-physical systems, and related domains.

At its core, GenESyS combines:

- a **simulation kernel**;
- a **model execution engine**;
- a **plugin system** for components and model data;
- a **parser/expression evaluation infrastructure**;
- **trace, event, and experiment support**;
- and project assets for GUI/build/development workflows.

The repository is especially interesting if you are looking for:

- a simulation codebase written primarily in **C++**;
- an architecture that separates **kernel**, **model**, **components**, and **extensions**;
- a platform that can host **discrete-event simulation**, but is not restricted to it;
- a simulator that exposes its internal design clearly enough to be studied, extended, or used for teaching.

---

## 1. Repository at a Glance

GitHub currently describes this repository as **"Generic and Expansible System Simulator"**. The top level of the repository contains, among other items:

- `source/`
- `models/`
- `documentation/`
- `projects/`
- `docker/`
- `temp/`
- `autoloadplugins.txt`

This already tells a lot about the project:

- **`source/`** holds the implementation of the kernel and plugins.
- **`models/`** is the natural place for example or test models.
- **`documentation/`** groups supporting written material.
- **`projects/`** suggests IDE/build project organization.
- **`docker/`** indicates container-oriented development or reproducibility support.
- **`autoloadplugins.txt`** shows that plugins are a first-class architectural concept.

---

## 2. What GenESyS Is

GenESyS is not best understood as a monolithic simulator with a fixed set of blocks. It is better understood as a **simulation platform** organized around a reusable kernel and an extensible ecosystem of components.

That distinction matters.

A conventional simulator is often perceived mainly through its user interface and its built-in modeling blocks. GenESyS, by contrast, is structured so that the **simulation engine**, the **model representation**, the **plugin system**, the **parser**, and the **event/trace infrastructure** can be studied and evolved as separate but connected subsystems.

This makes the repository useful for at least three kinds of work:

1. **Using the simulator** to build and run models.
2. **Extending the simulator** with new components, data definitions, or domains.
3. **Studying the simulator** as a software architecture for simulation research and education.

---

## 3. Architectural Overview

### 3.1 Core idea

At a high level, the repository organizes simulation around a few central abstractions:

- **Simulator** — the higher-level platform coordinator.
- **Model** — the central unit representing a simulation model.
- **ModelSimulation** — the execution engine for replications and simulation control.
- **Event** — the unit of state change in event-driven simulation.
- **Entity** — the dynamic object that moves through the model.
- **ModelComponent** — the behavior-bearing building block of process-oriented models.
- **ModelDataDefinition**-based elements — queues, resources, variables, statistics, and other model-level data.
- **OnEventManager** — software-level event notification/observer infrastructure.
- **Parser_if** and related parser infrastructure — expression evaluation and model semantics.
- **Plugins** — the main mechanism for extending the simulator.

### 3.2 Why this matters

This architecture allows GenESyS to support a classic simulation workflow:

- define a model,
- create or load its components and data,
- initialize simulation state,
- process events over simulated time,
- collect results,
- and expose enough internal structure to support teaching, experimentation, and extension.

---

## 4. Repository Organization

Below is the most useful way to think about the repository structure.

```text
Genesys-Simulator/
├── source/               # Kernel and plugin source code
├── models/               # Example / test / study models
├── documentation/        # Documentation and supporting material
├── projects/             # IDE/build project files and development assets
├── docker/               # Container support
├── temp/                 # Temporary/generated/intermediate files
├── autoloadplugins.txt   # Plugins loaded automatically by the platform
└── ...                   # Root config/build metadata
```

### 4.1 `source/`

This is the most important directory for anyone who wants to understand or extend GenESyS.

From the files visible in this repository, `source/` clearly contains at least:

- a **kernel/simulator** area with classes such as:
  - `Model.h`
  - `ModelSimulation.h`
  - `OnEventManager.h`
  - `Parser_if.h`
- a **plugins/components** area with components such as:
  - `Create.cpp`
  - and, by implication from the plugin list, many other simulation components.

A useful mental map is:

```text
source/
├── kernel/
│   └── simulator/        # Core classes: Model, ModelSimulation, Event, parser/event managers
└── plugins/
    └── components/       # Extensible simulation/modeling components
```

### 4.2 `models/`

This folder is intended for models used with the simulator. In practice, this is where users and developers would expect to find:

- example models;
- validation models;
- demonstration scenarios;
- or domain-specific test cases.

### 4.3 `documentation/`

This folder is for documentation and supporting material. In a repository like this, this usually includes technical notes, user-oriented documentation, design notes, or experimental documentation.

### 4.4 `projects/`

The presence of `projects/` together with repository language metadata showing **Makefile** and **QMake** suggests that the repository supports multiple development/build workflows and IDE configurations.

This is likely the place to look if you want to open the project in an IDE or inspect project-level build organization.

### 4.5 `docker/`

The repository includes a `docker/` folder and GitHub reports Dockerfile usage, which indicates some level of container-oriented setup or reproducibility support.

This folder is the first place to inspect if you want:

- containerized development;
- isolated execution;
- or reproducible environment setup.

### 4.6 `autoloadplugins.txt`

This file is one of the best entry points for understanding the breadth of GenESyS.

It lists the shared libraries/plugins that the simulator is prepared to autoload, which gives a direct view into the modeling domains the platform is designed to host.

---

## 5. Simulation Kernel: the Most Important Classes

Even without reading the entire codebase, a few files reveal the architectural backbone.

### 5.1 `Model`

`Model` is arguably the central class of the kernel.

It is responsible for representing a simulation model and for aggregating key services such as:

- component management;
- model data management;
- trace management;
- event management;
- persistence;
- parsing;
- simulation control;
- future events list;
- controls and responses.

From `Model.h`, GenESyS models are not just diagrams or containers of blocks. A model is an execution-aware structure that:

- can be checked,
- loaded/saved,
- owns future events,
- creates/removes entities,
- sends entities to components,
- and exposes simulation controls/responses.

### 5.2 `ModelSimulation`

`ModelSimulation` is the execution engine for simulation runs and replications.

Its responsibilities include:

- starting, pausing, stepping, and stopping simulation;
- storing the **simulated time**;
- keeping track of the **current event**;
- handling **replications**;
- applying **replication length** and **warm-up** settings;
- managing **breakpoints** and execution state.

The class exposes a very clear discrete-event simulation structure:

- `_initSimulation()`
- `_initReplication()`
- `_stepSimulation()`
- `_isReplicationEndCondition()`
- `_replicationEnded()`
- `_simulationEnded()`

That makes the codebase especially suitable for teaching and research on event-driven simulation engines.

### 5.3 `OnEventManager`

`OnEventManager` provides a software-level event notification layer.

This is distinct from the simulation event list used by the model itself.

Its job is to allow observers/handlers to react to events such as:

- model check success;
- model load/save;
- replication start/step/end;
- event processing;
- entity create/move/remove;
- simulation start/pause/resume/end;
- breakpoints.

This suggests that GenESyS was designed not only to execute simulations, but also to support:

- trace/inspection,
- GUI synchronization,
- debugging,
- and integration with external tooling.

### 5.4 Parser interface

`Parser_if` shows that expression parsing is part of the kernel contract.

This means model behavior is not restricted to fixed numeric parameters. Instead, the simulator is designed to evaluate expressions and attach semantics to textual model definitions.

This is especially important in a plugin-based simulator, where extensibility often requires extensible expression handling as well.

---

## 6. Plugin Architecture

GenESyS is explicitly plugin-oriented.

That is not a cosmetic feature: it is one of the main architectural ideas of the project.

The file `autoloadplugins.txt` lists a large number of `.so` plugin modules that the platform can load automatically. This is one of the clearest signs that GenESyS was designed as an **extensible simulation ecosystem**, not as a fixed simulator with a closed set of modeling blocks.

### 6.1 What the plugin list tells us

The autoload list includes, among many others:

#### Discrete-event / process-flow style plugins
- `create.so`
- `delay.so`
- `dispose.so`
- `process.so`
- `queue.so`
- `resource.so`
- `seize.so`
- `release.so`
- `record.so`
- `decide.so`
- `assign.so`
- `batch.so`
- `separate.so`
- `hold.so`
- `signal.so`
- `station.so`
- `route.so`
- `storage.so`
- `variable.so`

#### State-machine and control-style plugins
- `efsm.so`
- `efsmData.so`
- `fsm_state.so`
- `fsm_transition.so`
- `fsm_modalmodel.so`
- `defaultmodalmodel.so`

#### Continuous / numerical / differential-equation related plugins
- `diffequations.so`
- `lsode.so`
- `formula.so`
- `schedule.so`
- `octave.so`

#### Stochastic / analytical plugins
- `markovchain.so`

#### Cellular automata
- `cellularautomata.so`

#### Circuit / SPICE-related plugins
- `spicecircuit.so`
- `spicenode.so`
- `spicerunner.so`
- `resistor.so`
- `capacitor.so`
- `diode.so`
- `pmos.so`
- `nmos.so`
- `vsource.so`
- `vsine.so`
- `vpulse.so`
- `and.so`
- `or.so`
- `not.so`
- `nand.so`
- `nor.so`
- `xor.so`
- `xnor.so`

This breadth strongly suggests that GenESyS is intended as a **multi-paradigm simulation platform**.

### 6.2 Why the plugin system is important

A plugin-oriented simulator has major advantages:

- new behaviors can be added without redesigning the kernel;
- the same kernel can host multiple modeling paradigms;
- research prototypes can be added incrementally;
- educational use becomes richer because the code exposes the relation between kernel and modeling abstractions.

In the case of GenESyS, the plugin list strongly indicates that the platform is designed to bridge:

- discrete-event simulation,
- process-oriented modeling,
- state machines,
- continuous/hybrid modeling,
- analytical/stochastic constructs,
- cellular automata,
- and circuit/electronic modeling.

---

## 7. Example Component: `Create`

The file `source/plugins/components/Create.cpp` provides a good example of how plugins are implemented.

From this file, you can already see several architectural patterns:

- components are created as plugin-backed classes;
- they can expose metadata via `PluginInformation`;
- they interact with the model kernel,
- the parser,
- the simulation clock,
- and the future event list.

The `Create` component:

- inserts new entities into the model,
- sets entity attributes such as arrival time and type,
- computes time between creations,
- schedules future arrivals by creating new `Event` objects,
- and sends entities forward through the model.

That is a very strong example of how GenESyS implements classic process-oriented discrete-event simulation behavior while keeping the logic encapsulated in a plugin component.

---

## 8. What Kinds of Modeling Does This Repository Suggest?

Based on the visible kernel files and the autoloaded plugins, GenESyS appears to support or target at least the following modeling families:

- **Discrete-event simulation**
- **Process-oriented simulation**
- **Queue/resource-based systems**
- **State-machine-based modeling**
- **Differential-equation-based modeling**
- **Hybrid modeling**
- **Markov-chain-related modeling**
- **Cellular automata**
- **Circuit / SPICE-like modeling**
- **Expression-driven parametric models**

This makes the repository particularly interesting for people working at the boundary of:

- simulation software engineering,
- computer systems modeling,
- cyber-physical systems,
- modeling and simulation education,
- and research platforms for multi-paradigm simulation.

---

## 9. Build and Development Notes

GitHub currently reports the repository languages roughly as:

- **C++** (dominant)
- **Makefile**
- **C**
- **Shell**
- **QMake**
- **Dockerfile**

That suggests the project has evolved with more than one build/development route.

If you are trying to build or run GenESyS, the best starting points are:

- `projects/` — for IDE/build project organization;
- `docker/` — for containerized workflows;
- root build metadata / make-related assets;
- and the source tree itself, especially `source/kernel/simulator/`.

Because this repository appears to be long-lived and research-oriented, you should expect some historical layering in its build and project organization.

---

## 10. How to Read This Repository Efficiently

If you are new to the codebase, do **not** start from arbitrary plugins.

A better reading order is:

1. **Start with the kernel classes**
   - `source/kernel/simulator/Model.h`
   - `source/kernel/simulator/ModelSimulation.h`
   - `source/kernel/simulator/OnEventManager.h`
   - `source/kernel/simulator/Parser_if.h`

2. **Then inspect representative plugins**
   - start with components like `Create`, `Delay`, `Dispose`, `Seize`, `Release`, `Queue`, `Resource`

3. **Then inspect the plugin loading strategy**
   - `autoloadplugins.txt`

4. **Then inspect models, documentation, and project assets**
   - `models/`
   - `documentation/`
   - `projects/`
   - `docker/`

This order helps you build a mental model of the platform before diving into domain-specific extensions.

---

## 11. Why This Repository Is Worth Studying

This repository is valuable not only because it contains a simulator, but because it exposes a **simulation platform architecture**.

It is especially useful if you want to study:

- how an event-driven simulation engine is organized;
- how process-oriented simulation components can be implemented over an event-driven kernel;
- how queues, resources, entities, and events can be represented in code;
- how parser infrastructure can be integrated into a simulation platform;
- how plugins can turn a simulator into a multi-domain modeling environment;
- how one codebase can host discrete-event, state-machine, cellular automata, continuous, and circuit-oriented mechanisms under a common platform idea.

---

## 12. Suggested README Improvements for the Repository Maintainer

If you are the maintainer and want this repository to be easier for other people to adopt, the following additions would make a big difference:

1. Add a **Quick Start** section with one verified build path.
2. Add a **Minimal Example** section showing how to load/run a tiny model.
3. Add a **Repository Map** section with a short description of each top-level folder.
4. Add a **Plugin Architecture** section documenting how plugins are discovered, built, and loaded.
5. Add a **Supported Modeling Domains** section summarizing the plugin families.
6. Add a **Development Workflow** section describing IDEs, build systems, and Docker usage.
7. Add a **Roadmap / Current Status** section clarifying what is stable, experimental, or legacy.

---

## 13. In Short

GenESyS is a **generic and extensible simulation platform** built primarily in C++, organized around a simulation kernel plus an extensive plugin ecosystem.

If you are interested in:

- discrete-event simulation engines,
- simulation platform architecture,
- extensible modeling tools,
- educational or research-oriented simulation software,
- or multi-paradigm simulation frameworks,

this repository is worth reading closely.

---

## 14. Current Repository Facts Used in This README Draft

This draft was based on currently visible repository information such as:

- repository title/description: **Generic and Expansible System Simulator**;
- top-level folders: `docker`, `documentation`, `models`, `projects`, `source`, `temp`;
- root file: `autoloadplugins.txt`;
- kernel files including `Model.h`, `ModelSimulation.h`, `OnEventManager.h`, and `Parser_if.h`;
- plugin file example `source/plugins/components/Create.cpp`;
- GitHub language breakdown indicating **C++**, **Makefile**, **QMake**, **Shell**, and **Dockerfile** usage.

If you want, I can also produce a **second version of this README** optimized for one of these styles:

1. **research/academic README**,
2. **developer onboarding README**,
3. **user-facing README with installation-first structure**,
4. or **bilingual README (English + Portuguese)**.

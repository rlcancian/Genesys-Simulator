---
title: "GenESyS Wiki Documentation"
subtitle: "Consolidated repository documentation from the current wiki structure"
author: "OpenAI / ChatGPT consolidation for repository documentation"
date: "2026-04-07"
lang: en
papersize: a4
geometry: margin=1in
fontsize: 11pt
colorlinks: true
toc: true
toc-depth: 2
numbersections: true
header-includes:
  - |
    ```{=latex}
    \usepackage{microtype}
    \usepackage{setspace}
    \setstretch{1.05}
    \usepackage{fancyhdr}
    \pagestyle{fancy}
    \fancyhf{}
    \fancyhead[L]{GenESyS Wiki Documentation}
    \fancyhead[R]{\leftmark}
    \fancyfoot[C]{\thepage}
    \setlength{\headheight}{14pt}
    ```
---

# Home {#home}

Welcome to the **GenESyS** wiki.

GenESyS (**Ge**neric and E**xp**ansible **S**ystem Simulator) is a C++-based simulation platform organized as an extensible software architecture for modeling and simulation. It combines a reusable kernel, parser infrastructure, plugin-oriented extensibility, optional applications, and automated tests.

This consolidated document mirrors the current wiki organization and is intended to serve as a versionable documentation artifact for the repository.

> **Important**
>
> Some historical documentation may still reflect older repository layouts and older build workflows.
>
> For the current repository, the primary references are:
>
> - `CMakeLists.txt`
> - `CMakePresets.json`
> - the current `source/` tree

## What GenESyS Is

GenESyS is not best understood as a single fixed simulator with a closed set of built-in blocks. It is better understood as a **simulation platform** built around:

- a reusable **simulation kernel**;
- **parser / semantic infrastructure**;
- **plugin libraries** for components and data definitions;
- optional **applications** such as terminal, GUI, and web front ends;
- **automated tests** and preset-based build workflows.

This makes GenESyS useful for three main kinds of work:

1. using simulation models;
2. extending the simulator;
3. studying the simulator as a software architecture.

For the conceptual introduction, start with [What is GenESyS?](#what-is-genesys).

## Current Repository Reality

The most important current facts are:

- the main build entry point is the **repository root**;
- the build system is now **CMake-centered**;
- standard workflows are described by **CMake presets**;
- the codebase is organized primarily under `source/`;
- older references to `project` / `projects` as the main build path should be treated as legacy documentation.

For the current structural view, see [Repository Overview](#repository-overview).

## Current Build System

The current build system is centered on:

- `CMakeLists.txt`
- `CMakePresets.json`

### Technical baseline

- **Minimum CMake:** 3.24
- **C++ standard:** C++23
- **Preset generator:** `Ninja`

### Main build options

The root build currently exposes options such as:

- `GENESYS_TERMINAL_APPLICATION`
- `GENESYS_BUILD_GUI_APPLICATION`
- `GENESYS_BUILD_WEB_APPLICATION`
- `GENESYS_BUILD_TESTS`
- `GENESYS_BUILD_KERNEL`
- `GENESYS_BUILD_PARSER`
- `GENESYS_BUILD_PLUGINS`
- `GENESYS_BUILD_TOOLS`

For the practical workflow, go directly to [Build and Run](#build-and-run).

## Quick Start

### Debug build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

### Release build

```bash
cmake --preset release
cmake --build --preset release
```

### Terminal application

```bash
cmake --preset terminal-app
cmake --build --preset terminal-app
```

### Kernel unit tests

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

### Smoke tests

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke
ctest --preset tests-smoke
```

### Sanitizers

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

```bash
cmake --preset ubsan
cmake --build --preset ubsan
ctest --preset ubsan
```

For details and caveats, see [Build and Run](#build-and-run).

## Current Repository Structure

A useful current map is:

```text
Genesys-Simulator/
|-- CMakeLists.txt
|-- CMakePresets.json
|-- autoloadplugins.txt
|-- documentation/
|-- docker/
|-- models/
|-- source/
|   |-- applications/
|   |-- kernel/
|   |-- parser/
|   |-- plugins/
|   `-- tests/
`-- build/     # generated out-of-source build directories
```

For the detailed current structural explanation, see [Repository Overview](#repository-overview).

## GUI Status

The Qt GUI remains part of the repository, but it should be understood as one application layer over the broader platform.

The current GUI build is integrated into the root CMake workflow, but still depends on `qmake`.

For the GUI perspective, see [GUI and Interaction](#gui-and-interaction).

For the build perspective, see [Build and Run](#build-and-run).

## Testing

Testing is part of the normal development workflow.

The current repository includes:

- unit tests;
- smoke tests;
- kernel-focused test workflows;
- sanitizer-oriented workflows.

For the execution-oriented testing path, see [Build and Run](#build-and-run) and [Tracing and Breakpoints](#tracing-and-breakpoints).

## Plugin-Oriented Scope

One of the defining strengths of GenESyS is its plugin-oriented design.

The platform is intended to support multiple modeling families through a shared core and extensibility mechanisms.

For this perspective, read:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

## Suggested Wiki Navigation

### Start here

- [Home](#home)
- [What is GenESyS?](#what-is-genesys)
- [Getting Started](#getting-started)

### If you are new to the repository

Read in this order:

1. [What is GenESyS?](#what-is-genesys)
2. [Getting Started](#getting-started)
3. [Build and Run](#build-and-run)
4. [Repository Overview](#repository-overview)
5. [Core Architecture](#core-architecture)

### If you are mainly a user

Read:

- [Getting Started](#getting-started)
- [Build and Run](#build-and-run)
- [GUI and Interaction](#gui-and-interaction)
- [Models and Examples](#models-and-examples)
- [FAQ](#faq)

### If you are mainly a developer

Read:

- [Developer Guide](#developer-guide)
- [Build and Run](#build-and-run)
- [Repository Overview](#repository-overview)
- [Core Architecture](#core-architecture)
- [Plugin System](#plugin-system)

### If you want to understand execution semantics

Read:

- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [Events and Future Events](#events-and-future-events)
- [OnEventManager](#oneventmanager)
- [Tracing and Breakpoints](#tracing-and-breakpoints)

### If you want to understand extensibility

Read:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

### If you want examples and domain breadth

Read:

- [Models and Examples](#models-and-examples)
- [Modeling Domains](#modeling-domains)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

## Suggested Next Reading

For most readers, the best next pages after `Home` are:

1. [What is GenESyS?](#what-is-genesys)
2. [Getting Started](#getting-started)
3. [Build and Run](#build-and-run)
4. [Repository Overview](#repository-overview)
5. [Core Architecture](#core-architecture)

# What is GenESyS? {#what-is-genesys}

GenESyS is a C++-based simulation platform designed as an extensible software architecture for modeling and simulation.

It should not be understood merely as a fixed end-user simulator with a closed catalog of blocks. Instead, it is better understood as a platform organized around:

- a reusable **simulation kernel**;
- a **parser and expression** layer;
- a **plugin-oriented extensibility model**;
- optional **application layers** such as terminal, GUI, and web front ends;
- and **automated tests**.

## Why GenESyS Exists

The design of GenESyS is valuable because many simulation environments become difficult to extend, difficult to study, or tightly coupled to a single interface.

GenESyS separates execution concerns, model representation concerns, extensibility concerns, interaction concerns, and verification concerns. This makes it useful not only for users, but also for developers, students, and researchers.

## What Makes GenESyS Different

### It is platform-oriented

GenESyS is not just a simulator application. It is a platform that can host different modeling styles and different application layers.

### It is plugin-oriented

Plugins are not incidental in GenESyS. They are one of the main architectural mechanisms by which modeling behavior and domain-specific capabilities are added.

### It supports multiple modeling domains

GenESyS is intended to support a broad set of modeling families, including discrete-event simulation, state-machine-oriented modeling, cellular automata, differential equations, stochastic structures, circuit-oriented modeling, and hybrid combinations.

### It is educational and architectural

The repository is useful not only for running models, but also for understanding how a simulation platform can be engineered.

## How to Think About GenESyS

A productive way to think about GenESyS is as the combination of core execution infrastructure, model semantics, extensible components, user-facing interaction layers, and verification / observability.

## Who This Platform Is For

GenESyS is relevant to users, developers, students, and researchers.

## GenESyS as a Multi-Paradigm Simulator

One of the defining strengths of GenESyS is that it is not tied to a single modeling style. It is better understood as a **multi-paradigm simulation platform**, capable of supporting different modeling families through a shared kernel plus plugin-based extensibility.

To explore this perspective further, read [Modeling Domains](#modeling-domains) and [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys).

## GenESyS as a Software Project

From a software-engineering perspective, the current repository should be read as a modular project with clear subsystem boundaries: kernel, parser, plugins, applications, tests, and examples.

For the current structural view, see [Repository Overview](#repository-overview).

For the current build workflow, see [Build and Run](#build-and-run).

## Suggested Next Reading

1. [Getting Started](#getting-started)
2. [Build and Run](#build-and-run)
3. [Repository Overview](#repository-overview)
4. [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

# Getting Started {#getting-started}

This page is for readers who want a **practical and current onboarding path** into GenESyS.

It is intentionally narrower than [Home](#home). `Home` is the navigation hub; `Getting Started` is the first guided path.

## What You Should Do First

If you are new to GenESyS, do not start by reading isolated source files or arbitrary plugins.

A better first sequence is:

1. understand what GenESyS is conceptually;
2. build the current repository in a standard way;
3. understand the repository structure;
4. understand the core architecture;
5. only then go deeper into plugins, GUI, or internal classes.

## The Minimal Reading Path

For a sound first pass, read these pages in order:

1. [What is GenESyS?](#what-is-genesys)
2. [Build and Run](#build-and-run)
3. [Repository Overview](#repository-overview)
4. [Core Architecture](#core-architecture)

## The Minimal Operational Path

If your first goal is simply to get the repository working in its current form, go directly to [Build and Run](#build-and-run).

The standard general-purpose debug workflow is:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Choose the Right Next Branch

### If you want to run and inspect models

- [GUI and Interaction](#gui-and-interaction)
- [Models and Examples](#models-and-examples)
- [FAQ](#faq)

### If you want to understand simulator execution

- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [Events and Future Events](#events-and-future-events)

### If you want to extend the platform

- [Developer Guide](#developer-guide)
- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)

### If you want the broad domain perspective

- [Modeling Domains](#modeling-domains)
- [Standard Components](#standard-components)
- [Autoloaded Plugins](#autoloaded-plugins)

## Suggested Next Reading

1. [Build and Run](#build-and-run)
2. [Repository Overview](#repository-overview)
3. [Core Architecture](#core-architecture)
4. [GUI and Interaction](#gui-and-interaction)

# Build and Run {#build-and-run}

This page describes the **current** build and execution workflow for GenESyS.

The current build entry points are the root-level files:

- `CMakeLists.txt`
- `CMakePresets.json`

## Current Build Baseline

The repository currently uses:

- **CMake** as the main build system;
- **CMake presets** for standard workflows;
- **Ninja** as the generator used by the presets;
- **C++23** as the language standard.

## Main Build Modes

The preset file currently defines workflows for:

- debug build;
- release build;
- terminal application build;
- unit-test-only build;
- kernel unit-test build and run;
- smoke tests;
- AddressSanitizer;
- UndefinedBehaviorSanitizer.

## Standard Debug Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Release Build

```bash
cmake --preset release
cmake --build --preset release
```

## Build the Terminal Application

```bash
cmake --preset terminal-app
cmake --build --preset terminal-app
```

Related pages:

- [Getting Started](#getting-started)
- [Repository Overview](#repository-overview)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

## Unit Tests

### General unit-test preset

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit
ctest --preset tests-unit
```

### Kernel unit tests

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
ctest --preset tests-kernel-unit
```

The kernel-unit workflow is particularly important for work centered on the simulator core.

Related pages:

- [Developer Guide](#developer-guide)
- [Core Architecture](#core-architecture)
- [ModelSimulation](#modelsimulation)
- [OnEventManager](#oneventmanager)

## Smoke Tests

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke
ctest --preset tests-smoke
```

## Sanitizer Builds

### AddressSanitizer

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

### UndefinedBehaviorSanitizer

```bash
cmake --preset ubsan
cmake --build --preset ubsan
ctest --preset ubsan
```

## GUI Build

The GUI remains part of the current repository, but its build path must be understood precisely.

- the GUI is enabled from the **root CMake build**;
- the GUI source lives under `source/applications/gui/qt/GenesysQtGUI`;
- the GUI is currently orchestrated by CMake;
- but the actual GUI compilation path still depends on `qmake`.

### Example GUI build

```bash
cmake -S . -B build/gui -G Ninja \
  -DGENESYS_BUILD_GUI_APPLICATION=ON \
  -DGENESYS_BUILD_TESTS=OFF

cmake --build build/gui --target genesys_gui
```

## Out-of-Source Builds

Generated files should go under directories such as:

- `build/debug`
- `build/release`
- `build/tests-kernel-unit`
- `build/tests-smoke`
- `build/asan`
- `build/ubsan`

## Relationship with Other Pages

This page explains **how to build and run the current repository**.

It should be read together with:

- [Repository Overview](#repository-overview)
- [Developer Guide](#developer-guide)
- [Core Architecture](#core-architecture)
- [GUI and Interaction](#gui-and-interaction)

## Suggested Next Reading

1. [Repository Overview](#repository-overview)
2. [Developer Guide](#developer-guide)
3. [GUI and Interaction](#gui-and-interaction)
4. [Core Architecture](#core-architecture)

# Repository Overview {#repository-overview}

This page explains the **current repository structure** of GenESyS and how to interpret it.

The current repository should be interpreted primarily from the root-level CMake files and the `source/` tree.

## Root-Level Files

The most important root-level files for understanding the current repository are:

- `CMakeLists.txt`
- `CMakePresets.json`
- `autoloadplugins.txt`

## High-Level Current Structure

```text
Genesys-Simulator/
|-- CMakeLists.txt
|-- CMakePresets.json
|-- autoloadplugins.txt
|-- documentation/
|-- docker/
|-- models/
|-- source/
|   |-- applications/
|   |-- kernel/
|   |-- parser/
|   |-- plugins/
|   `-- tests/
`-- build/   # generated out-of-source build directories
```

## `source/kernel`

This is the core of the reusable simulator infrastructure.

Related pages:

- [Core Architecture](#core-architecture)
- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [OnEventManager](#oneventmanager)
- [Events and Future Events](#events-and-future-events)

## `source/parser`

This area contains parser and expression-related support.

Related page:

- [Parser and Expressions](#parser-and-expressions)

## `source/plugins`

This area contains plugin-related source code.

Related pages:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

## `source/applications`

This area contains optional application layers built on top of the platform.

Related pages:

- [GUI and Interaction](#gui-and-interaction)
- [Build and Run](#build-and-run)
- [Getting Started](#getting-started)

## `source/tests`

This area contains the automated test infrastructure.

Related pages:

- [Developer Guide](#developer-guide)
- [Tracing and Breakpoints](#tracing-and-breakpoints)
- [Build and Run](#build-and-run)

## `models`

This area contains models and examples.

Related page:

- [Models and Examples](#models-and-examples)

## `documentation`

This area contains supporting written material. When documentation and code disagree, the code and current root build files should prevail.

## `docker`

This area contains container-related assets.

Related page:

- [Docker Support](#docker-support)

## Relationship with `Repository Projects`

The page [Repository Projects](#repository-projects) should now be read historically or with caution.

## Suggested Next Reading

1. [Core Architecture](#core-architecture)
2. [Build and Run](#build-and-run)
3. [Plugin System](#plugin-system)
4. [Developer Guide](#developer-guide)

# Repository Projects {#repository-projects}

This page clarifies how the word **project** should be interpreted in the GenESyS repository **today**.

> **Important**
>
> This is **not** the best first page for newcomers.
>
> If you want the current structure or current build flow, prefer:
>
> - [Repository Overview](#repository-overview)
> - [Build and Run](#build-and-run)

## The Most Important Current Fact

For the current repository, the main build workflow is **not** centered on a `projects/` directory.

The repository is now built primarily from the root-level:

- `CMakeLists.txt`
- `CMakePresets.json`

## Historical Meaning of “Projects”

In earlier documentation or earlier repository states, “projects” may have referred to:

- IDE project files;
- build-support artifacts;
- older environment-specific organization;
- older opening/building workflows.

## Modern Interpretation

Today, the most useful interpretation of “repository projects” in GenESyS is architectural rather than directory-centric.

## When This Page Is Useful

This page is useful mainly when older documentation mentions project folders and you need disambiguation.

## Suggested Next Reading

1. [Repository Overview](#repository-overview)
2. [Build and Run](#build-and-run)
3. [Developer Guide](#developer-guide)
4. [Core Architecture](#core-architecture)

# Core Architecture {#core-architecture}

This page explains the current architectural core of GenESyS.

GenESyS should be understood not as a single rigid simulator application, but as a **simulation platform architecture** organized around a reusable kernel, parser infrastructure, plugin extensibility, application layers, and automated verification support.

## Main Architectural Layers

A good current high-level picture is:

1. **Kernel**
2. **Parser**
3. **Plugins**
4. **Applications**
5. **Tests and examples**

## Kernel

The kernel is the reusable execution and support foundation of the simulator.

Related pages:

- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [OnEventManager](#oneventmanager)
- [Events and Future Events](#events-and-future-events)

## Parser

The parser layer provides expression and semantic support required by the platform.

Related page:

- [Parser and Expressions](#parser-and-expressions)

## Plugins

Plugins are a first-class architectural mechanism in GenESyS.

Related pages:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

## Applications

Applications are the user-facing or integration-facing layers that sit on top of the core platform.

Related pages:

- [GUI and Interaction](#gui-and-interaction)
- [Build and Run](#build-and-run)

## Tests and Examples

Tests and models provide verification, regression resistance, operational examples, and didactic examples.

Related pages:

- [Models and Examples](#models-and-examples)
- [Tracing and Breakpoints](#tracing-and-breakpoints)
- [Developer Guide](#developer-guide)

## Core Architectural Relationships

A few central relationships matter:

- model-centered relationship;
- execution-centered relationship;
- event-centered relationship;
- observability-centered relationship;
- extensibility-centered relationship.

## Suggested Next Reading

1. [Model](#model)
2. [ModelSimulation](#modelsimulation)
3. [Events and Future Events](#events-and-future-events)
4. [Plugin System](#plugin-system)

# Developer Guide {#developer-guide}

This page is for contributors and maintainers who want to work on the GenESyS codebase safely, systematically, and in alignment with the current repository structure.

## Why This Page Matters

GenESyS is a multi-subsystem simulation platform. Development work should be guided by architectural boundaries and verification discipline.

## Start from the Current Build Reality

When developing in the current repository, start from the root build files:

- `CMakeLists.txt`
- `CMakePresets.json`

For the exact commands, see [Build and Run](#build-and-run).

## Understand the Subsystem You Are Touching

Before changing code, identify which subsystem you are actually modifying:

- `source/kernel`
- `source/parser`
- `source/plugins`
- `source/applications`
- `source/tests`
- `models`

## Kernel Changes

Before modifying kernel code, read:

- [Core Architecture](#core-architecture)
- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [OnEventManager](#oneventmanager)
- [Events and Future Events](#events-and-future-events)
- [Parser and Expressions](#parser-and-expressions)

## Plugin Changes

If you are implementing or modifying plugins, read:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

## GUI Changes

If you work on the GUI, read:

- [GUI and Interaction](#gui-and-interaction)
- [Tracing and Breakpoints](#tracing-and-breakpoints)
- [Build and Run](#build-and-run)
- [Core Architecture](#core-architecture)

## Use the Smallest Sensible Build Workflow

Use the preset that best matches your task:

- `debug`
- `terminal-app`
- `tests-unit`
- `tests-kernel-unit`
- `tests-smoke`
- `asan` / `ubsan`

## Test Before and After Significant Changes

When you modify execution logic, parser behavior, plugin loading, GUI synchronization logic, or tracing/breakpoint logic, verify behavior with focused tests and, when appropriate, broader smoke tests.

## Good Reading Order for Developers

1. [Repository Overview](#repository-overview)
2. [Build and Run](#build-and-run)
3. [Core Architecture](#core-architecture)
4. [Model](#model)
5. [ModelSimulation](#modelsimulation)
6. [Parser and Expressions](#parser-and-expressions)
7. [Plugin System](#plugin-system)
8. [Anatomy of a Plugin](#anatomy-of-a-plugin)
9. [GUI and Interaction](#gui-and-interaction)
10. [Tracing and Breakpoints](#tracing-and-breakpoints)

## Suggested Next Reading

1. [Build and Run](#build-and-run)
2. [Core Architecture](#core-architecture)
3. [ModelSimulation](#modelsimulation)
4. [Plugin System](#plugin-system)

# Simulation Concepts in GenESyS {#simulation-concepts-in-genesys}

This page introduces the main simulation concepts that help make sense of the GenESyS platform.

## Model Concept

A **model** is the structured representation of the system being simulated.

Related page: [Model](#model)

## Simulated Time

Simulated time is the time axis of the modeled system, not the wall-clock time of the computer running the simulator.

Related page: [ModelSimulation](#modelsimulation)

## Event

An **event** is a meaningful change in model state.

Related page: [Events and Future Events](#events-and-future-events)

## Future Events

A future-event structure stores state changes that are scheduled to occur later in simulated time.

## Component

A **component** is a modeling building block that contributes behavior to the model.

Related pages:

- [Standard Components](#standard-components)
- [Plugin System](#plugin-system)

## Model Data

Many models need supporting structures such as queues, resources, variables, and related definitions.

## Entity

An **entity** is often the dynamic object that moves through or is transformed by the model.

## Replication

A **replication** is one run of a simulation experiment under a given model setup.

## Warm-up

A **warm-up** period exists when early transient behavior should not be treated as representative of the system’s long-run behavior.

## Trace and Breakpoint

Tracing and breakpoints are part of the observability vocabulary of GenESyS.

Related pages:

- [Tracing and Breakpoints](#tracing-and-breakpoints)
- [OnEventManager](#oneventmanager)

## Expression and Parser Support

Model semantics often depend on formulas, conditions, and evaluated expressions.

Related page: [Parser and Expressions](#parser-and-expressions)

## Plugin

A **plugin** is an extensibility unit that allows GenESyS to support different kinds of components, domains, and modeling structures over a shared platform.

## Multi-Domain Modeling

GenESyS is not confined to one narrow simulation style.

Related page: [Modeling Domains](#modeling-domains)

## Suggested Next Reading

1. [Model](#model)
2. [ModelSimulation](#modelsimulation)
3. [Events and Future Events](#events-and-future-events)
4. [Plugin System](#plugin-system)

# Model {#model}

This page explains the role of the **Model** abstraction in GenESyS.

The model is one of the most central concepts in the platform. It is where structure, semantics, components, data definitions, and execution context come together.

## Why the Model Matters

A simulation platform needs some central abstraction that represents “the simulation model” as a coherent whole.

## The Model as an Integrating Structure

The model connects multiple architectural concerns:

- component structure;
- model data;
- simulation control;
- parser-supported semantics;
- event-related behavior;
- observability-related services.

## Relationship with Other Pages

This page should be read together with:

- [Core Architecture](#core-architecture)
- [ModelSimulation](#modelsimulation)
- [Events and Future Events](#events-and-future-events)
- [Plugin System](#plugin-system)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

## Suggested Next Reading

1. [ModelSimulation](#modelsimulation)
2. [Events and Future Events](#events-and-future-events)
3. [OnEventManager](#oneventmanager)
4. [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

# ModelSimulation {#modelsimulation}

`ModelSimulation` is one of the most important execution-oriented concepts in GenESyS.

## Why ModelSimulation Matters

A simulation model can describe structure, but something must still control execution.

## Main Responsibilities

1. control the simulation lifecycle;
2. manage replications;
3. maintain simulated time;
4. execute the event-driven loop;
5. handle reporting and breakpoints.

## The Simulation Lifecycle

A useful way to think about `ModelSimulation` is as the logic that governs the sequence:

1. prepare the simulation;
2. initialize a replication;
3. execute progression steps;
4. determine termination conditions;
5. finalize the replication;
6. finalize the overall simulation.

## Simulated Time

Simulated time is fundamental because it gives order and meaning to modeled changes.

Related page: [Events and Future Events](#events-and-future-events)

## Breakpoints and Observability

A simulator is much easier to understand and maintain when execution can be observed and interrupted meaningfully.

Related pages:

- [OnEventManager](#oneventmanager)
- [Tracing and Breakpoints](#tracing-and-breakpoints)

## What to Read Next

1. [Events and Future Events](#events-and-future-events)
2. [OnEventManager](#oneventmanager)
3. [Tracing and Breakpoints](#tracing-and-breakpoints)
4. [Model](#model)

# Events and Future Events {#events-and-future-events}

This page explains the role of events and future-event logic in GenESyS.

## Why Events Matter

In an event-driven simulation platform, the system progresses through meaningful changes associated with events.

## Events as Units of Change

An event can be understood as a meaningful change in the state of the modeled system.

## Future Events

A future-event structure exists because many modeled changes are not applied immediately. They are scheduled for simulated times that lie ahead.

## Simulated Time and Events

Simulated time becomes operationally meaningful when events are ordered and processed relative to that time axis.

Related page: [ModelSimulation](#modelsimulation)

## Event-Driven Progression

A common conceptual cycle is:

1. select the next relevant future event;
2. advance simulated time to that event;
3. apply the event’s effects;
4. generate or schedule additional future events as needed.

## Relationship with Other Pages

This page should be read together with:

- [ModelSimulation](#modelsimulation)
- [Model](#model)
- [OnEventManager](#oneventmanager)
- [Standard Components](#standard-components)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

## Suggested Next Reading

1. [ModelSimulation](#modelsimulation)
2. [OnEventManager](#oneventmanager)
3. [Standard Components](#standard-components)
4. [Tracing and Breakpoints](#tracing-and-breakpoints)

# OnEventManager {#oneventmanager}

`OnEventManager` should be understood as part of the observability and notification architecture of GenESyS.

## Why OnEventManager Matters

A simulator should not only execute. It should also be observable.

Observability matters for debugging, GUI synchronization, tracing, teaching, runtime inspection, and integration with other subsystems.

## OnEventManager vs Event-Driven Simulation Events

Future-event list semantics and software-level notification semantics are not the same thing. `OnEventManager` belongs to the software-level notification layer.

Related page: [Events and Future Events](#events-and-future-events)

## OnEventManager and GUI Synchronization

A GUI often needs to know when important things have happened in the simulator.

Related page: [GUI and Interaction](#gui-and-interaction)

## OnEventManager and Tracing

Tracing becomes much cleaner when there is a structured place through which meaningful events can be observed or announced.

Related page: [Tracing and Breakpoints](#tracing-and-breakpoints)

## Suggested Next Reading

1. [Tracing and Breakpoints](#tracing-and-breakpoints)
2. [Events and Future Events](#events-and-future-events)
3. [ModelSimulation](#modelsimulation)
4. [GUI and Interaction](#gui-and-interaction)

# Parser and Expressions {#parser-and-expressions}

This page explains the role of the parser and expression layer in the GenESyS platform.

## Why the Parser Layer Matters

A simulator becomes more powerful when behavior is not limited to fixed compiled constants.

Expression support allows the platform to represent model semantics that depend on:

- configurable values;
- formulas;
- textual expressions;
- interpreted parameters;
- model-level evaluation logic.

## Parser as Semantic Infrastructure

The parser layer should be understood as a semantic service that helps connect model definitions, configurable behaviors, plugin logic, and runtime evaluation.

## Relationship with Other Pages

This page should be read together with:

- [Core Architecture](#core-architecture)
- [Model](#model)
- [Plugin System](#plugin-system)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

## Suggested Next Reading

1. [Model](#model)
2. [Plugin System](#plugin-system)
3. [Core Architecture](#core-architecture)
4. [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)

# Plugin System {#plugin-system}

This page explains the role of the plugin system in the current GenESyS architecture.

Plugins are not an afterthought in GenESyS. They are one of the central architectural mechanisms through which the simulator becomes extensible and multi-domain.

## Why the Plugin System Matters

Without a plugin system, a simulator tends to be limited to whatever capabilities were compiled directly into a fixed core.

## Current Repository Placement

The current plugin source code is organized under:

- `source/plugins`

## Current Build View of Plugins

The current build graph treats plugins as a dedicated subsystem.

## What the Plugin System Enables

The plugin system enables GenESyS to support:

- standard simulation components;
- model-data constructs;
- domain-specific extensions;
- multiple modeling families;
- architectural experimentation.

## Relationship with Other Pages

For more specific reading, continue with:

- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)
- [Modeling Domains](#modeling-domains)

## Suggested Next Reading

1. [Anatomy of a Plugin](#anatomy-of-a-plugin)
2. [Autoloaded Plugins](#autoloaded-plugins)
3. [Standard Components](#standard-components)
4. [Modeling Domains](#modeling-domains)

# Anatomy of a Plugin {#anatomy-of-a-plugin}

This page explains how to think about the internal structure and responsibilities of a plugin in GenESyS.

## Why Plugin Anatomy Matters

In a plugin-oriented simulator, extensibility works only if plugins are more than arbitrary code fragments.

## Conceptual Elements of a Plugin

A useful way to analyze a plugin is to ask:

- What is its purpose?
- What does it attach to?
- What does it require from the kernel?
- What kind of modeling concern does it implement?

## Plugins and the Model

A plugin should never be analyzed in isolation from the model.

Related page: [Model](#model)

## Plugins and Execution

Many plugins matter because they influence the execution of a simulation model.

Related pages:

- [ModelSimulation](#modelsimulation)
- [Events and Future Events](#events-and-future-events)

## Relationship with Other Pages

This page is about the structure and reasoning of plugins.

For the larger system view, see [Plugin System](#plugin-system).

## Suggested Next Reading

1. [Plugin System](#plugin-system)
2. [Autoloaded Plugins](#autoloaded-plugins)
3. [Standard Components](#standard-components)
4. [Modeling Domains](#modeling-domains)

# Autoloaded Plugins {#autoloaded-plugins}

This page explains how to interpret the autoloaded plugin landscape in the current GenESyS repository.

One of the most revealing files in the repository is `autoloadplugins.txt`.

## Why This Page Matters

A plugin-oriented simulator is often understood most quickly by looking at the set of extensions it is prepared to recognize or load.

## What the List Tells Us

The autoloaded plugin list includes families associated with:

- process-oriented and discrete-event modeling;
- queueing and service-system modeling;
- control and modal-state structures;
- differential equations and numerical support;
- Markov and stochastic structures;
- cellular automata;
- circuit / SPICE / electronic modeling;
- digital logic;
- and other specialized behaviors.

## Standard Discrete-Event / Process-Flow Plugins

The list includes many names associated with standard process-oriented modeling flows, such as constructs related to create, delay, dispose, process, queue, resource, seize, release, route, station, storage, variable, record, decide, assign, batch, separate, hold, and signal.

## State-Oriented and Control-Oriented Plugins

The autoloaded list also contains plugin names associated with EFSM, FSM, modal models, and default modal behavior.

## Circuit / SPICE / Electronic Plugins

The plugin list includes names associated with SPICE circuit structure, SPICE nodes and runners, resistors, capacitors, diodes, PMOS / NMOS, sources and waveform generators, and digital logic gates.

## Suggested Next Reading

1. [Standard Components](#standard-components)
2. [Modeling Domains](#modeling-domains)
3. [Plugin System](#plugin-system)
4. [Models and Examples](#models-and-examples)

# Standard Components {#standard-components}

This page explains how to think about the standard component vocabulary of GenESyS.

## Why Standard Components Matter

Some plugins correspond to the core modeling vocabulary that many users will encounter early and often.

## What “Standard” Means Here

A standard component is a building block that:

- appears frequently in common modeling workflows;
- is conceptually central to ordinary simulator usage;
- helps express typical process-flow or model-structure behavior;
- and serves as part of the everyday modeling vocabulary of the platform.

## Typical Examples of Standard Components

The current plugin ecosystem clearly includes standard modeling building blocks associated with concepts such as create, delay, dispose, process, queue, resource, seize, release, route, station, record, decide, assign, batch, separate, hold, signal, storage, and variable.

## Relationship with Other Pages

This page should be read together with:

- [Plugin System](#plugin-system)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
- [Models and Examples](#models-and-examples)

## Suggested Next Reading

1. [Models and Examples](#models-and-examples)
2. [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
3. [Plugin System](#plugin-system)
4. [Modeling Domains](#modeling-domains)

# Modeling Domains {#modeling-domains}

GenESyS should be understood as a **multi-domain** and **multi-paradigm** simulation platform.

## Why Modeling Domains Matter

Understanding the supported modeling domains is important because it reveals the breadth of the platform.

## Discrete-Event Simulation

Discrete-event simulation is one of the most natural domains in which GenESyS operates.

## Queueing and Service-System Modeling

The presence of queues, resources, seize/release-style concepts, and service-flow components indicates clear support for queueing and service-system modeling.

## Logic / FSM / EFSM Modeling

The platform also has clear ties to logic-oriented and state-machine-oriented modeling, including FSM and EFSM-related structures.

## Cellular Automata

The plugin landscape of the platform also includes cellular automata support.

## Differential Equations and Dynamic Modeling

The presence of differential-equation-related and numerical-support-oriented plugins suggests support for dynamic-system modeling.

## Circuit / SPICE / Electronic Modeling

One of the strongest signals of domain breadth is the inclusion of circuit- and SPICE-related elements.

## Biological and Organic System Modeling

The breadth of the platform also makes it relevant to readers interested in biological, biochemical, or organic-system-inspired modeling directions.

## What to Read Next

1. [Plugin System](#plugin-system)
2. [Autoloaded Plugins](#autoloaded-plugins)
3. [Standard Components](#standard-components)
4. [Models and Examples](#models-and-examples)

# GUI and Interaction {#gui-and-interaction}

This page explains the role and current status of the GUI and user interaction layers in GenESyS.

## The Architectural Role of the GUI

The GUI is one possible interaction layer over the simulator platform. GenESyS should not be confused with its GUI.

## Current GUI Location

The current Qt GUI source is located under `source/applications/gui/qt/GenesysQtGUI`.

## Current GUI Build Status

- the GUI is enabled from the **root CMake workflow**;
- the GUI source lives under `source/applications/gui/qt/GenesysQtGUI`;
- CMake orchestrates the GUI build;
- but the GUI compilation path is still delegated to `qmake`.

## Practical GUI Build

```bash
cmake -S . -B build/gui -G Ninja \
  -DGENESYS_BUILD_GUI_APPLICATION=ON \
  -DGENESYS_BUILD_TESTS=OFF

cmake --build build/gui --target genesys_gui
```

## Relationship with Other Pages

This page should be read together with:

- [Build and Run](#build-and-run)
- [Core Architecture](#core-architecture)
- [Model](#model)
- [Tracing and Breakpoints](#tracing-and-breakpoints)
- [Developer Guide](#developer-guide)

## Suggested Next Reading

1. [Build and Run](#build-and-run)
2. [Model](#model)
3. [Tracing and Breakpoints](#tracing-and-breakpoints)
4. [Developer Guide](#developer-guide)

# Models and Examples {#models-and-examples}

This page explains how to think about the `models/` area and the example-oriented assets of GenESyS.

## Why Models Matter

Models matter because they show how abstract simulator concepts become concrete modeling artifacts.

They are useful for users, students, developers, and researchers.

## What You Should Look For in Example Models

When reading an example model, do not focus only on surface appearance.

A better reading strategy is to ask:

1. What is the modeling domain?
2. What is the main flow?
3. What is the supporting model data?
4. What are the expected outputs?
5. What are the architectural implications?

## Good Ways to Use the Example Models

- As a user: learn the vocabulary and flow.
- As a student: connect theory to concrete structures.
- As a developer: validate whether architectural changes preserve expected behavior.
- As a researcher: use examples as seeds for broader domain exploration.

## Relationship with Other Wiki Pages

This page should be read together with:

- [Standard Components](#standard-components)
- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
- [Modeling Domains](#modeling-domains)
- [Getting Started](#getting-started)

## What to Read Next

1. [Standard Components](#standard-components)
2. [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
3. [Modeling Domains](#modeling-domains)
4. [Getting Started](#getting-started)

# Tracing and Breakpoints {#tracing-and-breakpoints}

This page explains the role of tracing, breakpoints, and runtime observability in GenESyS.

## Why This Page Matters

Simulation software can be difficult to reason about if execution is treated as an opaque process.

Tracing and breakpoint support help inspect lifecycle transitions, execution progression, event-related behavior, model-state changes, and significant runtime milestones.

## Tracing and ModelSimulation

Tracing and breakpoints are strongly connected to `ModelSimulation` because the execution lifecycle is where many important transitions occur.

## Tracing and Events

Event-driven progression becomes much easier to inspect when event ordering and simulated-time advancement can be traced or paused.

## Tracing and OnEventManager

A notification-oriented layer such as `OnEventManager` is naturally related to structured observability.

## Tracing and GUI

A GUI can benefit significantly from structured observability because it may need to display, synchronize with, or react to runtime progression.

## Suggested Next Reading

1. [ModelSimulation](#modelsimulation)
2. [OnEventManager](#oneventmanager)
3. [Events and Future Events](#events-and-future-events)
4. [GUI and Interaction](#gui-and-interaction)

# Docker Support {#docker-support}

This page explains how to interpret Docker-related support in the current GenESyS repository.

## Why This Page Matters

A long-lived technical repository may include container-related assets for reasons such as reproducibility, environment isolation, onboarding convenience, experimental workflows, or deployment support.

GenESyS includes a `docker/` area, which signals that container-oriented support exists in the repository.

## Docker Is Not the Main Entry Point

The main build entry points are still:

- `CMakeLists.txt`
- `CMakePresets.json`

Docker should not be confused with the primary build workflow.

## Relationship with Other Pages

This page should be read together with:

- [Build and Run](#build-and-run)
- [Repository Overview](#repository-overview)
- [Developer Guide](#developer-guide)

## Suggested Next Reading

1. [Build and Run](#build-and-run)
2. [Repository Overview](#repository-overview)
3. [Developer Guide](#developer-guide)

# FAQ {#faq}

This page answers common questions about the **current** GenESyS repository and wiki.

## Build and Repository

### What is the main build system of the current repository?

The current repository is built primarily through the root-level CMake workflow.

See [Build and Run](#build-and-run).

### Is the repository still centered on a `projects/` folder?

No. Historical references may still exist, but the modern workflow is root-centered and CMake-based.

See [Repository Overview](#repository-overview) and [Repository Projects](#repository-projects).

### What is the safest first build to try?

For most readers, the safest first workflow is the standard debug build.

## GUI and Interaction

### Is the GUI the whole simulator?

No. The GUI is only one application layer over a broader platform architecture.

### Does the GUI use pure CMake?

Not entirely. The GUI is enabled from the root CMake workflow, but its current build path still depends on `qmake`.

## Architecture and Core Concepts

### Where do I learn what GenESyS actually is?

Start with [What is GenESyS?](#what-is-genesys).

### Where do I learn the main simulation concepts?

Read:

- [Simulation Concepts in GenESyS](#simulation-concepts-in-genesys)
- [Model](#model)
- [ModelSimulation](#modelsimulation)
- [Events and Future Events](#events-and-future-events)

## Plugins and Domains

### Where do I learn about plugins?

Read:

- [Plugin System](#plugin-system)
- [Anatomy of a Plugin](#anatomy-of-a-plugin)
- [Autoloaded Plugins](#autoloaded-plugins)
- [Standard Components](#standard-components)

### Is GenESyS only for discrete-event simulation?

No. It is better understood as a multi-domain, multi-paradigm simulation platform.

See [Modeling Domains](#modeling-domains).

## Development and Maintenance

### Where do I start if I want to modify code?

Start with:

1. [Developer Guide](#developer-guide)
2. [Build and Run](#build-and-run)
3. [Repository Overview](#repository-overview)
4. [Core Architecture](#core-architecture)

## Suggested Next Reading

1. [Getting Started](#getting-started)
2. [Build and Run](#build-and-run)
3. [Repository Overview](#repository-overview)
4. [Developer Guide](#developer-guide)

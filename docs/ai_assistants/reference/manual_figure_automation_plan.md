---
document_type: reference-plan
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# Manual Figure Automation Plan

## Purpose

This plan defines the future automation strategy for figures used in the
GenESyS manual. It covers user-facing screenshots, developer-facing diagrams,
and any scientific plots or tables that may later be generated from code or
verified data.

This is a planning reference only. It does not approve any external tool or
dependency by itself.

## 1. User Manual screenshots

Goal:

- compile and open the graphical applications;
- navigate menus, dialogs, panels, and tabs;
- prepare reproducible UI states;
- capture screenshots;
- crop and normalize images;
- save the files in the correct directory;
- generate captions and labels;
- insert `\includegraphics` references;
- validate the rendered PDF.

Candidate techniques to evaluate later:

- Qt test automation;
- Squish, only if already approved and available;
- `xdotool` where compatible;
- X11 or Wayland screenshot tooling;
- scripts written for the repository;
- accessibility-API-driven automation;
- desktop VM or CI execution with Xvfb;
- manual assisted capture when automation is unreliable.

No external dependency should be added without explicit review.

## 2. Developer Manual diagrams

Potential automated or semi-automated diagrams:

- UML class diagrams;
- sequence diagrams;
- component diagrams;
- package and dependency diagrams;
- repository trees;
- data flow diagrams;
- plugin lifecycle diagrams;
- CMake target relationship diagrams;
- architectural boundary diagrams.

Candidate techniques to evaluate later:

- TikZ;
- TikZ-UML;
- PlantUML;
- Graphviz;
- Doxygen diagrams;
- extraction scripts based on real code relationships;
- Mermaid only if a reliable LaTeX/PDF rendering path exists.

## 3. Scientific content

Automation targets that may later be useful:

- equations;
- algorithm pseudocode;
- tables;
- numerical examples;
- plots;
- simulation result charts.

## 4. Governance

Figure generation must remain reproducible and verified. Each figure should have
an explicit source of truth, stable naming, versioned assets when needed,
documented resolution, accessibility notes, and a human visual review before
publication.

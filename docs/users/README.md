# User documentation

This directory contains user-facing documentation configuration for GenESyS.

The current EFSM Network editor workflow is documented in
[EFSM_Network_Editor.md](EFSM_Network_Editor.md). The Doxygen configuration
includes it as the main page so it appears before the generated API reference.
The guide describes the tested integrated menu workflow and records its current
expression-validation and runtime limits.

Current Doxygen entrypoint:

    doxygen docs/users/DoxyfileUser

Doxygen working files are generated under:

    build/doxygen/users/

The user guide is part of the Doxygen main page. To rebuild the combined
reference PDF, run Doxygen from the repository root, build the generated LaTeX
with `latexmk -pdf -interaction=nonstopmode -halt-on-error refman.tex` from
`build/doxygen/users/latex/`, then copy `refman.pdf` to
`docs/users/GenESyS-User-Documentation.pdf`. The configured compatibility
stylesheet maps Unicode glyphs in source comments that the system pdfLaTeX
table does not provide.

Only the final PDF should be versioned here:

    docs/users/GenESyS-User-Documentation.pdf

Intermediate Doxygen artifacts must not be versioned.

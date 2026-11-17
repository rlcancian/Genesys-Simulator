# Multiline graphical labels — 2026-11-17

Base b69af467c, WorkInProgress. Initial tree clean, fetch confirmed 0 ahead/behind.
Maintainer explicitly authorized direct commit/push.

Shared caption layout reuses wrapTextToWidth with bold primary font metrics.
Words wrap normally; unbroken strings use the existing character-splitting fallback.
No line count limit. Name remains in the smaller regular font below the full Label.
All line heights and block gaps are measured. A uniform text-only transform fits
overflow to available height/width, with shadow padding, retaining font hierarchy.
No ports, item bounds, persisted values or plugin semantics change. Extremely large
captions become smaller; enlarge the item or zoom for readability. Arbitrarily long
text cannot remain at natural font size inside fixed finite geometry.

layoutText exposes the actual shared layout for tests; painting consumes the same
layout. Added deterministic font-metric tests for 1/2/3/4/7 lines, reconstruction of
all text, bold/size/alignment hierarchy, vertical ordering, and 400-character words.
The latter checks transformed rectangles fit and compares QImages against text-free
images to detect text outside object bounds in both styles and item kinds.
Existing GMDD tests cover empty-label fallback and live property updates.

Executed focused renderer suite: 13/13 PASS. GUI CMake/Ninja gui-app PASS.
Inspected /tmp/genesys-wrap-classic.png and /tmp/genesys-wrap-organic.png: multiline
bold label, smaller Name below. LaTeX make.sh PASS; published PDF regenerated.
User and developer manuals updated. No simulation/scientific behavior claims.

Final regression: QT_QPA_PLATFORM=offscreen ctest --preset tests-unit --parallel 4
--output-on-failure: 2150/2150 executed PASS, four pre-existing disabled, 43.99 s.
Final diff whitespace validation PASS. Final tests-unit build confirmation PASS.

# Expression Builder — parser safety and Qt6 property integration

> Development status: draft [PR #581](https://github.com/rlcancian/Genesys-Simulator/pull/581) targeting `WorkInProgress`; do not mark complete until the final-head CI and manual verification pass.

## Architecture and evidence levels

The feature depends on three ordered stages: **(1) side-effect-free syntax validation**, **(2) a shared function catalog**, and **(3) the Qt6 editor and transactional Property Editor integration**. Kernel and parser remain Qt-independent. The graphical application depends on the parser; no inverse dependency is introduced.

### 1. Parser contract

`source/parser/Genesys++-driver.h` declares `ExpressionSyntaxValidation` with `Valid`, `InvalidSyntax` and `UnresolvedIdentifiers`. `genesyspp_driver::validateSyntax(expression)` enables a temporary syntax-only mode and restores the evaluation mode afterward. In this mode, the existing `source/parser/parserBisonFlex/bisonparser.yy` grammar suppresses all semantic reductions, including assignments, model-value evaluation and probabilistic sampling. The checked-in `source/parser/GenesysParser.cpp` independently guards the generated semantic-action dispatcher, since the default build uses pre-generated sources.

`source/parser/parserBisonFlex/lexerparser.ll` and the checked-in `source/parser/Genesys++-scanner.cpp` report unresolved names and invalid raw characters rather than silently accepting them as model values. A missing symbol is **not** proof of syntactic correctness: `UnresolvedIdentifiers` is a separate status and may mask additional syntax errors until the referenced types exist. Validation is not semantic name resolution, parameter-domain checking or numerical execution.

Syntax mode does not provide a thread-safe reentrant scanner; the existing Flex/driver global lexical state remains a constraint. The test corpus demonstrates the safety invariant for selected grammar productions but is not an exhaustive formal proof of non-interference.

### 2. Shared GUI function metadata

`source/applications/gui/genesys/tools/expressionbuilder/ParserFunctionCatalog.h` owns the Qt-local catalog consumed by `DialogUtilityController` and `ExpressionBuilder`. Catalog entries are documentation/insertion hints, **not an independent grammar**. Token-only `numgr`/`atrgr` and incomplete/stub operations are not advertised as executable. When extending Bison/Flex functions, update the catalog with corresponding parser unit tests and catalog-validation tests in the same patch.

### 3. Property editor integration

`SimulationControlEditorHint::ParserExpression` identifies opt-in fields without coupling the kernel to Qt. `source/plugins/components/DiscreteProcessing/Delay.cpp` tags `DelayExpression` as the initial adopter. `ObjectPropertyBrowser` offers the dialog through context menu/Enter/double-click. The dialog buffers edits locally, validates without execution, and returns the proposed string only when accepted.

For a confirmed edit, `ObjectPropertyBrowser::_openExpressionBuilder` calls the existing `_applyVariantChange` path using the explicit specialized-editor commit flag. This centralizes mutation, control validation, applicable undo routing, and change notifications. Cancellation does not call `SimulationControl::setValue`. Only editable, active kernel bindings may commit; source-code editors and unrelated properties retain their existing editor contract.

## Validation and reproducibility

Required presets and run commands should follow [BUILD_TEST_PACKAGING.md](../ai_assistants/reference/BUILD_TEST_PACKAGING.md). Representative commands from the repository root are:

```sh
cmake --preset tests-unit
cmake --build --preset tests-unit
ctest --preset tests-unit --output-on-failure
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit
ctest --preset tests-kernel-unit --output-on-failure
cmake --preset tests-smoke
cmake --build --preset tests-smoke
ctest --preset tests-smoke --output-on-failure
```

Relevant test files are `source/tests/unit/test_parser_expressions.cpp` and `test_gui_expression_builder.cpp`. The GUI suite exercises valid/invalid text, catalog entries, read/write metadata, Cancel and a committed Delay expression in an offscreen Qt event loop. Re-run the existing Property Editor regression suite to detect changes to focus, specialized editor dispatch, persistence, and undo/redo.

**Important:** when changing grammar or lexer sources, verify the `GENESYS_PARSER_REGENERATE` build option with compatible Bison/Flex versions and compare the regenerated artifacts with the checked-in parser/scanner. The two representations currently implement an equivalent syntax-only semantic guard by different source forms; byte-for-byte equality is not claimed. Record the tool versions and build results. Confirm the final-head tests, Doxygen impact, generated PDF documentation and feature behavior before merging.

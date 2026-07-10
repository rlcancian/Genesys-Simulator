# ExpressionBuilder PropertyEditor Plan

## Goal

Refactor the current ExpressionBuilder concept so it is no longer exposed as a runtime tool from the main GUI menu. The editor must become a PropertyEditor-only helper that helps users compose parser expressions for model parameters and data definitions.

## Scope

- Remove any runtime entry points from the main menu or dialog utility controller.
- Rebuild the ExpressionBuilder as an editor integrated with the PropertyEditor surface.
- Use the parser grammar as the source of truth for available functions, symbols, and expression forms.
- Show the editor only when a property is a string-like field that is either explicitly marked as expression-bearing or can be safely detected as such.

## Requirements

- The editor must behave closer to Arena's Expression Builder.
- It must present the available parser functions and categories in a structured way.
- It must support previewing candidate expressions before committing them.
- It must validate expressions through the parser before writing the value back.
- It must remain hidden from the global runtime menu surface.

## Proposed Implementation Steps

1. Define a parser-backed metadata source for grammar functions and expression categories.
2. Add an expression-aware entry point inside the PropertyEditor for eligible string properties.
3. Introduce a safe detection contract for fields that are actually expressions, not arbitrary text.
4. Wire preview and validation through the parser before commit.
5. Add unit coverage for detection, function listing, and validation behavior.

## Non-Goals

- Do not reintroduce a standalone runtime menu action.
- Do not expose the builder as a generic GUI utility outside the PropertyEditor.
- Do not hard-code the function list in the UI once parser metadata exists.

## Notes

- The current ExpressionBuilder code may remain in the tree temporarily until the refactor replaces it.
- The parser grammar and the PropertyEditor binding model should be treated as the canonical inputs for the new implementation.

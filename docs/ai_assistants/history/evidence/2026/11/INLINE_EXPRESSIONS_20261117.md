---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
status: historical
---

# Optional Expression Builder and inline String editing

Base 193e5bc4b, WorkInProgress, clean tree, origin fetched with 0 ahead/behind.
Direct WorkInProgress commit/push explicitly authorized by maintainer.

Confirmed both reported hypotheses. _hasSpecializedEditor includes ParserExpression.
It was used in the variant factory's editor-creation filter, suppressing QLineEdit;
in _requiresCommitConfirmation, excluding expressions from ordinary String commit;
and in _applyVariantChange, rejecting inline changes without specializedEditorCommit.
Menu availability and exclusive editor ownership had been conflated.

Small Property Editor-only production fix: requiresDedicatedEditor identifies
SourceCodeString/CodeEditor. Only these suppress inline creation and ordinary commit.
_hasSpecializedEditor still controls context menu and existing specialized dispatch.
No parser, plugin setter, descriptor, hint, grammar or Builder source changed.
_isKernelEditingEnabled and active binding checks unchanged. Inline expressions
follow ordinary String validation, including allowing incomplete text until commit;
no added syntax gate. Builder acceptance still uses the explicit commit path.

Regression first failed on current base because inline QLineEdit was absent.
New real DelayExpression regression edits `2 +` then `2 + 3`, checks no mutation
while typing, one notification per commit, and save/load. Second regression checks
a real Queue Name protected by DataDefinition read-only permissions rejects commit.
A first readonly fixture incorrectly assumed components become read-only with empty
permission sets; replaced it with the actual protected DataDefinition contract.

Focused executable filter:
ExpressionBuilder*:*SourceCodePropertyEditor*:*IntegerStringAndBoolEditorsRemainFunctional
16/16 PASS, Qt offscreen, 1.69 seconds. Includes context-menu action, dialog Cancel,
dialog OK with single notification, persistence, normal Strings, numeric/boolean
editors, code-editor cancel and apply contracts, and both new regressions.
GUI gui-app and genesys_test_gui_gmdd_layout targets built with CMake/Ninja PASS.
Manual make.sh PASS, PDF regenerated; developer integration guide updated.
No full simulator suite run, as requested. No physical mouse/keyboard manual session;
Qt widget interaction tests are the functional evidence.

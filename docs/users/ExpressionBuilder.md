# Expression Builder (Qt6)

> Implementation status: draft [PR #581](https://github.com/rlcancian/Genesys-Simulator/pull/581). The workflows described here apply to that feature branch until it is integrated into `WorkInProgress`.

## Open an editable expression

In the graphical model, select a component whose Property Editor exposes a parser-expression field. The first supported example is the **Delay** component's **DelayExpression** property. Select that field and use the **Build expression...** context-menu action, double-click or press Enter to open the Expression Builder.

The dialog loads the property's current expression. Edit freely in the text area; selecting a category and a function shows its arguments, which can be filled in before choosing **Insert** (or double-clicking the function). The **Operators** category inserts arithmetic and logical syntax at the cursor. Function names and parameter arrangements come from the shared GUI reference catalog, not from a second manually maintained list.

Use **Validate** to check the expression against the installed GenESyS Bison/Flex grammar without evaluating it. A syntactically valid expression can still have undefined model symbols, invalid parameter ranges or runtime errors. References to unknown model elements are reported separately and must be resolved before the dialog accepts the expression. Validation does **not** consume random samples or execute a simulation.

Choose **OK** to validate and commit the text through the Property Editor's standard property-change pipeline, or **Cancel**/Esc to discard the dialog's edits. The expression is not changed in the model before successful acceptance. The standalone Expression Builder continues to support **Copy to Clipboard**.

## Scope and limitations

- Only properties explicitly identified as parser expressions open this editor; source-code fields, ordinary text and structured list editors retain their existing behavior.
- This editor checks grammar; it does not certify scientific or numerical correctness, execute the expression, or infer a complete type system.
- Unknown model references, context-dependent resources and queue attributes may require the model to contain the relevant definitions before syntax can be confirmed.
- At this stage **DelayExpression** is the initial integration point. Additional plugins must opt in explicitly using the kernel editor hint.

Related technical contracts: [developer guide](../developers/ExpressionBuilder.md).

# Editing an EFSM Network in the graphical interface

This guide describes the EFSM Network editor in the GenESyS Qt6 GUI. An EFSM
Network, its states, and its transitions are mathematical model objects. A
transition is a directed EFSM relation between states, not a process-flow
connection.

## Associate and open a Network

1. Select a `ModalModelDefault` in the process-model view.
2. In the Property Editor, expand its `Network` property. Choose an existing
   compatible Network, or create one after explicitly choosing its concrete
   type. Use the same property to replace or remove the association. Creating
   an association does not duplicate a Network that is shared by other
   adapters.
3. Select the adapter and choose **Open Network** from its context menu. The
   Network opens in a tab in the same main window. The action is enabled only
   when the adapter refers to a valid Network. A graphical Network data
   definition can also be opened with **Open Network**.

An attached Network can be opened before it contains states. Opening it does
not create or change the mathematical model. Network properties and adapter
bindings are edited in the Property Editor.

## Add states and transitions

1. In the Network tab, choose **Model > Create EFSM State** and enter a unique
   state name. Repeat for each state, or drag `FSMState` from the component tree
   into the Network canvas.
2. Select one state to create a self-loop, or select two states for a directed
   transition. To select both, keep `Ctrl` pressed while selecting the second
   state. Right-click a selected state and choose **Create EFSM Transition**.
   For two states, choose the source and destination in the dialog and enter a
   name. Parallel transitions between the same states are allowed.
3. To edit a transition, right-click its drawn path and choose
   **Edit EFSM Transition...**. The dialog edits its name, guard expression,
   output expression, input event, priority, trigger event, and probability
   expression.
4. Drag a state to change its diagram position; connected transition paths
   follow it. Select a state and choose **Model > Set Selected EFSM State as
   Initial** to set the initial state. Use **Model > Edit EFSM State...** to
   edit a state's name, entry action, exit action, or final-state flag.
5. Use **Edit > Delete** to remove selected EFSM items. Removing a state also
   removes its incident transitions. Supported creation, deletion, movement,
   and property edits participate in undo/redo.

The transition dialog stores expressions through the domain API. The GUI does
not parse or execute an expression merely to validate it.

## Save and reopen

Save the model in `.gen` format for its mathematical objects and use `.gui` to
retain diagram layout. Reopen the `.gui` file and use **Open Network** to reopen
the EFSM diagram. Older `.gui` files without EFSM layout data remain loadable;
the editor assigns deterministic positions when saved positions are absent.

## Scope

The editor presents EFSM states and transitions and highlights state and
transition information exposed by actual Network execution. It does not add a
separate simulation runtime or infer mathematical events from diagram drawing.

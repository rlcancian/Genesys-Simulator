# Editing an EFSM Network in the graphical interface

This guide describes the EFSM Network editor integrated into the GenESyS Qt6
GUI. Menu labels below are the English labels in the current GUI. The editor
uses the model's existing
`ModalModelDefault`, `EFSMNetwork`, states, and transitions; EFSM relations are
not process-flow connections.

## Create or attach a Network

1. Open a process model and select a `ModalModelDefault` component in its
   graphics view.
2. From the **Model** menu, choose **Create EFSM Network** and enter a unique
   Network name. Canceling the dialog leaves the model unchanged.
3. To attach an existing Network instead, choose **Select EFSM Network** and
   select the entry identified by its name and model ID. Selecting a new
   Network replaces the adapter's association; the previously registered
   Network remains in the model.
4. With the adapter selected, choose **Open EFSM Network**. The Network opens
   in a tab titled `Model > EFSM Network: NetworkName`. Adapters that refer to
   the same Network open the same context.

## Add states and transitions

1. In the Network tab, choose **Model > Create EFSM State** and enter a unique
   state name. Repeat for each state.
2. Select one state to create a self-loop, or select two states to create a
   directed relation. Choose **Model > Create EFSM Transition**, then select
   the source and destination in the dialog. The same source/destination pair
   can have parallel transitions; a transition name may be left empty.
3. Move a state by dragging it. This changes its diagram position and updates
   the drawn edges.
4. Select a state and use **Model > Set Selected EFSM State as Initial** to
   change the initial state.
5. Select one state and choose **Model > Edit EFSM State...** to change its
   name, entry action, exit action, or final-state flag. Select one transition
   and choose **Model > Edit EFSM Transition...** to edit its name, guard,
   output, input event, priority, trigger event, or probability expression.
6. In a Network tab, the normal **Edit > Delete** action removes selected EFSM
   items. Removing a state also removes its incident transitions. Undo/redo
   applies to supported EFSM creation, deletion, movement, and property edits.

The expression fields are stored by the domain APIs. This editor does not add
a parse-only syntax check; the current expression parser can evaluate an
expression, so the GUI does not execute it merely to validate input.

## Edit adapter bindings

Return to the process-model tab, select the `ModalModelDefault`, and choose
**Model > Edit EFSM Bindings...** to edit the Network's supported input and
output bindings. The binding dialog is available only when an EFSM Network is
attached.

## Save and reopen

Save the model using the graphical `.gui` format to keep both the mathematical
model and its diagram layout. The `.gen` data remains authoritative for the
Network, states, transitions, and their properties; `.gui` stores graphical
positions and edge presentation. Reopen the `.gui` file and use **Open EFSM
Network** again to restore the diagram. Older `.gui` files without EFSM layout
data remain loadable; the editor assigns deterministic positions when saved
positions are unavailable.

## Current scope

The integrated editor provides a diagram editor for EFSM Networks. It does not provide a
separate EFSM execution animation or a new runtime. Simulation semantics remain
in the existing GenESyS kernel. The guide records the tested menu and dialog
workflow; it does not claim that every possible expression is syntactically or
semantically validated by the GUI.

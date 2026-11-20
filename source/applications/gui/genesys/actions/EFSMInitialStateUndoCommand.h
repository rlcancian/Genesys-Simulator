#ifndef EFSMINITIALSTATEUNDOCOMMAND_H
#define EFSMINITIALSTATEUNDOCOMMAND_H

#include <QUndoCommand>
#include <vector>

class EFSMNetwork;
class FSMState;

class EFSMInitialStateUndoCommand final : public QUndoCommand {
public:
    EFSMInitialStateUndoCommand(EFSMNetwork* network, FSMState* state, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    bool _stateBelongsToNetwork(FSMState* state) const;
    bool _select(FSMState* state);

    EFSMNetwork* _network = nullptr;
    FSMState* _before = nullptr;
    FSMState* _after = nullptr;
    std::vector<std::pair<FSMState*, bool>> _beforeFlags;
};

#endif

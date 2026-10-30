#ifndef EFSMSTATEMEMBERSHIPUNDOCOMMAND_H
#define EFSMSTATEMEMBERSHIPUNDOCOMMAND_H

#include <QUndoCommand>
#include <memory>
#include <utility>
#include <vector>

class EFSMNetwork;
class EFSMTransition;
class FSMState;
class Model;

class EFSMStateMembershipUndoCommand final : public QUndoCommand {
public:
    EFSMStateMembershipUndoCommand(Model* model, EFSMNetwork* network,
                                   std::unique_ptr<FSMState> state, QUndoCommand* parent = nullptr);
    EFSMStateMembershipUndoCommand(Model* model, EFSMNetwork* network,
                                   FSMState* state, QUndoCommand* parent = nullptr);
    ~EFSMStateMembershipUndoCommand() override = default;
    void undo() override;
    void redo() override;

private:
    struct DetachedTransition {
        std::unique_ptr<EFSMTransition> transition;
        unsigned int ordinal = 0;
    };

    bool _belongs(FSMState* state) const;
    bool _insert();
    bool _remove();
    bool _restoreRemovedState();
    void _captureFlags();
    void _restoreFlags();

    Model* _model = nullptr;
    EFSMNetwork* _network = nullptr;
    FSMState* _state = nullptr;
    FSMState* _beforeInitial = nullptr;
    FSMState* _beforeCurrent = nullptr;
    std::vector<std::pair<FSMState*, bool>> _initialFlags;
    std::unique_ptr<FSMState> _ownedState;
    std::vector<DetachedTransition> _detachedTransitions;
    bool _insertionCommand = false;
};

#endif

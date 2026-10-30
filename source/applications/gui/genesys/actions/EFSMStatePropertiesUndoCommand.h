#ifndef EFSMSTATEPROPERTIESUNDOCOMMAND_H
#define EFSMSTATEPROPERTIESUNDOCOMMAND_H

#include <QUndoCommand>
#include <string>

class EFSMNetwork;
class FSMState;

class EFSMStatePropertiesUndoCommand final : public QUndoCommand {
public:
    struct Properties {
        std::string name;
        std::string entryActionExpression;
        std::string exitActionExpression;
        bool finalNode = false;
    };

    EFSMStatePropertiesUndoCommand(EFSMNetwork* network, FSMState* state,
                                   Properties updated, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    static Properties _read(const FSMState* state);
    bool _apply(const Properties& properties);

    EFSMNetwork* _network = nullptr;
    FSMState* _state = nullptr;
    Properties _before;
    Properties _after;
};

#endif

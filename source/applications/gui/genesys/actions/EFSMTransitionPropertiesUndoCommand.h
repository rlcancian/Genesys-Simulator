#ifndef EFSMTRANSITIONPROPERTIESUNDOCOMMAND_H
#define EFSMTRANSITIONPROPERTIESUNDOCOMMAND_H

#include <QUndoCommand>
#include <string>

class EFSMNetwork;
class EFSMTransition;

class EFSMTransitionPropertiesUndoCommand final : public QUndoCommand {
public:
    struct Properties {
        std::string name;
        std::string guardExpression;
        std::string outputExpression;
        std::string inputEvent;
        unsigned int priority = 0;
        std::string triggerEvent;
        std::string probabilityExpression;
    };

    EFSMTransitionPropertiesUndoCommand(EFSMNetwork* network, EFSMTransition* transition,
                                        Properties updated, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    bool _apply(const Properties& properties);
    static Properties _read(const EFSMTransition* transition);

    EFSMNetwork* _network = nullptr;
    EFSMTransition* _transition = nullptr;
    Properties _before;
    Properties _after;
};

#endif

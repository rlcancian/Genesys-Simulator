#ifndef MODALMODELBINDINGSUNDOCOMMAND_H
#define MODALMODELBINDINGSUNDOCOMMAND_H

#include <QUndoCommand>
#include <string>
#include <vector>

class ModalModelDefault;
class DefaultNetwork;

class ModalModelBindingsUndoCommand final : public QUndoCommand {
public:
    struct Bindings {
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
    };

    ModalModelBindingsUndoCommand(ModalModelDefault* adapter, DefaultNetwork* network,
                                  Bindings updated, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    bool _apply(const Bindings& bindings);
    ModalModelDefault* _adapter = nullptr;
    DefaultNetwork* _network = nullptr;
    Bindings _before;
    Bindings _after;
};

#endif

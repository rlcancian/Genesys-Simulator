#ifndef EFSMTRANSITIONUNDOCOMMAND_H
#define EFSMTRANSITIONUNDOCOMMAND_H

#include <QUndoCommand>
#include <memory>

class EFSMNetwork;
class EFSMTransition;

/**
 * Undoable insertion or removal of one EFSM transition.
 *
 * The Network owns an attached transition. This command owns it with a
 * unique_ptr only while detached, so undo never retains a pointer that the
 * Network has destroyed.
 */
class EFSMTransitionUndoCommand final : public QUndoCommand {
public:
	EFSMTransitionUndoCommand(EFSMNetwork* network, std::unique_ptr<EFSMTransition> transition,
							  QUndoCommand* parent = nullptr);
	EFSMTransitionUndoCommand(EFSMNetwork* network, EFSMTransition* transition,
							  QUndoCommand* parent = nullptr);
	~EFSMTransitionUndoCommand() override = default;

	void undo() override;
	void redo() override;

private:
	bool _attach();
	bool _detach();

	EFSMNetwork* _network = nullptr;
	EFSMTransition* _transition = nullptr;
	std::unique_ptr<EFSMTransition> _detachedTransition;
	unsigned int _ordinal = 0;
	bool _insertOnRedo = false;
};

#endif

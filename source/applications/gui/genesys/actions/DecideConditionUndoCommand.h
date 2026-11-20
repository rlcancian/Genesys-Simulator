#ifndef DECIDECONDITIONUNDOCOMMAND_H
#define DECIDECONDITIONUNDOCOMMAND_H

#include <QUndoCommand>
#include <QPointer>

#include "plugins/components/Decisions/Decide.h"

class GraphicalModelComponent;
class ModelGraphicsScene;

class DecideConditionUndoCommand final : public QUndoCommand {
public:
	DecideConditionUndoCommand(Decide* decide, GraphicalModelComponent* graphical,
		Decide::ConditionEdit edit, QUndoCommand* parent = nullptr);
	void undo() override;
	void redo() override;

private:
	void apply(bool after);
	void restoreRemovedConnection();

	Decide* _decide;
	QPointer<GraphicalModelComponent> _graphical;
	QPointer<ModelGraphicsScene> _scene;
	Decide::ConditionEdit _edit;
	bool _firstRedo = true;
};

#endif

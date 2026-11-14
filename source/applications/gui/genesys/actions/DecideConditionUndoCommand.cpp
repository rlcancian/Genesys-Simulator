#include "DecideConditionUndoCommand.h"

#include "graphicals/GraphicalComponentPort.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"

namespace {
GraphicalModelComponent* findGraphicalComponent(ModelGraphicsScene* scene, ModelComponent* component) {
	if (scene == nullptr || component == nullptr || scene->getGraphicalModelComponents() == nullptr) return nullptr;
	for (QGraphicsItem* item : *scene->getGraphicalModelComponents()) {
		auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
		if (graphical != nullptr && graphical->getComponent() == component) return graphical;
	}
	return nullptr;
}
}

DecideConditionUndoCommand::DecideConditionUndoCommand(Decide* decide, GraphicalModelComponent* graphical,
	Decide::ConditionEdit edit, QUndoCommand* parent)
	: QUndoCommand(parent), _decide(decide), _graphical(graphical),
	  _scene(graphical == nullptr ? nullptr : dynamic_cast<ModelGraphicsScene*>(graphical->scene())),
	  _edit(std::move(edit)) {
	setText(QStringLiteral("Edit Decide conditions"));
}

void DecideConditionUndoCommand::undo() {
	apply(false);
}

void DecideConditionUndoCommand::redo() {
	if (_firstRedo) {
		_firstRedo = false;
		return;
	}
	apply(true);
}

void DecideConditionUndoCommand::apply(bool after) {
	if (_decide == nullptr) return;
	const bool performOriginal = after;
	switch (_edit.kind) {
	case Decide::ConditionEdit::Kind::Insert:
		if (performOriginal) _decide->editConditionInsertAt(_edit.index, _edit.after);
		else _decide->editConditionRemoveAt(_edit.index);
		break;
	case Decide::ConditionEdit::Kind::Replace:
		_decide->editConditionReplaceAt(_edit.index, performOriginal ? _edit.after : _edit.before);
		break;
	case Decide::ConditionEdit::Kind::Remove:
		if (performOriginal) {
			_decide->editConditionRemoveAt(_edit.index);
		} else {
			_decide->editConditionInsertAt(_edit.index, _edit.before);
			if (_edit.removedTarget != nullptr) {
				_decide->getConnectionManager()->insertAtPort(_edit.index,
					new Connection{_edit.removedTarget, {_edit.removedTargetPort, {}}});
			}
		}
		break;
	case Decide::ConditionEdit::Kind::Move: {
		const unsigned int from = performOriginal ? _edit.index : _edit.destinationIndex;
		const unsigned int to = performOriginal ? _edit.destinationIndex : _edit.index;
		_decide->moveCondition(from, to);
		if (_graphical != nullptr) _graphical->moveOutputPort(from, to);
		break;
	}
	}

	unsigned int changedIndex = 0;
	bool inserted = false;
	if (_edit.kind != Decide::ConditionEdit::Kind::Move
		&& _decide->takeConditionPortEdit(changedIndex, inserted) && _graphical != nullptr) {
		_graphical->editOutputPort(changedIndex, inserted);
	}
	Decide::ConditionEdit ignoredEdit;
	if (_edit.kind != Decide::ConditionEdit::Kind::Move) _decide->takeConditionEdit(ignoredEdit);

	if (_edit.kind == Decide::ConditionEdit::Kind::Remove && !after) {
		restoreRemovedConnection();
	}
	if (_graphical != nullptr) _graphical->update();
	if (_scene != nullptr) {
		_scene->update();
		if (_graphical != nullptr) {
			_scene->notifyGraphicalModelChange(GraphicalModelEvent::EventType::EDIT,
				GraphicalModelEvent::EventObjectType::COMPONENT, _graphical);
		}
	}
}

void DecideConditionUndoCommand::restoreRemovedConnection() {
	if (_edit.removedTarget == nullptr || _scene == nullptr || _graphical == nullptr
		|| _graphical->getGraphicalOutputPorts().size() <= static_cast<int>(_edit.index)) {
		return;
	}
	auto* destination = findGraphicalComponent(_scene, _edit.removedTarget);
	if (destination == nullptr || destination->getGraphicalInputPorts().size() <= static_cast<int>(_edit.removedTargetPort)) {
		return;
	}
	_scene->addGraphicalConnection(
		_graphical->getGraphicalOutputPorts().at(static_cast<int>(_edit.index)),
		destination->getGraphicalInputPorts().at(static_cast<int>(_edit.removedTargetPort)), 0,
		_edit.removedTargetPort, false);
	_decide->getConnectionManager()->insertAtPort(_edit.index,
		new Connection{_edit.removedTarget, {_edit.removedTargetPort, {}}});
	_graphical->setOcupiedOutputPorts(_graphical->getOcupiedOutputPorts() + 1);
	destination->setOcupiedInputPorts(destination->getOcupiedInputPorts() + 1);
}

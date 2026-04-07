/*
 * The MIT License
 *
 * Copyright 2022 rlcancian.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * File:   QModelGraphicView.cpp
 * Author: rlcancian
 *
 * Created on 15 de fevereiro de 2022, 21:12
 */

#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalModelComponent.h"
#include "UtilGUI.h"
#include "TraitsGUI.h"
#include <Qt>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <fstream>
#include <streambuf>
#include <cmath>

ModelGraphicsView::ModelGraphicsView(QWidget *parent) : QGraphicsView(parent) {
	setInteractive(true);
	setEnabled(false);
    //
    // scene
	int iniPos = TraitsGUI<GView>::sceneCenter-TraitsGUI<GView>::sceneDistanceCenter;
	int tam = 2*TraitsGUI<GView>::sceneDistanceCenter;
	ModelGraphicsScene* scene = new ModelGraphicsScene(iniPos, iniPos, tam, tam, this);
	setScene(scene);
}

ModelGraphicsView::ModelGraphicsView(const ModelGraphicsView& orig) {
}

ModelGraphicsView::~ModelGraphicsView() {
}


//------------------------------------------------------------------

//GraphicalModelComponent* ModelGraphicsView::addGraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position) {
//	GraphicalModelComponent* item = ((ModelGraphicsScene*) scene())->addGraphicalModelComponent(plugin, component, position);
//	return item;
//}

ModelGraphicsScene* ModelGraphicsView::getScene() {
	return (ModelGraphicsScene*) scene();
}

void ModelGraphicsView::showGrid() {
	((ModelGraphicsScene*) scene())->showGrid();
}

void ModelGraphicsView::clear() {
	scene()->clear();
}

void ModelGraphicsView::beginConnection() {
	((ModelGraphicsScene*) scene())->beginConnection();
}

void ModelGraphicsView::selectModelComponent(ModelComponent* component) {
	QList<QGraphicsItem*>* list = ((ModelGraphicsScene*) scene())->getGraphicalModelComponents();
	for (QGraphicsItem* item : *list) {
		GraphicalModelComponent* gmc = (GraphicalModelComponent*) item;
		if (gmc->getComponent() == component) {
			gmc->setSelected(true);
		} else {
			gmc->setSelected(false);
		}
	}
}

void ModelGraphicsView::setSimulator(Simulator* simulator) {
	_simulator = simulator;
	((ModelGraphicsScene*) scene())->setSimulator(simulator);
}

void ModelGraphicsView::setPropertyEditor(PropertyEditorGenesys* propEditor) {
	_propertyEditor = propEditor;
	((ModelGraphicsScene*) scene())->setPropertyEditor(propEditor);
}

void ModelGraphicsView::setPropertyList(std::map<SimulationControl*, DataComponentProperty*>* propList) {
    _propertyList = propList;
    ((ModelGraphicsScene*) scene())->setPropertyList(propList);
}

void ModelGraphicsView::setPropertyEditorUI(std::map<SimulationControl*, DataComponentEditor*>* propEditorUI) {
    _propertyEditorUI = propEditorUI;
    ((ModelGraphicsScene*) scene())->setPropertyEditorUI(propEditorUI);
}

void ModelGraphicsView::setComboBox(std::map<SimulationControl*, ComboBoxEnum*>* propBox) {
    _propertyBox = propBox;
    ((ModelGraphicsScene*) scene())->setComboBox(propBox);
}

void ModelGraphicsView::setEnabled(bool enabled) {
	QGraphicsView::setEnabled(enabled);
	QBrush background;
	if (enabled) {
		// background
		//unsigned int colorVal1 = 255 * 13.0 / 16.0;
		//unsigned int colorVal2 = 255 * 15.0 / 16.0;
		background = QColor(UtilGUI::rgbaFromPacked(TraitsGUI<GView>::backgroundEnabledColor));//255, 255, 128, 64);
		//getScene()->showGrid();
	} else {
		// background
		background = UtilGUI::rgbaFromPacked(TraitsGUI<GView>::backgroundDisabledColor);//Qt::lightGray;
	}
	background.setStyle(Qt::SolidPattern);
	setBackgroundBrush(background);

}
//---------------------------------------------------------

void ModelGraphicsView::notifySceneMouseEventHandler(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->_sceneMouseEventHandler) {
		this->_sceneMouseEventHandler(mouseEvent);
	}
}

void ModelGraphicsView::notifySceneWheelInEventHandler() {
    if (this->_sceneWheelInEventHandler) {
        this->_sceneWheelInEventHandler();
    }
}

void ModelGraphicsView::notifySceneWheelOutEventHandler() {
    if (this->_sceneWheelOutEventHandler) {
        this->_sceneWheelOutEventHandler();
    }
}

/**
 * @brief Dispatches graphical model change event to registered callback.
 * @param modelGraphicsEvent Event payload from scene.
 *
 * @todo Replace callback with signal/slot to improve composability.
 */
void ModelGraphicsView::notifySceneGraphicalModelEventHandler(const GraphicalModelEvent& modelGraphicsEvent) {
	if (_notifyGraphicalModelEventHandlers && this->_sceneGraphicalModelEventHandler) {
        this->_sceneGraphicalModelEventHandler(modelGraphicsEvent);
    }
    /// @todo actualize property editor?
}

void ModelGraphicsView::setCanNotifyGraphicalModelEventHandlers(bool can) {
	_notifyGraphicalModelEventHandlers = can;
}

void ModelGraphicsView::clearEventHandlers() {
    _sceneMouseEventHandler = nullptr;
    _sceneWheelInEventHandler = nullptr;
    _sceneWheelOutEventHandler = nullptr;
    _sceneGraphicalModelEventHandler = nullptr;
}

//---------------------------------------------------------

void ModelGraphicsView::contextMenuEvent(QContextMenuEvent *event) {
	QGraphicsView::contextMenuEvent(event);
}

void ModelGraphicsView::dragEnterEvent(QDragEnterEvent *event) {
	QGraphicsView::dragEnterEvent(event);
	QString name = event->source()->objectName();
	//std::cout << "View source name: " << name.toStdString()<< std::endl;
	QWidget* widget = dynamic_cast<QWidget*> (event->source());
	if (widget != nullptr) {
		QTreeWidget* tree = dynamic_cast<QTreeWidget*> (widget);
		if (tree != nullptr) {
			if (tree->selectedItems().size() == 1) {
				QTreeWidgetItem* treeItem = tree->selectedItems().at(0);
				QString name = treeItem->whatsThis(0);
				//std::cout << "Drop name: " << name.toStdString() << std::endl;
				Plugin* plugin = _simulator->getPluginManager()->find(name.toStdString());
				if (plugin != nullptr) {
					event->setDropAction(Qt::CopyAction);
					event->accept();
					((ModelGraphicsScene*) scene())->setObjectBeingDragged(treeItem);
					return;
				}
			}
		}
	}
	event->setAccepted(false);
}

void ModelGraphicsView::keyPressEvent(QKeyEvent *event) {
	QGraphicsView::keyPressEvent(event);
}
void ModelGraphicsView::keyReleaseEvent(QKeyEvent *event) {
	QGraphicsView::keyReleaseEvent(event);
}



void ModelGraphicsView::wheelEvent(QWheelEvent *event) {
	QGraphicsView::wheelEvent(event);
	//event->
}

void ModelGraphicsView::setParentWidget(QWidget *parentWidget) {
	_parentWidget = parentWidget;
	((ModelGraphicsScene*) scene())->setParentWidget(parentWidget);
}

// Stores ruler visibility and refreshes the viewport overlays.
void ModelGraphicsView::setRuleVisible(bool visible) {
    _ruleVisible = visible;
    viewport()->update();
}

// Returns current ruler visibility used by the view menu state.
bool ModelGraphicsView::isRuleVisible() const {
    return _ruleVisible;
}

// Stores guide visibility and refreshes the viewport overlays.
void ModelGraphicsView::setGuidesVisible(bool visible) {
    _guidesVisible = visible;
    viewport()->update();
}

// Returns current guide visibility used by the view menu state.
bool ModelGraphicsView::isGuidesVisible() const {
    return _guidesVisible;
}

//------------------------------------------------------

void ModelGraphicsView::changed(const QList<QRectF> &region) {
	int i = 0;
}

void ModelGraphicsView::focusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason) {
	int i = 0;
}

void ModelGraphicsView::sceneRectChanged(const QRectF &rect) {
	int i = 0;
}

void ModelGraphicsView::selectionChanged() {
	int i = 0;
}

//------------------------------------------------------

// Draws lightweight rulers and center guides on top of scene content when enabled.
void ModelGraphicsView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect);
    const QRectF visibleRect = mapToScene(viewport()->rect()).boundingRect();
    if (!visibleRect.isValid()) {
        return;
    }

    if (_ruleVisible) {
        painter->save();
        QPen rulerPen(QColor(90, 90, 90, 180));
        painter->setPen(rulerPen);
        const qreal tickStep = 100.0;
        const qreal majorTick = 12.0;
        const qreal minorTick = 6.0;
        painter->drawLine(QPointF(visibleRect.left(), visibleRect.top()), QPointF(visibleRect.right(), visibleRect.top()));
        painter->drawLine(QPointF(visibleRect.left(), visibleRect.top()), QPointF(visibleRect.left(), visibleRect.bottom()));
        for (qreal x = std::floor(visibleRect.left() / tickStep) * tickStep; x <= visibleRect.right(); x += tickStep) {
            painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.top() + majorTick));
            painter->drawText(QPointF(x + 2.0, visibleRect.top() + 24.0), QString::number(static_cast<int>(x)));
        }
        for (qreal y = std::floor(visibleRect.top() / tickStep) * tickStep; y <= visibleRect.bottom(); y += tickStep) {
            painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.left() + minorTick, y));
            painter->drawText(QPointF(visibleRect.left() + 8.0, y - 2.0), QString::number(static_cast<int>(y)));
        }
        painter->restore();
    }

    if (_guidesVisible) {
        painter->save();
        QPen guidesPen(QColor(0, 120, 215, 150));
        guidesPen.setStyle(Qt::DashLine);
        painter->setPen(guidesPen);
        const QPointF center = visibleRect.center();
        painter->drawLine(QPointF(visibleRect.left(), center.y()), QPointF(visibleRect.right(), center.y()));
        painter->drawLine(QPointF(center.x(), visibleRect.top()), QPointF(center.x(), visibleRect.bottom()));
        painter->restore();
    }
}

//------------------------------------------------------

QList<QGraphicsItem *> ModelGraphicsView::selectedItems() {
	return ((ModelGraphicsScene*) scene())->selectedItems();
}

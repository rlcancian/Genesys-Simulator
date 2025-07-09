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
#include "TraitsGUI.h"
#include <Qt>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <fstream>
#include <streambuf>

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

QColor ModelGraphicsView::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
}

void ModelGraphicsView::setEnabled(bool enabled) {
	QGraphicsView::setEnabled(enabled);
	QBrush background;
	if (enabled) {
		// background
		//unsigned int colorVal1 = 255 * 13.0 / 16.0;
		//unsigned int colorVal2 = 255 * 15.0 / 16.0;
		background = QColor(myrgba(TraitsGUI<GView>::backgroundEnabledColor));//255, 255, 128, 64);
		//getScene()->showGrid();
	} else {
		// background
		background = myrgba(TraitsGUI<GView>::backgroundDisabledColor);//Qt::lightGray;
	}
	background.setStyle(Qt::SolidPattern);
	setBackgroundBrush(background);

}
//---------------------------------------------------------

void ModelGraphicsView::notifySceneMouseEventHandler(QGraphicsSceneMouseEvent* mouseEvent) {
	this->_sceneMouseEventHandler(mouseEvent);
}

void ModelGraphicsView::notifySceneWheelInEventHandler() {
    this->_sceneWheelInEventHandler();
}

void ModelGraphicsView::notifySceneWheelOutEventHandler() {
    this->_sceneWheelOutEventHandler();
}

void ModelGraphicsView::notifySceneGraphicalModelEventHandler(GraphicalModelEvent* modelGraphicsEvent) {
	if (_notifyGraphicalModelEventHandlers)
		this->_sceneGraphicalModelEventHandler(modelGraphicsEvent);
    /// @todo actualize property editor?
}

void ModelGraphicsView::setCanNotifyGraphicalModelEventHandlers(bool can) {
	_notifyGraphicalModelEventHandlers = can;
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

QList<QGraphicsItem *> ModelGraphicsView::selectedItems() {
	return ((ModelGraphicsScene*) scene())->selectedItems();
}


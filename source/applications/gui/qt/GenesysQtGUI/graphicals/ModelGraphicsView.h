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
 * File:   QModelGraphicView.h
 * Author: rlcancian
 *
 * Created on 15 de fevereiro de 2022, 21:12
 */

#ifndef QMODELGRAPHICVIEW_H
#define QMODELGRAPHICVIEW_H

#include <QGraphicsView>
#include <Qt>
#include <QColor>
#include <QStyle>
#include <QGraphicsSceneMouseEvent>
#include "graphicals/ModelGraphicsScene.h"
#include "propertyeditor/DataComponentProperty.h"
#include "propertyeditor/DataComponentEditor.h"
#include "propertyeditor/ComboBoxEnum.h"
#include "../../../../kernel/simulator/ModelComponent.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"
//#include "../../../../kernel/simulator/Plugin.h"

class ModelGraphicsView : public QGraphicsView {
public:
    ModelGraphicsView(QWidget *parent = nullptr);
    ModelGraphicsView(const ModelGraphicsView& orig);
    virtual ~ModelGraphicsView();
public: // editing graphic model
    // TODO: AddGraphicalModelComponent should be only on scene
    //GraphicalModelComponent* addGraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position);
    //bool removeGraphicalModelComponent(GraphicalModelComponent* gmc);
    //bool addGraphicalConnection(GraphicalComponentPort* sourcePort, GraphicalComponentPort* destinationPort);
    //bool removeGraphicalConnection(GraphicalConnection* gc);
    //bool addDrawing();
    //bool removeDrawing();
    //bool addAnimation();
    //bool removeAnimation();
    ModelGraphicsScene* getScene();
public:
    void showGrid();
    void clear();
    void beginConnection();
    void selectModelComponent(ModelComponent* component);
    void setSimulator(Simulator* simulator);
    void setPropertyEditor(PropertyEditorGenesys* propEditor);
    void setPropertyList(std::map<SimulationControl*, DataComponentProperty*>* propList);
    void setPropertyEditorUI(std::map<SimulationControl*, DataComponentEditor*>* propEditorUI);
    void setComboBox(std::map<SimulationControl*, ComboBoxEnum*>* propBox);
    void setEnabled(bool enabled);
    QList<QGraphicsItem*> selectedItems();
public: // events and notifications

    template<typename Class> void setSceneMouseEventHandler(Class * object, void (Class::*function)(QGraphicsSceneMouseEvent*)) {
		sceneMouseEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
		this->_sceneMouseEventHandler = handlerMethod;
	}

    template<typename Class> void setGraphicalModelEventHandler(Class * object, void (Class::*function)(GraphicalModelEvent*)) {
		sceneGraphicalModelEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
		this->_sceneGraphicalModelEventHandler = handlerMethod;
    }

    template <typename Class> void setSceneWheelInEventHandler(Class *object, void (Class::*function)()) {
        sceneWheelEventHandlerMethod handlerMethod = std::bind(function, object);
        this->_sceneWheelInEventHandler = handlerMethod;
    }

    template <typename Class> void setSceneWheelOutEventHandler(Class *object, void (Class::*function)()) {
        sceneWheelEventHandlerMethod handlerMethod = std::bind(function, object);
        this->_sceneWheelOutEventHandler = handlerMethod;
    }

    void notifySceneMouseEventHandler(QGraphicsSceneMouseEvent* mouseEvent);
    void notifySceneWheelInEventHandler();
    void notifySceneWheelOutEventHandler();
    void notifySceneGraphicalModelEventHandler(GraphicalModelEvent* modelGraphicsEvent);
    void setCanNotifyGraphicalModelEventHandlers(bool can);
    void setParentWidget(QWidget *parentWidget);
protected:// slots:
    void changed(const QList<QRectF> &region);
    void focusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason);
    void sceneRectChanged(const QRectF &rect);
    void selectionChanged();
protected: // virtual functions
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    //virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    //virtual void dragMoveEvent(QDragMoveEvent *event) override;
    //virtual void dropEvent(QDropEvent *event) override;
    //virtual bool event(QEvent *event) override;
    //virtual void focusInEvent(QFocusEvent *event) override;
    //virtual bool focusNextPrevChild(bool next) override;
    //virtual void focusOutEvent(QFocusEvent *event) override;
    //virtual void inputMethodEvent(QInputMethodEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    //virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    //virtual void mouseMoveEvent(QMouseEvent *event) override;
    //virtual void mousePressEvent(QMouseEvent *event) override;
    //virtual void mouseReleaseEvent(QMouseEvent *event) override;
    //virtual void paintEvent(QPaintEvent *event) override;
    //virtual void resizeEvent(QResizeEvent *event) override;
    //virtual void scrollContentsBy(int dx, int dy) override;
    //virtual void showEvent(QShowEvent *event) override;
    //virtual bool viewportEvent(QEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
private:
    QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
private:
	typedef std::function<void(QGraphicsSceneMouseEvent*) > sceneMouseEventHandlerMethod;
    typedef std::function<void()> sceneWheelEventHandlerMethod;
    typedef std::function<void(GraphicalModelEvent*) > sceneGraphicalModelEventHandlerMethod;
    sceneMouseEventHandlerMethod _sceneMouseEventHandler;
    sceneWheelEventHandlerMethod _sceneWheelInEventHandler;
    sceneWheelEventHandlerMethod _sceneWheelOutEventHandler;
    sceneGraphicalModelEventHandlerMethod _sceneGraphicalModelEventHandler;
    Simulator* _simulator = nullptr;
    PropertyEditorGenesys* _propertyEditor = nullptr;
    std::map<SimulationControl*, DataComponentProperty*>* _propertyList = nullptr;
    std::map<SimulationControl*, DataComponentEditor*>* _propertyEditorUI = nullptr;
    std::map<SimulationControl*, ComboBoxEnum*>* _propertyBox = nullptr;
    QWidget* _parentWidget;
    bool _notifyGraphicalModelEventHandlers = true;
};

#endif /* QMODELGRAPHICVIEW_H */


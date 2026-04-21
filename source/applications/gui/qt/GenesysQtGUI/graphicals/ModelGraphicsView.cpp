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
#include "systempreferences.h"
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
    centerOn(TraitsGUI<GView>::sceneCenter, TraitsGUI<GView>::sceneCenter);
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
        background = QColor(UtilGUI::rgbaFromPacked(SystemPreferences::diagramUsesThemeColors()
                                                        ? SystemPreferences::canvasBackgroundColor()
                                                        : TraitsGUI<GView>::backgroundEnabledColor));//255, 255, 128, 64);
		//getScene()->showGrid();
	} else {
		// background
        background = UtilGUI::rgbaFromPacked(SystemPreferences::diagramUsesThemeColors()
                                                 ? SystemPreferences::canvasDisabledBackgroundColor()
                                                 : TraitsGUI<GView>::backgroundDisabledColor);//Qt::lightGray;
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
    _contextMenuEventHandler = nullptr;
}

//---------------------------------------------------------

void ModelGraphicsView::contextMenuEvent(QContextMenuEvent *event) {
    if (_contextMenuEventHandler) {
        // Let the MainWindow-owned controller decide which action menu matches the clicked scene context.
        _contextMenuEventHandler(event);
        if (event->isAccepted()) {
            return;
        }
    }
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
        painter->resetTransform();

        QPen rulerPen(QColor(90, 90, 90, 180));
        painter->setPen(rulerPen);
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        // Passo base da régua em coordenadas de cena.
        // Ele continua sendo 100, mas os passos efetivos horizontal e vertical
        // podem ser aumentados dinamicamente para evitar sobreposição de textos.
        const qreal tickStep = 100.0;
        const qreal majorTick = 12.0;
        const qreal minorTick = 6.0;

        const QRect viewportRect = viewport()->rect();
        const QPointF topLeftScene = mapToScene(viewportRect.topLeft());
        const QPointF topRightScene = mapToScene(viewportRect.topRight());
        const QPointF bottomLeftScene = mapToScene(viewportRect.bottomLeft());

        // Usa a mesma fonte ligeiramente reduzida nas duas réguas.
        // A redução é local a este trecho e a fonte original é restaurada ao final.
        const QFont originalFont = painter->font();
        QFont rulerTextFont = originalFont;
        if (rulerTextFont.pointSizeF() > 0.0) {
            rulerTextFont.setPointSizeF(std::max<qreal>(1.0, rulerTextFont.pointSizeF() - 1.0));
        } else if (rulerTextFont.pixelSize() > 0) {
            rulerTextFont.setPixelSize(std::max(1, rulerTextFont.pixelSize() - 1));
        }
        painter->setFont(rulerTextFont);

        const QFontMetricsF rulerTextMetrics(rulerTextFont);

        // --- Ajuste dinâmico da régua horizontal ---
        // Aqui o problema principal é a largura do texto, então o passo horizontal
        // é aumentado para um múltiplo de 100 suficiente para evitar sobreposição.
        const QString leftXText = QString::number(static_cast<int>(visibleRect.left()));
        const QString rightXText = QString::number(static_cast<int>(visibleRect.right()));
        const QString zeroXText = QStringLiteral("0");
        const qreal horizontalTextWidth = std::max({
            rulerTextMetrics.horizontalAdvance(leftXText),
            rulerTextMetrics.horizontalAdvance(rightXText),
            rulerTextMetrics.horizontalAdvance(zeroXText)
        });
        const qreal minimumHorizontalLabelSpacingPx = horizontalTextWidth + 12.0;

        const qreal visibleSceneWidth = std::abs(topRightScene.x() - topLeftScene.x());
        const qreal baseTickPixelsX = (visibleSceneWidth > 0.0)
                ? (tickStep * static_cast<qreal>(viewportRect.width()) / visibleSceneWidth)
                : 0.0;

        const qreal tickStepX = (baseTickPixelsX > 0.0)
                ? (tickStep * std::max<qreal>(1.0, std::ceil(minimumHorizontalLabelSpacingPx / baseTickPixelsX)))
                : tickStep;

        // --- Ajuste dinâmico da régua vertical ---
        // Na vertical, o que importa é principalmente a altura do texto, não a largura.
        // Isso evita aumentar demais o espaçamento entre rótulos do eixo Y.
        const qreal verticalTextHeight = rulerTextMetrics.height();
        const qreal minimumVerticalLabelSpacingPx = verticalTextHeight + 4.0;

        const qreal visibleSceneHeight = std::abs(bottomLeftScene.y() - topLeftScene.y());
        const qreal baseTickPixelsY = (visibleSceneHeight > 0.0)
                ? (tickStep * static_cast<qreal>(viewportRect.height()) / visibleSceneHeight)
                : 0.0;

        const qreal tickStepY = (baseTickPixelsY > 0.0)
                ? (tickStep * std::max<qreal>(1.0, std::ceil(minimumVerticalLabelSpacingPx / baseTickPixelsY)))
                : tickStep;

        painter->drawLine(QPointF(viewportRect.left(), viewportRect.top()),
                          QPointF(viewportRect.right(), viewportRect.top()));
        painter->drawLine(QPointF(viewportRect.left(), viewportRect.top()),
                          QPointF(viewportRect.left(), viewportRect.bottom()));

        for (qreal x = std::floor(visibleRect.left() / tickStepX) * tickStepX; x <= visibleRect.right(); x += tickStepX) {
            const double t = (topRightScene.x() == topLeftScene.x()) ? 0.0 : (x - topLeftScene.x()) / (topRightScene.x() - topLeftScene.x());
            const qreal px = viewportRect.left() + t * viewportRect.width();
            painter->drawLine(QPointF(px, viewportRect.top()), QPointF(px, viewportRect.top() + majorTick));
            painter->drawText(QPointF(px + 2.0, viewportRect.top() + 24.0), QString::number(static_cast<int>(x)));
        }

        for (qreal y = std::floor(visibleRect.top() / tickStepY) * tickStepY; y <= visibleRect.bottom(); y += tickStepY) {
            const double t = (bottomLeftScene.y() == topLeftScene.y()) ? 0.0 : (y - topLeftScene.y()) / (bottomLeftScene.y() - topLeftScene.y());
            const qreal py = viewportRect.top() + t * viewportRect.height();
            painter->drawLine(QPointF(viewportRect.left(), py), QPointF(viewportRect.left() + minorTick, py));
            painter->drawText(QPointF(viewportRect.left() + 8.0, py - 2.0), QString::number(static_cast<int>(y)));
        }

        // Restaura a fonte original para não afetar outros desenhos.
        painter->setFont(originalFont);
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

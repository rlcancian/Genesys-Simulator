#include "mainwindow.h"
#include "ui_mainwindow.h"

//-----------------------------------------

/**
 * @brief Atualiza label de coordenadas com posição atual do mouse na cena.
 * @param mouseEvent Evento de mouse recebido da cena gráfica.
 */
void MainWindow::_onSceneMouseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
    QPointF pos = mouseEvent->scenePos();
    ui->labelMousePos->setText(QString::fromStdString("<" + std::to_string((int) pos.x()) + "," + std::to_string((int) pos.y()) + ">"));
}

/**
 * @brief Aumenta zoom da visualização gráfica via slider principal.
 */
void MainWindow::_onSceneWheelInEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value + TraitsGUI<GMainWindow>::zoomButtonChange);
}

/**
 * @brief Reduz zoom da visualização gráfica via slider principal.
 */
void MainWindow::_onSceneWheelOutEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value - TraitsGUI<GMainWindow>::zoomButtonChange);
}

/**
 * @brief Atualiza painéis dependentes quando o modelo gráfico muda.
 * @param event Evento de alteração gráfica (não utilizado diretamente neste handler).
 *
 * @todo Evoluir para atualização incremental por tipo de evento para reduzir custo.
 */
void MainWindow::_onSceneGraphicalModelEvent(const GraphicalModelEvent& /*event*/) {
    _actualizeTabPanes();
}

//-----------------------------------------

/**
 * @brief Slot de notificação de alteração da cena (redo/undo e ações de edição).
 * @param region Regiões invalidadas da cena.
 */
void MainWindow::sceneChanged(const QList<QRectF> &region) {
    /**
     * Bloco 1: sincroniza estado de undo/redo e flags de alteração textual.
     */
    bool canUndo = ui->graphicsView->getScene()->getUndoStack()->canUndo();
    bool canRedo = ui->graphicsView->getScene()->getUndoStack()->canRedo();

    ui->actionEditUndo->setEnabled(canUndo);
    ui->actionEditRedo->setEnabled(canRedo);

    _textModelHasChanged = canUndo;

    ui->graphicsView->scene()->update();

    /**
     * Bloco 2: habilita/desabilita ações de edição conforme itens/copias disponíveis.
     */
    bool res = _checkItemsScene();

    if (res) {
        ui->actionEditCut->setEnabled(true);
        ui->actionEditCopy->setEnabled(true);
    } else {
        ui->actionEditCut->setEnabled(false);
        ui->actionEditCopy->setEnabled(false);
    }

    if (!_draw_copy->empty() || !_gmc_copies->empty() || !_group_copy->empty() || !_ports_copies->empty()) {
        ui->actionEditPaste->setEnabled(true);
    } else {
        ui->actionEditPaste->setEnabled(false);
    }

    /**
     * Bloco 3: atualiza estado visual final da cena.
     */
    if (ui->graphicsView->getScene()->connectingStep() == 0)
        ui->actionGModelShowConnect->setChecked(false);

    ui->graphicsView->scene()->update();
}

/**
 * @brief Verifica se há itens relevantes na cena para habilitar operações de edição.
 * @return true se houver componentes/desenhos/animações no modelo gráfico.
 */
bool MainWindow::_checkItemsScene() {
    bool res = false;

    QList<QGraphicsItem *> *components = myScene()->getGraphicalModelComponents();
    QList<QGraphicsItem *> *geometries = myScene()->getGraphicalGeometries();
    QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();
    QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();
    QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();

    if (!components->empty() || !geometries->empty() || !counters->empty() || !variables->empty() || !timers->empty()) {
        res = true;
    }

    return res;
}
void MainWindow::sceneFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason) {
    // int a = 0;
}
//void sceneRectChanged(const QRectF &rect){}

/**
 * @brief Slot quando seleção de itens da cena muda.
 *
 * Atualiza o Property Editor para um único componente selecionado e limpa em caso contrário.
 */
void MainWindow::sceneSelectionChanged() {
    QGraphicsItem * item;
    GraphicalModelComponent* gmc;

    if (!ui->graphicsView->selectedItems().isEmpty()) {
        if (ui->graphicsView->selectedItems().size() == 1) {
            item = ui->graphicsView->selectedItems().at(0);
            gmc = dynamic_cast<GraphicalModelComponent*> (item);
            if (gmc != nullptr) {
                ui->treeViewPropertyEditor->setActiveObject(gmc, gmc->getComponent(), propertyGenesys, propertyList, propertyEditorUI, propertyBox);
                return;
            }
        }
    }
    // Se nenhum item estiver selecionado ou se mais de um item estiver selecionado
    ui->treeViewPropertyEditor->clear();
}

//-----------------------------------------

void MainWindow::sceneGraphicalModelChanged() {
    _actualizeTabPanes();
}

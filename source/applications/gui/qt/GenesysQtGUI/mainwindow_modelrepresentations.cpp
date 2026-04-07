#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "services/ModelLanguageSynchronizer.h"
#include "services/GraphvizModelExporter.h"
#include "services/CppModelExporter.h"

// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
//#include <streambuf>

// QT
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
// #include <qt5/QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsitem.h>
#include <QGraphicsScene>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QTimer>
#include <QUrl>

static QString _encodeGuiText(const QString& text) {
    return QString::fromUtf8(QUrl::toPercentEncoding(text));
}

static QString _decodeGuiText(const QString& text) {
    return QUrl::fromPercentEncoding(text.toUtf8());
}

void MainWindow::_actualizeModelSimLanguage() {
    // This wrapper delegates model-language synchronization to a dedicated phase-1 service.
    _modelLanguageSynchronizer->actualizeModelSimLanguage();
}

void MainWindow::_clearModelEditors() {
    ui->TextCodeEditor->clear();
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();
    ui->graphicsView->clear();
    ui->plainTextEditCppCode->clear();
    ui->treeWidgetComponents->clear();
    ui->treeWidgetDataDefnitions->clear();
}

bool MainWindow::_setSimulationModelBasedOnText() {
    // This wrapper delegates text-to-model synchronization while keeping MainWindow as temporary API surface.
    return _modelLanguageSynchronizer->setSimulationModelBasedOnText();
}

std::string MainWindow::_adjustDotName(std::string name) {
    // This wrapper delegates DOT-name normalization to the phase-1 Graphviz service.
    return _graphvizModelExporter->adjustDotName(std::move(name));
}

void MainWindow::_insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode) {
    // This wrapper delegates ranked DOT insertion to the phase-1 Graphviz service.
    _graphvizModelExporter->insertTextInDot(std::move(text), compLevel, compRank, dotmap, isNode);
}

void MainWindow::_recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) {
    // This wrapper delegates recursive DOT generation to the phase-1 Graphviz service.
    _graphvizModelExporter->recursiveCreateModelGraphicPicture(componentOrData, visited, dotmap);
}

std::string MainWindow::_addCppCodeLine(std::string line, unsigned int indent) {
    // This wrapper delegates C++ line formatting to the phase-1 exporter service.
    return _cppModelExporter->addCppCodeLine(line, indent);
}

void MainWindow::_actualizeModelCppCode() {
    // This wrapper delegates full C++ code export rendering to the phase-1 exporter service.
    _cppModelExporter->actualizeModelCppCode();
}

bool MainWindow::graphicalModelHasChanged() const {
    return _graphicalModelHasChanged;
}

void MainWindow::setGraphicalModelHasChanged(bool graphicalModelHasChanged) {
    _graphicalModelHasChanged = graphicalModelHasChanged;
    _actualizeTabPanes();
}

bool MainWindow::_createModelImage() {
    // This wrapper delegates model diagram image creation to the phase-1 Graphviz service.
    return _graphvizModelExporter->createModelImage([this]() {
        // This callback preserves MainWindow-controlled text-to-model synchronization flow.
        return this->_setSimulationModelBasedOnText();
    });
}

//-----------------------------------------------------------------

bool MainWindow::_saveTextModel(QFile *saveFile, QString data)
{
    QTextStream out(saveFile);

    try
    {
        static const QRegularExpression regex("[\n]");
        QStringList strList = data.split(regex);
        for (const QString &line : strList)
        {
            out << line << Qt::endl;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool MainWindow::_saveGraphicalModel(QString filename)
{
    QFile saveFile(filename);

    try
    {
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::information(this, tr("Unable to access file to save"),
                                     saveFile.errorString());
            return false;
        }

        _saveTextModel(&saveFile, ui->TextCodeEditor->toPlainText());

        QTextStream out(&saveFile);
        out << "#Genegys Graphic Model" << Qt::endl;
        QString line = "0\tView\t";
        line += "zoom=" + QString::number(ui->horizontalSlider_ZoomGraphical->value());
        line += ", grid=" + QString::number(ui->actionShowGrid->isChecked());
        line += ", rule=" + QString::number(ui->actionShowRule->isChecked());
        line += ", snap=" + QString::number(ui->actionShowSnap->isChecked());
        line += ", guides=" + QString::number(ui->actionShowGuides->isChecked());
        line += ", internals=" + QString::number(ui->actionShowInternalElements->isChecked());
        line += ", attached=" + QString::number(ui->actionShowAttachedElements->isChecked());
        line += ", diagrams=" + QString::number(ui->actionDiagrams->isChecked());
        line += ", viewpoint=(" + QString::number(ui->graphicsView->horizontalScrollBar()->value()) + "," + QString::number(ui->graphicsView->verticalScrollBar()->value()) + ")";
        out << line << Qt::endl;

        ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->getScene());

        if (scene)
        {
            QHash<QGraphicsItem*, int> persistedItemIds;
            int nextPersistedItemId = 1;

            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                GraphicalModelComponent *gmc = (GraphicalModelComponent *)item;
                if (gmc)
                {
                    int persistedId = nextPersistedItemId++;
                    persistedItemIds.insert(gmc, persistedId);
                    line = QString::fromStdString(std::to_string(gmc->getComponent()->getId()) + "\t" + gmc->getComponent()->getClassname() + "\t" + gmc->getComponent()->getName() + "\t" + "color=" + gmc->getColor().name().toStdString() + "\t" + "position=(" + std::to_string(gmc->scenePos().x()) + "," + std::to_string(gmc->scenePos().y() + gmc->getHeight()/2) + ")");
                    line += "\titemid=" + QString::number(persistedId);
                    out << line << Qt::endl;
                }
            }

            QList<QGraphicsItem*>* geometries = myScene()->getGraphicalGeometries();
            if (geometries && !geometries->isEmpty()) {
                out << Qt::endl;
                out << "#Geometries" << Qt::endl;

                for (QGraphicsItem* item : *geometries) {
                    if (item == nullptr) {
                        continue;
                    }
                    int persistedId = nextPersistedItemId++;
                    persistedItemIds.insert(item, persistedId);

                    if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
                        const QLineF l = lineItem->line();
                        line = QString("Geometry \t id=%1 \t type=line \t line=(%2,%3,%4,%5) \t pos=(%6,%7)")
                                   .arg(persistedId)
                                   .arg(l.x1(), 0, 'f', 4).arg(l.y1(), 0, 'f', 4)
                                   .arg(l.x2(), 0, 'f', 4).arg(l.y2(), 0, 'f', 4)
                                   .arg(lineItem->pos().x(), 0, 'f', 4).arg(lineItem->pos().y(), 0, 'f', 4);
                        out << line << Qt::endl;
                    } else if (QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
                        QRectF r = rectItem->rect().normalized();
                        line = QString("Geometry \t id=%1 \t type=rect \t rect=(%2,%3,%4,%5) \t pos=(%6,%7)")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(rectItem->pos().x(), 0, 'f', 4).arg(rectItem->pos().y(), 0, 'f', 4);
                        out << line << Qt::endl;
                    } else if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                        QRectF r = ellipseItem->rect().normalized();
                        line = QString("Geometry \t id=%1 \t type=ellipse \t rect=(%2,%3,%4,%5) \t pos=(%6,%7)")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(ellipseItem->pos().x(), 0, 'f', 4).arg(ellipseItem->pos().y(), 0, 'f', 4);
                        out << line << Qt::endl;
                    } else if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
                        QStringList points;
                        const QPolygonF polygon = polygonItem->polygon();
                        for (const QPointF& p : polygon) {
                            points << QString("%1,%2").arg(p.x(), 0, 'f', 4).arg(p.y(), 0, 'f', 4);
                        }
                        line = QString("Geometry \t id=%1 \t type=polygon \t points=%2 \t pos=(%3,%4)")
                                   .arg(persistedId)
                                   .arg(points.join(";"))
                                   .arg(polygonItem->pos().x(), 0, 'f', 4).arg(polygonItem->pos().y(), 0, 'f', 4);
                        out << line << Qt::endl;
                    } else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                        line = QString("Text \t id=%1 \t value=%2 \t pos=(%3,%4)")
                                   .arg(persistedId)
                                   .arg(_encodeGuiText(textItem->toPlainText()))
                                   .arg(textItem->pos().x(), 0, 'f', 4).arg(textItem->pos().y(), 0, 'f', 4);
                        out << line << Qt::endl;
                    }
                }
            }

            QList<QGraphicsItemGroup*>* groups = myScene()->getGraphicalGroups();
            if (groups && !groups->isEmpty()) {
                out << Qt::endl;
                out << "#Groups" << Qt::endl;

                int groupId = 0;
                for (QGraphicsItemGroup* group : *groups) {
                    if (group == nullptr) {
                        continue;
                    }
                    QStringList members;
                    for (QGraphicsItem* child : group->childItems()) {
                        if (persistedItemIds.contains(child)) {
                            members << QString::number(persistedItemIds.value(child));
                        }
                    }
                    if (!members.isEmpty()) {
                        line = QString("Group_%1 \t members=%2").arg(groupId++).arg(members.join(","));
                        out << line << Qt::endl;
                    }
                }
            }

            QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();

            if (counters) {
                if (!counters->empty()) {
                    out << Qt::endl;
                    out << "#Counters" << Qt::endl;
                    int id = 0;
                    for (AnimationCounter *counter : *counters) {
                        int idCounter = -1;
                        if (counter->getCounter() != nullptr) {
                            idCounter = counter->getCounter()->getId();
                        }
                        line = QString("Counter_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idCounter)
                                   .arg(counter->scenePos().x(), 0, 'f', 2)
                                   .arg(counter->scenePos().y(), 0, 'f', 2)
                                   .arg(counter->boundingRect().width(), 0, 'f', 2)
                                   .arg(counter->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();

            if (variables) {
                if (!variables->empty()) {
                    out << Qt::endl;
                    out << "#Variables" << Qt::endl;
                    int id = 0;
                    for (AnimationVariable *variable : *variables) {
                        int idVariable = -1;
                        if (variable->getVariable() != nullptr) {
                            idVariable = variable->getVariable()->getId();
                        }
                        line = QString("Variable_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idVariable)
                                   .arg(variable->scenePos().x(), 0, 'f', 2)
                                   .arg(variable->scenePos().y(), 0, 'f', 2)
                                   .arg(variable->boundingRect().width(), 0, 'f', 2)
                                   .arg(variable->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();

            if (timers) {
                if (!timers->empty()) {
                    out << Qt::endl;
                    out << "#Timers" << Qt::endl;
                    int id = 0;
                    for (AnimationTimer *timer : *timers) {
                        line = QString("Timer_%1 \t hour=%2 \t minute=%3 \t second=%4 \t format=%5 \t position=(%6,%7) \t width=%8 \t height=%9")
                                   .arg(id)
                                   .arg(timer->getInitialHours())
                                   .arg(timer->getInitialMinutes())
                                   .arg(timer->getInitialSeconds())
                                   .arg(static_cast<unsigned int>(timer->getTimeFormat()))
                                   .arg(timer->scenePos().x(), 0, 'f', 2)
                                   .arg(timer->scenePos().y(), 0, 'f', 2)
                                   .arg(timer->boundingRect().width(), 0, 'f', 2)
                                   .arg(timer->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            /*/Lines
            out << "LINE" << Qt::endl;
            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                QGraphicsLineItem *drawLine = dynamic_cast<QGraphicsLineItem *> (item);
                if (drawLine)
                {
                    line = QString::fromStdString(std::to_string(drawLine->))
                }
            }*/
        }

        saveFile.close();
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }

    //QString data = QString::fromStdString(dot);
    //QStringList strList = data.split(QRegExp("[\n]"), QString::SkipEmptyParts);
    //for (unsigned int i = 0; i < strList.size(); i++) {
    //	savefile << strList.at(i).toStdString() << std::endl;
    //}
}

Model *MainWindow::_loadGraphicalModel(std::string filename) {
    QFile file(QString::fromStdString(filename));

    Model *model = nullptr;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to access file to save"),
                                 file.errorString());
        return nullptr;
    }

    QFileInfo fileInfo(file.fileName());
    QString extension = fileInfo.suffix();

    if (extension != "gui") {
        model = simulator->getModelManager()->loadModel(file.fileName().toStdString());
        if (model != nullptr) _generateGraphicalModelFromModel();
        return model;
    }

    QString content = file.readAll();
    file.close();

    QStringList lines = content.split("\n");

    QStringList simulLang;
    QStringList gui;
    QStringList counters;
    QStringList variables;
    QStringList timers;
    QStringList geometries;
    QStringList groups;

    bool guiFlag = false;
    bool counterFlag = false;
    bool variableFlag = false;
    bool timerFlag = false;
    bool geometryFlag = false;
    bool groupFlag = false;

    for (const QString &line : lines) {
        if (line.startsWith("#Genegys Graphic Model")) {
            guiFlag = true;

            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Counters")) {
            counterFlag = true;

            guiFlag = false;
            variableFlag = false;
            timerFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }

        if (line.startsWith("#Variables")) {
            variableFlag = true;

            guiFlag = false;
            counterFlag = false;
            timerFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }

        if (line.startsWith("#Timers")) {
            timerFlag = true;

            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }

        if (line.startsWith("#Geometries")) {
            geometryFlag = true;

            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            groupFlag = false;
            continue;
        }

        if (line.startsWith("#Groups")) {
            groupFlag = true;

            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            geometryFlag = false;
            continue;
        }

        if (!guiFlag && !timerFlag && !counterFlag && !variableFlag && !geometryFlag && !groupFlag) {
            simulLang.append(line);
        } else {
            if (counterFlag) {
                counters.append(line);
            } else if (variableFlag) {
                variables.append(line);
            } else if (timerFlag) {
                timers.append(line);
            } else if (geometryFlag) {
                geometries.append(line);
            } else if (groupFlag) {
                groups.append(line);
            } else {
                gui.append(line);
            }
        }
    }

    file.close();

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString newFilename = QString("/tempFile-%1.gen").arg(currentDateTime.toString("yyyy-MM-dd-hh-mm-ss"));

    QString filePath = QDir::tempPath() + newFilename;
    QFile tempFile(filePath);
    tempFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream outStream(&tempFile);
    for (const QString& line : simulLang) {
        outStream << line << Qt::endl;
    }

    outStream.flush();

    std::string nameTempFile = tempFile.fileName().toStdString();
    model = simulator->getModelManager()->loadModel(nameTempFile);

    tempFile.close();

    QFile::remove(tempFile.fileName());

    if (model != nullptr) {
        std::list<ModelComponent*> c = * model->getComponentManager()->getAllComponents();

        _clearModelEditors();

        bool firstLine = true;
        // Armazena o estado da view para aplicar no final do load, após reconstrução da cena.
        bool hasPersistedViewState = false;
        int restoredViewpointX = 0;
        int restoredViewpointY = 0;
        bool restoredDiagrams = false;

        QHash<int, QGraphicsItem*> persistedItems;

        for (const QString& line : gui) {
            if (line.trimmed().isEmpty()) {
                continue;
            }

            if (firstLine) {
                QRegularExpression regex("(\\d+)\\s*View\\s*(.*)");
                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    const QString attributes = match.captured(2);
                    QRegularExpression regexZoom("zoom=(-?\\d+)");
                    QRegularExpression regexGrid("grid=(\\d+)");
                    QRegularExpression regexRule("rule=(\\d+)");
                    QRegularExpression regexSnap("snap=(\\d+)");
                    QRegularExpression regexGuides("guides=(\\d+)");
                    QRegularExpression regexInternals("internals=(\\d+)");
                    QRegularExpression regexAttached("attached=(\\d+)");
                    QRegularExpression regexDiagrams("diagrams=(\\d+)");
                    QRegularExpression regexViewpoint("viewpoint=\\(([-+]?\\d+\\.?\\d*),([-+]?\\d+\\.?\\d*)\\)");

                    int zoom = regexZoom.match(attributes).captured(1).toInt();
                    int grid = regexGrid.match(attributes).captured(1).toInt();
                    int rule = regexRule.match(attributes).captured(1).toInt();
                    int snap = regexSnap.match(attributes).captured(1).toInt();
                    int guides = regexGuides.match(attributes).captured(1).toInt();
                    int internals = regexInternals.match(attributes).captured(1).toInt();
                    int attached = regexAttached.match(attributes).captured(1).toInt();
                    int diagrams = regexDiagrams.match(attributes).captured(1).toInt();

                    QRegularExpressionMatch viewpointMatch = regexViewpoint.match(attributes);
                    int viewpointX = 0;
                    int viewpointY = 0;
                    if (viewpointMatch.hasMatch()) {
                        viewpointX = viewpointMatch.captured(1).toInt();
                        viewpointY = viewpointMatch.captured(2).toInt();
                    }

                    // Restaura o estado visual do grid sem usar toggle implícito.
                    ui->actionShowGrid->setChecked(grid != 0);
                    myScene()->setGridVisible(grid != 0);
                    // Restaura o snap de forma determinística e sincronizada com a action.
                    ui->actionShowSnap->setChecked(snap != 0);
                    myScene()->setSnapToGrid(snap != 0);
                    // Restores overlay action states and immediately applies them to the graphics view.
                    ui->actionShowRule->setChecked(rule != 0);
                    ui->actionShowGuides->setChecked(guides != 0);
                    ui->graphicsView->setRuleVisible(rule != 0);
                    ui->graphicsView->setGuidesVisible(guides != 0);
                    // Reaplica internals/attached acionando o mesmo fluxo usado pela interface.
                    ui->actionShowInternalElements->setChecked(internals != 0);
                    on_actionShowInternalElements_triggered();
                    ui->actionShowAttachedElements->setChecked(attached != 0);
                    on_actionShowAttachedElements_triggered();
                    // Armazena a flag de diagramas para aplicar após reconstrução dos itens.
                    restoredDiagrams = (diagrams != 0);
                    ui->actionDiagrams->setChecked(restoredDiagrams);

                    if (zoom > 0) {
                        ui->horizontalSlider_ZoomGraphical->setValue(zoom + TraitsGUI<GMainWindow>::zoomButtonChange);
                    }
                    // Adia a restauração do viewpoint para o final do load.
                    hasPersistedViewState = true;
                    restoredViewpointX = viewpointX;
                    restoredViewpointY = viewpointY;
                }
                firstLine = false;
                continue;
            }

            QStringList split = line.split("\t");
            if (split.size() < 5) {
                continue;
            }

            Util::identification id = split[0].toULong();

            // Component
            QString comp = split[1];

            // Color
            QString col = split[3];

            // Posição
            QString pos = split[4];

            // Expressao regular para pegar a cor
            QRegularExpression regexColor("color=#([0-9A-Fa-f]{6})");

            // Cria a expressao regular match
            QRegularExpressionMatch match = regexColor.match(col);

            QString hexColor;

            // Extrai a cor
            if (match.hasMatch()) {hexColor = match.captured(1);}

            // Cria a cor
            QColor color("#"+ hexColor);

            // Expressao regular para pegar a cor
            QRegularExpression regexPos("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)");
            QRegularExpression regexPosLegacy("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)");

            // Cria a expressao regular match
            match = regexPos.match(pos);

            QPoint position;

            // Extrai a posição
            if (match.hasMatch()) {
                // Extrai x e y
                qreal x = match.captured(1).toDouble();
                qreal y = match.captured(2).toDouble();

                // Seta x e y em pos
                position.setX(x);
                position.setY(y);
            } else {
                QRegularExpressionMatch legacyPosMatch = regexPosLegacy.match(pos);
                if (legacyPosMatch.hasMatch()) {
                    position.setX(legacyPosMatch.captured(1).toDouble());
                    position.setY(legacyPosMatch.captured(3).toDouble());
                }
            }

            // Pega a cena para adicionar o componente nela
            ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->scene());

            // Pega o Plugin
            Plugin* plugin = simulator->getPluginManager()->find(comp.toStdString());
            if (plugin == nullptr) {
                ui->textEdit_Console->append(QString("Warning: Could not find plugin \"%1\" while loading GUI item id=%2. Item ignored.")
                                             .arg(comp).arg(id));
                continue;
            }

            // Cria o componente no modelo
            ModelComponent* component = simulator->getModelManager()->current()->getComponentManager()->find(id);

            if (!component) continue;
            // Desenha na tela
            GraphicalModelComponent* loadedComponent = scene->addGraphicalModelComponent(plugin, component, position, color);
            if (loadedComponent != nullptr) {
                if (split.size() >= 6) {
                    QRegularExpression itemIdRegex("itemid=(\\d+)");
                    QRegularExpressionMatch itemIdMatch = itemIdRegex.match(split[5]);
                    if (itemIdMatch.hasMatch()) {
                        persistedItems.insert(itemIdMatch.captured(1).toInt(), loadedComponent);
                    }
                }
            }
        }

        QList<QGraphicsItem*> *graphicalComponents = ui->graphicsView->getScene()->getGraphicalModelComponents();

        for (unsigned int i = 0; i < (unsigned int) graphicalComponents->size(); i++) {
            GraphicalModelComponent *source = dynamic_cast<GraphicalModelComponent *> (graphicalComponents->at(i));
            if (source == nullptr || source->getComponent() == nullptr) {
                continue;
            }
            std::map<unsigned int, Connection*> *connections = source->getComponent()->getConnectionManager()->connections();
            if (connections == nullptr) {
                continue;
            }

            for (auto it = connections->begin(); it != connections->end(); ++it) {
                unsigned int portSource = it->first;
                Connection* connection = it->second;
                if (connection == nullptr || connection->component == nullptr) {
                    continue;
                }

                GraphicalModelComponent* destination = ui->graphicsView->getScene()->findGraphicalModelComponent(connection->component->getId());
                if (destination == nullptr || destination->getGraphicalInputPorts().empty()) {
                    continue;
                }
                unsigned int portDestination = destination->getGraphicalInputPorts().at(0)->portNum();
                if (portSource >= source->getGraphicalOutputPorts().size()) {
                    continue;
                }
                if (portDestination >= destination->getGraphicalInputPorts().size()) {
                    continue;
                }

                source->setOcupiedOutputPorts(source->getOcupiedOutputPorts() + 1);
                destination->setOcupiedInputPorts(destination->getOcupiedInputPorts() + 1);

                std::string nameSource = source->getComponent()->getName();
                GraphicalComponentPort* sourceport = source->getGraphicalOutputPorts().at(portSource);

                std::string nameDestination = destination->getComponent()->getName();
                GraphicalComponentPort* destport = destination->getGraphicalInputPorts().at(portDestination);

                ui->graphicsView->getScene()->addGraphicalConnection(sourceport, destport, portSource, portDestination);
            }
        }


        if (!counters.empty()) {
            QRegularExpression regex("Counter_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : counters) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationCounter *counter = new AnimationCounter();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    counter->setIdCounter(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    counter->setRect(newRect.normalized());
                    counter->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsCounter()->append(counter);

                    myScene()->addItem(counter);
                } else {
                    delete counter;
                }
            }
        }

        if (!variables.empty()) {
            QRegularExpression regex("Variable_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : variables) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationVariable *variable = new AnimationVariable();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    variable->setIdVariable(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    variable->setRect(newRect.normalized());
                    variable->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsVariable()->append(variable);

                    myScene()->addItem(variable);
                } else {
                    delete variable;
                }
            }
        }

        if (!timers.empty()) {
            QRegularExpression regex("Timer_(\\d+) \\s* hour=(\\d+) \\s* minute=(\\d+) \\s* second=(\\d+) \\s* format=(\\d+) \\s* position=\\(([^,]+),([^\\)]+)\\) \\s* width=([^\\t]+) \\s* height=([^\\t]+)");

            for (const QString& line : timers) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationTimer *timer = new AnimationTimer(myScene());

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int hour = match.captured(2).toInt();
                    int minute = match.captured(3).toInt();
                    int second = match.captured(4).toInt();
                    int format = match.captured(5).toInt();
                    qreal posX = match.captured(6).toDouble();
                    qreal posY = match.captured(7).toDouble();
                    int width = match.captured(8).toDouble();
                    int height = match.captured(9).toDouble();

                    timer->setInitialHours(hour);
                    timer->setInitialMinutes(minute);
                    timer->setInitialSeconds(second);
                    timer->setTimeFormat(Util::TimeFormat(format));

                    timer->setTime(0.0);

                    QRectF newRect = QRectF(0, 0, width, height);
                    timer->setRect(newRect.normalized());
                    timer->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsTimer()->append(timer);

                    myScene()->addItem(timer);

                } else {
                    delete timer;
                }
            }
        }

        if (!geometries.empty()) {
            QRegularExpression regexPos("\\s*pos=\\(([^,]+),([^\\)]+)\\)");
            QRegularExpression regexId("\\s*id=(\\d+)");
            QRegularExpression regexLine("\\s*line=\\(([^,]+),([^,]+),([^,]+),([^\\)]+)\\)");
            QRegularExpression regexRect("\\s*rect=\\(([^,]+),([^,]+),([^,]+),([^\\)]+)\\)");
            QRegularExpression regexPoints("\\s*points=([^\\t]+)");
            QRegularExpression regexText("\\s*value=([^\\t]+)");

            for (const QString& rawLine : geometries) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }
                QStringList tokens = rawLine.split("\t");
                if (tokens.size() < 4) {
                    continue;
                }

                QGraphicsItem* loadedItem = nullptr;
                QString type = tokens[2].trimmed();
                type.remove("type=");

                QRegularExpressionMatch idMatch = regexId.match(tokens[1]);
                int persistedId = idMatch.hasMatch() ? idMatch.captured(1).toInt() : -1;
                QRegularExpressionMatch posMatch = regexPos.match(tokens.last());
                QPointF itemPos(0.0, 0.0);
                if (posMatch.hasMatch()) {
                    itemPos.setX(posMatch.captured(1).toDouble());
                    itemPos.setY(posMatch.captured(2).toDouble());
                }

                if (type == "line") {
                    QRegularExpressionMatch m = regexLine.match(tokens[3]);
                    if (m.hasMatch()) {
                        QGraphicsLineItem* lineItem = new QGraphicsLineItem(m.captured(1).toDouble(), m.captured(2).toDouble(), m.captured(3).toDouble(), m.captured(4).toDouble());
                        lineItem->setPos(itemPos);
                        lineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                        lineItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        myScene()->addItem(lineItem);
                        myScene()->addDrawingGeometry(lineItem);
                        loadedItem = lineItem;
                    }
                } else if (type == "rect" || type == "ellipse") {
                    QRegularExpressionMatch m = regexRect.match(tokens[3]);
                    if (m.hasMatch()) {
                        const QRectF rect(m.captured(1).toDouble(), m.captured(2).toDouble(), m.captured(3).toDouble(), m.captured(4).toDouble());
                        if (type == "rect") {
                            QGraphicsRectItem* rectItem = new QGraphicsRectItem(rect.normalized());
                            rectItem->setPos(itemPos);
                            rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                            rectItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                            myScene()->addItem(rectItem);
                            myScene()->addDrawingGeometry(rectItem);
                            loadedItem = rectItem;
                        } else {
                            QGraphicsEllipseItem* ellipseItem = new QGraphicsEllipseItem(rect.normalized());
                            ellipseItem->setPos(itemPos);
                            ellipseItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                            ellipseItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                            myScene()->addItem(ellipseItem);
                            myScene()->addDrawingGeometry(ellipseItem);
                            loadedItem = ellipseItem;
                        }
                    }
                } else if (type == "polygon") {
                    QRegularExpressionMatch m = regexPoints.match(tokens[3]);
                    if (m.hasMatch()) {
                        QStringList pointsTokens = m.captured(1).split(";", Qt::SkipEmptyParts);
                        QPolygonF polygon;
                        for (const QString& pointToken : pointsTokens) {
                            QStringList coord = pointToken.split(",");
                            if (coord.size() == 2) {
                                polygon << QPointF(coord[0].toDouble(), coord[1].toDouble());
                            }
                        }
                        if (!polygon.isEmpty()) {
                            QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
                            polygonItem->setPos(itemPos);
                            polygonItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                            polygonItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                            myScene()->addItem(polygonItem);
                            myScene()->addDrawingGeometry(polygonItem);
                            loadedItem = polygonItem;
                        }
                    }
                } else if (tokens[0].trimmed() == "Text") {
                    QRegularExpressionMatch m = regexText.match(tokens[2]);
                    if (m.hasMatch()) {
                        QGraphicsTextItem* textItem = new QGraphicsTextItem(_decodeGuiText(m.captured(1)));
                        textItem->setPos(itemPos);
                        textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                        textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        textItem->setTextInteractionFlags(Qt::NoTextInteraction);
                        myScene()->addItem(textItem);
                        myScene()->addDrawingGeometry(textItem);
                        loadedItem = textItem;
                    }
                }

                if (persistedId > 0 && loadedItem != nullptr) {
                    persistedItems.insert(persistedId, loadedItem);
                }
            }
        }

        if (!groups.empty()) {
            QRegularExpression regexMembers("members=([^\\t]+)");
            for (const QString& rawLine : groups) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }
                QStringList tokens = rawLine.split("\t");
                if (tokens.size() < 2) {
                    continue;
                }
                QRegularExpressionMatch membersMatch = regexMembers.match(tokens[1]);
                if (!membersMatch.hasMatch()) {
                    continue;
                }
                QStringList members = membersMatch.captured(1).split(",", Qt::SkipEmptyParts);
                QList<QGraphicsItem*> groupItems;
                QList<GraphicalModelComponent*> groupComponents;
                for (const QString& idStr : members) {
                    int memberId = idStr.toInt();
                    if (persistedItems.contains(memberId)) {
                        QGraphicsItem* item = persistedItems.value(memberId);
                        groupItems.append(item);
                        if (GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(item)) {
                            groupComponents.append(component);
                        }
                    }
                }

                if (groupItems.size() > 1) {
                    // Reconstrói grupo somente com itens ainda não agrupados para evitar inconsistências.
                    bool canCreateGroup = true;
                    for (QGraphicsItem* item : groupItems) {
                        if (item == nullptr || item->group() != nullptr) {
                            canCreateGroup = false;
                            break;
                        }
                    }
                    if (!canCreateGroup) {
                        continue;
                    }

                    // Cria o grupo e reestabelece flags/estruturas internas usadas por group/ungroup.
                    QGraphicsItemGroup* group = new QGraphicsItemGroup();
                    for (QGraphicsItem* item : groupItems) {
                        group->addToGroup(item);
                    }
                    group->setHandlesChildEvents(false);
                    group->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    group->setFlag(QGraphicsItem::ItemIsMovable, true);
                    myScene()->addItem(group);
                    myScene()->getGraphicalGroups()->append(group);
                    myScene()->insertOldPositionItem(group, group->pos());
                    for (QGraphicsItem* child : group->childItems()) {
                        myScene()->insertOldPositionItem(child, child->pos());
                        child->setSelected(false);
                    }
                    group->setSelected(false);
                    if (!groupComponents.isEmpty()) {
                        myScene()->insertComponentGroup(group, groupComponents);
                    }
                }
            }
        }

        // Reaplica o estado de diagramas após reconstrução dos componentes/geometrias/grupos.
        on_actionDiagrams_triggered();
        // Reaplica o viewpoint no ciclo de eventos seguinte para garantir ranges válidos de scrollbar.
        if (hasPersistedViewState) {
            QTimer::singleShot(0, this, [this, restoredViewpointX, restoredViewpointY]() {
                QScrollBar* hBar = ui->graphicsView->horizontalScrollBar();
                QScrollBar* vBar = ui->graphicsView->verticalScrollBar();
                hBar->setValue(qBound(hBar->minimum(), restoredViewpointX, hBar->maximum()));
                vBar->setValue(qBound(vBar->minimum(), restoredViewpointY, vBar->maximum()));
            });
        }

        ui->textEdit_Console->append("\n");
        _modelfilename = QString::fromStdString(filename);
    }
    return model;
}


void MainWindow::_recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInLine) {
    PluginManager* pm = simulator->getPluginManager();
    GraphicalModelComponent *gmc;
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    Plugin* plugin = pm->find(component->getClassname());
    if (plugin == nullptr) {
        ui->textEdit_Console->append(QString::fromStdString(
            "Warning: Skipping component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ") because plugin \""
            + component->getClassname() + "\" is unavailable."));
        return;
    }
    // get color from category
    QColor color = Qt::white;
    auto colorIt = _pluginCategoryColor->find(plugin->getPluginInfo()->getCategory());
    if (colorIt != _pluginCategoryColor->end()) {
        color = colorIt->second;
    }
    gmc = scene->addGraphicalModelComponent(plugin, component, QPoint(*x, *y), color);
    if (gmc == nullptr) {
        ui->textEdit_Console->append(QString::fromStdString(
            "Warning: Failed to draw component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ")."));
        return;
    }
    map->insert({component,gmc});
    visited->insert(component);
    int yIni = *y;
    int xIni = *x;
    const int deltaY = TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 1.5;
    GraphicalComponentPort *sourceGraphicalPort, *destinyGraphicalPort;
    for(auto connectionMap: *component->getConnectionManager()->connections()) {
        if (connectionMap.second == nullptr || connectionMap.second->component == nullptr) {
            continue;
        }
        ModelComponent* nextComp = connectionMap.second->component;
        if (visited->find(nextComp)==visited->list()->end()) { // nextComponent was not visited yet
            if (++sequenceInLine==6) {
                *x -= 5 * TraitsGUI<GModelComponent>::width * 1.5;
                *y+= deltaY;
                sequenceInLine = 0;
            } else {
                *x += TraitsGUI<GModelComponent>::width * 1.5;
            }
            if (*y > *ymax)
                *ymax = *y;
            _recursivalyGenerateGraphicalModelFromModel(nextComp, visited, map, x, y, ymax, sequenceInLine);
            auto destinyIt = map->find(nextComp);
            if (destinyIt != map->end()) {
                GraphicalModelComponent *destinyGmc = destinyIt->second;
                if (connectionMap.first < gmc->getGraphicalOutputPorts().size()
                        && connectionMap.second->channel.portNumber < destinyGmc->getGraphicalInputPorts().size()) {
                    sourceGraphicalPort = gmc->getGraphicalOutputPorts().at(connectionMap.first);
                    destinyGraphicalPort = destinyGmc->getGraphicalInputPorts().at(connectionMap.second->channel.portNumber);
                    scene->addGraphicalConnection(sourceGraphicalPort, destinyGraphicalPort, connectionMap.first, connectionMap.second->channel.portNumber);
                }
            }
            *x = xIni;
            *y+= deltaY;
            sequenceInLine--;
        }
    }
    *y = yIni;
}

void MainWindow::_actualizeModelComponents(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetComponents->clear();
    if (m == nullptr) {
        return;
    }
    for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
        QList<QTreeWidgetItem *> items = ui->treeWidgetComponents->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
        if (items.size() == 0) {
            QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetComponents);
            treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
            treeComp->setText(1, QString::fromStdString(comp->getClassname()));
            treeComp->setText(2, QString::fromStdString(comp->getName()));
            std::string properties = "";
            for (auto prop : *comp->getProperties()->list()) {
                properties += prop->getName() + ":" + prop->getValue() + ", ";
            }
            properties = properties.substr(0, properties.length() - 2);
            treeComp->setText(3, QString::fromStdString(properties));
        }
    }
    ui->treeWidgetComponents->resizeColumnToContents(0);
    ui->treeWidgetComponents->resizeColumnToContents(1);
    ui->treeWidgetComponents->resizeColumnToContents(2);
}

void MainWindow::_actualizeModelTextHasChanged(bool hasChanged) {
    if (_textModelHasChanged != hasChanged) {
    }
    _textModelHasChanged = hasChanged;
}

void MainWindow::_actualizeModelDataDefinitions(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetDataDefnitions->clear();
    if (m == nullptr) {
        return;
    }
    for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
        for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {
            QList<QTreeWidgetItem *> items = ui->treeWidgetDataDefnitions->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
            if (items.size() == 0) {
                QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetDataDefnitions);
                treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
                treeComp->setText(1, QString::fromStdString(comp->getClassname()));
                treeComp->setText(2, QString::fromStdString(comp->getName()));
                std::string properties = "";
                for (auto prop : *comp->getProperties()->list()) {
                    properties += prop->getName() + ":" + prop->getValue() + ", ";
                }
                properties = properties.substr(0, properties.length() - 2);
                treeComp->setText(3, QString::fromStdString(properties));
            }
        }
    }
    ui->treeWidgetDataDefnitions->resizeColumnToContents(0);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(1);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(2);
}

void MainWindow::_actualizeGraphicalModel(SimulationEvent * re) {
    Event* event = re->getCurrentEvent();
    if (event != nullptr) {
        ui->graphicsView->selectModelComponent(event->getComponent());
    }
}

void MainWindow::_generateGraphicalModelFromModel() {
    Model* m=simulator->getModelManager()->current();
    if (m!=nullptr) {
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(false);
        //ui->graphicsView->getScene()->showGrid();
        int x, y, ymax;
        x=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().left();
        y=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().top();
        ymax=y;
        ComponentManager* cm = m->getComponentManager();
        if (cm == nullptr) {
            ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
            return;
        }
        List<ModelComponent*>* visited = new List<ModelComponent*>();
        std::map<ModelComponent*,GraphicalModelComponent*>* map = new std::map<ModelComponent*,GraphicalModelComponent*>();
        for(SourceModelComponent* source: *cm->getSourceComponents()) {
            _recursivalyGenerateGraphicalModelFromModel(source, visited, map, &x, &y, &ymax, 0);
            y= ymax + TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 3; // get heigth mapped to scene??
        }
        // check if any component remains unvisited
        bool foundNotVisited;
        do {
            foundNotVisited = false;
            for (ModelComponent* comp: *cm->getAllComponents()) {
                if (visited->find(comp) == visited->list()->end()) { // found a compponent not visited yet
                    foundNotVisited = true;
                    visited->insert(comp);
                    // recursive create
                }
            }
        } while (foundNotVisited);
        delete map;
        delete visited;
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
    }
}


//-----------------------------------------

void MainWindow::_initModelGraphicsView() {
    // Registers scene/view callbacks used by MainWindow controllers.
    ((ModelGraphicsView*) (ui->graphicsView))->setSceneMouseEventHandler(this, &MainWindow::_onSceneMouseEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelInEventHandler(this, &MainWindow::_onSceneWheelInEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelOutEventHandler(this, &MainWindow::_onSceneWheelOutEvent);
    ((ModelGraphicsView*) (ui->graphicsView))->setGraphicalModelEventHandler(this, &MainWindow::_onSceneGraphicalModelEvent);
    _connectSceneSignals();

    // Applies persisted overlay states to the graphics view when initializing a scene.
    ui->graphicsView->setRuleVisible(ui->actionShowRule->isChecked());
    ui->graphicsView->setGuidesVisible(ui->actionShowGuides->isChecked());

    // Cria uma stack undo/redo
    ui->graphicsView->getScene()->setUndoStack(new QUndoStack(this));
}

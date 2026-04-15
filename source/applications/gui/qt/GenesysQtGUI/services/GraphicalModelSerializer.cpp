#include "GraphicalModelSerializer.h"

#include "../TraitsGUI.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../animations/AnimationCounter.h"
#include "../animations/AnimationVariable.h"
#include "../animations/AnimationTimer.h"
#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/simulator/ModelManager.h"

#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QHash>
#include <QFont>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSlider>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

namespace {

QString encodePortPositions(GraphicalModelComponent* component) {
    QStringList positions;
    auto appendPorts = [&positions](const QList<GraphicalComponentPort*>& ports, const QString& prefix) {
        for (GraphicalComponentPort* port : ports) {
            if (port == nullptr) {
                continue;
            }
            positions << QString("%1%2=(%3,%4)")
                             .arg(prefix)
                             .arg(port->portNum())
                             .arg(port->pos().x(), 0, 'f', 4)
                             .arg(port->pos().y(), 0, 'f', 4);
        }
    };

    if (component != nullptr) {
        appendPorts(component->getGraphicalInputPorts(), "in");
        appendPorts(component->getGraphicalOutputPorts(), "out");
    }
    return positions.join(";");
}

QHash<QString, QPointF> decodePortPositions(const QString& token) {
    QHash<QString, QPointF> positions;
    QRegularExpression portsTokenRegex("\\s*ports=(.*)");
    QRegularExpressionMatch portsTokenMatch = portsTokenRegex.match(token);
    if (!portsTokenMatch.hasMatch()) {
        return positions;
    }

    QRegularExpression portRegex("(in|out)(\\d+)=\\(([-+]?\\d+\\.?\\d*),([-+]?\\d+\\.?\\d*)\\)");
    QRegularExpressionMatchIterator matches = portRegex.globalMatch(portsTokenMatch.captured(1));
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        const QString key = match.captured(1) + match.captured(2);
        positions.insert(key, QPointF(match.captured(3).toDouble(), match.captured(4).toDouble()));
    }
    return positions;
}

void restorePortPositions(GraphicalModelComponent* component, const QHash<QString, QPointF>& positions) {
    if (component == nullptr || positions.isEmpty()) {
        return;
    }

    auto restorePorts = [&positions](const QList<GraphicalComponentPort*>& ports, const QString& prefix) {
        for (GraphicalComponentPort* port : ports) {
            if (port == nullptr) {
                continue;
            }
            const QString key = prefix + QString::number(port->portNum());
            if (positions.contains(key)) {
                port->setPos(positions.value(key));
            }
        }
    };

    restorePorts(component->getGraphicalInputPorts(), "in");
    restorePorts(component->getGraphicalOutputPorts(), "out");
}

}

// Build the serializer with explicit, narrow dependencies from MainWindow.
GraphicalModelSerializer::GraphicalModelSerializer(Simulator* simulator,
                                                   QWidget* ownerWidget,
                                                   QPlainTextEdit* modelTextEditor,
                                                   ModelGraphicsView* graphicsView,
                                                   QSlider* zoomSlider,
                                                   QAction* actionShowGrid,
                                                   QAction* actionShowRule,
                                                   QAction* actionShowSnap,
                                                   QAction* actionShowGuides,
                                                   QAction* actionShowInternalElements,
                                                   QAction* actionShowAttachedElements,
                                                   QAction* actionDiagrams,
                                                   QTextEdit* console,
                                                   QString* modelFilename,
                                                   std::function<void()> clearModelEditors,
                                                   std::function<void()> rebuildGraphicalModelFromModel,
                                                   std::function<void()> applyShowInternalElements,
                                                   std::function<void()> applyShowAttachedElements,
                                                   std::function<void()> applyDiagramsVisibility)
    : _simulator(simulator),
      _ownerWidget(ownerWidget),
      _modelTextEditor(modelTextEditor),
      _graphicsView(graphicsView),
      _zoomSlider(zoomSlider),
      _actionShowGrid(actionShowGrid),
      _actionShowRule(actionShowRule),
      _actionShowSnap(actionShowSnap),
      _actionShowGuides(actionShowGuides),
      _actionShowInternalElements(actionShowInternalElements),
      _actionShowAttachedElements(actionShowAttachedElements),
      _actionDiagrams(actionDiagrams),
      _console(console),
      _modelFilename(modelFilename),
      _clearModelEditors(std::move(clearModelEditors)),
      _rebuildGraphicalModelFromModel(std::move(rebuildGraphicalModelFromModel)),
      _applyShowInternalElements(std::move(applyShowInternalElements)),
      _applyShowAttachedElements(std::move(applyShowAttachedElements)),
      _applyDiagramsVisibility(std::move(applyDiagramsVisibility)) {}

// Encode free-form GUI text safely for persistence records.
QString GraphicalModelSerializer::encodeGuiText(const QString& text) {
    return QString::fromUtf8(QUrl::toPercentEncoding(text));
}

// Decode persisted GUI text back to plain UTF-8 text.
QString GraphicalModelSerializer::decodeGuiText(const QString& text) {
    return QUrl::fromPercentEncoding(text.toUtf8());
}

// Preserve the existing text persistence format line-by-line.
bool GraphicalModelSerializer::saveTextModel(QFile* saveFile, const QString& data) const {
    QTextStream out(saveFile);

    try {
        static const QRegularExpression regex("[\n]");
        QStringList strList = data.split(regex);
        for (const QString& line : strList) {
            out << line << Qt::endl;
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

QString encodePersistedText(const QString& text) {
    return QString::fromUtf8(QUrl::toPercentEncoding(text));
}

QString decodePersistedText(const QString& text) {
    return QUrl::fromPercentEncoding(text.toUtf8());
}

QString commonGraphicsState(QGraphicsItem* item) {
    if (item == nullptr) {
        return {};
    }
    return QString("rotation=%1 \t scale=%2 \t z=%3 \t visible=%4 \t opacity=%5")
        .arg(item->rotation(), 0, 'f', 4)
        .arg(item->scale(), 0, 'f', 4)
        .arg(item->zValue(), 0, 'f', 4)
        .arg(item->isVisible() ? 1 : 0)
        .arg(item->opacity(), 0, 'f', 4);
}

QString penState(const QPen& pen) {
    return QString("pen=(%1,%2,%3)")
        .arg(pen.color().name(QColor::HexArgb))
        .arg(pen.widthF(), 0, 'f', 4)
        .arg(static_cast<int>(pen.style()));
}

QString brushState(const QBrush& brush) {
    return QString("brush=(%1,%2)")
        .arg(brush.color().name(QColor::HexArgb))
        .arg(static_cast<int>(brush.style()));
}

void applyCommonGraphicsState(const QString& line, QGraphicsItem* item) {
    if (item == nullptr) {
        return;
    }

    QRegularExpression regexRotation("\\brotation=([^\\t]+)");
    QRegularExpression regexScale("\\bscale=([^\\t]+)");
    QRegularExpression regexZ("\\bz=([^\\t]+)");
    QRegularExpression regexVisible("\\bvisible=([^\\t]+)");
    QRegularExpression regexOpacity("\\bopacity=([^\\t]+)");

    QRegularExpressionMatch match = regexRotation.match(line);
    if (match.hasMatch()) {
        item->setRotation(match.captured(1).trimmed().toDouble());
    }
    match = regexScale.match(line);
    if (match.hasMatch()) {
        item->setScale(match.captured(1).trimmed().toDouble());
    }
    match = regexZ.match(line);
    if (match.hasMatch()) {
        item->setZValue(match.captured(1).trimmed().toDouble());
    }
    match = regexVisible.match(line);
    if (match.hasMatch()) {
        item->setVisible(match.captured(1).trimmed().toInt() != 0);
    }
    match = regexOpacity.match(line);
    if (match.hasMatch()) {
        item->setOpacity(match.captured(1).trimmed().toDouble());
    }
}

bool decodePenState(const QString& line, QPen* pen) {
    if (pen == nullptr) {
        return false;
    }
    QRegularExpression regexPen("\\bpen=\\((#[0-9A-Fa-f]{6,8}),([^,]+),(\\d+)\\)");
    QRegularExpressionMatch match = regexPen.match(line);
    if (!match.hasMatch()) {
        return false;
    }
    pen->setColor(QColor(match.captured(1)));
    pen->setWidthF(match.captured(2).toDouble());
    pen->setStyle(static_cast<Qt::PenStyle>(match.captured(3).toInt()));
    return true;
}

bool decodeBrushState(const QString& line, QBrush* brush) {
    if (brush == nullptr) {
        return false;
    }
    QRegularExpression regexBrush("\\bbrush=\\((#[0-9A-Fa-f]{6,8}),(\\d+)\\)");
    QRegularExpressionMatch match = regexBrush.match(line);
    if (!match.hasMatch()) {
        return false;
    }
    brush->setColor(QColor(match.captured(1)));
    brush->setStyle(static_cast<Qt::BrushStyle>(match.captured(2).toInt()));
    return true;
}

void applyShapeStyleState(const QString& line, QAbstractGraphicsShapeItem* item) {
    if (item == nullptr) {
        return;
    }
    QPen pen = item->pen();
    if (decodePenState(line, &pen)) {
        item->setPen(pen);
    }
    QBrush brush = item->brush();
    if (decodeBrushState(line, &brush)) {
        item->setBrush(brush);
    }
}

void applyTextStyleState(const QString& line, QGraphicsTextItem* item) {
    if (item == nullptr) {
        return;
    }

    QRegularExpression regexColor("\\btextcolor=(#[0-9A-Fa-f]{6,8})");
    QRegularExpression regexFont("\\bfont=([^\\t]+)");
    QRegularExpression regexWidth("\\btextwidth=([^\\t]+)");

    QRegularExpressionMatch match = regexColor.match(line);
    if (match.hasMatch()) {
        item->setDefaultTextColor(QColor(match.captured(1)));
    }
    match = regexFont.match(line);
    if (match.hasMatch()) {
        QFont font;
        if (font.fromString(decodePersistedText(match.captured(1).trimmed()))) {
            item->setFont(font);
        }
    }
    match = regexWidth.match(line);
    if (match.hasMatch()) {
        item->setTextWidth(match.captured(1).trimmed().toDouble());
    }
}

// Persist the complete graphical model, including view options and overlays.
bool GraphicalModelSerializer::saveGraphicalModel(const QString& filename) const {
    QFile saveFile(filename);

    try {
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::information(_ownerWidget, QObject::tr("Unable to access file to save"),
                                     saveFile.errorString());
            return false;
        }

        saveTextModel(&saveFile, _modelTextEditor->toPlainText());

        QTextStream out(&saveFile);
        out << "#Genegys Graphic Model" << Qt::endl;
        QString line = "0\tView\t";
        line += "zoom=" + QString::number(_zoomSlider->value());
        line += ", grid=" + QString::number(_actionShowGrid->isChecked());
        line += ", rule=" + QString::number(_actionShowRule->isChecked());
        line += ", snap=" + QString::number(_actionShowSnap->isChecked());
        line += ", guides=" + QString::number(_actionShowGuides->isChecked());
        line += ", internals=" + QString::number(_actionShowInternalElements->isChecked());
        line += ", attached=" + QString::number(_actionShowAttachedElements->isChecked());
        line += ", diagrams=" + QString::number(_actionDiagrams->isChecked());
        line += ", viewpoint=(" + QString::number(_graphicsView->horizontalScrollBar()->value()) + "," + QString::number(_graphicsView->verticalScrollBar()->value()) + ")";
        out << line << Qt::endl;

        ModelGraphicsScene* scene = static_cast<ModelGraphicsScene*>(_graphicsView->getScene());

        if (scene) {
            QHash<QGraphicsItem*, int> persistedItemIds;
            int nextPersistedItemId = 1;

            for (QGraphicsItem* item : *scene->getGraphicalModelComponents()) {
                GraphicalModelComponent* gmc = static_cast<GraphicalModelComponent*>(item);
                if (gmc) {
                    int persistedId = nextPersistedItemId++;
                    persistedItemIds.insert(gmc, persistedId);
                    line = QString::fromStdString(std::to_string(gmc->getComponent()->getId()) + "\t" + gmc->getComponent()->getClassname() + "\t" + gmc->getComponent()->getName() + "\t" + "color=" + gmc->getColor().name().toStdString() + "\t" + "position=(" + std::to_string(gmc->scenePos().x()) + "," + std::to_string(gmc->scenePos().y() + gmc->getHeight()/2) + ")");
                    line += "\titemid=" + QString::number(persistedId);
                    line += "\tports=" + encodePortPositions(gmc);
                    out << line << Qt::endl;
                }
            }

            QList<QGraphicsItem*>* graphicalDataDefinitions = scene->getGraphicalModelDataDefinitions();
            if (graphicalDataDefinitions != nullptr && !graphicalDataDefinitions->isEmpty()) {
                out << Qt::endl;
                out << "#DataDefinitions" << Qt::endl;

                // Persist data definition nodes with stable ids so layout can be restored after reload.
                for (QGraphicsItem* item : *graphicalDataDefinitions) {
                    GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
                    if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
                        continue;
                    }

                    ModelDataDefinition* dataDefinition = gmdd->getDataDefinition();
                    int persistedId = nextPersistedItemId++;
                    persistedItemIds.insert(gmdd, persistedId);

                    line = QString("DataDefinition \t id=%1 \t dataid=%2 \t classname=%3 \t name=%4 \t position=(%5,%6) \t itemid=%7")
                               .arg(persistedId)
                               .arg(dataDefinition->getId())
                               .arg(QString::fromStdString(dataDefinition->getClassname()))
                               .arg(encodeGuiText(QString::fromStdString(dataDefinition->getName())))
                               .arg(gmdd->scenePos().x(), 0, 'f', 4)
                               .arg(gmdd->scenePos().y(), 0, 'f', 4)
                               .arg(persistedId);
                    out << line << Qt::endl;
                }
            }

            QList<QGraphicsItem*>* geometries = scene->getGraphicalGeometries();
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
                        line = QString("Geometry \t id=%1 \t type=line \t line=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9")
                                   .arg(persistedId)
                                   .arg(l.x1(), 0, 'f', 4).arg(l.y1(), 0, 'f', 4)
                                   .arg(l.x2(), 0, 'f', 4).arg(l.y2(), 0, 'f', 4)
                                   .arg(lineItem->pos().x(), 0, 'f', 4).arg(lineItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(lineItem))
                                   .arg(penState(lineItem->pen()));
                        out << line << Qt::endl;
                    } else if (QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
                        QRectF r = rectItem->rect().normalized();
                        line = QString("Geometry \t id=%1 \t type=rect \t rect=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9 \t %10")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(rectItem->pos().x(), 0, 'f', 4).arg(rectItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(rectItem))
                                   .arg(penState(rectItem->pen()))
                                   .arg(brushState(rectItem->brush()));
                        out << line << Qt::endl;
                    } else if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                        QRectF r = ellipseItem->rect().normalized();
                        line = QString("Geometry \t id=%1 \t type=ellipse \t rect=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9 \t %10")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(ellipseItem->pos().x(), 0, 'f', 4).arg(ellipseItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(ellipseItem))
                                   .arg(penState(ellipseItem->pen()))
                                   .arg(brushState(ellipseItem->brush()));
                        out << line << Qt::endl;
                    } else if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
                        QStringList points;
                        const QPolygonF polygon = polygonItem->polygon();
                        for (const QPointF& p : polygon) {
                            points << QString("%1,%2").arg(p.x(), 0, 'f', 4).arg(p.y(), 0, 'f', 4);
                        }
                        line = QString("Geometry \t id=%1 \t type=polygon \t points=%2 \t pos=(%3,%4) \t %5 \t %6 \t %7")
                                   .arg(persistedId)
                                   .arg(points.join(";"))
                                   .arg(polygonItem->pos().x(), 0, 'f', 4).arg(polygonItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(polygonItem))
                                   .arg(penState(polygonItem->pen()))
                                   .arg(brushState(polygonItem->brush()));
                        out << line << Qt::endl;
                    } else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                        line = QString("Text \t id=%1 \t value=%2 \t pos=(%3,%4) \t %5 \t textcolor=%6 \t font=%7 \t textwidth=%8")
                                   .arg(persistedId)
                                   .arg(encodeGuiText(textItem->toPlainText()))
                                   .arg(textItem->pos().x(), 0, 'f', 4).arg(textItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(textItem))
                                   .arg(textItem->defaultTextColor().name(QColor::HexArgb))
                                   .arg(encodePersistedText(textItem->font().toString()))
                                   .arg(textItem->textWidth(), 0, 'f', 4);
                        out << line << Qt::endl;
                    }
                }
            }

            QList<QGraphicsItemGroup*>* groups = scene->getGraphicalGroups();
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

            QList<AnimationCounter*>* counters = scene->getAnimationsCounter();
            if (counters && !counters->empty()) {
                out << Qt::endl;
                out << "#Counters" << Qt::endl;
                int id = 0;
                for (AnimationCounter* counter : *counters) {
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

            QList<AnimationVariable*>* variables = scene->getAnimationsVariable();
            if (variables && !variables->empty()) {
                out << Qt::endl;
                out << "#Variables" << Qt::endl;
                int id = 0;
                for (AnimationVariable* variable : *variables) {
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

            QList<AnimationTimer*>* timers = scene->getAnimationsTimer();
            if (timers && !timers->empty()) {
                out << Qt::endl;
                out << "#Timers" << Qt::endl;
                int id = 0;
                for (AnimationTimer* timer : *timers) {
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

        saveFile.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Restore model text and GUI state from .gui or defer to .gen loading flow.
Model* GraphicalModelSerializer::loadGraphicalModel(const std::string& filename) const {
    QFile file(QString::fromStdString(filename));

    Model* model = nullptr;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(_ownerWidget, QObject::tr("Unable to access file to save"),
                                 file.errorString());
        return nullptr;
    }

    QFileInfo fileInfo(file.fileName());
    QString extension = fileInfo.suffix();

    if (extension != "gui") {
        model = _simulator->getModelManager()->loadModel(file.fileName().toStdString());
        if (model != nullptr) {
            _rebuildGraphicalModelFromModel();
        }
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
    QStringList dataDefinitions;

    bool guiFlag = false;
    bool counterFlag = false;
    bool variableFlag = false;
    bool timerFlag = false;
    bool geometryFlag = false;
    bool groupFlag = false;
    bool dataDefinitionFlag = false;

    for (const QString& line : lines) {
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
            dataDefinitionFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }
        if (line.startsWith("#Variables")) {
            variableFlag = true;
            guiFlag = false;
            counterFlag = false;
            timerFlag = false;
            dataDefinitionFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }
        if (line.startsWith("#Timers")) {
            timerFlag = true;
            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            dataDefinitionFlag = false;
            geometryFlag = false;
            groupFlag = false;
            continue;
        }
        if (line.startsWith("#DataDefinitions")) {
            dataDefinitionFlag = true;
            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
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
            dataDefinitionFlag = false;
            groupFlag = false;
            continue;
        }
        if (line.startsWith("#Groups")) {
            groupFlag = true;
            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            dataDefinitionFlag = false;
            geometryFlag = false;
            continue;
        }

        if (!guiFlag && !timerFlag && !counterFlag && !variableFlag && !geometryFlag && !groupFlag && !dataDefinitionFlag) {
            simulLang.append(line);
        } else if (counterFlag) {
            counters.append(line);
        } else if (variableFlag) {
            variables.append(line);
        } else if (timerFlag) {
            timers.append(line);
        } else if (dataDefinitionFlag) {
            dataDefinitions.append(line);
        } else if (geometryFlag) {
            geometries.append(line);
        } else if (groupFlag) {
            groups.append(line);
        } else {
            gui.append(line);
        }
    }

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

    model = _simulator->getModelManager()->loadModel(tempFile.fileName().toStdString());
    tempFile.close();
    QFile::remove(tempFile.fileName());

    if (model != nullptr) {
        ModelGraphicsScene* scene = _graphicsView->getScene();
        // Rebuild the base graphical topology from a single source of truth before applying persisted GUI overlays.
        _clearModelEditors();
        // Mark persisted-layout restore so builder defaults do not override saved grouping/positions.
        scene->setRestoringPersistedGuiLayout(true);
        _rebuildGraphicalModelFromModel();

        struct PersistedComponentState {
            Util::identification componentId = 0;
            QPointF position;
            int itemId = -1;
            QHash<QString, QPointF> portPositions;
        };

        struct PersistedDataDefinitionState {
            Util::identification dataId = 0;
            QString className;
            QString name;
            QPointF position;
            int itemId = -1;
        };

        bool firstLine = true;
        bool hasPersistedViewState = false;
        int restoredViewpointX = 0;
        int restoredViewpointY = 0;
        QHash<int, QGraphicsItem*> persistedItems;
        QHash<Util::identification, PersistedComponentState> persistedComponentsById;
        QHash<Util::identification, PersistedDataDefinitionState> persistedDataDefinitionsById;
        QHash<QString, PersistedDataDefinitionState> persistedDataDefinitionsByClassAndName;

        for (const QString& line : gui) {
            if (line.trimmed().isEmpty()) {
                continue;
            }
            if (firstLine) {
                QRegularExpression regex("(\\d+)\\s*View\\s*(.*)");
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    const QString attributes = match.captured(2);
                    const int zoom = QRegularExpression("zoom=(-?\\d+)").match(attributes).captured(1).toInt();
                    const int grid = QRegularExpression("grid=(\\d+)").match(attributes).captured(1).toInt();
                    const int rule = QRegularExpression("rule=(\\d+)").match(attributes).captured(1).toInt();
                    const int snap = QRegularExpression("snap=(\\d+)").match(attributes).captured(1).toInt();
                    const int guides = QRegularExpression("guides=(\\d+)").match(attributes).captured(1).toInt();
                    const int internals = QRegularExpression("internals=(\\d+)").match(attributes).captured(1).toInt();
                    const int attached = QRegularExpression("attached=(\\d+)").match(attributes).captured(1).toInt();
                    const int diagrams = QRegularExpression("diagrams=(\\d+)").match(attributes).captured(1).toInt();
                    QRegularExpressionMatch viewpointMatch = QRegularExpression("viewpoint=\\(([-+]?\\d+\\.?\\d*),([-+]?\\d+\\.?\\d*)\\)").match(attributes);
                    int viewpointX = 0;
                    int viewpointY = 0;
                    if (viewpointMatch.hasMatch()) {
                        viewpointX = viewpointMatch.captured(1).toInt();
                        viewpointY = viewpointMatch.captured(2).toInt();
                    }

                    _actionShowGrid->setChecked(grid != 0);
                    _graphicsView->getScene()->setGridVisible(grid != 0);
                    _actionShowSnap->setChecked(snap != 0);
                    _graphicsView->getScene()->setSnapToGrid(snap != 0);
                    _actionShowRule->setChecked(rule != 0);
                    _actionShowGuides->setChecked(guides != 0);
                    _graphicsView->setRuleVisible(rule != 0);
                    _graphicsView->setGuidesVisible(guides != 0);
                    _actionShowInternalElements->setChecked(internals != 0);
                    _applyShowInternalElements();
                    _actionShowAttachedElements->setChecked(attached != 0);
                    _applyShowAttachedElements();
                    _actionDiagrams->setChecked(diagrams != 0);

                    if (zoom > 0) {
                        _zoomSlider->setValue(zoom + TraitsGUI<GMainWindow>::zoomButtonChange);
                    }

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

            PersistedComponentState state;
            state.componentId = split[0].toULongLong();
            QString pos = split[4];

            QPointF position;
            QRegularExpressionMatch posMatch = QRegularExpression("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)").match(pos);
            if (posMatch.hasMatch()) {
                position.setX(posMatch.captured(1).toDouble());
                position.setY(posMatch.captured(2).toDouble());
            } else {
                QRegularExpressionMatch legacyPosMatch = QRegularExpression("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)").match(pos);
                if (legacyPosMatch.hasMatch()) {
                    position.setX(legacyPosMatch.captured(1).toDouble());
                    position.setY(legacyPosMatch.captured(3).toDouble());
                }
            }
            state.position = position;

            if (split.size() >= 6) {
                QRegularExpressionMatch itemIdMatch = QRegularExpression("itemid=(\\d+)").match(split[5]);
                if (itemIdMatch.hasMatch()) {
                    state.itemId = itemIdMatch.captured(1).toInt();
                }
            }
            for (int i = 6; i < split.size(); ++i) {
                if (split[i].trimmed().startsWith("ports=")) {
                    state.portPositions = decodePortPositions(split[i]);
                    break;
                }
            }
            persistedComponentsById.insert(state.componentId, state);
        }

        // Restore persisted component positions by matching existing items created by GraphicalModelBuilder.
        for (auto it = persistedComponentsById.constBegin(); it != persistedComponentsById.constEnd(); ++it) {
            const PersistedComponentState& state = it.value();
            GraphicalModelComponent* existingComponent = _graphicsView->getScene()->findGraphicalModelComponent(state.componentId);
            if (existingComponent == nullptr) {
                continue;
            }
            QPointF componentPos(state.position.x(), state.position.y() - existingComponent->getHeight() / 2.0);
            existingComponent->setPos(componentPos);
            existingComponent->setOldPosition(componentPos);
            restorePortPositions(existingComponent, state.portPositions);
            if (state.itemId > 0) {
                persistedItems.insert(state.itemId, existingComponent);
            }
        }

        if (!dataDefinitions.empty()) {
            // Parse persisted data definition layout metadata for id-based and fallback lookup.
            QRegularExpression regexDataId("\\s*dataid=(\\d+)");
            QRegularExpression regexClassname("\\s*classname=([^\\t]+)");
            QRegularExpression regexName("\\s*name=([^\\t]+)");
            QRegularExpression regexPosition("\\s*position=\\(([^,]+),([^\\)]+)\\)");
            QRegularExpression regexItemId("\\s*itemid=(\\d+)");

            for (const QString& rawLine : dataDefinitions) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }

                QStringList tokens = rawLine.split("\t");
                if (tokens.size() < 6) {
                    continue;
                }

                PersistedDataDefinitionState state;
                QRegularExpressionMatch dataIdMatch = regexDataId.match(tokens[2]);
                if (dataIdMatch.hasMatch()) {
                    state.dataId = dataIdMatch.captured(1).toULongLong();
                }
                QRegularExpressionMatch classMatch = regexClassname.match(tokens[3]);
                if (classMatch.hasMatch()) {
                    state.className = classMatch.captured(1).trimmed();
                }
                QRegularExpressionMatch nameMatch = regexName.match(tokens[4]);
                if (nameMatch.hasMatch()) {
                    state.name = decodeGuiText(nameMatch.captured(1).trimmed());
                }
                QRegularExpressionMatch posMatch = regexPosition.match(tokens[5]);
                if (posMatch.hasMatch()) {
                    state.position = QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble());
                } else {
                    continue;
                }
                if (tokens.size() >= 7) {
                    QRegularExpressionMatch itemIdMatch = regexItemId.match(tokens[6]);
                    if (itemIdMatch.hasMatch()) {
                        state.itemId = itemIdMatch.captured(1).toInt();
                    }
                }

                if (state.dataId > 0) {
                    persistedDataDefinitionsById.insert(state.dataId, state);
                }
                persistedDataDefinitionsByClassAndName.insert(state.className + "#" + state.name, state);
            }

            // Apply persisted data definition positions to existing items created by GraphicalModelBuilder.
            QList<QGraphicsItem*>* graphicalDataDefinitions = scene->getGraphicalModelDataDefinitions();
            if (graphicalDataDefinitions != nullptr) {
                for (QGraphicsItem* item : *graphicalDataDefinitions) {
                    GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
                    if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
                        continue;
                    }

                    ModelDataDefinition* dataDefinition = gmdd->getDataDefinition();
                    bool applied = false;
                    auto byId = persistedDataDefinitionsById.find(dataDefinition->getId());
                    if (byId != persistedDataDefinitionsById.end()) {
                        gmdd->setPos(byId->position);
                        gmdd->setOldPosition(byId->position.x(), byId->position.y());
                        if (byId->itemId > 0) {
                            persistedItems.insert(byId->itemId, gmdd);
                        }
                        applied = true;
                    }

                    if (!applied) {
                        const QString fallbackKey = QString::fromStdString(dataDefinition->getClassname()) + "#"
                                                    + QString::fromStdString(dataDefinition->getName());
                        auto byClassAndName = persistedDataDefinitionsByClassAndName.find(fallbackKey);
                        if (byClassAndName != persistedDataDefinitionsByClassAndName.end()) {
                            gmdd->setPos(byClassAndName->position);
                            gmdd->setOldPosition(byClassAndName->position.x(), byClassAndName->position.y());
                            if (byClassAndName->itemId > 0) {
                                persistedItems.insert(byClassAndName->itemId, gmdd);
                            }
                        }
                    }
                }
            }
        }

        if (!counters.empty()) {
            QRegularExpression regex("Counter_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");
            for (const QString& line : counters) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }
                AnimationCounter* counter = new AnimationCounter();
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    counter->setIdCounter(match.captured(2).toInt());
                    counter->setRect(QRectF(0, 0, match.captured(5).toDouble(), match.captured(6).toDouble()).normalized());
                    counter->setPos(QPointF(match.captured(3).toDouble(), match.captured(4).toDouble()));
                    _graphicsView->getScene()->getAnimationsCounter()->append(counter);
                    _graphicsView->getScene()->addItem(counter);
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
                AnimationVariable* variable = new AnimationVariable();
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    variable->setIdVariable(match.captured(2).toInt());
                    variable->setRect(QRectF(0, 0, match.captured(5).toDouble(), match.captured(6).toDouble()).normalized());
                    variable->setPos(QPointF(match.captured(3).toDouble(), match.captured(4).toDouble()));
                    _graphicsView->getScene()->getAnimationsVariable()->append(variable);
                    _graphicsView->getScene()->addItem(variable);
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
                AnimationTimer* timer = new AnimationTimer(_graphicsView->getScene());
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    timer->setInitialHours(match.captured(2).toInt());
                    timer->setInitialMinutes(match.captured(3).toInt());
                    timer->setInitialSeconds(match.captured(4).toInt());
                    timer->setTimeFormat(Util::TimeFormat(match.captured(5).toInt()));
                    timer->setTime(0.0);
                    timer->setRect(QRectF(0, 0, match.captured(8).toDouble(), match.captured(9).toDouble()).normalized());
                    timer->setPos(QPointF(match.captured(6).toDouble(), match.captured(7).toDouble()));
                    _graphicsView->getScene()->getAnimationsTimer()->append(timer);
                    _graphicsView->getScene()->addItem(timer);
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
                QRegularExpressionMatch posMatch = regexPos.match(rawLine);
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
                        _graphicsView->getScene()->addItem(lineItem);
                        _graphicsView->getScene()->addDrawingGeometry(lineItem);
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
                            _graphicsView->getScene()->addItem(rectItem);
                            _graphicsView->getScene()->addDrawingGeometry(rectItem);
                            loadedItem = rectItem;
                        } else {
                            QGraphicsEllipseItem* ellipseItem = new QGraphicsEllipseItem(rect.normalized());
                            ellipseItem->setPos(itemPos);
                            ellipseItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                            ellipseItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                            _graphicsView->getScene()->addItem(ellipseItem);
                            _graphicsView->getScene()->addDrawingGeometry(ellipseItem);
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
                            _graphicsView->getScene()->addItem(polygonItem);
                            _graphicsView->getScene()->addDrawingGeometry(polygonItem);
                            loadedItem = polygonItem;
                        }
                    }
                } else if (tokens[0].trimmed() == "Text") {
                    QRegularExpressionMatch m = regexText.match(tokens[2]);
                    if (m.hasMatch()) {
                        QGraphicsTextItem* textItem = new QGraphicsTextItem(decodeGuiText(m.captured(1)));
                        textItem->setPos(itemPos);
                        textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                        textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        textItem->setTextInteractionFlags(Qt::NoTextInteraction);
                        _graphicsView->getScene()->addItem(textItem);
                        _graphicsView->getScene()->addDrawingGeometry(textItem);
                        loadedItem = textItem;
                    }
                }

                if (persistedId > 0 && loadedItem != nullptr) {
                    applyCommonGraphicsState(rawLine, loadedItem);
                    if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(loadedItem)) {
                        QPen pen = lineItem->pen();
                        if (decodePenState(rawLine, &pen)) {
                            lineItem->setPen(pen);
                        }
                    } else if (QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(loadedItem)) {
                        applyShapeStyleState(rawLine, shapeItem);
                    } else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(loadedItem)) {
                        applyTextStyleState(rawLine, textItem);
                    }
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

                    QGraphicsItemGroup* group = new QGraphicsItemGroup();
                    for (QGraphicsItem* item : groupItems) {
                        group->addToGroup(item);
                    }
                    group->setHandlesChildEvents(false);
                    group->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    group->setFlag(QGraphicsItem::ItemIsMovable, true);
                    _graphicsView->getScene()->addItem(group);
                    _graphicsView->getScene()->getGraphicalGroups()->append(group);
                    _graphicsView->getScene()->insertOldPositionItem(group, group->pos());
                    for (QGraphicsItem* child : group->childItems()) {
                        _graphicsView->getScene()->insertOldPositionItem(child, child->pos());
                        child->setSelected(false);
                    }
                    group->setSelected(false);
                    if (!groupComponents.isEmpty()) {
                        _graphicsView->getScene()->insertComponentGroup(group, groupComponents);
                    }
                }
            }
        }

        _applyDiagramsVisibility();
        scene->setRestoringPersistedGuiLayout(false);
        if (hasPersistedViewState) {
            QTimer::singleShot(0, _ownerWidget, [this, restoredViewpointX, restoredViewpointY]() {
                QScrollBar* hBar = _graphicsView->horizontalScrollBar();
                QScrollBar* vBar = _graphicsView->verticalScrollBar();
                hBar->setValue(qBound(hBar->minimum(), restoredViewpointX, hBar->maximum()));
                vBar->setValue(qBound(vBar->minimum(), restoredViewpointY, vBar->maximum()));
            });
        }
        _graphicsView->getScene()->setPersistedGuiRestoreInProgress(false);

        _console->append("\n");
        *_modelFilename = QString::fromStdString(filename);
    }

    return model;
}

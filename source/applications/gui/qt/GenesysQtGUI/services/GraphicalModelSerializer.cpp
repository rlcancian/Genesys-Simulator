#include "GraphicalModelSerializer.h"
#include "GraphicalModelBuilder.h"

#include "../TraitsGUI.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../animations/AnimationCounter.h"
#include "../animations/AnimationPlaceholder.h"
#include "../animations/AnimationVariable.h"
#include "../animations/AnimationTimer.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginManager.h"

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
#include <QPointer>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStringConverter>
#include <QSlider>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <set>
#include <vector>

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

bool decodeComponentColor(const QString& token, QColor* color) {
    if (color == nullptr) {
        return false;
    }
    QRegularExpression colorRegex("\\s*color=(#[0-9A-Fa-f]{6,8})");
    QRegularExpressionMatch colorMatch = colorRegex.match(token);
    if (!colorMatch.hasMatch()) {
        return false;
    }
    QColor parsedColor(colorMatch.captured(1));
    if (!parsedColor.isValid()) {
        return false;
    }
    *color = parsedColor;
    return true;
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

QString stripGuiCommentPrefix(const QString& line) {
    QString normalized = line.trimmed();
    if (normalized.startsWith("#")) {
        normalized = normalized.mid(1).trimmed();
    }
    return normalized;
}

QString sectionHeaderLine(const QString& sectionName) {
    return QString("\n# %1\n").arg(sectionName);
}

QString sectionFooterLine() {
    return QString();
}

QString commentedLine(const QString& payload) {
    return QString("# %1").arg(payload);
}

bool containsPersistenceMarkers(const QString& text) {
    return text.contains("# Genesys Simulation Model")
           || text.contains("# Genesys Graphic Model")
           || text.contains("# 0 Show")
           || text.contains("# 0 GUI")
           || text.contains("# Draws")
           || text.contains("# Animations")
           || text.contains("# Graphical Plugins")
           || text.contains("# Graphical Model Data Definitions")
           || text.contains("# Graphical Model Components")
           || text.contains("# Groups")
           || text.contains("# Model Data Definitions")
           || text.contains("# Model Components");
}

QString recoverFromUtf16Mojibake(const QString& mojibakeText) {
    if (mojibakeText.isEmpty()) {
        return {};
    }

    QByteArray asBigEndianBytes;
    asBigEndianBytes.reserve(mojibakeText.size() * 2);
    QByteArray asLittleEndianBytes;
    asLittleEndianBytes.reserve(mojibakeText.size() * 2);

    for (QChar ch : mojibakeText) {
        const ushort codeUnit = ch.unicode();
        asBigEndianBytes.append(static_cast<char>((codeUnit >> 8) & 0xFF));
        asBigEndianBytes.append(static_cast<char>(codeUnit & 0xFF));
        asLittleEndianBytes.append(static_cast<char>(codeUnit & 0xFF));
        asLittleEndianBytes.append(static_cast<char>((codeUnit >> 8) & 0xFF));
    }

    const auto decodeCandidate = [](const QByteArray& bytes) {
        QString decoded = QString::fromUtf8(bytes);
        if (decoded.isEmpty()) {
            decoded = QString::fromLatin1(bytes);
        }
        return decoded;
    };

    QString recovered = decodeCandidate(asBigEndianBytes);
    if (containsPersistenceMarkers(recovered)) {
        return recovered;
    }

    recovered = decodeCandidate(asLittleEndianBytes);
    if (containsPersistenceMarkers(recovered)) {
        return recovered;
    }

    return {};
}

QString decodePersistedFileText(const QByteArray& rawBytes) {
    if (rawBytes.isEmpty()) {
        return {};
    }
    if (rawBytes.startsWith("\xEF\xBB\xBF")) {
        return QString::fromUtf8(rawBytes.mid(3));
    }
    if (rawBytes.startsWith("\xFF\xFE")) {
        QStringDecoder decoder(QStringConverter::Utf16LE);
        return decoder.decode(rawBytes.mid(2));
    }
    if (rawBytes.startsWith("\xFE\xFF")) {
        QStringDecoder decoder(QStringConverter::Utf16BE);
        return decoder.decode(rawBytes.mid(2));
    }

    // Backward compatibility: tolerate UTF-16 files without BOM from previous buggy saves.
    int zeroByteCount = 0;
    for (char byte : rawBytes) {
        if (byte == '\0') {
            ++zeroByteCount;
        }
    }
    if (zeroByteCount > rawBytes.size() / 8) {
        QStringDecoder decoder(QStringConverter::Utf16LE);
        const QString decoded = decoder.decode(rawBytes);
        if (!decoded.isEmpty()) {
            return decoded;
        }
    }

    QString decoded = QString::fromUtf8(rawBytes);
    if (containsPersistenceMarkers(decoded)) {
        return decoded;
    }

    // Recovery path for legacy mojibake where UTF-8 bytes were previously interpreted as UTF-16 code units.
    const QString recovered = recoverFromUtf16Mojibake(decoded);
    if (!recovered.isEmpty()) {
        return recovered;
    }

    return decoded;
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
                                                   QAction* actionShowEditableElements,
                                                   QAction* actionShowAttachedElements,
                                                   QAction* actionShowRecursiveElements,
                                                   QTextEdit* console,
                                                   QString* modelFilename,
                                                   std::function<void()> clearModelEditors,
                                                   std::function<void()> rebuildGraphicalModelFromModel,
                                                   std::function<void()> applyShowInternalElements,
                                                   std::function<void()> applyShowEditableElements,
                                                   std::function<void()> applyShowAttachedElements)
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
      _actionShowEditableElements(actionShowEditableElements),
      _actionShowAttachedElements(actionShowAttachedElements),
      _actionShowRecursiveElements(actionShowRecursiveElements),
      _console(console),
      _modelFilename(modelFilename),
      _clearModelEditors(std::move(clearModelEditors)),
      _rebuildGraphicalModelFromModel(std::move(rebuildGraphicalModelFromModel)),
      _applyShowInternalElements(std::move(applyShowInternalElements)),
      _applyShowEditableElements(std::move(applyShowEditableElements)),
      _applyShowAttachedElements(std::move(applyShowAttachedElements)) {}

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
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(false);

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

QString itemFlagsState(QGraphicsItem* item) {
    if (item == nullptr) {
        return {};
    }
    return QString("flags=(%1,%2)")
        .arg(item->flags().testFlag(QGraphicsItem::ItemIsSelectable) ? 1 : 0)
        .arg(item->flags().testFlag(QGraphicsItem::ItemIsMovable) ? 1 : 0);
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

void applyItemFlagsState(const QString& line, QGraphicsItem* item) {
    if (item == nullptr) {
        return;
    }
    QRegularExpression regexFlags("\\bflags=\\((\\d+),(\\d+)\\)");
    QRegularExpressionMatch match = regexFlags.match(line);
    if (!match.hasMatch()) {
        return;
    }
    item->setFlag(QGraphicsItem::ItemIsSelectable, match.captured(1).toInt() != 0);
    item->setFlag(QGraphicsItem::ItemIsMovable, match.captured(2).toInt() != 0);
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
    const QStringList tokens = line.split("\t");
    for (const QString& tokenRaw : tokens) {
        const QString token = tokenRaw.trimmed();
        if (token.startsWith("textcolor=")) {
            const QString colorValue = token.mid(QString("textcolor=").size()).trimmed();
            item->setDefaultTextColor(QColor(colorValue));
            continue;
        }
        if (token.startsWith("font=")) {
            const QString encodedFont = token.mid(QString("font=").size()).trimmed();
            QFont font;
            const QString decodedFont = decodePersistedText(encodedFont);
            if (font.fromString(decodedFont)) {
                item->setFont(font);
            }
            continue;
        }
        if (token.startsWith("textwidth=")) {
            const QString widthValue = token.mid(QString("textwidth=").size()).trimmed();
            item->setTextWidth(widthValue.toDouble());
            continue;
        }
    }
}

// Persist the complete graphical model, including view options and overlays.
bool GraphicalModelSerializer::saveGraphicalModel(const QString& filename) const {
    try {
        if (_simulator == nullptr || _simulator->getModelManager() == nullptr || _simulator->getModelManager()->current() == nullptr) {
            return false;
        }

        // Persist the canonical kernel state in GEN format first, using the .gui filename.
        if (!_simulator->getModelManager()->saveModel(filename.toStdString())) {
            QMessageBox::warning(_ownerWidget, QObject::tr("Save Model"), QObject::tr("Could not save kernel model to file."));
            return false;
        }

        QFile saveFile(filename);
        if (!saveFile.open(QIODevice::Append | QIODevice::Text)) {
            QMessageBox::information(_ownerWidget, QObject::tr("Unable to access file to save"),
                                     saveFile.errorString());
            return false;
        }

        QTextStream out(&saveFile);
        out.setEncoding(QStringConverter::Utf8);
        out.setGenerateByteOrderMark(false);
        out << Qt::endl;
        out << "# Genesys Graphic Model" << Qt::endl;
        out << commentedLine(QString("0 GUI app=GenesysQtGUI, qt=%1, savedAt=%2")
                                 .arg(QT_VERSION_STR)
                                 .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)))
            << Qt::endl;
        QString line = "0 Show ";
        line += "zoom=" + QString::number(_zoomSlider->value());
        line += ", grid=" + QString::number(_actionShowGrid->isChecked());
        line += ", rule=" + QString::number(_actionShowRule->isChecked());
        line += ", snap=" + QString::number(_actionShowSnap->isChecked());
        line += ", guides=" + QString::number(_actionShowGuides->isChecked());
        line += ", internals=" + QString::number(_actionShowInternalElements->isChecked()
                                                 || _actionShowEditableElements->isChecked());
        line += ", attached=" + QString::number(_actionShowAttachedElements->isChecked());
        line += ", statistics=" + QString::number(_actionShowInternalElements->isChecked());
        line += ", editable=" + QString::number(_actionShowEditableElements->isChecked());
        line += ", shared=" + QString::number(_actionShowAttachedElements->isChecked());
        line += ", recursive=" + QString::number(_actionShowRecursiveElements->isChecked());
        line += ", viewpoint=(" + QString::number(_graphicsView->horizontalScrollBar()->value()) + "," + QString::number(_graphicsView->verticalScrollBar()->value()) + ")";
        out << commentedLine(line) << Qt::endl;

        ModelGraphicsScene* scene = static_cast<ModelGraphicsScene*>(_graphicsView->getScene());

        if (scene) {
            QHash<QGraphicsItem*, int> persistedItemIds;
            int nextAutonomousItemId = -1;

            out << sectionHeaderLine("Draws");
            QList<QGraphicsItem*>* geometries = scene->getGraphicalGeometries();
            if (geometries != nullptr) {
                for (QGraphicsItem* item : *geometries) {
                    if (item == nullptr) {
                        continue;
                    }
                    const int persistedId = nextAutonomousItemId--;
                    persistedItemIds.insert(item, persistedId);

                    if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
                        const QLineF l = lineItem->line();
                        line = QString("%1 \t DrawLine \t line=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9 \t %10")
                                   .arg(persistedId)
                                   .arg(l.x1(), 0, 'f', 4).arg(l.y1(), 0, 'f', 4)
                                   .arg(l.x2(), 0, 'f', 4).arg(l.y2(), 0, 'f', 4)
                                   .arg(lineItem->pos().x(), 0, 'f', 4).arg(lineItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(lineItem))
                                   .arg(itemFlagsState(lineItem))
                                   .arg(penState(lineItem->pen()));
                        out << commentedLine(line) << Qt::endl;
                    } else if (QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
                        QRectF r = rectItem->rect().normalized();
                        line = QString("%1 \t DrawRectangle \t rect=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9 \t %10 \t %11")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(rectItem->pos().x(), 0, 'f', 4).arg(rectItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(rectItem))
                                   .arg(itemFlagsState(rectItem))
                                   .arg(penState(rectItem->pen()))
                                   .arg(brushState(rectItem->brush()));
                        out << commentedLine(line) << Qt::endl;
                    } else if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                        QRectF r = ellipseItem->rect().normalized();
                        line = QString("%1 \t DrawEllipse \t rect=(%2,%3,%4,%5) \t pos=(%6,%7) \t %8 \t %9 \t %10 \t %11")
                                   .arg(persistedId)
                                   .arg(r.x(), 0, 'f', 4).arg(r.y(), 0, 'f', 4)
                                   .arg(r.width(), 0, 'f', 4).arg(r.height(), 0, 'f', 4)
                                   .arg(ellipseItem->pos().x(), 0, 'f', 4).arg(ellipseItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(ellipseItem))
                                   .arg(itemFlagsState(ellipseItem))
                                   .arg(penState(ellipseItem->pen()))
                                   .arg(brushState(ellipseItem->brush()));
                        out << commentedLine(line) << Qt::endl;
                    } else if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
                        QStringList points;
                        const QPolygonF polygon = polygonItem->polygon();
                        for (const QPointF& p : polygon) {
                            points << QString("%1,%2").arg(p.x(), 0, 'f', 4).arg(p.y(), 0, 'f', 4);
                        }
                        line = QString("%1 \t DrawPolygon \t points=%2 \t pos=(%3,%4) \t %5 \t %6 \t %7 \t %8")
                                   .arg(persistedId)
                                   .arg(points.join(";"))
                                   .arg(polygonItem->pos().x(), 0, 'f', 4).arg(polygonItem->pos().y(), 0, 'f', 4)
                                   .arg(commonGraphicsState(polygonItem))
                                   .arg(itemFlagsState(polygonItem))
                                   .arg(penState(polygonItem->pen()))
                                   .arg(brushState(polygonItem->brush()));
                        out << commentedLine(line) << Qt::endl;
                    } else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                        line = QString("%1 \t DrawText \t value=").arg(persistedId);
                        line += encodeGuiText(textItem->toPlainText());
                        line += QString(" \t pos=(%1,%2) \t %3 \t %4 \t textcolor=%5 \t font=")
                                    .arg(textItem->pos().x(), 0, 'f', 4)
                                    .arg(textItem->pos().y(), 0, 'f', 4)
                                    .arg(commonGraphicsState(textItem))
                                    .arg(itemFlagsState(textItem))
                                    .arg(textItem->defaultTextColor().name(QColor::HexArgb));
                        line += encodePersistedText(textItem->font().toString());
                        line += QString(" \t textwidth=%1").arg(textItem->textWidth(), 0, 'f', 4);
                        out << commentedLine(line) << Qt::endl;
                    }
                }
            }
            out << sectionFooterLine();

            out << sectionHeaderLine("Animations");
            QList<AnimationCounter*>* counters = scene->getAnimationsCounter();
            if (counters != nullptr) {
                for (AnimationCounter* counter : *counters) {
                    if (counter == nullptr) {
                        continue;
                    }
                    const int persistedId = nextAutonomousItemId--;
                    persistedItemIds.insert(counter, persistedId);
                    int idCounter = -1;
                    if (counter->getCounter() != nullptr) {
                        idCounter = counter->getCounter()->getId();
                    }
                    line = QString("%1 \t AnimationCounter \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                               .arg(persistedId)
                               .arg(idCounter)
                               .arg(counter->scenePos().x(), 0, 'f', 2)
                               .arg(counter->scenePos().y(), 0, 'f', 2)
                               .arg(counter->boundingRect().width(), 0, 'f', 2)
                               .arg(counter->boundingRect().height(), 0, 'f', 2);
                    out << commentedLine(line) << Qt::endl;
                }
            }

            QList<AnimationVariable*>* variables = scene->getAnimationsVariable();
            if (variables != nullptr) {
                for (AnimationVariable* variable : *variables) {
                    if (variable == nullptr) {
                        continue;
                    }
                    const int persistedId = nextAutonomousItemId--;
                    persistedItemIds.insert(variable, persistedId);
                    int idVariable = -1;
                    if (variable->getVariable() != nullptr) {
                        idVariable = variable->getVariable()->getId();
                    }
                    line = QString("%1 \t AnimationVariable \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                               .arg(persistedId)
                               .arg(idVariable)
                               .arg(variable->scenePos().x(), 0, 'f', 2)
                               .arg(variable->scenePos().y(), 0, 'f', 2)
                               .arg(variable->boundingRect().width(), 0, 'f', 2)
                               .arg(variable->boundingRect().height(), 0, 'f', 2);
                    out << commentedLine(line) << Qt::endl;
                }
            }

            QList<AnimationTimer*>* timers = scene->getAnimationsTimer();
            if (timers != nullptr) {
                for (AnimationTimer* timer : *timers) {
                    if (timer == nullptr) {
                        continue;
                    }
                    const int persistedId = nextAutonomousItemId--;
                    persistedItemIds.insert(timer, persistedId);
                    line = QString("%1 \t AnimationTimer \t hour=%2 \t minute=%3 \t second=%4 \t format=%5 \t position=(%6,%7) \t width=%8 \t height=%9")
                               .arg(persistedId)
                               .arg(timer->getInitialHours())
                               .arg(timer->getInitialMinutes())
                               .arg(timer->getInitialSeconds())
                               .arg(static_cast<unsigned int>(timer->getTimeFormat()))
                               .arg(timer->scenePos().x(), 0, 'f', 2)
                               .arg(timer->scenePos().y(), 0, 'f', 2)
                               .arg(timer->boundingRect().width(), 0, 'f', 2)
                               .arg(timer->boundingRect().height(), 0, 'f', 2);
                    out << commentedLine(line) << Qt::endl;
                }
            }

            QList<AnimationPlaceholder*>* placeholders = scene->getAnimationsPlaceholder();
            if (placeholders != nullptr) {
                for (AnimationPlaceholder* placeholder : *placeholders) {
                    if (placeholder == nullptr) {
                        continue;
                    }
                    const int persistedId = nextAutonomousItemId--;
                    persistedItemIds.insert(placeholder, persistedId);
                    line = QString("%1 \t AnimationPlaceholder \t type=").arg(persistedId);
                    line += encodeGuiText(placeholder->getAnimationType());
                    line += " \t target=";
                    line += encodeGuiText(placeholder->getTargetName());
                    line += QString(" \t position=(%1,%2) \t width=%3 \t height=%4")
                                .arg(placeholder->scenePos().x(), 0, 'f', 2)
                                .arg(placeholder->scenePos().y(), 0, 'f', 2)
                                .arg(placeholder->boundingRect().width(), 0, 'f', 2)
                                .arg(placeholder->boundingRect().height(), 0, 'f', 2);
                    out << commentedLine(line) << Qt::endl;
                }
            }
            out << sectionFooterLine();

            out << sectionHeaderLine("Graphical Plugins");
            out << sectionFooterLine();

            out << sectionHeaderLine("Graphical Model Data Definitions");
            QList<QGraphicsItem*>* graphicalDataDefinitions = scene->getGraphicalModelDataDefinitions();
            if (graphicalDataDefinitions != nullptr) {
                for (QGraphicsItem* item : *graphicalDataDefinitions) {
                    GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
                    if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
                        continue;
                    }

                    ModelDataDefinition* dataDefinition = gmdd->getDataDefinition();
                    const int persistedId = static_cast<int>(dataDefinition->getId());
                    persistedItemIds.insert(gmdd, persistedId);
                    line = QString("%1 \t %2 \t name=%3 \t position=(%4,%5)")
                               .arg(persistedId)
                               .arg(QString::fromStdString(dataDefinition->getClassname()))
                               .arg(encodeGuiText(QString::fromStdString(dataDefinition->getName())))
                               .arg(gmdd->scenePos().x(), 0, 'f', 4)
                               .arg(gmdd->scenePos().y(), 0, 'f', 4);
                    out << commentedLine(line) << Qt::endl;
                }
            }
            out << sectionFooterLine();

            out << sectionHeaderLine("Graphical Model Components");
            for (QGraphicsItem* item : *scene->getGraphicalModelComponents()) {
                GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item);
                if (gmc == nullptr || gmc->getComponent() == nullptr) {
                    continue;
                }
                const int persistedId = static_cast<int>(gmc->getComponent()->getId());
                persistedItemIds.insert(gmc, persistedId);
                line = QString("%1 \t %2 \t name=%3 \t color=%4 \t position=(%5,%6) \t ports=%7")
                           .arg(persistedId)
                           .arg(QString::fromStdString(gmc->getComponent()->getClassname()))
                           .arg(encodeGuiText(QString::fromStdString(gmc->getComponent()->getName())))
                           .arg(gmc->getColor().name(QColor::HexArgb))
                           .arg(gmc->scenePos().x(), 0, 'f', 4)
                           .arg(gmc->scenePos().y() + gmc->getHeight() / 2.0, 0, 'f', 4)
                           .arg(encodePortPositions(gmc));
                out << commentedLine(line) << Qt::endl;
            }
            out << sectionFooterLine();

            out << sectionHeaderLine("Groups");
            QList<QGraphicsItemGroup*>* groups = scene->getGraphicalGroups();
            if (groups != nullptr) {
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
                    if (members.isEmpty()) {
                        continue;
                    }
                    const int groupId = nextAutonomousItemId--;
                    persistedItemIds.insert(group, groupId);
                    line = QString("%1 \t Group \t %2").arg(groupId).arg(members.join(" "));
                    out << commentedLine(line) << Qt::endl;
                }
            }
            out << sectionFooterLine();
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

    if (extension.compare("gui", Qt::CaseInsensitive) != 0) {
        model = _simulator->getModelManager()->loadModel(file.fileName().toStdString());
        if (model != nullptr) {
            _rebuildGraphicalModelFromModel();
        }
        return model;
    }

    QString content = decodePersistedFileText(file.readAll());
    file.close();

    QStringList lines = content.split("\n");
    QStringList simulLang;
    QStringList gui;
    QStringList animations;
    QStringList counters;
    QStringList variables;
    QStringList timers;
    QStringList placeholders;
    QStringList geometries;
    QStringList groups;
    QStringList dataDefinitions;
    QStringList components;

    enum class GuiSection {
        Header,
        Draws,
        Animations,
        Plugins,
        DataDefinitions,
        Components,
        Groups,
        LegacyCounters,
        LegacyVariables,
        LegacyTimers,
        LegacyPlaceholders,
        LegacyGeometries,
        LegacyDataDefinitions,
        LegacyGroups
    };

    bool guiBlockStarted = false;
    GuiSection activeSection = GuiSection::Header;

    for (const QString& line : lines) {
        const QString normalized = stripGuiCommentPrefix(line);

        if (!guiBlockStarted) {
            if (normalized.compare("Genesys Graphic Model", Qt::CaseInsensitive) == 0
                || normalized.compare("Genegys Graphic Model", Qt::CaseInsensitive) == 0) {
                guiBlockStarted = true;
                activeSection = GuiSection::Header;
                continue;
            }
            simulLang.append(line);
            continue;
        }

        if (normalized.isEmpty()) {
            continue;
        }
        if (normalized == "[" || normalized == "]0+") {
            continue;
        }

        if (normalized.compare("Draws", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::Draws;
            continue;
        }
        if (normalized.compare("Animations", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::Animations;
            continue;
        }
        if (normalized.compare("Graphical Plugins", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::Plugins;
            continue;
        }
        if (normalized.compare("Graphical Model Data Definitions", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::DataDefinitions;
            continue;
        }
        if (normalized.compare("Graphical Model Components", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::Components;
            continue;
        }
        if (normalized.compare("Groups", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::Groups;
            continue;
        }

        // Legacy section names for backward compatibility.
        if (normalized.compare("Counters", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyCounters;
            continue;
        }
        if (normalized.compare("Variables", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyVariables;
            continue;
        }
        if (normalized.compare("Timers", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyTimers;
            continue;
        }
        if (normalized.compare("AnimationPlaceholders", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyPlaceholders;
            continue;
        }
        if (normalized.compare("Geometries", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyGeometries;
            continue;
        }
        if (normalized.compare("DataDefinitions", Qt::CaseInsensitive) == 0) {
            activeSection = GuiSection::LegacyDataDefinitions;
            continue;
        }

        switch (activeSection) {
            case GuiSection::Draws:
                geometries.append(normalized);
                break;
            case GuiSection::Animations:
                animations.append(normalized);
                break;
            case GuiSection::Plugins:
                break;
            case GuiSection::DataDefinitions:
                dataDefinitions.append(normalized);
                break;
            case GuiSection::Components:
                components.append(normalized);
                break;
            case GuiSection::Groups:
                groups.append(normalized);
                break;
            case GuiSection::LegacyCounters:
                counters.append(normalized);
                break;
            case GuiSection::LegacyVariables:
                variables.append(normalized);
                break;
            case GuiSection::LegacyTimers:
                timers.append(normalized);
                break;
            case GuiSection::LegacyPlaceholders:
                placeholders.append(normalized);
                break;
            case GuiSection::LegacyGeometries:
                geometries.append(normalized);
                break;
            case GuiSection::LegacyDataDefinitions:
                dataDefinitions.append(normalized);
                break;
            case GuiSection::LegacyGroups:
                groups.append(normalized);
                break;
            case GuiSection::Header:
            default:
                gui.append(normalized);
                break;
        }
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString newFilename = QString("/tempFile-%1.gen").arg(currentDateTime.toString("yyyy-MM-dd-hh-mm-ss"));
    QString filePath = QDir::tempPath() + newFilename;
    QFile tempFile(filePath);
    tempFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream outStream(&tempFile);
    outStream.setEncoding(QStringConverter::Utf8);
    outStream.setGenerateByteOrderMark(false);
    for (const QString& line : simulLang) {
        outStream << line << Qt::endl;
    }
    outStream.flush();

    model = _simulator->getModelManager()->loadModel(tempFile.fileName().toStdString());
    tempFile.close();
    QFile::remove(tempFile.fileName());

    if (model != nullptr) {
        ModelGraphicsScene* scene = _graphicsView->getScene();
        class ScopedPersistedGuiRestore {
        public:
            explicit ScopedPersistedGuiRestore(ModelGraphicsScene* guardedScene) : _guardedScene(guardedScene) {
                if (_guardedScene != nullptr) {
                    _guardedScene->setRestoringPersistedGuiLayout(true);
                    _guardedScene->setPersistedGuiRestoreInProgress(true);
                }
            }

            ~ScopedPersistedGuiRestore() {
                if (_guardedScene != nullptr) {
                    _guardedScene->setRestoringPersistedGuiLayout(false);
                    _guardedScene->setPersistedGuiRestoreInProgress(false);
                }
            }

        private:
            ModelGraphicsScene* _guardedScene = nullptr;
        };

        struct PersistedComponentState {
            Util::identification componentId = 0;
            QPointF position;
            QColor color;
            bool hasColor = false;
            int itemId = 0;
            QHash<QString, QPointF> portPositions;
        };

        struct PersistedDataDefinitionState {
            Util::identification dataId = 0;
            QString className;
            QString name;
            QPointF position;
            int itemId = 0;
        };

        bool hasPersistedViewState = false;
        int restoredViewpointX = 0;
        int restoredViewpointY = 0;
        QHash<int, QGraphicsItem*> persistedItems;
        QHash<Util::identification, PersistedComponentState> persistedComponentsById;
        QHash<Util::identification, PersistedDataDefinitionState> persistedDataDefinitionsById;
        QHash<QString, PersistedDataDefinitionState> persistedDataDefinitionsByClassAndName;

        const auto applyViewStateFromAttributes = [&](const QString& attributes) {
            const int zoom = QRegularExpression("zoom=(-?\\d+)").match(attributes).captured(1).toInt();
            const int grid = QRegularExpression("grid=(\\d+)").match(attributes).captured(1).toInt();
            const int rule = QRegularExpression("rule=(\\d+)").match(attributes).captured(1).toInt();
            const int snap = QRegularExpression("snap=(\\d+)").match(attributes).captured(1).toInt();
            const int guides = QRegularExpression("guides=(\\d+)").match(attributes).captured(1).toInt();
            const int internals = QRegularExpression("internals=(\\d+)").match(attributes).captured(1).toInt();
            const int attached = QRegularExpression("attached=(\\d+)").match(attributes).captured(1).toInt();
            QRegularExpressionMatch statisticsMatch = QRegularExpression("statistics=(\\d+)").match(attributes);
            QRegularExpressionMatch editableMatch = QRegularExpression("editable=(\\d+)").match(attributes);
            QRegularExpressionMatch sharedMatch = QRegularExpression("shared=(\\d+)").match(attributes);
            QRegularExpressionMatch recursiveMatch = QRegularExpression("recursive=(\\d+)").match(attributes);
            const bool showStatistics = statisticsMatch.hasMatch()
                                            ? statisticsMatch.captured(1).toInt() != 0
                                            : internals != 0;
            const bool showEditable = editableMatch.hasMatch()
                                          ? editableMatch.captured(1).toInt() != 0
                                          : internals != 0;
            const bool showShared = sharedMatch.hasMatch()
                                        ? sharedMatch.captured(1).toInt() != 0
                                        : attached != 0;
            const bool showRecursive = recursiveMatch.hasMatch()
                                           ? recursiveMatch.captured(1).toInt() != 0
                                           : true;
            QRegularExpressionMatch viewpointMatch =
                QRegularExpression("viewpoint=\\(([-+]?\\d+\\.?\\d*),([-+]?\\d+\\.?\\d*)\\)").match(attributes);
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
            _actionShowInternalElements->setChecked(showStatistics);
            scene->setShowStatisticsDataDefinitions(showStatistics);
            _actionShowEditableElements->setChecked(showEditable);
            scene->setShowEditableDataDefinitions(showEditable);
            _actionShowAttachedElements->setChecked(showShared);
            scene->setShowSharedDataDefinitions(showShared);
            _actionShowRecursiveElements->setChecked(showRecursive);
            scene->setShowRecursiveDataDefinitions(showRecursive);

            const int minZoom = _zoomSlider->minimum();
            const int maxZoom = _zoomSlider->maximum();
            const int clampedZoom = qBound(minZoom, zoom, maxZoom);
            // Force transform recomputation even when the persisted zoom equals the current slider value.
            if (_zoomSlider->value() == clampedZoom) {
                const int nudgedZoom = (clampedZoom < maxZoom) ? (clampedZoom + 1) : (clampedZoom - 1);
                if (nudgedZoom >= minZoom && nudgedZoom <= maxZoom) {
                    _zoomSlider->setValue(nudgedZoom);
                }
            }
            _zoomSlider->setValue(clampedZoom);

            hasPersistedViewState = true;
            restoredViewpointX = viewpointX;
            restoredViewpointY = viewpointY;
        };

        // Apply persisted visual flags before rebuilding any graphical layers.
        for (const QString& guiLine : gui) {
            const QString line = guiLine.trimmed();
            if (line.isEmpty()) {
                continue;
            }
            QRegularExpressionMatch showMatch = QRegularExpression("^0\\s+(Show|View)\\s+(.*)$").match(line);
            if (showMatch.hasMatch()) {
                applyViewStateFromAttributes(showMatch.captured(2));
            }
        }

        // Force eager materialization/check of all modeldata (including dynamic internals/attached)
        // before rebuilding and restoring the graphical layer from persisted GUI records.
        const auto materializeAllRelatedDataDefinitions = [&]() {
            if (model == nullptr || model->getDataManager() == nullptr) {
                return;
            }

            std::set<Util::identification> processedDefinitions;
            bool discoveredNewDefinitions = false;
            do {
                discoveredNewDefinitions = false;
                std::vector<ModelDataDefinition*> snapshot;
                for (const std::string& className : model->getDataManager()->getDataDefinitionClassnames()) {
                    List<ModelDataDefinition*>* definitions =
                        model->getDataManager()->getDataDefinitionList(className);
                    if (definitions == nullptr) {
                        continue;
                    }
                    for (ModelDataDefinition* definition : *definitions->list()) {
                        if (definition != nullptr) {
                            snapshot.push_back(definition);
                        }
                    }
                }

                for (ModelDataDefinition* definition : snapshot) {
                    if (definition == nullptr || processedDefinitions.count(definition->getId()) > 0) {
                        continue;
                    }
                    processedDefinitions.insert(definition->getId());
                    std::string ignoredError;
                    ModelDataDefinition::CreateRelatedDataElements(definition, ignoredError);
                    discoveredNewDefinitions = true;
                }

                if (model->getComponentManager() != nullptr) {
                    for (ModelComponent* component : *model->getComponentManager()->getAllComponents()) {
                        if (component == nullptr || processedDefinitions.count(component->getId()) > 0) {
                            continue;
                        }
                        processedDefinitions.insert(component->getId());
                        std::string ignoredError;
                        ModelDataDefinition::CreateRelatedDataElements(component, ignoredError);
                        discoveredNewDefinitions = true;
                    }
                }
            } while (discoveredNewDefinitions);
        };

        materializeAllRelatedDataDefinitions();

        // Materialize internal/attached data definitions before restoring persisted graphical layout.
        model->check();

        // Rebuild the base graphical topology from a single source of truth before applying persisted GUI overlays.
        _clearModelEditors();
        // Keep persisted-layout restore flags exception-safe during the whole reconstruction sequence.
        ScopedPersistedGuiRestore scopedPersistedGuiRestore(scene);
        _rebuildGraphicalModelFromModel();

        const auto parsePersistedComponentLine = [&](const QString& rawLine) {
            const QString line = rawLine.trimmed();
            if (line.isEmpty()) {
                return;
            }
            QStringList split = line.split("\t", Qt::SkipEmptyParts);
            if (split.size() < 2) {
                return;
            }

            PersistedComponentState state;
            bool idOk = false;
            state.componentId = split[0].trimmed().toULongLong(&idOk);
            if (!idOk || state.componentId == 0) {
                return;
            }

            QPointF position;
            bool hasPosition = false;
            for (int i = 2; i < split.size(); ++i) {
                const QString token = split[i].trimmed();

                QColor parsedColor;
                if (decodeComponentColor(token, &parsedColor)) {
                    state.color = parsedColor;
                    state.hasColor = true;
                    continue;
                }

                QRegularExpressionMatch posMatch =
                    QRegularExpression("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)").match(token);
                if (posMatch.hasMatch()) {
                    position.setX(posMatch.captured(1).toDouble());
                    position.setY(posMatch.captured(2).toDouble());
                    hasPosition = true;
                    continue;
                }

                QRegularExpressionMatch legacyPosMatch =
                    QRegularExpression("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)").match(token);
                if (legacyPosMatch.hasMatch()) {
                    position.setX(legacyPosMatch.captured(1).toDouble());
                    position.setY(legacyPosMatch.captured(3).toDouble());
                    hasPosition = true;
                    continue;
                }

                QRegularExpressionMatch itemIdMatch = QRegularExpression("itemid=(-?\\d+)").match(token);
                if (itemIdMatch.hasMatch()) {
                    state.itemId = itemIdMatch.captured(1).toInt();
                    continue;
                }

                if (token.startsWith("ports=")) {
                    state.portPositions = decodePortPositions(token);
                    continue;
                }
            }

            if (!hasPosition) {
                return;
            }
            state.position = position;
            persistedComponentsById.insert(state.componentId, state);
        };

        for (const QString& guiLine : gui) {
            const QString line = guiLine.trimmed();
            if (line.isEmpty()) {
                continue;
            }

            QRegularExpressionMatch showMatch = QRegularExpression("^0\\s+(Show|View)\\s+(.*)$").match(line);
            if (showMatch.hasMatch()) {
                applyViewStateFromAttributes(showMatch.captured(2));
                continue;
            }

            if (line.startsWith("0 GUI", Qt::CaseInsensitive)) {
                continue;
            }

            // Backward compatibility: legacy files kept component lines in the header section.
            parsePersistedComponentLine(line);
        }

        for (const QString& componentLine : components) {
            parsePersistedComponentLine(componentLine);
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
            if (state.hasColor) {
                existingComponent->setColor(state.color);
            }
            restorePortPositions(existingComponent, state.portPositions);
            persistedItems.insert(static_cast<int>(state.componentId), existingComponent);
            if (state.itemId != 0) {
                persistedItems.insert(state.itemId, existingComponent);
            }
        }

        // Rebuild data-definition visibility after applying persisted header flags and component positions.
        // Persisted GMDD positions are restored immediately after this synchronization pass.
        GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_simulator, scene);

        if (!dataDefinitions.empty()) {
            // Parse persisted data definition layout metadata for id-based and fallback lookup.
            QRegularExpression regexDataId("\\s*dataid=(\\d+)");
            QRegularExpression regexClassname("\\s*classname=([^\\t]+)");
            QRegularExpression regexName("\\s*name=([^\\t]+)");
            QRegularExpression regexPosition("\\s*position=\\(([^,]+),([^\\)]+)\\)");
            QRegularExpression regexItemId("\\s*itemid=(-?\\d+)");

            for (const QString& rawLine : dataDefinitions) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }

                QStringList tokens = rawLine.split("\t", Qt::SkipEmptyParts);
                if (tokens.size() < 2) {
                    continue;
                }

                PersistedDataDefinitionState state;
                bool hasPosition = false;

                bool startsWithLegacyPrefix = tokens[0].trimmed().compare("DataDefinition", Qt::CaseInsensitive) == 0;
                int tokenIndex = 0;
                if (startsWithLegacyPrefix) {
                    tokenIndex = 1;
                } else {
                    bool idOk = false;
                    state.dataId = tokens[0].trimmed().toULongLong(&idOk);
                    if (idOk) {
                        tokenIndex = 1;
                    }
                    if (tokens.size() >= 2) {
                        state.className = tokens[1].trimmed();
                        tokenIndex = 2;
                    }
                }

                for (int i = tokenIndex; i < tokens.size(); ++i) {
                    const QString token = tokens[i].trimmed();

                    if (state.dataId == 0) {
                        bool idOk = false;
                        Util::identification parsedId = token.toULongLong(&idOk);
                        if (idOk && parsedId > 0) {
                            state.dataId = parsedId;
                            continue;
                        }
                    }
                    if (state.className.isEmpty() && !token.contains("=") && token != "DataDefinition") {
                        state.className = token;
                        continue;
                    }

                    QRegularExpressionMatch dataIdMatch = regexDataId.match(token);
                    if (dataIdMatch.hasMatch()) {
                        state.dataId = dataIdMatch.captured(1).toULongLong();
                        continue;
                    }

                    QRegularExpressionMatch classMatch = regexClassname.match(token);
                    if (classMatch.hasMatch()) {
                        state.className = classMatch.captured(1).trimmed();
                        continue;
                    }

                    QRegularExpressionMatch nameMatch = regexName.match(token);
                    if (nameMatch.hasMatch()) {
                        state.name = decodeGuiText(nameMatch.captured(1).trimmed());
                        continue;
                    }

                    QRegularExpressionMatch posMatch = regexPosition.match(token);
                    if (posMatch.hasMatch()) {
                        state.position = QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble());
                        hasPosition = true;
                        continue;
                    }

                    QRegularExpressionMatch itemIdMatch = regexItemId.match(token);
                    if (itemIdMatch.hasMatch()) {
                        state.itemId = itemIdMatch.captured(1).toInt();
                        continue;
                    }
                }

                if (!hasPosition || state.dataId == 0 || state.className.trimmed().isEmpty()) {
                    continue;
                }
                if (state.dataId > 0) {
                    persistedDataDefinitionsById.insert(state.dataId, state);
                }
                persistedDataDefinitionsByClassAndName.insert(state.className + "#" + state.name, state);
            }

            const auto resolvePersistedDataDefinition =
                [&](const PersistedDataDefinitionState& state) -> ModelDataDefinition* {
                if (model == nullptr || model->getDataManager() == nullptr || state.className.trimmed().isEmpty()) {
                    return nullptr;
                }
                ModelDataManager* dataManager = model->getDataManager();
                const std::string className = state.className.trimmed().toStdString();
                if (state.dataId > 0) {
                    ModelDataDefinition* byId = dataManager->getDataDefinition(className, state.dataId);
                    if (byId != nullptr) {
                        return byId;
                    }
                }
                if (!state.name.trimmed().isEmpty()) {
                    return dataManager->getDataDefinition(className, state.name.toStdString());
                }
                return nullptr;
            };

            const auto applyPersistedDataDefinitionState =
                [&](const PersistedDataDefinitionState& state) {
                ModelDataDefinition* dataDefinition = resolvePersistedDataDefinition(state);
                if (dataDefinition == nullptr) {
                    return;
                }

                GraphicalModelDataDefinition* gmdd = scene->findGraphicalModelDataDefinition(dataDefinition);
                if (gmdd == nullptr && _simulator != nullptr && _simulator->getPluginManager() != nullptr) {
                    Plugin* plugin = _simulator->getPluginManager()->find(dataDefinition->getClassname());
                    if (plugin != nullptr) {
                        gmdd = scene->addGraphicalModelDataDefinition(plugin,
                                                                      dataDefinition,
                                                                      state.position,
                                                                      QColor(220, 220, 220));
                    }
                }
                if (gmdd == nullptr) {
                    return;
                }

                gmdd->setPos(state.position);
                gmdd->setOldPosition(state.position.x(), state.position.y());
                if (state.dataId > 0) {
                    persistedItems.insert(static_cast<int>(state.dataId), gmdd);
                }
                if (state.itemId != 0) {
                    persistedItems.insert(state.itemId, gmdd);
                }
            };

            // First apply persisted data-definition positions to canonical items.
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
                        applyPersistedDataDefinitionState(byId.value());
                        applied = true;
                    }

                    if (!applied) {
                        const QString fallbackKey = QString::fromStdString(dataDefinition->getClassname()) + "#"
                                                    + QString::fromStdString(dataDefinition->getName());
                        auto byClassAndName = persistedDataDefinitionsByClassAndName.find(fallbackKey);
                        if (byClassAndName != persistedDataDefinitionsByClassAndName.end()) {
                            applyPersistedDataDefinitionState(byClassAndName.value());
                        }
                    }
                }
            }

            // Ensure persisted GMDD entries are not silently dropped when canonical sync does not materialize them.
            for (auto byId = persistedDataDefinitionsById.constBegin();
                 byId != persistedDataDefinitionsById.constEnd();
                 ++byId) {
                applyPersistedDataDefinitionState(byId.value());
            }
            for (auto byClassAndName = persistedDataDefinitionsByClassAndName.constBegin();
                 byClassAndName != persistedDataDefinitionsByClassAndName.constEnd();
                 ++byClassAndName) {
                applyPersistedDataDefinitionState(byClassAndName.value());
            }
        }

        if (!animations.empty()) {
            QRegularExpression regexPosition("\\s*position=\\(([^,]+),([^\\)]+)\\)");
            QRegularExpression regexSize("\\s*width=([^\\t]+)\\s*\\t\\s*height=([^\\t]+)");

            for (const QString& rawLine : animations) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }
                const QStringList tokens = rawLine.split("\t", Qt::SkipEmptyParts);
                if (tokens.size() < 2) {
                    continue;
                }

                bool idOk = false;
                const int persistedId = tokens[0].trimmed().toInt(&idOk);
                if (!idOk) {
                    continue;
                }
                const QString animationType = tokens[1].trimmed();

                if (animationType.compare("AnimationCounter", Qt::CaseInsensitive) == 0) {
                    AnimationCounter* counter = new AnimationCounter();
                    QRegularExpressionMatch idMatch = QRegularExpression("\\s*id=(-?\\d+)").match(rawLine);
                    QRegularExpressionMatch posMatch = regexPosition.match(rawLine);
                    QRegularExpressionMatch sizeMatch = regexSize.match(rawLine);
                    if (idMatch.hasMatch() && posMatch.hasMatch() && sizeMatch.hasMatch()) {
                        counter->setIdCounter(idMatch.captured(1).toInt());
                        counter->setRect(QRectF(0, 0, sizeMatch.captured(1).toDouble(), sizeMatch.captured(2).toDouble()).normalized());
                        counter->setPos(QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble()));
                        _graphicsView->getScene()->getAnimationsCounter()->append(counter);
                        _graphicsView->getScene()->addItem(counter);
                        persistedItems.insert(persistedId, counter);
                    } else {
                        delete counter;
                    }
                    continue;
                }

                if (animationType.compare("AnimationVariable", Qt::CaseInsensitive) == 0) {
                    AnimationVariable* variable = new AnimationVariable();
                    QRegularExpressionMatch idMatch = QRegularExpression("\\s*id=(-?\\d+)").match(rawLine);
                    QRegularExpressionMatch posMatch = regexPosition.match(rawLine);
                    QRegularExpressionMatch sizeMatch = regexSize.match(rawLine);
                    if (idMatch.hasMatch() && posMatch.hasMatch() && sizeMatch.hasMatch()) {
                        variable->setIdVariable(idMatch.captured(1).toInt());
                        variable->setRect(QRectF(0, 0, sizeMatch.captured(1).toDouble(), sizeMatch.captured(2).toDouble()).normalized());
                        variable->setPos(QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble()));
                        _graphicsView->getScene()->getAnimationsVariable()->append(variable);
                        _graphicsView->getScene()->addItem(variable);
                        persistedItems.insert(persistedId, variable);
                    } else {
                        delete variable;
                    }
                    continue;
                }

                if (animationType.compare("AnimationTimer", Qt::CaseInsensitive) == 0) {
                    AnimationTimer* timer = new AnimationTimer(_graphicsView->getScene());
                    QRegularExpressionMatch hourMatch = QRegularExpression("\\s*hour=(\\d+)").match(rawLine);
                    QRegularExpressionMatch minuteMatch = QRegularExpression("\\s*minute=(\\d+)").match(rawLine);
                    QRegularExpressionMatch secondMatch = QRegularExpression("\\s*second=(\\d+)").match(rawLine);
                    QRegularExpressionMatch formatMatch = QRegularExpression("\\s*format=(\\d+)").match(rawLine);
                    QRegularExpressionMatch posMatch = regexPosition.match(rawLine);
                    QRegularExpressionMatch sizeMatch = regexSize.match(rawLine);
                    if (hourMatch.hasMatch() && minuteMatch.hasMatch() && secondMatch.hasMatch()
                        && formatMatch.hasMatch() && posMatch.hasMatch() && sizeMatch.hasMatch()) {
                        timer->setInitialHours(hourMatch.captured(1).toInt());
                        timer->setInitialMinutes(minuteMatch.captured(1).toInt());
                        timer->setInitialSeconds(secondMatch.captured(1).toInt());
                        timer->setTimeFormat(Util::TimeFormat(formatMatch.captured(1).toInt()));
                        timer->setTime(0.0);
                        timer->setRect(QRectF(0, 0, sizeMatch.captured(1).toDouble(), sizeMatch.captured(2).toDouble()).normalized());
                        timer->setPos(QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble()));
                        _graphicsView->getScene()->getAnimationsTimer()->append(timer);
                        _graphicsView->getScene()->addItem(timer);
                        persistedItems.insert(persistedId, timer);
                    } else {
                        delete timer;
                    }
                    continue;
                }

                if (animationType.compare("AnimationPlaceholder", Qt::CaseInsensitive) == 0) {
                    QRegularExpressionMatch typeMatch = QRegularExpression("\\s*type=([^\\t]+)").match(rawLine);
                    QRegularExpressionMatch targetMatch = QRegularExpression("\\s*target=([^\\t]*)").match(rawLine);
                    QRegularExpressionMatch posMatch = regexPosition.match(rawLine);
                    QRegularExpressionMatch sizeMatch = regexSize.match(rawLine);
                    if (!typeMatch.hasMatch() || !targetMatch.hasMatch() || !posMatch.hasMatch() || !sizeMatch.hasMatch()) {
                        continue;
                    }

                    AnimationPlaceholder* placeholder = new AnimationPlaceholder(decodeGuiText(typeMatch.captured(1).trimmed()));
                    placeholder->setTargetName(decodeGuiText(targetMatch.captured(1).trimmed()));
                    placeholder->setRect(QRectF(0, 0, sizeMatch.captured(1).toDouble(), sizeMatch.captured(2).toDouble()).normalized());
                    placeholder->setPos(QPointF(posMatch.captured(1).toDouble(), posMatch.captured(2).toDouble()));
                    _graphicsView->getScene()->getAnimationsPlaceholder()->append(placeholder);
                    _graphicsView->getScene()->addItem(placeholder);
                    persistedItems.insert(persistedId, placeholder);
                    continue;
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

        if (!placeholders.empty()) {
            QRegularExpression regex("AnimationPlaceholder_(\\d+) \\t type=([^\\t]+) \\t target=([^\\t]*) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");
            for (const QString& line : placeholders) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }
                QRegularExpressionMatch match = regex.match(line);
                if (!match.hasMatch()) {
                    continue;
                }
                AnimationPlaceholder* placeholder = new AnimationPlaceholder(decodeGuiText(match.captured(2).trimmed()));
                placeholder->setTargetName(decodeGuiText(match.captured(3).trimmed()));
                placeholder->setRect(QRectF(0, 0, match.captured(6).toDouble(), match.captured(7).toDouble()).normalized());
                placeholder->setPos(QPointF(match.captured(4).toDouble(), match.captured(5).toDouble()));
                _graphicsView->getScene()->getAnimationsPlaceholder()->append(placeholder);
                _graphicsView->getScene()->addItem(placeholder);
            }
        }

        if (!geometries.empty()) {
            QRegularExpression regexPos("\\s*(?:pos|position)=\\(([^,]+),([^\\)]+)\\)");
            QRegularExpression regexId("\\s*id=(-?\\d+)");
            QRegularExpression regexLine("\\s*line=\\(([^,]+),([^,]+),([^,]+),([^\\)]+)\\)");
            QRegularExpression regexRect("\\s*rect=\\(([^,]+),([^,]+),([^,]+),([^\\)]+)\\)");
            QRegularExpression regexPoints("\\s*points=([^\\t]+)");
            QRegularExpression regexText("\\s*value=([^\\t]+)");
            QRegularExpression regexLegacyDrawType("\\bDraw(Line|Rectangle|Ellipse|Polygon|Text)\\b",
                                                   QRegularExpression::CaseInsensitiveOption);
            QRegularExpression regexLeadingId("^\\s*(?:-\\s*)?(-?\\d+)\\b");

            for (const QString& rawLine : geometries) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }
                QStringList tokens = rawLine.split("\t", Qt::SkipEmptyParts);
                const QString compactLine = rawLine.simplified();

                QGraphicsItem* loadedItem = nullptr;
                QString type;
                if (tokens.size() >= 1 && tokens[0].trimmed().compare("Geometry", Qt::CaseInsensitive) == 0) {
                    if (tokens.size() < 4) {
                        continue;
                    }
                    type = tokens[2].trimmed();
                    type.remove("type=");
                } else if (tokens.size() >= 1 && tokens[0].trimmed().compare("Text", Qt::CaseInsensitive) == 0) {
                    type = "text";
                } else if (tokens.size() >= 2 && tokens[1].trimmed().startsWith("Draw", Qt::CaseInsensitive)) {
                    const QString drawType = tokens[1].trimmed();
                    if (drawType.compare("DrawLine", Qt::CaseInsensitive) == 0) {
                        type = "line";
                    } else if (drawType.compare("DrawRectangle", Qt::CaseInsensitive) == 0) {
                        type = "rect";
                    } else if (drawType.compare("DrawEllipse", Qt::CaseInsensitive) == 0) {
                        type = "ellipse";
                    } else if (drawType.compare("DrawPolygon", Qt::CaseInsensitive) == 0) {
                        type = "polygon";
                    } else if (drawType.compare("DrawText", Qt::CaseInsensitive) == 0) {
                        type = "text";
                    }
                } else {
                    QRegularExpressionMatch legacyTypeMatch = regexLegacyDrawType.match(compactLine);
                    if (legacyTypeMatch.hasMatch()) {
                        const QString drawType = "Draw" + legacyTypeMatch.captured(1);
                        if (drawType.compare("DrawLine", Qt::CaseInsensitive) == 0) {
                            type = "line";
                        } else if (drawType.compare("DrawRectangle", Qt::CaseInsensitive) == 0) {
                            type = "rect";
                        } else if (drawType.compare("DrawEllipse", Qt::CaseInsensitive) == 0) {
                            type = "ellipse";
                        } else if (drawType.compare("DrawPolygon", Qt::CaseInsensitive) == 0) {
                            type = "polygon";
                        } else if (drawType.compare("DrawText", Qt::CaseInsensitive) == 0) {
                            type = "text";
                        }
                    }
                }
                if (type.isEmpty()) {
                    continue;
                }

                int persistedId = 0;
                bool persistedIdOk = false;
                if (tokens.size() >= 2 &&
                    (tokens[0].trimmed().compare("Geometry", Qt::CaseInsensitive) == 0
                     || tokens[0].trimmed().compare("Text", Qt::CaseInsensitive) == 0)) {
                    QRegularExpressionMatch idMatch = regexId.match(tokens[1]);
                    if (idMatch.hasMatch()) {
                        persistedId = idMatch.captured(1).toInt();
                        persistedIdOk = true;
                    }
                } else if (tokens.size() >= 1) {
                    persistedId = tokens[0].trimmed().toInt(&persistedIdOk);
                }
                if (!persistedIdOk) {
                    QRegularExpressionMatch leadingIdMatch = regexLeadingId.match(compactLine);
                    if (leadingIdMatch.hasMatch()) {
                        persistedId = leadingIdMatch.captured(1).toInt(&persistedIdOk);
                    }
                }
                QRegularExpressionMatch posMatch = regexPos.match(rawLine);
                QPointF itemPos(0.0, 0.0);
                if (posMatch.hasMatch()) {
                    itemPos.setX(posMatch.captured(1).toDouble());
                    itemPos.setY(posMatch.captured(2).toDouble());
                }

                if (type == "line") {
                    QRegularExpressionMatch m = regexLine.match(rawLine);
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
                    QRegularExpressionMatch m = regexRect.match(rawLine);
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
                    QRegularExpressionMatch m = regexPoints.match(rawLine);
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
                } else if (type == "text") {
                    QRegularExpressionMatch m = regexText.match(rawLine);
                    if (m.hasMatch()) {
                        QGraphicsTextItem* textItem = new QGraphicsTextItem(decodeGuiText(m.captured(1).trimmed()));
                        textItem->setPos(itemPos);
                        textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
                        textItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                        textItem->setTextInteractionFlags(Qt::NoTextInteraction);
                        _graphicsView->getScene()->addItem(textItem);
                        _graphicsView->getScene()->addDrawingGeometry(textItem);
                        loadedItem = textItem;
                    }
                }

                if (loadedItem != nullptr) {
                    applyItemFlagsState(rawLine, loadedItem);
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
                    if (persistedIdOk && persistedId != 0) {
                        persistedItems.insert(persistedId, loadedItem);
                    }
                }
            }
        }

        if (!groups.empty()) {
            struct PendingGroup {
                int groupId = 0;
                QList<int> memberIds;
            };

            QList<PendingGroup> pendingGroups;
            QRegularExpression regexLegacyMembers("members=([^\\t]+)");

            for (const QString& rawLine : groups) {
                if (rawLine.trimmed().isEmpty()) {
                    continue;
                }
                const QStringList tokens = rawLine.split("\t", Qt::SkipEmptyParts);
                if (tokens.isEmpty()) {
                    continue;
                }

                PendingGroup pending;
                bool groupIdOk = false;
                pending.groupId = tokens[0].trimmed().toInt(&groupIdOk);

                QStringList memberTokens;
                if (tokens[0].trimmed().startsWith("Group_")) {
                    QRegularExpressionMatch membersMatch = regexLegacyMembers.match(rawLine);
                    if (membersMatch.hasMatch()) {
                        memberTokens = membersMatch.captured(1).split(",", Qt::SkipEmptyParts);
                    }
                } else if (tokens.size() >= 2 && tokens[1].trimmed().compare("Group", Qt::CaseInsensitive) == 0) {
                    for (int i = 2; i < tokens.size(); ++i) {
                        memberTokens.append(tokens[i].trimmed().split(" ", Qt::SkipEmptyParts));
                    }
                } else {
                    QRegularExpressionMatch membersMatch = regexLegacyMembers.match(rawLine);
                    if (membersMatch.hasMatch()) {
                        memberTokens = membersMatch.captured(1).split(",", Qt::SkipEmptyParts);
                    }
                }

                for (const QString& memberToken : memberTokens) {
                    bool memberIdOk = false;
                    const int memberId = memberToken.trimmed().toInt(&memberIdOk);
                    if (memberIdOk) {
                        pending.memberIds.append(memberId);
                    }
                }

                if (!pending.memberIds.isEmpty()) {
                    if (!groupIdOk) {
                        pending.groupId = 0;
                    }
                    pendingGroups.append(pending);
                }
            }

            bool madeProgress = true;
            while (madeProgress && !pendingGroups.isEmpty()) {
                madeProgress = false;
                for (int i = 0; i < pendingGroups.size();) {
                    const PendingGroup& pending = pendingGroups.at(i);
                    QList<QGraphicsItem*> groupItems;
                    QList<GraphicalModelComponent*> groupComponents;
                    bool allResolved = true;
                    for (int memberId : pending.memberIds) {
                        if (!persistedItems.contains(memberId) || persistedItems.value(memberId) == nullptr) {
                            allResolved = false;
                            break;
                        }
                        QGraphicsItem* item = persistedItems.value(memberId);
                        groupItems.append(item);
                        if (GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(item)) {
                            groupComponents.append(component);
                        }
                    }
                    if (!allResolved || groupItems.isEmpty()) {
                        ++i;
                        continue;
                    }

                    bool canCreateGroup = true;
                    for (QGraphicsItem* item : groupItems) {
                        if (item == nullptr) {
                            canCreateGroup = false;
                            break;
                        }
                        if (dynamic_cast<QGraphicsItemGroup*>(item) == nullptr && item->group() != nullptr) {
                            canCreateGroup = false;
                            break;
                        }
                    }
                    if (!canCreateGroup) {
                        ++i;
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
                    if (pending.groupId != 0) {
                        persistedItems.insert(pending.groupId, group);
                    }

                    pendingGroups.removeAt(i);
                    madeProgress = true;
                }
            }
        }

        // Recompute diagram-arrow endpoints after all persisted positions and groups are restored.
        // Without this pass, arrows can remain anchored to pre-restore coordinates.
        scene->actualizeDiagramArrows();

        if (hasPersistedViewState) {
            QPointer<ModelGraphicsView> targetView(_graphicsView);
            QTimer::singleShot(0, _ownerWidget, [targetView, restoredViewpointX, restoredViewpointY]() {
                if (targetView.isNull()) {
                    return;
                }
                QScrollBar* hBar = targetView->horizontalScrollBar();
                QScrollBar* vBar = targetView->verticalScrollBar();
                if (hBar == nullptr || vBar == nullptr) {
                    return;
                }
                hBar->setValue(qBound(hBar->minimum(), restoredViewpointX, hBar->maximum()));
                vBar->setValue(qBound(vBar->minimum(), restoredViewpointY, vBar->maximum()));
            });
        }

        _console->append("\n");
        *_modelFilename = QString::fromStdString(filename);
    }

    return model;
}

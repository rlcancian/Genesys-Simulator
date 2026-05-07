#include "AnimationVariable.h"

#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Entity.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "../../../../../plugins/data/Logic/Variable.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <QStringList>

namespace {

ModelDataDefinition* resolveDataDefinition(Model* model, const QString& name, int id) {
    if (model == nullptr) {
        return nullptr;
    }

    ModelDataManager* dataManager = model->getDataManager();
    if (dataManager == nullptr) {
        return nullptr;
    }

    if (id >= 0) {
        if (ModelDataDefinition* definition = dataManager->getDataDefinition(Util::TypeOf<Variable>(), static_cast<Util::identification>(id))) {
            return definition;
        }
        if (ModelDataDefinition* definition = dataManager->getDataDefinition(Util::TypeOf<Attribute>(), static_cast<Util::identification>(id))) {
            return definition;
        }
    }

    const std::string targetName = name.trimmed().toStdString();
    if (targetName.empty() || targetName == "None") {
        return nullptr;
    }

    if (ModelDataDefinition* definition = dataManager->getDataDefinition(Util::TypeOf<Variable>(), targetName)) {
        return definition;
    }
    if (ModelDataDefinition* definition = dataManager->getDataDefinition(Util::TypeOf<Attribute>(), targetName)) {
        return definition;
    }
    return nullptr;
}

const std::list<unsigned int>* definitionDimensions(const ModelDataDefinition* definition) {
    if (definition == nullptr) {
        return nullptr;
    }
    if (definition->getClassname() == Util::TypeOf<Variable>()) {
        return static_cast<const Variable*>(definition)->getDimensionSizes();
    }
    if (definition->getClassname() == Util::TypeOf<Attribute>()) {
        return static_cast<const Attribute*>(definition)->getDimensionSizes();
    }
    return nullptr;
}

double readDefinitionValue(ModelDataDefinition* definition, Entity* entity, const std::string& index) {
    if (definition == nullptr) {
        return 0.0;
    }
    if (definition->getClassname() == Util::TypeOf<Variable>()) {
        return static_cast<Variable*>(definition)->getValue(index);
    }
    if (definition->getClassname() == Util::TypeOf<Attribute>()) {
        if (entity != nullptr) {
            return entity->getAttributeValue(definition->getName(), index);
        }
        return static_cast<Attribute*>(definition)->getInitialValue(index);
    }
    return 0.0;
}

std::string buildIndexKey(const QVector<unsigned int>& indexes) {
    std::string key;
    for (int i = 0; i < indexes.size(); ++i) {
        if (i > 0) {
            key += ",";
        }
        key += std::to_string(indexes.at(i));
    }
    return key;
}

QString joinSliceLabels(const QVector<unsigned int>& sliceIndexes) {
    QStringList parts;
    for (int i = 0; i < sliceIndexes.size(); ++i) {
        parts << QString("Dim %1=%2").arg(i + 3).arg(sliceIndexes.at(i));
    }
    return parts.join("  ");
}

} // namespace

AnimationVariable::AnimationVariable() : _isDrawingInicialized(true) {
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setActive(true);
    setSelected(false);
}

void AnimationVariable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (_dataDefinition == nullptr && _model != nullptr) {
        _dataDefinition = resolveDataDefinition(_model, _targetName, _targetId);
    }

    const QRectF bounds = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(QColor(30, 40, 60), 1.2));
    painter->setBrush(QColor(245, 248, 252));
    painter->drawRoundedRect(bounds, 3.0, 3.0);

    const QString title = !_targetName.trimmed().isEmpty() && _targetName != "None"
                              ? _targetName
                              : QStringLiteral("Unbound");
    const QString typeLabel = !_cachedTypeName.isEmpty() ? QString(" (%1)").arg(_cachedTypeName) : QString();
    const QString headerText = title + typeLabel;
    const qreal headerHeight = _cachedShape.size() > 2 ? 20.0 : 0.0;
    QRectF content = bounds.adjusted(4.0, 4.0, -4.0, -4.0);

    QFont headerFont = painter->font();
    headerFont.setBold(true);
    headerFont.setPixelSize(std::max(8, static_cast<int>(bounds.height() / 8.0)));
    painter->setFont(headerFont);
    painter->setPen(QColor(40, 40, 40));
    painter->drawText(QRectF(content.left(), content.top(), content.width(), std::max<qreal>(14.0, headerHeight)),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      headerText);

    if (_cachedShape.isEmpty() || _cachedValues.isEmpty()) {
        QFont fallbackFont = painter->font();
        fallbackFont.setPixelSize(std::max(8, static_cast<int>(bounds.height() / 4.0)));
        painter->setFont(fallbackFont);
        painter->setPen(QColor(70, 70, 70));
        painter->drawText(content.adjusted(0.0, headerHeight, 0.0, 0.0), Qt::AlignCenter, QStringLiteral("No data"));
    } else {
        const int rows = _cachedShape.size() == 1 ? 1 : static_cast<int>(_cachedShape.at(0));
        const int cols = _cachedShape.size() == 1 ? static_cast<int>(_cachedShape.at(0)) : static_cast<int>(_cachedShape.at(1));
        const QRectF gridBounds = content.adjusted(0.0, headerHeight, 0.0, 0.0);
        const qreal cellWidth = cols > 0 ? gridBounds.width() / cols : gridBounds.width();
        const qreal cellHeight = rows > 0 ? gridBounds.height() / rows : gridBounds.height();
        QFont cellFont = painter->font();
        cellFont.setPixelSize(std::max(7, static_cast<int>(std::min(cellWidth / 3.0, cellHeight / 2.0))));
        painter->setFont(cellFont);
        painter->setPen(QPen(QColor(60, 60, 60), 1.0));
        painter->setBrush(Qt::NoBrush);

        if (_cachedShape.size() > 2) {
            const QString slices = joinSliceLabels(_sliceIndexes);
            if (!slices.isEmpty()) {
                painter->drawText(QRectF(gridBounds.left(), gridBounds.top() - 16.0, gridBounds.width(), 14.0),
                                  Qt::AlignLeft | Qt::AlignVCenter,
                                  slices);
            }
        }

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const int flatIndex = row * cols + col;
                if (flatIndex >= _cachedValues.size()) {
                    continue;
                }
                const QRectF cell(gridBounds.left() + (col * cellWidth),
                                  gridBounds.top() + (row * cellHeight),
                                  cellWidth,
                                  cellHeight);
                painter->drawRect(cell);
                painter->drawText(cell.adjusted(2.0, 2.0, -2.0, -2.0),
                                  Qt::AlignCenter,
                                  QString::number(_cachedValues.at(flatIndex), 'g', 12));
            }
        }
    }

    if (isSelected()) {
        const qreal cornerSize = 10.0;
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawRect(QRectF(-cornerSize, -cornerSize, cornerSize, cornerSize));
        painter->drawRect(QRectF(bounds.topRight() - QPointF(0, cornerSize), QSizeF(cornerSize, cornerSize)));
        painter->drawRect(QRectF(-cornerSize, bounds.height(), cornerSize, cornerSize));
        painter->drawRect(QRectF(bounds.bottomRight(), QSizeF(cornerSize, cornerSize)));
    }
}

double AnimationVariable::getValue() {
    return _value;
}

ModelDataDefinition* AnimationVariable::getDataDefinition() const {
    return _dataDefinition;
}

Variable* AnimationVariable::getVariable() {
    return dynamic_cast<Variable*>(_dataDefinition);
}

Attribute* AnimationVariable::getAttribute() {
    return dynamic_cast<Attribute*>(_dataDefinition);
}

QString AnimationVariable::getTargetName() const {
    return _targetName;
}

int AnimationVariable::getTargetId() const {
    return _targetId;
}

unsigned int AnimationVariable::getDimensionCount() const {
    const std::list<unsigned int>* dimensions = definitionDimensions(_dataDefinition);
    return dimensions != nullptr ? static_cast<unsigned int>(dimensions->size()) : 0u;
}

unsigned int AnimationVariable::getDimensionSize(unsigned int dimensionIndex) const {
    const std::list<unsigned int>* dimensions = definitionDimensions(_dataDefinition);
    if (dimensions == nullptr || dimensions->empty()) {
        return 0u;
    }
    auto it = dimensions->begin();
    const unsigned int maxIndex = static_cast<unsigned int>(dimensions->size() - 1);
    std::advance(it, std::min<unsigned int>(dimensionIndex, maxIndex));
    return *it;
}

unsigned int AnimationVariable::getSliceIndex(unsigned int dimensionIndex) const {
    return dimensionIndex < static_cast<unsigned int>(_sliceIndexes.size()) ? _sliceIndexes.at(static_cast<int>(dimensionIndex)) : 0u;
}

void AnimationVariable::setValue(double value) {
    _value = value;
    if (!_cachedShape.isEmpty() && _cachedShape.size() == 1 && !_cachedValues.isEmpty()) {
        _cachedValues[0] = value;
    }
    update();
}

void AnimationVariable::setDataDefinition(ModelDataDefinition *definition) {
    _dataDefinition = definition;
    if (definition == nullptr) {
        _targetId = -1;
    } else {
        _targetName = QString::fromStdString(definition->getName());
        _targetId = static_cast<int>(definition->getId());
        _sliceIndexes.resize(std::max(0, static_cast<int>(getDimensionCount()) - 2));
    }
    refreshValue();
}

void AnimationVariable::setVariable(Variable *variable) {
    setDataDefinition(variable);
}

void AnimationVariable::setAttribute(Attribute *attribute) {
    setDataDefinition(attribute);
}

void AnimationVariable::setModel(Model *model) {
    _model = model;
    if (_dataDefinition == nullptr) {
        _dataDefinition = resolveDataDefinition(_model, _targetName, _targetId);
    }
    refreshValue();
}

void AnimationVariable::setTargetName(std::string name) {
    _targetName = QString::fromStdString(name);
    _dataDefinition = resolveDataDefinition(_model, _targetName, _targetId);
    if (_dataDefinition != nullptr) {
        _targetId = static_cast<int>(_dataDefinition->getId());
        _sliceIndexes.resize(std::max(0, static_cast<int>(getDimensionCount()) - 2));
    }
    refreshValue();
}

void AnimationVariable::setIdVariable(int id) {
    _targetId = id;
    _dataDefinition = resolveDataDefinition(_model, _targetName, _targetId);
    if (_dataDefinition != nullptr) {
        _targetName = QString::fromStdString(_dataDefinition->getName());
        _sliceIndexes.resize(std::max(0, static_cast<int>(getDimensionCount()) - 2));
    }
    refreshValue();
}

void AnimationVariable::setSliceIndex(unsigned int dimensionIndex, unsigned int value) {
    if (dimensionIndex >= static_cast<unsigned int>(_sliceIndexes.size())) {
        _sliceIndexes.resize(static_cast<int>(dimensionIndex) + 1);
    }
    _sliceIndexes[static_cast<int>(dimensionIndex)] = value;
    refreshValue();
}

void AnimationVariable::setWhenLoaded(QList<ModelDataDefinition *> *definitions) {
    if (definitions == nullptr) {
        return;
    }
    for (ModelDataDefinition* definition : *definitions) {
        if (definition == nullptr) {
            continue;
        }
        if (definition->getName() == _targetName.toStdString() || static_cast<int>(definition->getId()) == _targetId) {
            setDataDefinition(definition);
            return;
        }
    }
}

void AnimationVariable::refreshValue(Entity *entity) {
    if (_dataDefinition == nullptr && _model != nullptr) {
        _dataDefinition = resolveDataDefinition(_model, _targetName, _targetId);
    }

    _cachedValues.clear();
    _cachedShape.clear();
    _cachedTypeName.clear();

    if (_dataDefinition == nullptr) {
        _value = 0.0;
        _cachedValues.append(_value);
        _cachedShape.append(1u);
        update();
        return;
    }

    _cachedTypeName = QString::fromStdString(_dataDefinition->getClassname());
    const std::list<unsigned int>* dimensions = definitionDimensions(_dataDefinition);
    QVector<unsigned int> sizes;
    if (dimensions != nullptr) {
        for (unsigned int size : *dimensions) {
            sizes.append(size);
        }
    }

    if (sizes.isEmpty()) {
        _cachedShape.append(1u);
        _cachedValues.append(readDefinitionValue(_dataDefinition, entity, ""));
        _value = _cachedValues.first();
        update();
        return;
    }

    if (sizes.size() == 1) {
        const unsigned int count = sizes.first();
        _cachedShape.append(1u);
        _cachedShape.append(count);
        _cachedValues.reserve(static_cast<int>(count));
        for (unsigned int i = 0; i < count; ++i) {
            _cachedValues.append(readDefinitionValue(_dataDefinition, entity, std::to_string(i)));
        }
        _value = !_cachedValues.isEmpty() ? _cachedValues.first() : 0.0;
        update();
        return;
    }

    const unsigned int rows = sizes.at(0);
    const unsigned int cols = sizes.at(1);
    _cachedShape.append(rows);
    _cachedShape.append(cols);

    QVector<unsigned int> sliceIndexes = _sliceIndexes;
    if (sliceIndexes.size() < static_cast<int>(sizes.size() - 2)) {
        sliceIndexes.resize(static_cast<int>(sizes.size() - 2));
    }

    _cachedValues.reserve(static_cast<int>(rows * cols));
    for (unsigned int row = 0; row < rows; ++row) {
        for (unsigned int col = 0; col < cols; ++col) {
            QVector<unsigned int> indexPath;
            indexPath.reserve(static_cast<int>(sizes.size()));
            indexPath.append(row);
            indexPath.append(col);
            for (int slice = 0; slice < sliceIndexes.size(); ++slice) {
                indexPath.append(sliceIndexes.at(slice));
            }
            _cachedValues.append(readDefinitionValue(_dataDefinition, entity, buildIndexKey(indexPath)));
        }
    }
    _value = !_cachedValues.isEmpty() ? _cachedValues.first() : 0.0;
    update();
}

void AnimationVariable::startDrawing(QGraphicsSceneMouseEvent *event) {
    _isResizing = true;
    _startPoint = event->scenePos();
    setPos(_startPoint);
}

void AnimationVariable::continueDrawing(QGraphicsSceneMouseEvent *event) {
    if (_isResizing) {
        QPointF delta = event->scenePos() - _startPoint;
        QRectF newRect = QRectF(0, 0, delta.x(), delta.y());
        setRect(newRect.normalized());
        update();
    }
}

void AnimationVariable::stopDrawing(QGraphicsSceneMouseEvent *event) {
    adjustSizeAndPosition(event);
    _isResizing = false;
    _isDrawingFinalized = true;
}

void AnimationVariable::adjustSizeAndPosition(QGraphicsSceneMouseEvent *event) {
    qreal minimumX = std::min(_startPoint.x(), event->scenePos().x());
    qreal minimumY = std::min(_startPoint.y(), event->scenePos().y());
    qreal maximumX = std::max(_startPoint.x(), event->scenePos().x());
    qreal maximumY = std::max(_startPoint.y(), event->scenePos().y());

    QRectF newRect = QRectF(0, 0, maximumX - minimumX, maximumY - minimumY);
    setRect(newRect.normalized());
    setPos(QPointF(minimumX, minimumY));
    update();
}

bool AnimationVariable::isDrawingInicialized() {
    return _isDrawingInicialized;
}

bool AnimationVariable::isDrawingFinalized() {
    return _isDrawingFinalized;
}

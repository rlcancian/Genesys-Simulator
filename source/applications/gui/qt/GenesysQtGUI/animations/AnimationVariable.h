#ifndef ANIMATIONVARIABLE_H
#define ANIMATIONVARIABLE_H

#include <QString>
#include <QList>
#include <QVector>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <string>

class Attribute;
class Entity;
class Model;
class ModelDataDefinition;
class Variable;

class AnimationVariable : public QGraphicsRectItem {
public:
    AnimationVariable();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    double getValue();
    ModelDataDefinition *getDataDefinition() const;
    Variable *getVariable();
    Attribute *getAttribute();
    QString getTargetName() const;
    int getTargetId() const;
    unsigned int getDimensionCount() const;
    unsigned int getDimensionSize(unsigned int dimensionIndex) const;
    unsigned int getSliceIndex(unsigned int dimensionIndex) const;

    void setValue(double value);
    void setDataDefinition(ModelDataDefinition *definition);
    void setVariable(Variable *variable);
    void setAttribute(Attribute *attribute);
    void setModel(Model *model);
    void setTargetName(std::string name);
    void setIdVariable(int id);
    void setSliceIndex(unsigned int dimensionIndex, unsigned int value);
    void setWhenLoaded(QList<ModelDataDefinition *> *definitions);
    void refreshValue(Entity *entity = nullptr);

    void startDrawing(QGraphicsSceneMouseEvent *event);
    void continueDrawing(QGraphicsSceneMouseEvent *event);
    void stopDrawing(QGraphicsSceneMouseEvent *event);
    void adjustSizeAndPosition(QGraphicsSceneMouseEvent *event);
    bool isDrawingInicialized();
    bool isDrawingFinalized();

private:
    double _value = 0.0;
    Model *_model = nullptr;
    ModelDataDefinition *_dataDefinition = nullptr;
    QString _targetName = "None";
    int _targetId = -1;
    QVector<unsigned int> _sliceIndexes;
    QVector<double> _cachedValues;
    QVector<unsigned int> _cachedShape;
    QString _cachedTypeName;
    QString _cachedTitle;
    QPointF _startPoint = QPointF(0, 0);
    bool _isResizing = false;
    bool _isDrawingInicialized = false;
    bool _isDrawingFinalized = false;
};

#endif // ANIMATIONVARIABLE_H

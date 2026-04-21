#ifndef ANIMATIONPLACEHOLDER_H
#define ANIMATIONPLACEHOLDER_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>

class AnimationPlaceholder : public QGraphicsRectItem {
public:
    explicit AnimationPlaceholder(const QString& animationType);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    QString getAnimationType() const;
    QString getTargetName() const;
    void setTargetName(const QString& targetName);

    void startDrawing(QGraphicsSceneMouseEvent* event);
    void continueDrawing(QGraphicsSceneMouseEvent* event);
    void stopDrawing(QGraphicsSceneMouseEvent* event);
    void adjustSizeAndPosition(QGraphicsSceneMouseEvent* event);
    bool isDrawingInicialized() const;
    bool isDrawingFinalized() const;

private:
    QString _animationType;
    QString _targetName;
    QPointF _startPoint = QPointF(0, 0);
    bool _isResizing = false;
    bool _isDrawingInicialized = false;
    bool _isDrawingFinalized = false;
};

class AnimationAttribute : public AnimationPlaceholder {
public:
    AnimationAttribute();
};

class AnimationEntity : public AnimationPlaceholder {
public:
    AnimationEntity();
};

class AnimationEvent : public AnimationPlaceholder {
public:
    AnimationEvent();
};

class AnimationExpression : public AnimationPlaceholder {
public:
    AnimationExpression();
};

class AnimationPlot : public AnimationPlaceholder {
public:
    AnimationPlot();
};

class AnimationQueueDisplay : public AnimationPlaceholder {
public:
    AnimationQueueDisplay();
};

class AnimationResource : public AnimationPlaceholder {
public:
    AnimationResource();
};

class AnimationStation : public AnimationPlaceholder {
public:
    AnimationStation();
};

class AnimationStatistics : public AnimationPlaceholder {
public:
    AnimationStatistics();
};

#endif // ANIMATIONPLACEHOLDER_H

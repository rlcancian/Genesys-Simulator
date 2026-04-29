#ifndef ANIMATIONPLACEHOLDER_H
#define ANIMATIONPLACEHOLDER_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QVector>
#include <QString>

class Model;

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
    enum class PlotType {
        Line = 0,
        Bar = 1
    };

    struct DataSeries {
        QString label;
        QString expression;
        QColor color = QColor(31, 119, 180);
        QVector<double> values;
    };

    AnimationPlot();

    QString getTitle() const;
    void setTitle(const QString& title);
    QString getYAxisTitle() const;
    void setYAxisTitle(const QString& title);
    PlotType getPlotType() const;
    void setPlotType(PlotType type);
    QString getDatasetsText() const;
    void setDatasetsText(const QString& datasetsText);
    void clearValues();
    void appendSample(Model* model);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    static QVector<DataSeries> parseDatasetsText(const QString& datasetsText);
    static QString datasetsTextFromSeries(const QVector<DataSeries>& series);

    QString _title = "Plot";
    QString _yAxisTitle = "Value";
    PlotType _plotType = PlotType::Line;
    QVector<DataSeries> _series;
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

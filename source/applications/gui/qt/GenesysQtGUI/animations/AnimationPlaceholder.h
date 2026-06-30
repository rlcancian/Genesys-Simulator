#ifndef ANIMATIONPLACEHOLDER_H
#define ANIMATIONPLACEHOLDER_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QVector>
#include <QString>
#include <limits>

#include "kernel/statistics/Collector_if.h"

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

    // Resets runtime-only overlay state at simulation start (overridden by plugin placeholders).
    virtual void resetRuntimeState();

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
        qreal lineWidth = 2.0;
        QVector<double> values;
    };

    AnimationPlot();

    QString getTitle() const;
    void setTitle(const QString& title);
    QString getYAxisTitle() const;
    void setYAxisTitle(const QString& title);
    QString getXAxisTitle() const;
    void setXAxisTitle(const QString& title);
    PlotType getPlotType() const;
    void setPlotType(PlotType type);
    QString getDatasetsText() const;
    void setDatasetsText(const QString& datasetsText);
    bool getShowGridLines() const;
    void setShowGridLines(bool show);
    bool getShowTicks() const;
    void setShowTicks(bool show);
    double getXAxisMin() const;
    void setXAxisMin(double value);
    double getXAxisMax() const;
    void setXAxisMax(double value);
    double getYAxisMin() const;
    void setYAxisMin(double value);
    double getYAxisMax() const;
    void setYAxisMax(double value);
    void clearValues();
    void appendSample(Model* model);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    static QVector<DataSeries> parseDatasetsText(const QString& datasetsText);
    static QString datasetsTextFromSeries(const QVector<DataSeries>& series);

    QString _title = "Plot";
    QString _xAxisTitle = "Time";
    QString _yAxisTitle = "Value";
    PlotType _plotType = PlotType::Line;
    bool _showGridLines = true;
    bool _showTicks = true;
    double _xAxisMin = std::numeric_limits<double>::quiet_NaN();
    double _xAxisMax = std::numeric_limits<double>::quiet_NaN();
    double _yAxisMin = std::numeric_limits<double>::quiet_NaN();
    double _yAxisMax = std::numeric_limits<double>::quiet_NaN();
    QVector<DataSeries> _series;
};

class AnimationStatistics : public AnimationPlaceholder {
public:
    AnimationStatistics();

    void setCollector(Collector_if* collector);
    Collector_if* getCollector() const;

    // Refreshes the displayed values from the linked collector.
    void refreshValue();
    void clearRuntimeState();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    Collector_if* _collector = nullptr;
    double _lastValue = 0.0;
    unsigned long _numSamples = 0;
    QString _collectorName;
};

#endif // ANIMATIONPLACEHOLDER_H

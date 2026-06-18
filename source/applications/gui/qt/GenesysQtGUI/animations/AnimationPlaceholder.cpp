#include "AnimationPlaceholder.h"

#include "../../../../../kernel/simulator/model/Model.h"

#include <QFontMetricsF>
#include <QPainterPath>
#include <QRegularExpression>
#include <QStyleOptionGraphicsItem>

#include <algorithm>
#include <cmath>

AnimationPlaceholder::AnimationPlaceholder(const QString& animationType)
    : _animationType(animationType) {
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setActive(true);
    setSelected(false);
}

void AnimationPlaceholder::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QRectF bounds = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(Qt::black, 1.5));
    painter->setBrush(QColor(245, 245, 245));
    painter->drawRect(bounds);

    painter->setPen(QPen(Qt::red, 3.0));
    painter->drawLine(bounds.topLeft(), bounds.bottomRight());
    painter->drawLine(bounds.bottomLeft(), bounds.topRight());

    QString label = _animationType;
    if (!_targetName.trimmed().isEmpty()) {
        label += "\n" + _targetName.trimmed();
    }

    QFont font = painter->font();
    const int fontSize = qMax(8, qMin(static_cast<int>(bounds.width() / 8.0), static_cast<int>(bounds.height() / 4.0)));
    font.setPixelSize(fontSize);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(bounds.adjusted(4.0, 4.0, -4.0, -4.0), Qt::AlignCenter | Qt::TextWordWrap, label);

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

QString AnimationPlaceholder::getAnimationType() const {
    return _animationType;
}

QString AnimationPlaceholder::getTargetName() const {
    return _targetName;
}

void AnimationPlaceholder::setTargetName(const QString& targetName) {
    _targetName = targetName;
    update();
}

void AnimationPlaceholder::startDrawing(QGraphicsSceneMouseEvent* event) {
    _isDrawingInicialized = true;
    _isResizing = true;
    _startPoint = event->scenePos();
    setPos(_startPoint);
}

void AnimationPlaceholder::continueDrawing(QGraphicsSceneMouseEvent* event) {
    if (!_isResizing) {
        return;
    }

    const QPointF delta = event->scenePos() - _startPoint;
    setRect(QRectF(0, 0, delta.x(), delta.y()).normalized());
    update();
}

void AnimationPlaceholder::stopDrawing(QGraphicsSceneMouseEvent* event) {
    adjustSizeAndPosition(event);
    _isResizing = false;
    _isDrawingFinalized = true;
}

void AnimationPlaceholder::adjustSizeAndPosition(QGraphicsSceneMouseEvent* event) {
    const qreal minimumX = qMin(_startPoint.x(), event->scenePos().x());
    const qreal minimumY = qMin(_startPoint.y(), event->scenePos().y());
    const qreal maximumX = qMax(_startPoint.x(), event->scenePos().x());
    const qreal maximumY = qMax(_startPoint.y(), event->scenePos().y());

    setRect(QRectF(0, 0, maximumX - minimumX, maximumY - minimumY).normalized());
    setPos(QPointF(minimumX, minimumY));
    update();
}

bool AnimationPlaceholder::isDrawingInicialized() const {
    return _isDrawingInicialized;
}

bool AnimationPlaceholder::isDrawingFinalized() const {
    return _isDrawingFinalized;
}

AnimationAttribute::AnimationAttribute() : AnimationPlaceholder("Attribute") {
}

AnimationEntity::AnimationEntity() : AnimationPlaceholder("Entity") {
}

AnimationEvent::AnimationEvent() : AnimationPlaceholder("Event") {
}

AnimationExpression::AnimationExpression() : AnimationPlaceholder("Expression") {
}

AnimationPlot::AnimationPlot() : AnimationPlaceholder("Plot") {
    setDatasetsText("Series 1|TNOW|#1f77b4|2");
}

QString AnimationPlot::getTitle() const {
    return _title;
}

void AnimationPlot::setTitle(const QString& title) {
    _title = title;
    update();
}

QString AnimationPlot::getYAxisTitle() const {
    return _yAxisTitle;
}

void AnimationPlot::setYAxisTitle(const QString& title) {
    _yAxisTitle = title;
    update();
}

QString AnimationPlot::getXAxisTitle() const {
    return _xAxisTitle;
}

void AnimationPlot::setXAxisTitle(const QString& title) {
    _xAxisTitle = title;
    update();
}

AnimationPlot::PlotType AnimationPlot::getPlotType() const {
    return _plotType;
}

void AnimationPlot::setPlotType(PlotType type) {
    _plotType = type;
    update();
}

bool AnimationPlot::getShowGridLines() const {
    return _showGridLines;
}

void AnimationPlot::setShowGridLines(bool show) {
    _showGridLines = show;
    update();
}

bool AnimationPlot::getShowTicks() const {
    return _showTicks;
}

void AnimationPlot::setShowTicks(bool show) {
    _showTicks = show;
    update();
}

double AnimationPlot::getXAxisMin() const {
    return _xAxisMin;
}

void AnimationPlot::setXAxisMin(double value) {
    _xAxisMin = value;
    update();
}

double AnimationPlot::getXAxisMax() const {
    return _xAxisMax;
}

void AnimationPlot::setXAxisMax(double value) {
    _xAxisMax = value;
    update();
}

double AnimationPlot::getYAxisMin() const {
    return _yAxisMin;
}

void AnimationPlot::setYAxisMin(double value) {
    _yAxisMin = value;
    update();
}

double AnimationPlot::getYAxisMax() const {
    return _yAxisMax;
}

void AnimationPlot::setYAxisMax(double value) {
    _yAxisMax = value;
    update();
}

QString AnimationPlot::getDatasetsText() const {
    return datasetsTextFromSeries(_series);
}

void AnimationPlot::setDatasetsText(const QString& datasetsText) {
    _series = parseDatasetsText(datasetsText);
    update();
}

void AnimationPlot::clearValues() {
    for (DataSeries& series : _series) {
        series.values.clear();
    }
    update();
}

void AnimationPlot::appendSample(Model* model) {
    if (model == nullptr) {
        return;
    }

    for (DataSeries& series : _series) {
        bool success = false;
        std::string errorMessage;
        const double value = model->parseExpression(series.expression.toStdString(), success, errorMessage);
        if (success && std::isfinite(value)) {
            series.values.append(value);
        } else {
            series.values.append(0.0);
        }
    }
    update();
}

void AnimationPlot::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QRectF bounds = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::black, 1.0));
    painter->setBrush(QColor(255, 255, 255));
    painter->drawRect(bounds);

    QRectF plotArea = bounds.adjusted(42.0, 28.0, -10.0, -30.0);
    if (plotArea.width() < 20.0 || plotArea.height() < 20.0) {
        AnimationPlaceholder::paint(painter, option, widget);
        return;
    }

    painter->setPen(QPen(Qt::black, 1.0));
    QFont titleFont = painter->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(11);
    painter->setFont(titleFont);
    painter->drawText(bounds.adjusted(4.0, 2.0, -4.0, -2.0), Qt::AlignTop | Qt::AlignHCenter, _title);

    painter->setFont(QFont());
    const QPen axisPen(QColor(90, 90, 90), 1.0);
    painter->setPen(axisPen);
    painter->drawLine(plotArea.bottomLeft(), plotArea.topLeft());
    painter->drawLine(plotArea.bottomLeft(), plotArea.bottomRight());

    int sampleCount = 0;
    bool hasValue = false;
    double autoMinValue = 0.0;
    double autoMaxValue = 1.0;
    for (const DataSeries& series : _series) {
        sampleCount = std::max(sampleCount, static_cast<int>(series.values.size()));
        for (double value : series.values) {
            autoMinValue = hasValue ? std::min(autoMinValue, value) : value;
            autoMaxValue = hasValue ? std::max(autoMaxValue, value) : value;
            hasValue = true;
        }
    }

    const bool hasExplicitXAxisMin = !std::isnan(_xAxisMin);
    const bool hasExplicitXAxisMax = !std::isnan(_xAxisMax);
    const bool hasExplicitYAxisMin = !std::isnan(_yAxisMin);
    const bool hasExplicitYAxisMax = !std::isnan(_yAxisMax);
    const double xMin = hasExplicitXAxisMin ? _xAxisMin : 0.0;
    const double xMax = hasExplicitXAxisMax ? _xAxisMax : static_cast<double>(std::max(1, sampleCount - 1));
    double minValue = hasExplicitYAxisMin ? _yAxisMin : (hasValue ? autoMinValue : 0.0);
    double maxValue = hasExplicitYAxisMax ? _yAxisMax : (hasValue ? autoMaxValue : 1.0);
    double xStart = xMin;
    double xEnd = xMax;

    if (!hasValue) {
        painter->setPen(QColor(90, 90, 90));
        painter->drawText(plotArea, Qt::AlignCenter, "No samples");
    } else {
        if (qFuzzyCompare(xStart, xEnd)) {
            xStart -= 1.0;
            xEnd += 1.0;
        }
        if (qFuzzyCompare(minValue, maxValue)) {
            minValue -= 1.0;
            maxValue += 1.0;
        }
        if (!hasExplicitYAxisMin && minValue > 0.0) {
            minValue = 0.0;
        }

        const double ySpan = std::max(1e-9, maxValue - minValue);
        const double xSpan = std::max(1e-9, xEnd - xStart);
        const auto pointFor = [&](int sampleIndex, double value) {
            const double xValue = static_cast<double>(sampleIndex);
            const double xRatio = (xValue - xStart) / xSpan;
            const double yRatio = (value - minValue) / ySpan;
            return QPointF(plotArea.left() + xRatio * plotArea.width(), plotArea.bottom() - yRatio * plotArea.height());
        };

        if (_showGridLines) {
            painter->setPen(QPen(QColor(220, 220, 220), 1.0, Qt::DashLine));
            const int ticks = 5;
            for (int i = 1; i < ticks; ++i) {
                const qreal y = plotArea.bottom() - (plotArea.height() * i / ticks);
                painter->drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
            }
            for (int i = 1; i < ticks; ++i) {
                const qreal x = plotArea.left() + (plotArea.width() * i / ticks);
                painter->drawLine(QPointF(x, plotArea.top()), QPointF(x, plotArea.bottom()));
            }
        }

        for (int seriesIndex = 0; seriesIndex < _series.size(); ++seriesIndex) {
            const DataSeries& series = _series.at(seriesIndex);
            QPen seriesPen(series.color, std::max<qreal>(1.0, series.lineWidth));
            painter->setBrush(QBrush(series.color));
            painter->setPen(seriesPen);
            if (_plotType == PlotType::Line) {
                QPainterPath path;
                for (int i = 0; i < series.values.size(); ++i) {
                    QPointF point = pointFor(i, series.values.at(i));
                    if (i == 0) {
                        path.moveTo(point);
                    } else {
                        path.lineTo(point);
                    }
                }
                painter->drawPath(path);
            } else {
                const double groupWidth = sampleCount > 0 ? plotArea.width() / sampleCount : plotArea.width();
                const double barWidth = std::max(2.0, groupWidth / std::max(1, static_cast<int>(_series.size())) - 2.0);
                for (int i = 0; i < series.values.size(); ++i) {
                    const QPointF point = pointFor(i, series.values.at(i));
                    const double x = plotArea.left() + i * groupWidth + seriesIndex * (barWidth + 2.0);
                    painter->drawRect(QRectF(x, point.y(), barWidth, plotArea.bottom() - point.y()));
                }
            }
        }
    }

    if (_showTicks) {
        painter->setPen(axisPen);
        const QFontMetrics fm(painter->font());
        const int ticks = 5;
        for (int i = 0; i <= ticks; ++i) {
            const qreal y = plotArea.bottom() - (plotArea.height() * i / ticks);
            painter->drawLine(QPointF(plotArea.left() - 4.0, y), QPointF(plotArea.left(), y));
            const double value = minValue + ((maxValue - minValue) * i / ticks);
            painter->drawText(QRectF(0, y - fm.height() / 2.0, plotArea.left() - 6.0, fm.height()),
                              Qt::AlignRight | Qt::AlignVCenter,
                              QString::number(value, 'g', 4));
        }
        for (int i = 0; i <= ticks; ++i) {
            const qreal x = plotArea.left() + (plotArea.width() * i / ticks);
            painter->drawLine(QPointF(x, plotArea.bottom()), QPointF(x, plotArea.bottom() + 4.0));
            const double value = xStart + ((xEnd - xStart) * i / ticks);
            painter->drawText(QRectF(x - 20.0, plotArea.bottom() + 4.0, 40.0, fm.height()),
                              Qt::AlignHCenter | Qt::AlignTop,
                              QString::number(value, 'g', 4));
        }
    }

    painter->setPen(Qt::black);
    painter->drawText(QRectF(plotArea.left(), bounds.bottom() - 24.0, plotArea.width(), 16.0), Qt::AlignHCenter, _xAxisTitle);
    painter->save();
    painter->translate(12.0, plotArea.center().y() + 30.0);
    painter->rotate(-90.0);
    painter->drawText(QRectF(-plotArea.height() / 2.0, -8.0, plotArea.height(), 16.0), Qt::AlignHCenter, _yAxisTitle);
    painter->restore();

    int legendX = static_cast<int>(plotArea.left());
    const int legendY = static_cast<int>(bounds.bottom() - 20.0);
    for (const DataSeries& series : _series) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(series.color);
        painter->drawRect(QRectF(legendX, legendY + 5, 10, 10));
        painter->setPen(Qt::black);
        painter->drawText(QRectF(legendX + 14, legendY, 80, 20), Qt::AlignVCenter | Qt::AlignLeft, series.label);
        legendX += 96;
    }

    if (isSelected()) {
        painter->setPen(QPen(Qt::black, 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(bounds.adjusted(1.0, 1.0, -1.0, -1.0));
    }
}

QVector<AnimationPlot::DataSeries> AnimationPlot::parseDatasetsText(const QString& datasetsText) {
    QVector<DataSeries> result;
    const QStringList lines = datasetsText.split(QRegularExpression("[\\r\\n;]+"), Qt::SkipEmptyParts);
    for (const QString& rawLine : lines) {
        const QStringList fields = rawLine.split("|");
        if (fields.size() < 2) {
            continue;
        }
        DataSeries series;
        series.label = fields.at(0).trimmed();
        series.expression = fields.at(1).trimmed();
        if (fields.size() >= 3) {
            const QColor color(fields.at(2).trimmed());
            if (color.isValid()) {
                series.color = color;
            }
        }
        if (fields.size() >= 4) {
            bool ok = false;
            const double width = fields.at(3).trimmed().toDouble(&ok);
            if (ok && width > 0.0) {
                series.lineWidth = width;
            }
        }
        if (!series.label.isEmpty() && !series.expression.isEmpty()) {
            result.append(series);
        }
    }
    return result;
}

QString AnimationPlot::datasetsTextFromSeries(const QVector<DataSeries>& seriesList) {
    QStringList lines;
    for (const DataSeries& series : seriesList) {
        lines << QString("%1|%2|%3|%4").arg(series.label, series.expression, series.color.name()).arg(series.lineWidth, 0, 'f', 2);
    }
    return lines.join("\n");
}

AnimationQueueDisplay::AnimationQueueDisplay() : AnimationPlaceholder("Queue") {
}

AnimationResource::AnimationResource() : AnimationPlaceholder("Resource") {
}

AnimationStation::AnimationStation() : AnimationPlaceholder("Station") {
}

AnimationStatistics::AnimationStatistics() : AnimationPlaceholder("Statistics") {
}

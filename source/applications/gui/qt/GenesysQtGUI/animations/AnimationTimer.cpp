#include "AnimationTimer.h"
#include "../graphicals/ModelGraphicsScene.h"

#include <cmath>

double AnimationTimer::_conversionFactorToSeconds = 1.0;

AnimationTimer::AnimationTimer(ModelGraphicsScene* myScene) : _myScene(myScene), _isDrawingInicialized(true) {
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setActive(true);
    setSelected(false);
    //if (_timeFormat == Util::TimeFormat(12))
    //    _hours = 12; // commented. I want time starts in zero 00:00:00
}

void AnimationTimer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::blue);
    painter->drawRect(boundingRect());

    QString timeText;
    if (_showMicrosseconds) {
        timeText = QString("%1:%2:%3.%4").arg(_hours, 2, 10, QLatin1Char('0')).arg(_minutes, 2, 10, QLatin1Char('0')).
                                          arg(_seconds, 2, 10, QLatin1Char('0')).arg(
                                              _microsseconds, 6, 10, QLatin1Char('0'));
    }
    else {
        timeText = QString("%1:%2:%3").arg(_hours, 2, 10, QLatin1Char('0')).arg(_minutes, 2, 10, QLatin1Char('0')).
                                       arg(_seconds, 2, 10, QLatin1Char('0'));
    }
    QFont font = painter->font();

    // Ajusta o tamanho da fonte com base nas dimensões do retângulo
    int fontSize = qMin(boundingRect().width() / 5, boundingRect().height() / 2);
    // Ajuste os valores conforme necessário
    font.setPixelSize(fontSize);

    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(boundingRect(), Qt::AlignCenter, timeText);

    if (isSelected()) {
        // Tamanho dos quadrados dos cantos
        qreal cornerSize = 10.0;

        // Sem borda
        painter->setPen(Qt::NoPen);

        // Cor dos quadrados
        painter->setBrush(Qt::black);

        // Cantos superiores
        painter->drawRect(QRectF(-cornerSize, -cornerSize, cornerSize, cornerSize));
        painter->drawRect(QRectF(boundingRect().topRight() - QPointF(0, cornerSize), QSizeF(cornerSize, cornerSize)));

        // Cantos inferiores
        painter->drawRect(QRectF(-cornerSize, boundingRect().height(), cornerSize, cornerSize));
        painter->drawRect(QRectF(boundingRect().bottomRight(), QSizeF(cornerSize, cornerSize)));
    }
}

// Getters
double AnimationTimer::getTime() {
    return _time;
}

unsigned int AnimationTimer::getInitialHours() {
    return _initialHours;
}

unsigned int AnimationTimer::getInitialMinutes() {
    return _initialMinutes;
}

unsigned int AnimationTimer::getInitialSeconds() {
    return _initialSeconds;
}

unsigned int AnimationTimer::getHours() {
    return _hours;
}

unsigned int AnimationTimer::getMinutes() {
    return _minutes;
}

unsigned int AnimationTimer::getSeconds() {
    return _seconds;
}

Util::TimeFormat AnimationTimer::getTimeFormat() {
    return _timeFormat;
}

// Setters
void AnimationTimer::setTime(double value) {
    _time = value * _conversionFactorToSeconds;
    convertSeconds();
    update();
}

void AnimationTimer::setConversionFactorToSeconds(double factor) {
    _conversionFactorToSeconds = factor;
}

void AnimationTimer::setInitialHours(unsigned int initialHours) {
    _initialHours = initialHours;
}

void AnimationTimer::setInitialMinutes(unsigned int initialMinutes) {
    _initialMinutes = initialMinutes;
}

void AnimationTimer::setInitialSeconds(unsigned int initialSeconds) {
    _initialSeconds = initialSeconds;
}

void AnimationTimer::setHours(unsigned int hours) {
    _hours = hours;
}

void AnimationTimer::setMinutes(unsigned int minutes) {
    _minutes = minutes;
}

void AnimationTimer::setSeconds(unsigned int seconds) {
    _seconds = seconds;
}

void AnimationTimer::setTimeFormat(Util::TimeFormat timeFormat) {
    _timeFormat = timeFormat;
}

void AnimationTimer::startDrawing(QGraphicsSceneMouseEvent* event) {
    _isResizing = true;
    _startPoint = event->scenePos();
    setPos(_startPoint);
}

void AnimationTimer::continueDrawing(QGraphicsSceneMouseEvent* event) {
    if (_isResizing) {
        // Obtém a posição atual do mouse
        QPointF newPos = event->scenePos();

        // Calcula a diferença entre a posição atual do mouse e a posição inicial do retângulo
        QPointF delta = newPos - _startPoint;

        // Cria o novo retângulo com base na diferença de posição do mouse
        QRectF newRect = QRectF(0, 0, delta.x(), delta.y());

        // Troca o retângulo atual para o novo com as modificações
        setRect(newRect.normalized());

        // Atualiza o item na cena
        update();
    }
}

void AnimationTimer::stopDrawing(QGraphicsSceneMouseEvent* event) {
    // Cria um novo retângulo com as dimensões e posições corretas
    adjustSizeAndPosition(event);

    // Termina de desenhar/redimencionar o retângulo
    _isResizing = false;

    // Indica que acabou de desenhar o objeto
    _isDrawingFinalized = true;
}

void AnimationTimer::adjustSizeAndPosition(QGraphicsSceneMouseEvent* event) {
    qreal minimunX = 0.0;
    qreal minimunY = 0.0;

    qreal maximunX = 0.0;
    qreal maximunY = 0.0;

    if (_startPoint.x() <= event->scenePos().x()) {
        minimunX = _startPoint.x();
        maximunX = event->scenePos().x();
    }
    else {
        minimunX = event->scenePos().x();
        maximunX = _startPoint.x();
    }

    if (_startPoint.y() <= event->scenePos().y()) {
        minimunY = _startPoint.y();
        maximunY = event->scenePos().y();
    }
    else {
        minimunY = event->scenePos().y();
        maximunY = _startPoint.y();
    }

    qreal width = maximunX - minimunX;
    qreal height = maximunY - minimunY;

    // Cria um novo retângulo com as dimensões calculadas
    QRectF newRect = QRectF(0, 0, width, height);

    // Troca o retângulo atual para o novo com as modificações
    setRect(newRect.normalized());

    // Pega a posição
    QPointF position = QPointF(minimunX, minimunY);

    // Seta a posição
    setPos(position);

    // Atualiza o item na cena
    update();
}

void AnimationTimer::convertSeconds() {
    unsigned int timeFormatValue = static_cast<unsigned int>(_timeFormat);
    const long long initialSeconds =
            static_cast<long long>(_initialHours) * 3600
            + static_cast<long long>(_initialMinutes) * 60
            + static_cast<long long>(_initialSeconds);
    const long long totalMicroseconds =
            std::llround(_time * 1000000.0) + initialSeconds * 1000000LL;
    const long long totalSeconds = totalMicroseconds / 1000000LL;

    _microsseconds = static_cast<unsigned int>(totalMicroseconds % 1000000LL);
    _hours = static_cast<unsigned int>(totalSeconds / 3600);
    const unsigned int remainingSeconds = static_cast<unsigned int>(totalSeconds % 3600);
    _minutes = remainingSeconds / 60;
    _seconds = remainingSeconds % 60;

    // Arruma de acordo com o formato de horas
    if (timeFormatValue == 24) {
        _hours %= timeFormatValue;
    }
    else {
        //if (_hours == 0 || _hours == 24) {
        //    _hours = 12;
        //} else
        if (_hours >= 1 && _hours <= 12) {
            // Horas entre 1 e 12 permanecem as mesmas
            _hours += 0;
        }
        else if (_hours >= 13 && _hours <= 23) {
            _hours = _hours - 12;
        }
    }
}

bool AnimationTimer::isDrawingInicialized() {
    return _isDrawingInicialized;
}

bool AnimationTimer::isDrawingFinalized() {
    return _isDrawingFinalized;
}

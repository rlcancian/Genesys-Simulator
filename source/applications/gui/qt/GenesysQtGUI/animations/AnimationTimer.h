#ifndef ANIMATIONTIMER_H
#define ANIMATIONTIMER_H

#include <QGraphicsRectItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "../../../../kernel/util/Util.h"

class ModelGraphicsScene;

class AnimationTimer : public QGraphicsRectItem {
public:
    // Construtor
    AnimationTimer(ModelGraphicsScene* myScene);

    // Sobrecarga do método paint
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Getters
    double getTime();
    unsigned int getInitialHours();
    unsigned int getInitialMinutes();
    unsigned int getInitialSeconds();
    unsigned int getHours();
    unsigned int getMinutes();
    unsigned int getSeconds();
    Util::TimeFormat getTimeFormat();

    // Setters
    void setTime(double value);
    static void setConversionFactorToSeconds(double factor);
    void setInitialHours(unsigned int initialHours);
    void setInitialMinutes(unsigned int initialMinutes);
    void setInitialSeconds(unsigned int initialSeconds);
    void setHours(unsigned int hours);
    void setMinutes(unsigned int minutes);
    void setSeconds(unsigned int seconds);
    void setTimeFormat(Util::TimeFormat timeFormat);

    // Outros
    void startDrawing(QGraphicsSceneMouseEvent* event); // Inicia o desenho da tela
    void continueDrawing(QGraphicsSceneMouseEvent* event); // Continua o desenho na tela
    void stopDrawing(QGraphicsSceneMouseEvent* event); // Finaliza o desenho na tela
    void adjustSizeAndPosition(QGraphicsSceneMouseEvent* event); // Ajusta posição e dimensão do retângulo no final
    void convertSeconds(); // converte o tempo recebido em segundos para horas, dias e segundos
    bool isDrawingInicialized(); // Diz se o relógio começou a ser desenhado
    bool isDrawingFinalized(); // Diz se o relógio terminou de ser desenhado

private:
    ModelGraphicsScene* _myScene;
    double _time = 0.0;
    static double _conversionFactorToSeconds;
    QPointF _startPoint = QPointF(0, 0);
    unsigned int _initialHours = 0;
    unsigned int _initialMinutes = 0;
    unsigned int _initialSeconds = 0;
    unsigned int _hours = 0;
    unsigned int _minutes = 0;
    unsigned int _seconds = 0;
    unsigned int _microsseconds = 0;
    Util::TimeFormat _timeFormat = Util::TimeFormat(24);
    bool _showMicrosseconds = true;
    bool _isResizing = false;
    bool _isDrawingInicialized = false;
    bool _isDrawingFinalized = false;
};

#endif // ANIMATIONTIMER_H

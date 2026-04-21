#ifndef ANIMATIONCOUNTER_H
#define ANIMATIONCOUNTER_H

#include <QGraphicsRectItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "kernel/simulator/Counter.h"

class AnimationCounter : public QGraphicsRectItem {
public:
    // Construtor
    AnimationCounter();

    // Sobrecarga do método paint
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // Getters
    double getValue();
    Counter *getCounter();

    // Setters
    void setValue(double value);
    void setCounter(Counter *myCounter);
    void setIdCounter(int id);
    void setNameCounter(std::string name);

    // Outros
    void startDrawing(QGraphicsSceneMouseEvent *event); // Inicia o desenho da tela
    void continueDrawing(QGraphicsSceneMouseEvent *event); // Continua o desenho na tela
    void stopDrawing(QGraphicsSceneMouseEvent *event); // Finaliza o desenho na tela
    void adjustSizeAndPosition(QGraphicsSceneMouseEvent *event); // Ajusta posição e dimensão do retângulo no final
    bool isDrawingInicialized(); // Diz se o contador começou a ser desenhada
    bool isDrawingFinalized(); // Diz se o contador terminou de ser desenhada
    void setWhenLoaded(QList<Counter *> *counters); // seta o contador quando o modelo é carregado

private:
    double _value = 0.0;
    QPointF _startPoint = QPointF(0, 0);
    bool _isResizing = false;
    bool _isDrawingInicialized = false;
    bool _isDrawingFinalized = false;
    Counter *_counter = nullptr;
    int _idCounter = -1;
    std::string _counterName = "None";
};

#endif // ANIMATIONCOUNTER_H

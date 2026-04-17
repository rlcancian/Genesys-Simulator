#ifndef ANIMATIONVARIABLE_H
#define ANIMATIONVARIABLE_H

#include <QGraphicsRectItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "plugins/data/DiscreteProcessing/Variable.h"

class AnimationVariable : public QGraphicsRectItem {
public:
    // Construtor
    AnimationVariable();

    // Sobrecarga do método paint
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // Getters
    double getValue();
    Variable *getVariable();

    // Setters
    void setValue(double value);
    void setVariable(Variable *variable);
    void setIdVariable(int id);
    void setNameVariable(std::string name);

    // Outros
    void startDrawing(QGraphicsSceneMouseEvent *event); // Inicia o desenho da tela
    void continueDrawing(QGraphicsSceneMouseEvent *event); // Continua o desenho na tela
    void stopDrawing(QGraphicsSceneMouseEvent *event); // Finaliza o desenho na tela
    void adjustSizeAndPosition(QGraphicsSceneMouseEvent *event); // Ajusta posição e dimensão do retângulo no final
    bool isDrawingInicialized(); // Diz se a variável começou a ser desenhada
    bool isDrawingFinalized(); // Diz se a variável terminou de ser desenhada
    void setWhenLoaded(QList<Variable *> *variables); // seta o contador quando o modelo é carregado

private:
    double _value = 0.0;
    QPointF _startPoint = QPointF(0, 0);
    bool _isResizing = false;
    bool _isDrawingInicialized = false;
    bool _isDrawingFinalized = false;
    Variable *_variable = nullptr;
    int _idVariable = -1;
    std::string _variableName = "None";
};

#endif // ANIMATIONVARIABLE_H

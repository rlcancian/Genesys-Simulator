#ifndef ANIMATIONTRANSITION_H
#define ANIMATIONTRANSITION_H

#include <QList>
#include <QPainterPath>
#include <QPointF>
#include <QTimer>
#include <QVariantAnimation>
#include <QElapsedTimer>

#include "graphicals/GraphicalImageAnimation.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalConnection.h"

// Em segundos
#define TEMPO_EXECUCAO_ANIMACAO 2.5

class ModelGraphicsScene;

class AnimationTransition : public QVariantAnimation
{
public:
    // Construtor
    AnimationTransition(ModelGraphicsScene* myScene, ModelComponent* graphicalStartComponent, ModelComponent* graphicalEndComponent, bool viewSimulation);
    // Destrutor
    ~AnimationTransition();

    // Getters
    GraphicalModelComponent* getGraphicalStartComponent() const;
    GraphicalModelComponent* getGraphicalEndComponent() const;
    GraphicalConnection* getGraphicalConnection() const;
    static double getTimeExecution();
    QList<QPointF> getPointsForAnimation() const;
    GraphicalImageAnimation* getImageAnimation() const;
    unsigned int getPortNumber() const;
    // Expose whether transition has all runtime prerequisites to safely animate.
    bool isReadyToRun() const;

    // Setters
    void setImageAnimation(GraphicalImageAnimation* imageAnimation);
    static void setTimeExecution(double timeExecution);
    static void setPause(bool pause);
    static void setRunning(bool running);

    // Inicia a animação
    void startAnimation();
    // Finaliza a animação
    void stopAnimation();
    // Reinicia a animação
    void restartAnimation();
    // Configuração informações para a animação
    void configureAnimation();
    // Atualiza a duração da animação se ela for alterada em tempo de execução
    void updateDurationIfNeeded();
    // Conecta o sinal valueChanged da animação ao slot onAnimationValueChanged
    void connectValueChangedSignal();
    // Conecta o sinal finished da animação ao slot onAnimationFinished
    void connectFinishedSignal();

public slots:
    // Slot para ser conectado ao sinal valueChanged
    void onAnimationValueChanged(const QVariant& value);
    // Slot que será conectado a um sinal
    void onAnimationFinished();

private:
    ModelGraphicsScene* _myScene;
    GraphicalModelComponent* _graphicalStartComponent;
    GraphicalModelComponent* _graphicalEndComponent;
    GraphicalConnection* _graphicalConnection;
    static double _timeExecution;
    static double _oldTimeExecution;
    static bool _pause;
    static bool _running;
    QList<QPointF> _pointsForAnimation;
    QPainterPath _pathForAnimation;
    GraphicalImageAnimation* _imageAnimation;
    unsigned int _portNumber;
    qreal _currentProgress;
    bool _viewSimulation;
    // Track terminal lifecycle paths to keep cleanup idempotent.
    bool _isStopping;
    // Track whether finished callback was already handled.
    bool _isFinishedHandled;
    bool _usesPathAnimation;
};

#endif // ANIMATIONTRANSITION_H

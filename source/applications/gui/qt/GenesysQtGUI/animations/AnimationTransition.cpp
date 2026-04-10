#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#include "AnimationTransition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalImageAnimation.h"

// Inicializando variáveis estáticas

double AnimationTransition::_timeExecution = TEMPO_EXECUCAO_ANIMACAO; // Define um valor inicial para o timeExecution
double AnimationTransition::_oldTimeExecution = TEMPO_EXECUCAO_ANIMACAO;

bool AnimationTransition::_pause = false; // Define um valor inicial para o pause
bool AnimationTransition::_running = true; // Define um valor inicial para o running

AnimationTransition::AnimationTransition(ModelGraphicsScene* myScene, ModelComponent* graphicalStartComponent, ModelComponent* graphicalEndComponent, bool viewSimulation) :
    _myScene(myScene),
    // Initialize potentially deferred members to safe defaults for early-return paths.
    _graphicalStartComponent(nullptr),
    _graphicalEndComponent(nullptr),
    _graphicalConnection(nullptr),
    _imageAnimation(nullptr),
    _portNumber(0),
    _currentProgress(0.0),
    _viewSimulation(viewSimulation),
    // Initialize lifecycle flags for idempotent terminal cleanup paths.
    _isStopping(false),
    _isFinishedHandled(false){

    // Abort construction when required input pointers are missing.
    if (_myScene == nullptr || graphicalStartComponent == nullptr || graphicalEndComponent == nullptr) {
        // Log constructor early exit when mandatory input pointers are missing.
        qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingInput sceneNull=" << (_myScene == nullptr)
                << "sourceNull=" << (graphicalStartComponent == nullptr)
                << "destinationNull=" << (graphicalEndComponent == nullptr);
        return;
    }

    // Pega o componente gráfico de início e fim da animação
    _graphicalStartComponent = _myScene->findGraphicalModelComponent(graphicalStartComponent->getId());
    _graphicalEndComponent = _myScene->findGraphicalModelComponent(graphicalEndComponent->getId());

    // Stop setup when either graphical endpoint is not present in the scene.
    if (_graphicalStartComponent == nullptr || _graphicalEndComponent == nullptr) {
        // Log constructor early exit when graphical endpoints are not found in the scene.
        qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingGraphicalEndpoint sourceId="
                << graphicalStartComponent->getId()
                << "destinationId=" << graphicalEndComponent->getId();
        return;
    }

    if (_graphicalStartComponent && !_graphicalStartComponent->getGraphicalOutputPorts().empty()) {
        QList<GraphicalComponentPort *> startComponentOutputPorts = _graphicalStartComponent->getGraphicalOutputPorts();
        GraphicalConnection* connection = nullptr;

        // Pega a conexão gráfica em que a animação de transição irá percorrer
        for (unsigned int i = 0; i < (unsigned int) startComponentOutputPorts.size(); i++) {
            if (!startComponentOutputPorts.at(i)->getConnections()->empty()) {
                if (startComponentOutputPorts.at(i)->getConnections()->at(0)->getDestination()->component->getId() == _graphicalEndComponent->getComponent()->getId()) {
                    connection = startComponentOutputPorts.at(i)->getConnections()->at(0);
                    _graphicalConnection = connection;
                    _portNumber = i;
                    break;
                }
            }
        }

        // Abort when no matching graphical connection is found for this transition.
        if (connection == nullptr) {
            _graphicalEndComponent = nullptr;
            _graphicalConnection = nullptr;
            // Log constructor early exit when no compatible graphical connection exists.
            qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingConnection sourceId="
                    << graphicalStartComponent->getId()
                    << "destinationId=" << graphicalEndComponent->getId();
            return;
        }

        // Pega o componente de destino do evento/animação de transição
        ModelComponent *destinationComponent;
        destinationComponent = connection->getDestination()->component;
        _graphicalEndComponent = _myScene->findGraphicalModelComponent(destinationComponent->getId());

        // Pega os pontos na tela em que a animação deve ocorrer
        QList<QPointF> pointsConnection = connection->getPoints();

        // Abort transition setup when the connection geometry does not provide the required points.
        if (pointsConnection.size() < 4) {
            _graphicalEndComponent = nullptr;
            _graphicalConnection = nullptr;
            // Log constructor early exit when connection geometry has insufficient points.
            qInfo() << "GUI AnimationTransition ctor earlyExit reason=insufficientPoints count=" << pointsConnection.size();
            return;
        }

        // Tamanho para imagem
        const int imageWidth = 50;
        const int imageHeight = 50;

        // Configurando pontos a serem percorridos na animação
        QPointF startPoint = QPointF(pointsConnection.first().x(), pointsConnection.first().y() - (imageHeight / 2));
        QPointF firstIntermediatePoint = QPointF(pointsConnection.at(1).x() - (imageWidth / 4), pointsConnection.at(1).y() - (imageHeight / 2));
        QPointF secondIntermediatePoint = QPointF(pointsConnection.at(2).x() - (imageWidth / 4), pointsConnection.at(2).y() - (imageHeight / 2));
        QPointF endPoint = QPointF(pointsConnection.last().x() - imageWidth, pointsConnection.last().y() - imageHeight / 2);

        // Adiciona novos pontos à lista
        _pointsForAnimation.append(startPoint);
        _pointsForAnimation.append(firstIntermediatePoint);
        _pointsForAnimation.append(secondIntermediatePoint);
        _pointsForAnimation.append(endPoint);

        // Carrega uma imagem
        _imageAnimation = new GraphicalImageAnimation(startPoint, imageWidth, imageHeight, _graphicalStartComponent->getAnimationImageName());

        // Configura a animação
        configureAnimation();
        // Log constructor completion with endpoint ids and runtime data used by the animation.
        qInfo() << "GUI AnimationTransition ctor sourceId=" << graphicalStartComponent->getId()
                << "destinationId=" << graphicalEndComponent->getId()
                << "imageCreated=" << (_imageAnimation != nullptr)
                << "pointsCount=" << _pointsForAnimation.size();
    }
}

AnimationTransition::~AnimationTransition(){
    this->stopAnimation();
}

// Getters
GraphicalModelComponent* AnimationTransition::getGraphicalStartComponent() const {
    return _graphicalStartComponent;
}

GraphicalModelComponent* AnimationTransition::getGraphicalEndComponent() const {
    return _graphicalEndComponent;
}

GraphicalConnection* AnimationTransition::getGraphicalConnection() const {
    return _graphicalConnection;
}

double AnimationTransition::getTimeExecution() {
    return _timeExecution;
}

QList<QPointF> AnimationTransition::getPointsForAnimation() const {
    return _pointsForAnimation;
}

GraphicalImageAnimation* AnimationTransition::getImageAnimation() const {
    return _imageAnimation;
}

unsigned int AnimationTransition::getPortNumber() const {
    return _portNumber;
}

// Validate transition runtime invariants before starting or resuming animation.
bool AnimationTransition::isReadyToRun() const {
    return _myScene != nullptr
            && _graphicalStartComponent != nullptr
            && _graphicalEndComponent != nullptr
            && _graphicalConnection != nullptr
            && _imageAnimation != nullptr
            && _pointsForAnimation.size() >= 2;
}

// Setters
void AnimationTransition::setImageAnimation(GraphicalImageAnimation* imageAnimation) {
    _imageAnimation = imageAnimation;
}

void AnimationTransition::setTimeExecution(double timeExecution) {
    _timeExecution = timeExecution;
}

void AnimationTransition::setPause(bool pause) {
    _pause = pause;
}

void AnimationTransition::setRunning(bool running) {
    _running = running;
}

// Outros
void AnimationTransition::startAnimation() {
    // Log start request readiness and current animation duration before running.
    qInfo() << "GUI AnimationTransition startAnimation ready=" << isReadyToRun()
            << "durationMs=" << duration();
    // Block animation start when transition invariants are not fully satisfied.
    if (!isReadyToRun()) {
        return;
    }

    // Add the animation image only if it is not already in this scene.
    if (_imageAnimation->scene() != _myScene) {
        _myScene->addItem(_imageAnimation);
    }

    // Atualiza a cena
    _myScene->update();

    // Inicia a animação
    start();
}

void AnimationTransition::stopAnimation() {
    // Log stop request including idempotent path and image ownership status.
    qInfo() << "GUI AnimationTransition stopAnimation idempotent=" << _isStopping
            << "hasImage=" << (_imageAnimation != nullptr);
    // Return immediately when stop cleanup is already in progress.
    if (_isStopping) {
        return;
    }

    // Mark terminal stop path to keep cleanup idempotent across callbacks/destructor.
    _isStopping = true;

    // Stop only when animation is not already stopped.
    if (state() != QAbstractAnimation::Stopped) {
        stop();
    }

    // Remove image only when both pointers are valid and image belongs to this scene.
    if (_myScene != nullptr && _imageAnimation != nullptr && _imageAnimation->scene() == _myScene) {
        _myScene->removeItem(_imageAnimation);
    }

    // Delete the owned image item explicitly on terminal stop cleanup.
    if (_imageAnimation != nullptr) {
        delete _imageAnimation;
        _imageAnimation = nullptr;
    }

    // Update scene only when scene pointer is valid.
    if (_myScene != nullptr) {
        _myScene->update();
    }
}

void AnimationTransition::restartAnimation() {
    // Log restart request readiness and current animation duration before running.
    qInfo() << "GUI AnimationTransition restartAnimation ready=" << isReadyToRun()
            << "durationMs=" << duration();
    // Block animation restart when transition invariants are not fully satisfied.
    if (!isReadyToRun() || duration() <= 0) {
        return;
    }

    // Configura o progresso
    setCurrentTime(_currentProgress * duration());

    // Reinicia a animação
    start();
}

void AnimationTransition::configureAnimation() {
    // Configura informações da animação
    setDuration(_timeExecution * 1000); // Define a duração da animação (em ms, porém _timeExecution é dado em s)
    setStartValue(0.0); // Valor para ponto de partida do progresso da animação
    setEndValue(1.0); // Valor atingido ao término da animação
    setEasingCurve(QEasingCurve::Linear); // Curva de atenuação entre os pontos no progresso da animação

    connectValueChangedSignal();
    connectFinishedSignal();
}

void AnimationTransition::updateDurationIfNeeded() {
    if (_oldTimeExecution != _timeExecution) {
        _oldTimeExecution = _timeExecution;

        double newTimeExecution;

        if (_oldTimeExecution < _timeExecution) {
            qreal elapsedTime = currentTime();
            newTimeExecution = _timeExecution - elapsedTime;
        } else {
            newTimeExecution = _timeExecution;
        }

        if (newTimeExecution < 0.05) {
            newTimeExecution = 0.05;
        }

        _timeExecution = newTimeExecution;

        setDuration(_timeExecution * 1000);
    }
}

void AnimationTransition::connectValueChangedSignal() {
    // Conecta o sinal valueChanged ao slot onAnimationValueChanged
    QObject::connect(this, &QVariantAnimation::valueChanged, this, &AnimationTransition::onAnimationValueChanged);
}


void AnimationTransition::onAnimationValueChanged(const QVariant& value) {
    // Stop and exit immediately when simulation runtime is no longer running.
    if (_running == false) {
        // Log runtime deviation when animation receives ticks while running flag is false.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=runningFalse";
        stopAnimation();
        return;
    }

    // Pause and exit immediately to avoid running animation logic while paused.
    if (_pause == true) {
        // Log runtime deviation when animation receives ticks while paused.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=pausedTrue";
        pause();
        return;
    }

    // Validate mandatory runtime pointers and geometry before processing interpolation.
    if (_myScene == nullptr || _imageAnimation == nullptr || _pointsForAnimation.size() < 2) {
        // Log runtime deviation when mandatory animation guards are invalid.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=invalidGuards"
                << "sceneNull=" << (_myScene == nullptr)
                << "imageNull=" << (_imageAnimation == nullptr)
                << "pointsCount=" << _pointsForAnimation.size();
        return;
    }

    updateDurationIfNeeded();

    // Progresso atual da animação (valor entre startValue e endValue)
    _currentProgress = value.toReal();

    // Compute total path distance and exit on degenerate geometry.
    int numSegments = _pointsForAnimation.size() - 1;
    qreal totalDistance = 0.0;
    for (int i = 0; i < numSegments; ++i) {
        totalDistance += QLineF(_pointsForAnimation[i], _pointsForAnimation[i + 1]).length();
    }
    if (totalDistance <= 0.0) {
        // Log runtime deviation when total path geometry distance is degenerate.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=totalDistanceNonPositive";
        return;
    }

    qreal distanceCovered = _currentProgress * totalDistance;
    qreal currentDistance = 0.0;

    // Find the active segment while keeping segment index inside valid bounds.
    int currentSegment = 0;
    while (currentSegment < numSegments &&
           currentDistance + QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length() < distanceCovered) {
        currentDistance += QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length();
        ++currentSegment;
    }
    if (currentSegment < 0 || currentSegment >= numSegments) {
        // Log runtime deviation when current interpolation segment index is invalid.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=invalidSegmentIndex"
                << "currentSegment=" << currentSegment
                << "numSegments=" << numSegments;
        return;
    }

    // Guard segment interpolation against zero-length segments before dividing.
    qreal segmentLength = QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length();
    if (segmentLength <= 0.0) {
        // Log runtime deviation when active segment length is non-positive.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=segmentLengthNonPositive";
        return;
    }

    qreal segmentProgress = (distanceCovered - currentDistance) / segmentLength;
    QPointF start = _pointsForAnimation[currentSegment];
    QPointF end = _pointsForAnimation[currentSegment + 1];
    QPointF imagePosition = start * (1 - segmentProgress) + end * segmentProgress;

    // Apply interpolated position and refresh the scene in validated runtime path.
    _imageAnimation->setPos(imagePosition);
    _myScene->update();
}

void AnimationTransition::connectFinishedSignal() {
    // Conecta o sinal finished ao slot onAnimationFinished
    QObject::connect(this, &QVariantAnimation::finished, this, &AnimationTransition::onAnimationFinished);
}

void AnimationTransition::onAnimationFinished() {
    // Log finished callback entry and whether this callback was already processed.
    qInfo() << "GUI AnimationTransition onAnimationFinished begin alreadyHandled=" << _isFinishedHandled;
    // Return when finished callback cleanup already ran before.
    if (_isFinishedHandled) {
        return;
    }

    // Mark finished callback as handled for idempotent terminal cleanup.
    _isFinishedHandled = true;

    // Only notify queue insertion when both scene and destination component are valid.
    if (_myScene != nullptr && _graphicalEndComponent != nullptr) {
        // Log queue-insert dispatch performed on a valid destination component.
        qInfo() << "GUI AnimationTransition onAnimationFinished animateQueueInsert=true";
        _myScene->animateQueueInsert(_graphicalEndComponent->getComponent(), _viewSimulation);
    }

    // Remove image only when it is valid and still attached to this scene.
    if (_myScene != nullptr && _imageAnimation != nullptr && _imageAnimation->scene() == _myScene) {
        // Log image removal from scene during finished cleanup.
        qInfo() << "GUI AnimationTransition onAnimationFinished cleanup=removeImageFromScene";
        _myScene->removeItem(_imageAnimation);
    }

    // Delete the owned image item explicitly after animation completion.
    if (_imageAnimation != nullptr) {
        // Log image object destruction during finished cleanup.
        qInfo() << "GUI AnimationTransition onAnimationFinished cleanup=deleteImage";
        delete _imageAnimation;
        _imageAnimation = nullptr;
    }

    // Update scene only when scene pointer is valid.
    if (_myScene != nullptr) {
        _myScene->update();
    }
}

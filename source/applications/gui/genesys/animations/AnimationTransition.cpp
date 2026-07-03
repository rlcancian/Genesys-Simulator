#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#include "AnimationTransition.h"
#include "graphicals/GraphicalConnectionStyle.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalImageAnimation.h"

// Inicializando variáveis estáticas

double AnimationTransition::_timeExecution = TEMPO_EXECUCAO_ANIMACAO; // Define um valor inicial para o timeExecution
double AnimationTransition::_oldTimeExecution = TEMPO_EXECUCAO_ANIMACAO;

bool AnimationTransition::_pause = false; // Define um valor inicial para o pause
bool AnimationTransition::_running = true; // Define um valor inicial para o running

AnimationTransition::AnimationTransition(ModelGraphicsScene* myScene, ModelComponent* graphicalStartComponent,
                                         ModelComponent* graphicalEndComponent, bool viewSimulation) :
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
    _isFinishedHandled(false),
    _usesPathAnimation(false) {
    // Abort construction when required input pointers are missing.
    if (_myScene == nullptr || graphicalStartComponent == nullptr || graphicalEndComponent == nullptr) {
        // Log constructor early exit with transition correlation fields.
        qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingInput sceneNull=" << (_myScene == nullptr)
            << "transitionPtr=" << this
            << "sourceNull=" << (graphicalStartComponent == nullptr)
            << "destinationNull=" << (graphicalEndComponent == nullptr);
        return;
    }

    // Pega o componente gráfico de início e fim da animação
    _graphicalStartComponent = _myScene->findGraphicalModelComponent(graphicalStartComponent->getId());
    _graphicalEndComponent = _myScene->findGraphicalModelComponent(graphicalEndComponent->getId());

    // Stop setup when either graphical endpoint is not present in the scene.
    if (_graphicalStartComponent == nullptr || _graphicalEndComponent == nullptr) {
        // Log constructor early exit with endpoint ids for transition correlation.
        qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingGraphicalEndpoint sourceId="
            << "transitionPtr=" << this
            << graphicalStartComponent->getId()
            << "destinationId=" << graphicalEndComponent->getId();
        return;
    }

    if (_graphicalStartComponent && !_graphicalStartComponent->getGraphicalOutputPorts().empty()) {
        QList<GraphicalComponentPort*> startComponentOutputPorts = _graphicalStartComponent->getGraphicalOutputPorts();
        GraphicalConnection* connection = nullptr;

        // Pega a conexão gráfica em que a animação de transição irá percorrer
        for (unsigned int i = 0; i < (unsigned int)startComponentOutputPorts.size(); i++) {
            if (!startComponentOutputPorts.at(i)->getConnections()->empty()) {
                if (startComponentOutputPorts.at(i)->getConnections()->at(0)->getDestination()->component->getId() ==
                    _graphicalEndComponent->getComponent()->getId()) {
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
            // Log constructor early exit when no graphical connection can be resolved.
            qInfo() << "GUI AnimationTransition ctor earlyExit reason=missingConnection sourceId="
                << "transitionPtr=" << this
                << graphicalStartComponent->getId()
                << "destinationId=" << graphicalEndComponent->getId();
            return;
        }

        // Pega o componente de destino do evento/animação de transição
        ModelComponent* destinationComponent;
        destinationComponent = connection->getDestination()->component;
        _graphicalEndComponent = _myScene->findGraphicalModelComponent(destinationComponent->getId());

        // Tamanho para imagem
        const int imageWidth = 50;
        const int imageHeight = 50;

        // Pega os pontos na tela em que a animação deve ocorrer
        QList<QPointF> pointsConnection = connection->getPoints();
        QPointF startPoint;
        _usesPathAnimation = connection->usesCurvedStyle();
        if (_usesPathAnimation) {
            _pathForAnimation = connection->animationPathForImage(imageWidth, imageHeight);
            if (_pathForAnimation.length() <= 0.0) {
                _graphicalEndComponent = nullptr;
                _graphicalConnection = nullptr;
                qInfo() << "GUI AnimationTransition ctor earlyExit reason=invalidCurvedPath transitionPtr=" << this
                    << "sourceId=" << graphicalStartComponent->getId()
                    << "destinationId=" << graphicalEndComponent->getId();
                return;
            }
            startPoint = GraphicalConnectionStyle::pointAtProgress(_pathForAnimation, 0.0);
            _pointsForAnimation.append(startPoint);
            _pointsForAnimation.append(GraphicalConnectionStyle::pointAtProgress(_pathForAnimation, 1.0));
        } else {
            // Abort transition setup when the connection geometry does not provide the required points.
            if (pointsConnection.size() < 4) {
                _graphicalEndComponent = nullptr;
                _graphicalConnection = nullptr;
                // Log constructor early exit when connection geometry cannot build animation path.
                qInfo() << "GUI AnimationTransition ctor earlyExit reason=insufficientPoints transitionPtr=" << this
                    << "sourceId=" << graphicalStartComponent->getId()
                    << "destinationId=" << graphicalEndComponent->getId()
                    << "pointsCount=" << pointsConnection.size();
                return;
            }

            // Configurando pontos a serem percorridos na animação
            startPoint = QPointF(pointsConnection.first().x(), pointsConnection.first().y() - (imageHeight / 2));
            QPointF firstIntermediatePoint = QPointF(pointsConnection.at(1).x() - (imageWidth / 4),
                                                     pointsConnection.at(1).y() - (imageHeight / 2));
            QPointF secondIntermediatePoint = QPointF(pointsConnection.at(2).x() - (imageWidth / 4),
                                                      pointsConnection.at(2).y() - (imageHeight / 2));
            QPointF endPoint = QPointF(pointsConnection.last().x() - imageWidth,
                                       pointsConnection.last().y() - imageHeight / 2);

            // Adiciona novos pontos à lista
            _pointsForAnimation.append(startPoint);
            _pointsForAnimation.append(firstIntermediatePoint);
            _pointsForAnimation.append(secondIntermediatePoint);
            _pointsForAnimation.append(endPoint);
        }

        // Carrega uma imagem
        _imageAnimation = new GraphicalImageAnimation(startPoint, imageWidth, imageHeight,
                                                      _graphicalStartComponent->getAnimationImageName());

        // Configura a animação
        configureAnimation();
        // Log constructor completion with transition pointer and path readiness context.
        qInfo() << "GUI AnimationTransition ctor sourceId=" << graphicalStartComponent->getId()
            << "transitionPtr=" << this
            << "destinationId=" << graphicalEndComponent->getId()
            << "ready=" << isReadyToRun()
            << "hasImage=" << (_imageAnimation != nullptr)
            << "pointsCount=" << _pointsForAnimation.size()
            << "usesPathAnimation=" << _usesPathAnimation;
    }
}

AnimationTransition::~AnimationTransition() {
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
    // Log start request with transition correlation keys and runtime invariants.
    qInfo() << "GUI AnimationTransition startAnimation ready=" << isReadyToRun()
        << "transitionPtr=" << this
        << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
        << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
        << "hasImage=" << (_imageAnimation != nullptr)
        << "pointsCount=" << _pointsForAnimation.size()
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
    // Log stop request with transition pointer and endpoint correlation metadata.
    qInfo() << "GUI AnimationTransition stopAnimation idempotent=" << _isStopping
        << "transitionPtr=" << this
        << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
        << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
        << "ready=" << isReadyToRun()
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
    // Log restart request with transition correlation keys and runtime invariants.
    qInfo() << "GUI AnimationTransition restartAnimation ready=" << isReadyToRun()
        << "transitionPtr=" << this
        << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
        << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
        << "hasImage=" << (_imageAnimation != nullptr)
        << "pointsCount=" << _pointsForAnimation.size()
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
        }
        else {
            newTimeExecution = _timeExecution;
        }

        if (newTimeExecution < 0.001) {
            newTimeExecution = 0.001;
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
        // Log running-flag deviation including transition correlation keys.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=runningFalse"
            << "transitionPtr=" << this;
        stopAnimation();
        return;
    }

    // Pause and exit immediately to avoid running animation logic while paused.
    if (_pause == true) {
        // Log pause-flag deviation including transition correlation keys.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=pausedTrue"
            << "transitionPtr=" << this;
        pause();
        return;
    }

    // Validate mandatory runtime pointers and geometry before processing interpolation.
    if (_myScene == nullptr || _imageAnimation == nullptr || _pointsForAnimation.size() < 2) {
        // Log invalid runtime guard deviation with full transition context.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=invalidGuards"
            << "transitionPtr=" << this
            << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
            << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
            << "ready=" << isReadyToRun()
            << "hasImage=" << (_imageAnimation != nullptr)
            << "sceneNull=" << (_myScene == nullptr)
            << "imageNull=" << (_imageAnimation == nullptr)
            << "pointsCount=" << _pointsForAnimation.size();
        return;
    }

    updateDurationIfNeeded();

    // Progresso atual da animação (valor entre startValue e endValue)
    _currentProgress = value.toReal();

    if (_usesPathAnimation) {
        if (_pathForAnimation.length() <= 0.0) {
            qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=invalidCurvedPath"
                << "transitionPtr=" << this;
            return;
        }
        _imageAnimation->setPos(GraphicalConnectionStyle::pointAtProgress(_pathForAnimation, _currentProgress));
        _myScene->update();
        return;
    }

    // Compute total path distance and exit on degenerate geometry.
    int numSegments = _pointsForAnimation.size() - 1;
    qreal totalDistance = 0.0;
    for (int i = 0; i < numSegments; ++i) {
        totalDistance += QLineF(_pointsForAnimation[i], _pointsForAnimation[i + 1]).length();
    }
    if (totalDistance <= 0.0) {
        // Log degenerate path deviation including transition correlation keys.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=totalDistanceNonPositive"
            << "transitionPtr=" << this
            << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
            << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
            << "pointsCount=" << _pointsForAnimation.size();
        return;
    }

    qreal distanceCovered = _currentProgress * totalDistance;
    qreal currentDistance = 0.0;

    // Find the active segment while keeping segment index inside valid bounds.
    int currentSegment = 0;
    while (currentSegment < numSegments &&
        currentDistance + QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length()
        < distanceCovered) {
        currentDistance += QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).
            length();
        ++currentSegment;
    }
    if (currentSegment < 0 || currentSegment >= numSegments) {
        // Log invalid segment-index deviation including transition correlation keys.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=invalidSegmentIndex"
            << "transitionPtr=" << this
            << "currentSegment=" << currentSegment
            << "numSegments=" << numSegments;
        return;
    }

    // Guard segment interpolation against zero-length segments before dividing.
    qreal segmentLength = QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length();
    if (segmentLength <= 0.0) {
        // Log zero-length segment deviation including transition correlation keys.
        qInfo() << "GUI AnimationTransition onAnimationValueChanged deviation=segmentLengthNonPositive"
            << "transitionPtr=" << this;
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
    // Log finished callback entry with transition correlation context.
    qInfo() << "GUI AnimationTransition onAnimationFinished begin alreadyHandled=" << _isFinishedHandled;
    qInfo() << "GUI AnimationTransition onAnimationFinished context transitionPtr=" << this
        << "sourceId=" << (_graphicalStartComponent ? _graphicalStartComponent->getComponent()->getId() : 0)
        << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0)
        << "ready=" << isReadyToRun()
        << "hasImage=" << (_imageAnimation != nullptr)
        << "pointsCount=" << _pointsForAnimation.size();
    // Return when finished callback cleanup already ran before.
    if (_isFinishedHandled) {
        return;
    }

    // Mark finished callback as handled for idempotent terminal cleanup.
    _isFinishedHandled = true;

    // Insert queue animation only when destination queue graphics infrastructure is initialized.
    bool shouldAnimateQueueInsert = false;
    if (_graphicalEndComponent != nullptr && _graphicalEndComponent->hasQueue()) {
        QList<QList<GraphicalImageAnimation*>*>* imagesQueue = _graphicalEndComponent->getImagesQueue();
        shouldAnimateQueueInsert = (imagesQueue != nullptr && !imagesQueue->empty() && imagesQueue->at(0) != nullptr);
    }
    qInfo() << "GUI AnimationTransition onAnimationFinished queueInsertDecision="
        << (shouldAnimateQueueInsert ? "insert" : "skip")
        << "destinationId=" << (_graphicalEndComponent ? _graphicalEndComponent->getComponent()->getId() : 0);
    if (_myScene != nullptr && _graphicalEndComponent != nullptr && shouldAnimateQueueInsert) {
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

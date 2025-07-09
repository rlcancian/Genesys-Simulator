#include <QEventLoop>
#include <QThread>
#include <QCoreApplication>

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
    _graphicalEndComponent(nullptr),
    _imageAnimation(nullptr),
    _viewSimulation(viewSimulation){

    // Pega o componente gráfico de início e fim da animação
    _graphicalStartComponent = _myScene->findGraphicalModelComponent(graphicalStartComponent->getId());
    _graphicalEndComponent = _myScene->findGraphicalModelComponent(graphicalEndComponent->getId());

    if (_graphicalStartComponent && !_graphicalStartComponent->getGraphicalOutputPorts().empty()) {
        QList<GraphicalComponentPort *> startComponentOutputPorts = _graphicalStartComponent->getGraphicalOutputPorts();
        GraphicalConnection* connection = nullptr;

        // Pega a conexão gráfica em que a animação de transição irá percorrer
        for (unsigned int i = 0; i < (unsigned int) startComponentOutputPorts.size(); i++) {
            if (!startComponentOutputPorts.at(i)->getConnections()->empty()) {
                if (startComponentOutputPorts.at(i)->getConnections()->at(0)->getDestination()->component->getId() == _graphicalEndComponent->getComponent()->getId()) {
                    connection = startComponentOutputPorts.at(i)->getConnections()->at(0);
                    _portNumber = i;
                    break;
                }
            }
        }

        // Pega o componente de destino do evento/animação de transição
        ModelComponent *destinationComponent;
        if (connection == nullptr) {
            _graphicalEndComponent = nullptr;
            return;
        }

        destinationComponent = connection->getDestination()->component;
        _graphicalEndComponent = _myScene->findGraphicalModelComponent(destinationComponent->getId());

        // Pega os pontos na tela em que a animação deve ocorrer
        QList<QPointF> pointsConnection = connection->getPoints();

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
    // Adiciona a imagem na cena
    _myScene->addItem(_imageAnimation);

    // Atualiza a cena
    _myScene->update();

    // Inicia a animação
    start();
}

void AnimationTransition::stopAnimation() {
    // Remove a imagem na cena
    _myScene->removeItem(_imageAnimation);

    // Para a animação
    stop();

    // Atualiza a cena
    _myScene->update();

    // Emite o sinal de encerramento de animação (caso opte por encerrar por essa função)
    emit finished();
}

void AnimationTransition::restartAnimation() {
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
    if (_running == false)
        stopAnimation();

    if (_pause == true) {
        pause();
    }

    updateDurationIfNeeded();

    // Progresso atual da animação (valor entre startValue e endValue)
    _currentProgress = value.toReal();

    // Se há pontos a serem percorridos, entra na condição
    if (!_pointsForAnimation.isEmpty()) {
        // Número de segmentos, ou seja, quantas linhas serão percorridas
        int numSegments = _pointsForAnimation.size() - 1;

        // Distância total a ser percorrida pela animação
        qreal totalDistance = 0.0;

        // Calcula a distância total entre os pontos
        for (int i = 0; i < numSegments; ++i) {
            totalDistance += QLineF(_pointsForAnimation[i], _pointsForAnimation[i + 1]).length();
        }

        qreal distanceCovered = _currentProgress * totalDistance;
        qreal currentDistance = 0.0;

        // Encontra o segmento onde a imagem está atualmente
        int currentSegment = 0;
        while (currentSegment < numSegments && currentDistance + QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length() < distanceCovered) {
            currentDistance += QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length();
            ++currentSegment;
        }

        // Calcula a posição interpolada dentro do segmento atual
        qreal segmentProgress = (distanceCovered - currentDistance) / QLineF(_pointsForAnimation[currentSegment], _pointsForAnimation[currentSegment + 1]).length();
        QPointF start = _pointsForAnimation[currentSegment];
        QPointF end = _pointsForAnimation[currentSegment + 1];
        QPointF imagePosition = start * (1 - segmentProgress) + end * segmentProgress;

        // Nova posição da imagem
        _imageAnimation->setPos(imagePosition);

        // Atualiza a cena
        _myScene->update();
    }
}

void AnimationTransition::connectFinishedSignal() {
    // Conecta o sinal finished ao slot onAnimationFinished
    QObject::connect(this, &QVariantAnimation::finished, this, &AnimationTransition::onAnimationFinished);
}

void AnimationTransition::onAnimationFinished() {
    // Adiciona animação de fila se for o caso
    if(_graphicalEndComponent != nullptr)
        _myScene->animateQueueInsert(_graphicalEndComponent->getComponent(), _viewSimulation);

    // Remove o item da cena
    _myScene->removeItem(_imageAnimation);

    // Atualiza a cena
    _myScene->update();
}

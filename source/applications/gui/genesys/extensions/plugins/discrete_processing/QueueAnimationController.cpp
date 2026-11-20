#include "QueueAnimationController.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

// Contrutor
QueueAnimationController::QueueAnimationController(ModelGraphicsScene* myScene, GraphicalModelComponent* graphicalComponent) :
    _myScene(myScene),
    _graphicalComponent(nullptr)
{
        _graphicalComponent = graphicalComponent;
}

// Destrutor
QueueAnimationController::~QueueAnimationController(){}

// Getters
GraphicalModelComponent* QueueAnimationController::getGraphicalComponent() const {
    return _graphicalComponent;
}

// Verifica a necessidade de adicionar imagem na fila com base no tamanho da fila do componente
void QueueAnimationController::verifyAddAnimationQueue(bool visivible) {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasAnimationTracks()) {
            QList<ModelDataDefinition *> queues = _graphicalComponent->getAnimationTrackMap()->keys();

            for (ModelDataDefinition *data : queues) {
                auto* queue = dynamic_cast<Queue*>(data);
                if (queue == nullptr) continue;
                unsigned int newQueueSize = (unsigned int) queue->size();
                unsigned int oldQueueSize = _graphicalComponent->getAnimationTrackSize(queue);
                unsigned int indexQueue = _graphicalComponent->getAnimationTrackIndex(queue);

                if (newQueueSize > oldQueueSize) {
                    unsigned int width = 30;
                    unsigned int height = 30;

                    unsigned int quantityAdd = newQueueSize - oldQueueSize;

                    for (unsigned int i = 0; i < quantityAdd; i++) {
                        QString animationImageName = _graphicalComponent->getAnimationImageName();

                        QPointF position = calculatePositionImageQueue(indexQueue, (i + 1) + oldQueueSize, width, height);

                        GraphicalImageAnimation *image = new GraphicalImageAnimation(position, width, height, animationImageName);

                        bool res = _graphicalComponent->insertAnimationImage(queue, image, oldQueueSize + i + 1);

                        if (res) {
                            _myScene->addItem(image);
                            image->setVisible(visivible);
                        }

                        _myScene->update();
                    }
                }
            }
        }
    }
}

// Verifica a necessidade de remover imagem da fila com base no tamanho da fila do componente
void QueueAnimationController::verifyRemoveAnimationQueue() {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasAnimationTracks()) {
            QList<ModelDataDefinition *> queues = _graphicalComponent->getAnimationTrackMap()->keys();

            for (ModelDataDefinition *data : queues) {
                auto* queue = dynamic_cast<Queue*>(data);
                if (queue == nullptr) continue;
                unsigned int newQueueSize = (unsigned int) queue->size();
                unsigned int oldQueueSize = _graphicalComponent->getAnimationTrackSize(queue);

                if (newQueueSize < oldQueueSize) {
                    unsigned int quantityRemoved = oldQueueSize - newQueueSize;

                    QList<GraphicalImageAnimation *> *imageRemoved = _graphicalComponent->removeAnimationImages(queue, quantityRemoved, newQueueSize);
                    if (imageRemoved) {
                        for (unsigned int i = 0; i < (unsigned int) imageRemoved->size(); i++) {
                            // pega a imagem a ser removida
                            GraphicalImageAnimation *image = imageRemoved->at(i);

                            // remove ela da cena
                            _myScene->removeItem(image);

                            // libera o espaço de memória dela
                            delete image;
                        }
                        // atualiza a cena
                        _myScene->update();

                        // limpa a lista que foi retornada
                        imageRemoved->clear();

                        // libera o espaço de memória da lista
                        delete imageRemoved;
                    }
                }
            }
        }
    }
}

// Se o componente for um componente de fila, adiciona animação
void QueueAnimationController::addAnimationQueue(bool visivible) {
    // Guard queue animation insertion when queue graphics infrastructure is not initialized.
    if (_graphicalComponent) {
        if (_graphicalComponent->hasAnimationTracks()) {
            QList<QList<GraphicalImageAnimation *>*> *imagesQueue = _graphicalComponent->getAnimationTracks();
            if (imagesQueue == nullptr || imagesQueue->empty() || imagesQueue->at(0) == nullptr) {
                qInfo() << "AnimationQueue: queue animation insertion skipped due to uninitialized queue graphics infrastructure";
                return;
            }

            QString animationImageName = _graphicalComponent->getAnimationImageName();
            unsigned int width = 30;
            unsigned int height = 30;

            unsigned int sizeQueue = (unsigned int) imagesQueue->at(0)->size();

            QPointF position = calculatePositionImageQueue(0, sizeQueue, width, height);

            GraphicalImageAnimation *image = new GraphicalImageAnimation(position, width, height, animationImageName);

            bool res = _graphicalComponent->insertAnimationImage(image);

            if (res) {
                _myScene->addItem(image);
                image->setVisible(visivible);
            }

            _myScene->update();
        }
        else {
            qInfo() << "AnimationQueue: queue animation insertion skipped due to uninitialized queue graphics infrastructure";
        }
    } else {
        qInfo() << "AnimationQueue: queue animation insertion skipped due to uninitialized queue graphics infrastructure";
    }
}

// Se o componente for um componente de fila, remove a animação
void QueueAnimationController::removeAnimationQueue() {
    // Guard batch queue removal path to avoid out-of-range access when queue list is empty.
    if (_graphicalComponent) {
        if (_graphicalComponent->hasAnimationTracks()) {

            unsigned int images = 1;

            if (_graphicalComponent->getComponent()->getClassname() == "Batch") {
                QList<QList<GraphicalImageAnimation *>*> *imagesQueue = _graphicalComponent->getAnimationTracks();
                if (imagesQueue != nullptr && !imagesQueue->empty() && imagesQueue->at(0) != nullptr)
                    images = (unsigned int) _graphicalComponent->getAnimationTracks()->at(0)->size();
            }

            for (unsigned int i = 0; i < images; i++) {
                GraphicalImageAnimation *imageRemoved = _graphicalComponent->removeAnimationImage();

                if (imageRemoved) {
                    _myScene->removeItem(imageRemoved);

                    delete imageRemoved;
                }

                _myScene->update();
            }
        }
    }
}

// Calcula onde a imagem será colocada
QPointF QueueAnimationController::calculatePositionImageQueue(unsigned int indexQueue, unsigned int sizeQueue, unsigned int width, unsigned int height) {
    unsigned int multiplierX = sizeQueue + 1;
    unsigned int multiplierY = indexQueue + 1;
    unsigned int spaceBetween = 2;

    qreal gmcPosX = _graphicalComponent->sceneBoundingRect().topRight().x();
    qreal gmcPosY = _graphicalComponent->sceneBoundingRect().topRight().y();

    qreal calculatePositionX = gmcPosX - ((multiplierX * width) + ((multiplierX - 1) * spaceBetween));
    qreal calculatePositionY = gmcPosY - ((height * multiplierY) + (multiplierY * spaceBetween));

    QPointF position(calculatePositionX, calculatePositionY);

    return position;
}

#include "AnimationQueue.h"
#include "graphicals/ModelGraphicsScene.h"

// Contrutor
AnimationQueue::AnimationQueue(ModelGraphicsScene* myScene, ModelComponent* graphicalComponent) :
    _myScene(myScene),
    _graphicalComponent(nullptr)
{
        _graphicalComponent = _myScene->findGraphicalModelComponent(graphicalComponent->getId());
}

// Destrutor
AnimationQueue::~AnimationQueue(){}

// Getters
GraphicalModelComponent* AnimationQueue::getGraphicalComponent() const {
    return _graphicalComponent;
}

// Verifica a necessidade de adicionar imagem na fila com base no tamanho da fila do componente
void AnimationQueue::verifyAddAnimationQueue(bool visivible) {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasQueue()) {
            QList<Queue *> queues = _graphicalComponent->getMapQueue()->keys();

            for (Queue *queue : queues) {
                unsigned int newQueueSize = (unsigned int) queue->size();
                unsigned int oldQueueSize = _graphicalComponent->getSizeQueue(queue);
                unsigned int indexQueue = _graphicalComponent->getIndexQueue(queue);

                if (newQueueSize > oldQueueSize) {
                    unsigned int width = 30;
                    unsigned int height = 30;

                    unsigned int quantityAdd = newQueueSize - oldQueueSize;

                    for (unsigned int i = 0; i < quantityAdd; i++) {
                        QString animationImageName = _graphicalComponent->getAnimationImageName();

                        QPointF position = calculatePositionImageQueue(indexQueue, (i + 1) + oldQueueSize, width, height);

                        GraphicalImageAnimation *image = new GraphicalImageAnimation(position, width, height, animationImageName);

                        bool res = _graphicalComponent->insertImageQueue(queue, image);

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
void AnimationQueue::verifyRemoveAnimationQueue() {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasQueue()) {
            QList<Queue *> queues = _graphicalComponent->getMapQueue()->keys();

            for (Queue *queue : queues) {
                unsigned int newQueueSize = (unsigned int) queue->size();
                unsigned int oldQueueSize = _graphicalComponent->getSizeQueue(queue);

                if (newQueueSize < oldQueueSize) {
                    unsigned int quantityRemoved = oldQueueSize - newQueueSize;

                    QList<GraphicalImageAnimation *> *imageRemoved = _graphicalComponent->removeImageQueue(queue, quantityRemoved);
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
void AnimationQueue::addAnimationQueue(bool visivible) {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasQueue()) {
            QString animationImageName = _graphicalComponent->getAnimationImageName();
            unsigned int width = 30;
            unsigned int height = 30;

            unsigned int sizeQueue = (unsigned int) _graphicalComponent->getImagesQueue()->at(0)->size();

            QPointF position = calculatePositionImageQueue(0, sizeQueue, width, height);

            GraphicalImageAnimation *image = new GraphicalImageAnimation(position, width, height, animationImageName);

            bool res = _graphicalComponent->insertImageQueue(image);

            if (res) {
                _myScene->addItem(image);
                image->setVisible(visivible);
            }

            _myScene->update();
        }
    }
}

// Se o componente for um componente de fila, remove a animação
void AnimationQueue::removeAnimationQueue() {
    if (_graphicalComponent) {
        if (_graphicalComponent->hasQueue()) {

            unsigned int images = 1;

            if (_graphicalComponent->getComponent()->getClassname() == "Batch") {
                if (!_graphicalComponent->getImagesQueue()->empty())
                    images = (unsigned int) _graphicalComponent->getImagesQueue()->at(0)->size();
            }

            for (unsigned int i = 0; i < images; i++) {
                GraphicalImageAnimation *imageRemoved = _graphicalComponent->removeImageQueue();

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
QPointF AnimationQueue::calculatePositionImageQueue(unsigned int indexQueue, unsigned int sizeQueue, unsigned int width, unsigned int height) {
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

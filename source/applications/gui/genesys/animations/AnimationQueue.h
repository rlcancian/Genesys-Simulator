#ifndef ANIMATIONQUEUE_H
#define ANIMATIONQUEUE_H

#include "graphicals/GraphicalModelComponent.h"

class ModelGraphicsScene;

class AnimationQueue
{
public:
    // Construtor
    AnimationQueue(ModelGraphicsScene* myScene, ModelComponent* component);
    // Destrutor
    ~AnimationQueue();

    // Getters
    GraphicalModelComponent* getGraphicalComponent() const;

    // quando uma entidade esta movendo em relacao a um componente que tem fila, adiciona a animação
    void addAnimationQueue(bool visivible);
    // quando uma entidade esta movendo de um componente que tem fila, remove a animação
    void removeAnimationQueue();

    // faz a animação de acordo com o tamanho da fila, mas não ficou muito condizente com a visualização da animação
    // verifica se precisa adiconar animação e adiciona se for o caso (observando valores da fila)
    void verifyAddAnimationQueue(bool visivible);
    // verifica se precisa remover animação e remove se for o caso (observando valores da fila)
    void verifyRemoveAnimationQueue();

    // retorna a posição em que a animação da fila será desenhada
    QPointF calculatePositionImageQueue(unsigned int indexQueue, unsigned int sizeQueue, unsigned int width, unsigned int height);

private:
    ModelGraphicsScene* _myScene;
    GraphicalModelComponent* _graphicalComponent;
};

#endif // ANIMATIONQUEUE_H

#include "GraphicalImageAnimation.h"
#include <algorithm>
#include <QPainter>

GraphicalImageAnimation::GraphicalImageAnimation(const QPointF startPoint, unsigned int width, unsigned int height, const QString imageName) :
    _startPoint(startPoint), _width(width), _height(height), _imageName(imageName){

    // Carrega as informações da imagem
    this->updateImage();
}

GraphicalImageAnimation::~GraphicalImageAnimation(){}

// Getters
QString GraphicalImageAnimation::getImagePath() const {
    return _imagePath;
}

QString GraphicalImageAnimation::getImageName() const {
    return _imageName;
}

QPointF GraphicalImageAnimation::getStartPoint() const {
    return _startPoint;
}

unsigned int GraphicalImageAnimation::getWidth() const {
    return _width;
}

unsigned int GraphicalImageAnimation::getHeight() const {
    return _height;
}

// Setters
void GraphicalImageAnimation::setImageName(const QString &imageName) {
    this->_imageName = imageName;

    updateImage();
}

void GraphicalImageAnimation::setStartPoint(const QPointF &startPoint) {
    this->_startPoint = startPoint;

    updateImage();
}

void GraphicalImageAnimation::setWidth(unsigned int width) {
    this->_width = width;

    updateImage();
}

void GraphicalImageAnimation::setHeight(unsigned int height) {
    this->_height = height;

    updateImage();
}

// Outros
void GraphicalImageAnimation::updateImage() {
    // Build the image path from Qt resources to remove relative filesystem dependency.
    _imagePath = _resourceBasePath + _imageName;

    // Load from Qt resources first and fallback to an in-memory marker pixmap if missing.
    QPixmap source(_imagePath);
    if (source.isNull()) {
        qInfo() << "GraphicalImageAnimation: fallback image used for missing resource" << _imagePath;
        source = buildFallbackPixmap();
    }
    // Resize only after ensuring a valid source pixmap.
    QPixmap resizedImage = source.scaled(_width, _height);

    // Define a imagem do item atual
    setPixmap(resizedImage);

    // Define a posição inicial da imagem
    setPos(_startPoint);
}

QPixmap GraphicalImageAnimation::buildFallbackPixmap() const {
    // Build a clear visual placeholder with safe minimum dimensions.
    const int safeWidth = std::max(8, static_cast<int>(_width));
    const int safeHeight = std::max(8, static_cast<int>(_height));
    QPixmap fallback(safeWidth, safeHeight);
    fallback.fill(Qt::white);

    QPainter painter(&fallback);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(0, 0, safeWidth - 1, safeHeight - 1);
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(0, 0, safeWidth - 1, safeHeight - 1);
    painter.drawLine(0, safeHeight - 1, safeWidth - 1, 0);

    return fallback;
}

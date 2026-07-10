#ifndef GRAPHICALIMAGEANIMATION_H
#define GRAPHICALIMAGEANIMATION_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class GraphicalImageAnimation : public QGraphicsPixmapItem {
public:
    // Contrutor
    GraphicalImageAnimation(const QPointF startPoint, unsigned int width, unsigned int height, const QString imageName);
    // Destrutor
    ~GraphicalImageAnimation();

    // Getters
    QString getImagePath() const;
    QString getImageName() const;
    QPointF getStartPoint() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;

    // Setters
    void setImageName(const QString &imageName);
    void setStartPoint(const QPointF &startPoint);
    void setWidth(unsigned int imageWidth);
    void setHeight(unsigned int imageHeight);

    // Atualiza informações da imagem
    void updateImage();

private:
    // Builds an in-memory marker image when the file-based image cannot be loaded.
    QPixmap buildFallbackPixmap() const;

    // Use a Qt resource base path to avoid working-directory dependent file paths.
    QString _resourceBasePath = ":/animations/";

    // Atributos
    QString _imagePath;
    QPointF _startPoint;
    unsigned int _width;
    unsigned int _height;
    QString _imageName;
};

#endif // GRAPHICALIMAGEANIMATION_H

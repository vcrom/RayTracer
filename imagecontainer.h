#ifndef IMAGECONTAINER_H
#define IMAGECONTAINER_H

#include <QLabel>
#include <QResizeEvent>

#include "raytracer.h"

class ImageContainer : public QWidget
{
    Q_OBJECT
public:
    ImageContainer(QWidget *parent = 0);
    ~ImageContainer();
    const QPixmap* pixmap() const;

public slots:
    void saveImage(const QString& fileName);
    void renderImage();
    void setPixmap(const QPixmap& pix);

signals:
    void resized(const QSize& size);

protected:
   void paintEvent(QPaintEvent *);
   void resizeEvent(QResizeEvent *event);

private:
    //void renderScene(QPixmap &image, const QSize& size);
    RayTracer _ray_tracer;
    QImage _image;
    QSize _size;

    QPixmap _pixmap;

    const Qt::AspectRatioMode aspect_ratio_mode =  Qt::IgnoreAspectRatio;
    //KeepAspectRatio
};

#endif // IMAGECONTAINER_H

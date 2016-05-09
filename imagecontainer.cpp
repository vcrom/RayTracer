#include "imagecontainer.h"

ImageContainer::ImageContainer(QWidget *parent) :
    QWidget(parent)
{
}

ImageContainer::~ImageContainer()
{
    //_label
}

void ImageContainer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    _size = event->size();
}

const QPixmap* ImageContainer::pixmap() const {
    return &_pixmap;
}

void ImageContainer::setPixmap (const QPixmap &pix){
    _pixmap =pix;
}

/*
void ImageContainer::renderScene(QPixmap &pixmap, const QSize& size)
{
    auto pixels = _ray_tracer.render(size.width(), size.height());
    _image = QImage(&pixels[0], size.width(), size.height(), size.width()*3, QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(QImage(_image));
}
*/
#include <iostream>
void ImageContainer::saveImage(const QString& fileName)
{
    if(!_image.save(fileName, "PPM", 100))
    {
        std::cerr << "Problem saving the image" << std::endl;
    }
}

void ImageContainer::renderImage()
{
    auto pixels = _ray_tracer.render(_size.width(), _size.height());
    _image = QImage(&pixels[0], _size.width(), _size.height(), _size.width()*3, QImage::Format_RGB888);
    auto pixmap = QPixmap::fromImage(_image.copy(0,0,_image.width(), _image.height()));
    setPixmap(pixmap);
    update();
    _image = QImage(&pixels[0], _size.width(), _size.height(), _size.width()*3, QImage::Format_RGB888);
}

#include <QPainter>
void ImageContainer::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    if (_pixmap.isNull()) return;

    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    QSize pixSize = _pixmap.size();
    pixSize.scale(event->rect().size(), aspect_ratio_mode);
    QPixmap scaledPix = _pixmap.scaled(pixSize,
                                       aspect_ratio_mode,
                                       Qt::SmoothTransformation
                                       );

    painter.drawPixmap(QPoint(), scaledPix);
}

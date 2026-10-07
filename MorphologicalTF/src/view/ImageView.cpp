#include "view/ImageView.h"

#include <QImage>
#include <QResizeEvent>

ImageView::ImageView(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    // Let the window shrink below the size of the loaded image
    setMinimumSize(1, 1);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    clearImage();
}

void ImageView::setImage(const QImage &image)
{
    m_source = QPixmap::fromImage(image);
    setStyleSheet(QString()); // remove the drop zone frame
    updatePixmap();
}

void ImageView::clearImage()
{
    m_source = QPixmap();
    setText(tr("Drop an image here\nor use File > Open..."));
    setStyleSheet(QStringLiteral(
        "QLabel { border: 2px dashed palette(mid); color: palette(mid); font-size: 16px; }"));
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updatePixmap();
}

void ImageView::updatePixmap()
{
    if (m_source.isNull())
        return;

    // Only scale down: small images stay 1:1, so individual pixels are not blurred
    if (m_source.width() <= width() && m_source.height() <= height())
        setPixmap(m_source);
    else
        setPixmap(m_source.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

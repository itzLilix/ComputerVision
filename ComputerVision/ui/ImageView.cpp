#include "imageview.h"
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

ImageView::ImageView(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(800, 800);
    setAcceptDrops(true);
}

void ImageView::setImage(const QImage& image)
{
    sourceImage = image;
    updateScaledPixmap();
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    updateScaledPixmap();
}

void ImageView::updateScaledPixmap() {
    if (sourceImage.isNull()) {
        setText(tr("Drag && drop an image here"));
        return;
    }

    const QPixmap scaled = QPixmap::fromImage(sourceImage)
        .scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    setPixmap(scaled);
}

void ImageView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ImageView::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile())
        return;

    emit imageDropped(urls.first().toLocalFile());
    event->acceptProposedAction();
}
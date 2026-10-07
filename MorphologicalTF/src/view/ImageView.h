#pragma once

#include <QLabel>
#include <QPixmap>

class QImage;

// Dumb display widget: shows an image scaled down to fit, or a drop zone hint
// when there is no image. It does not know about the model or the view model.
class ImageView : public QLabel
{
    Q_OBJECT

public:
    explicit ImageView(QWidget *parent = nullptr);

public slots:
    void setImage(const QImage &image);
    void clearImage();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updatePixmap();

    QPixmap m_source; // full-size pixmap, rescaled on every resize
};

#pragma once

#include <QLabel>
#include <QImage>

class ImageView : public QLabel {
    Q_OBJECT

public:
    explicit ImageView(QWidget* parent = nullptr);

    void setImage(const QImage& image);

signals:
    void imageDropped(const QString& filePath);

protected:
    void resizeEvent(QResizeEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QImage sourceImage;

    void updateScaledPixmap();
};
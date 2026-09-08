#pragma once

#include <QObject>
#include <QImage>

class ImageModel : public QObject {
    Q_OBJECT

public:
    explicit ImageModel(QObject* parent = nullptr);

    const QImage& image() const { return m_currentImage; }
    const QString& filePath() const { return m_filePath; }

public slots:
    bool loadFile(const QString path);
    void clearImage();

signals:
    void newImageLoaded(const QImage& image, const QString& m_filePath);
    void imageChanged(const QImage& image);
    void imageCleared();

private:
    QImage m_originalImage;
    QImage m_currentImage;
    QString m_filePath;
};
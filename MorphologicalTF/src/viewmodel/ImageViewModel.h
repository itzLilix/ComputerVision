#pragma once

#include <QImage>
#include <QObject>
#include <QString>

class ImageModel;

// Mediator between ImageModel and the widgets.
// Exposes state and commands in a view-friendly form; it contains no widgets
// and no dialogs, so it can be unit-tested without a GUI.
class ImageViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ImageViewModel(ImageModel &model, QObject *parent = nullptr);

    // State
    bool hasImage() const;
    bool isModified() const;
    const QImage &currentImage() const;
    QString statusText() const;
    QString suggestedSaveName() const;

    // Helpers for file dialogs and drag'n'drop
    static QString openFileFilter();
    static QString saveFileFilter();
    static bool isSupportedImageFile(const QString &path);

public slots:
    // Commands
    void openImage(const QString &path);
    void saveImage(const QString &path);
    void closeImage();
    void resetToOriginal();

signals:
    void imageLoaded();
    void imageChanged();
    void imageCleared();
    void errorOccurred(const QString &message);

private:
    ImageModel &m_model;
};

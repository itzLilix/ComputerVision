#pragma once

#include <QImage>
#include <QObject>
#include <QString>

// Holds the loaded image in two instances:
//  - original: exactly what was read from disk, never modified afterwards
//  - current:  the working copy that every operation (filters, morphology...) replaces
// The model knows nothing about widgets or dialogs.
class ImageModel : public QObject
{
    Q_OBJECT

public:
    explicit ImageModel(QObject *parent = nullptr);

    bool hasImage() const { return !m_original.isNull(); }
    bool isModified() const { return m_modified; } // current has edits that were not saved
    const QImage &original() const { return m_original; }
    const QImage &current() const { return m_current; }
    const QString &filePath() const { return m_filePath; }

    // Return false on failure and put a human-readable reason into *error.
    // A failed load keeps the previously opened image untouched.
    bool load(const QString &path, QString *error = nullptr);
    bool save(const QString &path, QString *error = nullptr); // saves current
    void close();

    // Entry points for image operations: the result of a filter goes here.
    void setCurrent(const QImage &image);
    void resetToOriginal();

signals:
    // Lifecycle: a different document. Panels should reset their state here.
    void imageLoaded();  // a new image was opened (emitted before imageChanged)
    void imageCleared(); // the image was closed

    // Content: the pixels of current differ from before.
    // Also emitted right after imageLoaded, so a display widget needs only this one.
    void imageChanged();

private:
    QImage m_original;
    QImage m_current;
    QString m_filePath;
    bool m_modified = false;
};

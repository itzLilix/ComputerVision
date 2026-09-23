#pragma once

#include <QDateTime>;

struct ExifData {
    QString dateTimeOriginal;
    QString cameraMake;
    QString cameraModel;
    qreal exposureTime = 0.0;
    qreal fNumber = 0.0;
    qint64 isoSpeed = 0;
    qint64 orientation = 1;
};

struct ImageFileMetadata
{
    QString path;
    qint64 fileSizeBytes = 0;
    QString format;
    QDateTime lastModified;
    std::optional<ExifData> exif;
};

struct ImageLiveMetadata {
    qint64 width = 0;
    qint64 height = 0;
	qint64 colorDepth = 0;
	QString colorSpace;
	QString colorModel;
};
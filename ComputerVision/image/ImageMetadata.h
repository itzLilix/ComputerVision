#pragma once

#include <QString>
#include <QDateTime>

struct ImageMetadata {
    QString path;
    qint64 fileSizeBytes = 0;
    QString format;
    QDateTime lastModified;
    std::optional<ExifData> exif;
};

struct ImageLiveMetadata {
	qint64 resolution[2] = { 0, 0 };
	qint64 colorDepth = 0;
	QString colorSpace;
};

struct ExifData {
    QString dateTimeOriginal;
    QString cameraMake;
    QString cameraModel;
    QString exposureTime;
    double  fNumber = 0.0;
    int     isoSpeed = 0;
    int     orientation = 1;
};
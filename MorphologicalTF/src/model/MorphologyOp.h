#pragma once

#include <QCoreApplication>
#include <QList>
#include <QString>

enum class MorphologyOp { Erosion, Dilation, Opening, Closing, Gradient, TopHat, BlackHat };

namespace MorphologyOps {

inline const QList<MorphologyOp> &all()
{
    static const QList<MorphologyOp> ops = {
        MorphologyOp::Erosion, MorphologyOp::Dilation, MorphologyOp::Opening, MorphologyOp::Closing,
        MorphologyOp::Gradient, MorphologyOp::TopHat, MorphologyOp::BlackHat,
    };
    return ops;
}

inline QString displayName(MorphologyOp op)
{
    switch (op) {
    case MorphologyOp::Erosion:  return QCoreApplication::translate("MorphologyOps", "Erosion");
    case MorphologyOp::Dilation: return QCoreApplication::translate("MorphologyOps", "Dilation");
    case MorphologyOp::Opening:  return QCoreApplication::translate("MorphologyOps", "Opening");
    case MorphologyOp::Closing:  return QCoreApplication::translate("MorphologyOps", "Closing");
    case MorphologyOp::Gradient: return QCoreApplication::translate("MorphologyOps", "Gradient");
    case MorphologyOp::TopHat:   return QCoreApplication::translate("MorphologyOps", "Top Hat");
    case MorphologyOp::BlackHat: return QCoreApplication::translate("MorphologyOps", "Black Hat");
    }
    return {};
}

} // namespace MorphologyOps

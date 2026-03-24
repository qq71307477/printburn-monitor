#ifndef QT_COMPAT_H
#define QT_COMPAT_H

#include <QtGlobal>
#include <QString>

// Qt 5.14 弃用了 QString::SkipEmptyParts，推荐使用 Qt::SkipEmptyParts
// 此宏提供跨版本兼容性
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    #define SPLIT_SKIP_EMPTY Qt::SkipEmptyParts
#else
    #define SPLIT_SKIP_EMPTY QString::SkipEmptyParts
#endif

#endif // QT_COMPAT_H
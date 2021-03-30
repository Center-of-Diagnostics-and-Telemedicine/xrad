#ifndef STYLE_SHEET_UTILS_H 
#define STYLE_SHEET_UTILS_H

#include "pre.h"
#include <QString>
#include <QColor>

QString getBackColorStyleSheet(int r, int g, int b, int a);

QString getBackColorStyleSheet(QColor color);

QString getStyleSheet(QColor color);




#endif // STYLE_SHEET_UTILS_H

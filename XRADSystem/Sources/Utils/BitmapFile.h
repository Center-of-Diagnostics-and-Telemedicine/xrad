#ifndef XRAD__File_BitmapFile_h
#define XRAD__File_BitmapFile_h

#include <XRADBasic/Sources/SampleTypes/ColorSample.h>

XRAD_BEGIN

//! \brief Запись растрового изображения в файл bmp
void	WriteBitmapFile(const char *filename, const uint8_t *bits, size_t n_rows, size_t n_cols);

//! \brief Запись растрового изображения в файл bmp
void	WriteBitmapFile(const char *filename, const ColorPixel *bits, size_t n_rows, size_t n_cols);

XRAD_END

#endif // XRAD__File_BitmapFile_h

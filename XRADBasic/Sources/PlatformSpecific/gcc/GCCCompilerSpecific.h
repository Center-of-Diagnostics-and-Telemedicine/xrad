#ifndef XRAD__File_GCCVisualCSpecific_h
#define XRAD__File_GCCVisualCSpecific_h
//--------------------------------------------------------------
/*!
	\addtogroup gr_CompilerSpecificGCC
	@{

	\file
	\brief Определения для GNU C++. Этот файл должен включаться до включения стандартных
	заголовочных файлов
*/
//--------------------------------------------------------------

//--------------------------------------------------------------
//
// Обязательные определения
//
//--------------------------------------------------------------

//! \brief Компилятор GCC. Значение константы равно __GNUC__ (старший номер версии)
//!
//! См. __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__.
#define XRAD_COMPILER_GNUC __GNUC__

#ifdef __SIZE_MAX__
	#if __SIZE_MAX__ == 0xffffffffffffffffUL
		#define XRAD_SIZE_T_BITS 64
	#elif __SIZE_MAX__ == 0xffffffffUL
		#define XRAD_SIZE_T_BITS 32
	#else
		#error "Unknown __SIZE_MAX__ value."
	#endif
#else
	#error "Unknown platform."
#endif

//! \brief Задает порядок байтов little endian
#define XRAD_ENDIAN XRAD_LITTLE_ENDIAN

//! \brief Задает функцию выхода в отладчик (для отладочной сборки, может быть пустым)
#define XRAD_FORCE_DEBUG_BREAK_FUNCTION()

//! \brief Макроопределение требуется в MSVC для подавления предупреждений компилятора.
//! Для gcc это пустое определение
#define XRAD_CheckedIterator

namespace xrad
{

//! \brief Значение черного цвета для полутонового изображения
#define	gray_pixel_black 0u

//! \brief Значение белого цвета для полутонового изображения
#define	gray_pixel_white 255u

//! \brief Тип пикселя полутонового изображения
typedef	unsigned char	rgb_pixel_component_type;

} // namespace xrad

//--------------------------------------------------------------
//
// Опциональные определения
//
//--------------------------------------------------------------

#define XRAD_USE_GCC_VERSION

#ifdef __unix__
//! \brief Реализация кодировок текста: char = UTF-8, wchar_t = UTF-32
#define XRAD_USE_CHAR_UTF8_WCHAR_UTF32_VERSION
#else
#error Unknown platform.
#endif

//--------------------------------------------------------------
//! @} <!-- ^group gr_CompilerSpecificGCC -->
#endif // XRAD__File_GCCVisualCSpecific_h

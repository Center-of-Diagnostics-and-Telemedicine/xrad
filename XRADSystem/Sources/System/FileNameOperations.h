#ifndef XRAD__File_FileNameOperations_h
#define XRAD__File_FileNameOperations_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <string>

// TODO: Провести рефакторинг: передавать в функции строки по ссылке.

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Разобрать имя файла с путем на составные части: имя, путь, расширение

	\param filename [in] -- исходное имя файла.
	\param path [out] -- путь к файлу, может быть nullptr. Если исходное имя файла содержит
		путь, то здесь будет непустая строка, оканчивающаяся символом path_separator().
		Если исходное имя файла не содержит путь, здесь будет пустая строка.
	\param name_with_extension [out] -- имя файла (без пути, с расширением).
	\param name_without_extension [out] -- имя файла без расширения, может быть nullptr.
		Для имен файлов вида ".name" здесь будет пустая строка, а ".name" будет
		записано в расширение.
	\param extension [out] -- расширение файла, может быть nullptr.
		Если исходное имя файла содержит расширение, то здесь будет непустая строка,
		начинающаяся с точки. Если исходное имя файла не содержит путь, то здесь
		будет пустая строка.

	Расширением считается часть имени файла после последней точки, включая эту точку.
	Например:
	- "data.zip" -> "data" + ".zip";
	- "data.tar.gz" -> "data.tar" + ".gz";
	- "data" -> "data" + "";
	- "data." -> "data" + ".";
	- ".data" -> "" + ".data".

	Разделитель пути включается в путь, а точка включается в расширение для возможности
	однозначного восстановления полного имени файла из полученных частей. При таком подходе
	достаточно просто конкатенировать полученные строки.
*/
void SplitFilename(const string &filename, string *path, string *name_with_extension,
		string *name_without_extension = nullptr, string *extension = nullptr);

/*!
	\brief Разобрать имя файла с путем на составные части: имя, путь, расширение

	См. функцию с тем же именем от string.
*/
void SplitFilename(const wstring &filename, wstring *path, wstring *name_with_extension,
		wstring *name_without_extension = nullptr, wstring *extension = nullptr);

/*!
	\brief Получить путь к файлу
*/
inline wstring	file_path(wstring complete_filename_with_path)
{
	wstring	result;
	SplitFilename(complete_filename_with_path, &result, nullptr, nullptr, nullptr);
	return result;
}

/*!
	\brief Получить имя файла без пути, но с расширением
*/
inline wstring	filename_with_extension(wstring complete_filename_with_path)
{
	wstring	result;
	SplitFilename(complete_filename_with_path, nullptr, &result, nullptr, nullptr);
	return result;
}

/*!
	\brief Получить имя файла без пути и расширения
*/
inline wstring	filename_without_extension(wstring complete_filename_with_path)
{
	wstring	result;
	SplitFilename(complete_filename_with_path, nullptr, nullptr, &result, nullptr);
	return result;
}

/*!
	\brief Получить расширение файла
*/
inline wstring	file_extension(wstring complete_filename_with_path)
{
	wstring	result;
	SplitFilename(complete_filename_with_path, nullptr, nullptr, nullptr, &result);
	return result;
}

//--------------------------------------------------------------

/*!
	\brief Удалить конечный разделитель пути из строки: "/a/b/" -&gt; "/a/b"
*/
wstring RemoveTrailingPathSeparator(const wstring &path);

//--------------------------------------------------------------

/*!
	\brief Нормализовать имя файла для сравнения с другим нормализованным именем файла

	Предназначена для анализа имен файлов, полученных при чтении каталога.

	Функция приводит имя файла к верхнему регистру. В будущем может осуществлять какие-то
	ещё преобразования (например, нормализацию составных символов, приведение к NFD).

	Функция не заменяет '\\' на '/' и обратно, оставляет без изменения последовательности
	"./", "../path/" и т.п.

	См. \ref pg_FileSystem.
*/
string CmpNormalizeFilename(const string &filename);
//! \brief Нормализовать имя файла для сравнения с другим нормализованным именем файла.
//! Аналогична CmpNormalizeFilename(const string &filename)
wstring CmpNormalizeFilename(const wstring &filename);

//--------------------------------------------------------------

/*!
	\brief Получение путей в виде, пригодном для использования в системных функциях обращения к папкам и файлам

	Функция заменяет '\\' на '/' и обратно, оставляет без изменения последовательности
	"./", "../path/" и т.п.

	См. \ref pg_FileSystem.

	Эти функции устаревшие. Следует использовать функции GetPathGenericFromAutodetect() и т.п.
*/
string GetPathMachineReadable(const string &path_original);
wstring GetPathMachineReadable(const wstring &path_original);
string GetPathHumanReadable(const string &path_original);
wstring GetPathHumanReadable(const wstring &path_original);

//! \brief Преобразовать путь в формат generic (UNIX), автоопределение исходного формата
//! (для преобразования входных данных)
//!
//! См. \ref pg_FileSystem.
wstring GetPathGenericFromAutodetect(const wstring &path);

//! \brief Преобразовать путь из формата generic в формат ОС (для отображения пользователю)
//!
//! См. \ref pg_FileSystem.
wstring GetPathNativeFromGeneric(const wstring &path);

//! \brief Комбинация GetPathNativeFromGeneric и GetPathGenericFromAutodetect.
//! Эта функция должна исчезнуть после приведения в порядок работы с форматами файлов
//!
//! См. \ref pg_FileSystem.
wstring GetPathNativeFromAutodetect(const wstring &path);

//! \brief Преобразовать путь в формат ОС для передачи в функции открытия файлов.
//! В Windows решает проблему открытия файлов с очень длинными путями (больше MAX_PATH)
//!
//! См. \ref pg_FileSystem.
wstring GetPathSystemRawFromGeneric(const wstring &path);

//! \brief Комбинация GetPathSystemRawFromGeneric и GetPathGenericFromAutodetect.
//! Эта функция должна исчезнуть после приведения в порядок работы с форматами файлов
//!
//! См. \ref pg_FileSystem.
wstring GetPathSystemRawFromAutodetect(const wstring &path);

//--------------------------------------------------------------

//TODO все спонтанно встречающиеся литералы в коде следует заменить на эти
//TODO возможно, сделует сделать эти функции платформо-зависимыми. В частности, на MacOS (и, видимо, на всех unix) символ '\' является дозволенным в имени файла
template<class T> inline bool	is_path_separator(T c);
template<> inline bool	is_path_separator<wchar_t>(wchar_t c){ return (c==L'/' || c==L'\\'); }
template<> inline bool	is_path_separator<char>(char c){ return (c=='/' || c=='\\'); }
constexpr char path_separator(){ return '/'; }
constexpr wchar_t wpath_separator(){ return L'/'; }
constexpr char extension_separator() { return '.'; }
constexpr wchar_t wextension_separator() { return L'.'; }

//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_FileNameOperations_h

#include "pre.h"
#include "shared_cfile.h"

#include <XRADSystem/System.h>
#include <XRADSystem/Sources/System/SystemConfig.h>
#include <type_traits>
#include <utility>

#if defined(XRAD_COMPILER_MSC)
#include <sys/stat.h>
#elif defined(XRAD_COMPILER_GNUC)
#include <sys/types.h>
#include <sys/stat.h>
#endif

XRAD_BEGIN

//--------------------------------------------------------------

file_size_t filesize(FILE *file)
{
#if defined(XRAD_COMPILER_MSC)
	struct __stat64 st;
	if (_fstat64(_fileno(file), &st))
		return 0;
	static_assert(std::is_same<std::make_unsigned_t<decltype(st.st_size)>, file_size_t>::value, "Invalid types");
	return st.st_size;
#elif defined(XRAD_COMPILER_GNUC)
	struct stat64 st;
	if (fstat64(fileno(file), &st))
		return 0;
	static_assert(std::is_same<std::make_unsigned_t<decltype(st.st_size)>, file_size_t>::value, "Invalid types");
	return st.st_size;
#else
	#error Unknown platform.
#endif
}

//--------------------------------------------------------------

namespace
{

void DestroyCFilePointer(FILE *file)
{
	DestroyCFile(file);
}

} // namespace

//--------------------------------------------------------------

void shared_cfile::open(const string &filename, const string &mode)
{
	open(convert_to_wstring(filename), convert_to_wstring(mode));
}

void shared_cfile::open(const wstring &path_in, const wstring &mode)
{
	// допускается использование одного контейнера
	// для разных файлов. если ранее контейнер был
	// занят, освобождаем
	FILE	*file;
#if defined(XRAD_USE_CFILE_WIN32_VERSION)
	file = _wfopen(GetPathSystemRawFromAutodetect(path_in).c_str(), mode.c_str());
#elif defined(XRAD_USE_CFILE_UNIX_VERSION)
	file = fopen(convert_to_string(GetPathSystemRawFromAutodetect(path_in)).c_str(),
			convert_to_string(mode).c_str());
#else
	#error Unknown platform.
#endif
	if(!file)
	{
		throw file_container_error(convert_to_string(ssprintf(
				L"CFileContainer::fopen(). File \"%ls\" could not be opened with options \"%ls\".",
				EnsureType<const wchar_t*>(GetPathNativeFromAutodetect(path_in).c_str()),
				EnsureType<const wchar_t*>(mode.c_str()))));
	}

	m_controled_file = shared_ptr<FILE>(file, DestroyCFilePointer);
}

void shared_cfile::close()
{
	m_controled_file.reset(); //гарантирует обнуление указателя, если ссылка последняя, то вызывает деструктор и закрывает файл
}

size_t shared_cfile::read(void *ptr, size_t size, size_t nmemb)
{
	return fread(ptr, size, nmemb, get_file());
}

size_t shared_cfile::write(const void *ptr, size_t size, size_t nmemb)
{
	return fwrite(ptr, size, nmemb, get_file());
}

file_size_t shared_cfile::size() const
{
	return filesize(get_file());
}

file_offset_t shared_cfile::tell() const
{
#if defined(XRAD_COMPILER_MSC)
	// MSVC 2015+: Используем _ftelli64, возвращает __int64.
	// Проверяем совпадение типа возвращаемого значения _ftelli64.
	static_assert(std::is_same<decltype(_ftelli64(std::declval<FILE*>())), file_offset_t>::value,
			"Invalid types.");
	return _ftelli64(get_file());
#elif defined(XRAD_COMPILER_GNUC)
	// Проверяем совпадение типа возвращаемого значения ftello64.
	static_assert(std::is_same<decltype(ftello64(std::declval<FILE*>())), file_offset_t>::value,
			"Invalid types.");
	return ftello64(get_file());
#else
	#error Unknown platform.
#endif
}

int shared_cfile::seek(file_offset_t offset, int whence)
{
#if defined(XRAD_COMPILER_MSC)
	// MSVC 2015+: Используем _fseeki64, принимает __int64.
	// Проверяем совпадение типа возвращаемого значения _ftelli64.
	static_assert(std::is_same<decltype(_fseeki64), int (FILE*, file_offset_t, int)>::value,
			"Invalid types.");
	return _fseeki64(get_file(), offset, whence);
#elif defined(XRAD_COMPILER_GNUC)
	return fseeko64(get_file(), offset, whence);
#else
	#error Unknown platform.
#endif
}

fpos_t shared_cfile::get_pos() const
{
	fpos_t result;
	fgetpos(get_file(), &result);
	return result;
}

void	shared_cfile::set_pos(fpos_t pos)
{
	fsetpos(get_file(), &pos);
}

void	shared_cfile::flush()
{
	fflush(get_file());
}

//--------------------------------------------------------------

XRAD_END

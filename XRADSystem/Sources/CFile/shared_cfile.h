#ifndef XRAD__File_shared_cfile_h
#define XRAD__File_shared_cfile_h

#include <XRADBasic/DataArrayIO.h>
#include <XRADSystem/Sources/System/FileSystemDefs.h>
#include <memory>
#include <cstdio>
#include <stdexcept>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

/*!
	\brief Получить размер файла _на файловой системе_ по указателю

	Внимание! Если в файл производится запись, функция не учитывает данные, не сброшенные на файловую
	систему из буферов рантайм-библиотеки в памяти. В таком случае перед вызовом этой функции нужно
	вызвать fflush(file) или использовать другие способы определения размера файла.
*/
file_size_t filesize(FILE *file);

//--------------------------------------------------------------

class file_container_error : public logic_error
{
public:
	explicit file_container_error(const char* what_arg) : logic_error(what_arg) {}
	explicit file_container_error(const string& what_arg) : logic_error(what_arg) {}
};


//--------------------------------------------------------------

/*!
	\brief Обертка для стандартного FILE*, которая автоматически закрывает файл,
	когда он более не нужен

	\note
	По стандарту для FILE* предусмотрена только адресация типом long.
	Мы используем 64-битную адресацию.
	Под разными системами существуют средства для 64-битной адресации файлов,
	но они не являются переносимыми.
*/
class	shared_cfile
{
public:
	shared_cfile() {}
	shared_cfile(const string &path_in, const string &mode) { open(path_in, mode); }
	shared_cfile(const wstring &path_in, const wstring &mode) { open(path_in, mode); }

	//! \brief Открыть файл. В случае ошибки исключение
	void open(const string &path_in, const string &mode);
	//! \brief Открыть файл. В случае ошибки исключение
	void open(const wstring &path_in, const wstring &mode);

	bool is_open() const { return m_controled_file.get() != NULL; }
	int	error() { return ferror(get_file()); }
	void close();

	FILE	*c_file() { return get_file(); }

	size_t	read(void *ptr, size_t size, size_t nmemb);
	size_t	write(const void *ptr, size_t size, size_t nmemb);

	//! \brief Размер файла, тип file_size_t
	file_size_t	size() const;

	//! \brief Аналог ftell, но с типом file_offset_t
	file_offset_t tell() const;

	//! \brief Аналог fseek (такие же параметр whence и возвращаемое значение), но с типом
	//! file_offset_t для смещения
	int	seek(file_offset_t offset, int whence);

	//! \brief Получить объект с позицией в файле.
	//! Внимание: fpos_t может быть не целочисленным типом
	fpos_t	get_pos() const;
	//! \brief Перейти к позиции, полученной ранее через get_pos().
	//! Внимание: fpos_t может быть не целочисленным типом
	void	set_pos(fpos_t pos);

	void	flush();

	template<class T>
	size_t	read_numbers(T &buffer, ioNumberOptions inDataF)
	{
		return fread_numbers(buffer, get_file(), inDataF);
	}

	template<class T>
	size_t	write_numbers(const T &buffer, ioNumberOptions inDataF)
	{
		return fwrite_numbers(buffer, get_file(), inDataF);
	}

private:
	shared_ptr<FILE>	m_controled_file;

private:
	inline FILE* get_file() const
	{
		XRAD_ASSERT_THROW(is_open());
		return m_controled_file.get();
	}
};

//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_shared_cfile_h

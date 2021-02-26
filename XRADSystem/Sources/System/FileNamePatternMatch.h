﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FileNamePatternMatch.h
//--------------------------------------------------------------
#ifndef XRAD__File_FileNamePatternMatch_h
#define XRAD__File_FileNamePatternMatch_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <string>
#include <set>

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Класс для фильтрации имен файлов по маске вида "*.txt", "*.txt;*.doc"

	Фильтрация производится без учета регистра символов.

	Маска имен файлов имеет вид:
	- "" — не производить фильтрацию (все имена файлов подходят);
	- "*", "*.*" — то же, что "", не производить фильтрацию;
	- имя файла без метасимволов (например, "noname.txt");
	- маска вида "*." + расширение:
	  - "*.txt" — имена файлов с расширением ".txt";
	  - "*.tar.gz" — имена файлов с двойным расширением ".tar.gz";
	  - "*." — имена файлов без точек (без расширения);
	- набор из нескольких масок, разделенных ";": "*.txt;*.doc" — имена файлов
	  с расширениями ".txt", ".doc".

	Для маски вида "*.ext" символы "*", "?", "/", "\\" расширении файла не допускаются.

	В текущей реализации для маски вида "*.ext" начальные и конечные пробелы в маске отрезаются.
	Например, маска " *.txt ; *.doc " эквивалентна "*.txt;*.doc". Пробелы в середине
	маски учитываются ("*.extension with spaces").
	Хотя начальные и конечные пробелы в маске отрезаются, их наличие крайне нежелательно.
	В будущих версиях начальные и конечные пробелы могут начать учитываться.

	Для маски вида "filename.ext" начальные и конечные пробелы сохраняются.
	В частности, маска " " создает фильтр, которому удовлетворяет имя файла, сотоящее из
	одного пробела.

	Если задан набор из нескольких масок, и среди них встречается "*" или "*.*",
	то весь фильтр игнорируется (все имена файлов подходят).

	Маски других форматов игнорируются (отличные от "filename.ext", "*.extension", "*." и "*").

	\note
	Для сложных случаев фильтрации можно расширить формат так, чтобы он поддерживал
	регулярные выражения C++. К примеру, фильтр вида "/regexp/re_options" трактовать
	как регулярное выражение.
*/
class FileNamePatternMatch
{
	public:
		/*!
			\brief Создать фильтр для маски filter

			\param filter [in] Маска имен файлов.
		*/
		FileNamePatternMatch(const wstring &filter);
		bool empty() const { return filenames.empty() && filters.empty(); }
		bool operator()(const wstring &filename) const;
	private:
		//! \brief Имена файлов без метасимволов (для точного соответствия) в верхнем регистре
		set<wstring> filenames;
		//! \brief Строки вида ".EXT" в верхнем регистре
		set<wstring> filters;
};

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FileNamePatternMatch_h

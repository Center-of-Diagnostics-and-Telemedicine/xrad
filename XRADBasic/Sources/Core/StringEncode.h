/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef StringEncode_h__
#define StringEncode_h__
/*!
	\file
	\date 2015-12-22 15:24
	\author kns
	\brief Внутренний файл библиотеки
*/
//--------------------------------------------------------------

#include "String.h"
#include <locale>

XRAD_BEGIN

/*!
	\page pg_StringEncoding Кодировка строк

	# Соглашение о кодировках #

	В библиотеке приняты следующие соглашения о кодировке текста для разных типов символов и строк:
	- char, string — зависит от контекста, однобайтовая кодировка платформо-зависимая (TODO) или какая-то другая (UTF-8),
		эти типы также могут использоваться для хранения двоичных данных;
	- wchar, wstring — платформо-зависимая многобайтовая кодировка (в Windows UTF-16, в Linux может быть UTF-32);
	- uchar, ustring — UTF-8;
	- char16_t, u16string — UTF-16 (пока для внутреннего использования, TODO);
	- char32_t, u32string — UTF-32.

	Стандарты, определяющие кодировки:
	- UTF-8: RFC 3629 (устаревшая версия RFC 2279 не поддерживается), ISO/IEC 10646 Annex D;
	- UTF-16: RFC 2781, ISO/IEC 10646, приложение Q;
	- UTF-32: ISO 10646.

	Все кодировки делятся на два класса:
	- платформо-зависимые: типы char, wchar;
	- платформо-независимые: типы uchar, char16_t, char32_t.

	Преобразования кодировок в Windows:
	- string &lt;-&gt; wstring — базовая платформо-зависимая процедура;
	- wstring &lt;-&gt; u16string — базовая платформо-зависимая процедура, осуществляющая
		преобразование типа wchar_t &lt;-&gt; char16_t;
	- преобразования между платформо-независимыми кодировками UTF-8, UTF-16 и UTF-32 платформо-независимые;
	- преобразования между платформо-зависимыми и платфоромо-независимыми кодировками производятся через шлюз
		wstring &lt;-&gt; u16string.

	Преобразование кодировок в других ОС:
	- В Linux кодировки string и ustring, а также wstring и u32string могут быть эквивалентными.
		В этом случае нетривиальные платформо-зависимые процедуры преобразования кодировок могут не понадобиться.



	# TODO #

	В последнее время в Linux де-факто платформо-зависимая однобайтовая кодировка — UTF-8,
	всё API C/C++ (fopen, fstream...) работает с кодировкой UTF-8.
	API C/C++ с wchar_t нет в стандарте, в нём нет необходимости в Linux, и оно там отсутствует.

	В Windows до сих пор UTF-8 не может полноценно использоваться в качестве однобайтной кодировки.
	Обычное API C/C++ не позволяет полноценно работать с файлами.
	Для полноценной работы с файлами необходимо использовать платформо-зависимое API с wchar_t,
	работающее в кодировке UTF-16 (кодировка UTF-16 подразумевается для wchar_t, начиная с Windows 2000).

	Следует придумать прослойку для API работы с файлами. Было бы хорошо совсем отказаться от машинно-зависимых
	кодировок в пользу UTF-8, за исключением, быть может, тех мест, где происходит чтение и запись строк в файлы
	и где из соображений совместимости нужна опция поддержки машинно-зависимых кодировок текста. Впрочем,
	в таких местах желательно иметь возможность указать и конкретную кодировку, например 1251, 1252, UTF-8.

	Для функций GUI проблема кодировки не столь серьезна, поскольку для GUI отсутствует стандартное API C/C++,
	в любом случае для него пишется прослойка, и мы можем по своему усмотрению ввести соглашение о кодировках
	текста в этой прослойке с машинно-независимой стороны.



	# Дополнительные замечания #

	В MacOS в файловом API могут возникать побочные эффекты, связанные с нормализацией текста, основанной на декомпозиции (decomposing).
	Эта проблема примерно того же рода, что и проблема регистронезависимости имен файлов в Windows.
*/

enum	literals_encoding
{
	e_ignore_literals_encoding = 0,
	e_encode_literals=1
};

enum	literals_decoding
{
	e_ignore_literals_decoding = 0,
	e_decode_literals = 1
};



// Универсальные преобразования (UTF-8, UTF-16, UTF-32)

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
u32string ustring_to_u32string(const uchar_t *str, size_t length, int default_character, bool *error);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
u32string ustring_to_u32string(const ustring &str, int default_character = default_ascii_character(), bool *error = nullptr);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
ustring u32string_to_ustring(const u32string &str, int default_character = default_ascii_character(), bool *error = nullptr);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
u32string u16string_to_u32string(const u16string &str, int default_character = default_ascii_character(), bool *error = nullptr);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
u16string u32string_to_u16string(const u32string &str, int default_character = default_ascii_character(), bool *error = nullptr);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
u16string ustring_to_u16string(const ustring &str, int default_character = default_ascii_character(), bool *error = nullptr);

/*!
	\param default_char Код для замены недопустимых символов, значение в диапазоне [0, 127].
*/
ustring u16string_to_ustring(const u16string &str, int default_character = default_ascii_character(), bool *error = nullptr);



// Машинно-зависимые преобразования (строки char, wchar_t)

wstring string_to_wstring(const string &str, literals_decoding literals_option);
ustring string_to_ustring(const string &str, literals_decoding literals_option);
u16string string_to_u16string(const string &str, literals_decoding literals_option);
u32string string_to_u32string(const string &str, literals_decoding literals_option);

string	wstring_to_string(const wstring &str, literals_encoding literals_option);
ustring wstring_to_ustring(const wstring &str);
u16string wstring_to_u16string(const wstring &str);
u32string wstring_to_u32string(const wstring &str);

string	ustring_to_string(const ustring &str, literals_encoding literals_option);
wstring	ustring_to_wstring(const ustring &str);

string u16string_to_string(const u16string &str16, literals_encoding literals_option);
wstring u16string_to_wstring(const u16string &str16);

string u32string_to_string(const u32string &str32, literals_encoding literals_option);
wstring u32string_to_wstring(const u32string &str32);

//! Преобразование контейнеров, содержащих строки utf-8. Содержимое не изменяется
inline ustring	string8_to_ustring(const string8 &str){return ustring(str.begin(), str.end()); }
inline string8	ustring_to_string8(const ustring &str){return string(str.begin(), str.end()); }

//
inline string	convert_to_string(const string &str, literals_encoding literals_option = e_encode_literals){(void)literals_option; return str;}
inline string	convert_to_string(const wstring &str, literals_encoding literals_option = e_encode_literals){return wstring_to_string(str, literals_option);}
inline string	convert_to_string(const ustring &str, literals_encoding literals_option = e_encode_literals){ return ustring_to_string(str, literals_option); }
inline string	convert_to_string(const u16string &str, literals_encoding literals_option = e_encode_literals){ return u16string_to_string(str, literals_option); }
inline string	convert_to_string(const u32string &str, literals_encoding literals_option = e_encode_literals){ return u32string_to_string(str, literals_option); }

inline wstring	convert_to_wstring(const string &str, literals_decoding literals_option = e_decode_literals){return string_to_wstring(str, literals_option);}
inline wstring	convert_to_wstring(const wstring &str, literals_decoding literals_option = e_decode_literals){(void)literals_option; return str;}
inline wstring	convert_to_wstring(const ustring &str, literals_encoding literals_option = e_encode_literals){ (void)literals_option; return ustring_to_wstring(str); }
inline wstring	convert_to_wstring(const u16string &str, literals_encoding literals_option = e_encode_literals){ (void)literals_option; return u16string_to_wstring(str); }
inline wstring	convert_to_wstring(const u32string &str, literals_encoding literals_option = e_encode_literals){ (void)literals_option; return u32string_to_wstring(str); }

inline ustring	convert_to_ustring(const string &str, literals_decoding literals_option = e_decode_literals){ return string_to_ustring(str, literals_option); }
inline ustring	convert_to_ustring(const wstring &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return wstring_to_ustring(str); }
inline ustring	convert_to_ustring(const ustring &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return str; }
inline ustring	convert_to_ustring(const u16string &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return u16string_to_ustring(str); }
inline ustring	convert_to_ustring(const u32string &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return u32string_to_ustring(str); }

inline u32string	convert_to_u32string(const string &str, literals_decoding literals_option = e_decode_literals){ return string_to_u32string(str, literals_option); }
inline u32string	convert_to_u32string(const wstring &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return wstring_to_u32string(str); }
inline u32string	convert_to_u32string(const ustring &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return ustring_to_u32string(str); }
inline u32string	convert_to_u32string(const u16string &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return u16string_to_u32string(str); }
inline u32string	convert_to_u32string(const u32string &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return str; }

//! Преобразование в string, содержащей в себе utf-8 кодировку
inline string8	convert_to_string8(const string &str, literals_decoding literals_option = e_decode_literals){ return ustring_to_string8(convert_to_ustring(str, literals_option)); }
inline string8	convert_to_string8(const wstring &str, literals_decoding literals_option = e_decode_literals){ return ustring_to_string8(convert_to_ustring(str, literals_option)); }
inline string8	convert_to_string8(const ustring &str, literals_decoding literals_option = e_decode_literals){ (void)literals_option; return ustring_to_string8(str); }
inline string8	convert_to_string8(const u16string &str, literals_decoding literals_option = e_decode_literals){ return ustring_to_string8(convert_to_ustring(str, literals_option)); }
inline string8	convert_to_string8(const u32string &str, literals_decoding literals_option = e_decode_literals){ return ustring_to_string8(convert_to_ustring(str, literals_option)); }

//! Преобразование string, содержащей в себе utf-8 кодировку, к другим строковым типам
inline string		string8_to_string(const string8 &str, literals_encoding literals_option = e_encode_literals){ return convert_to_string(string8_to_ustring(str), literals_option); }
inline wstring		string8_to_wstring(const string8 &str, literals_encoding literals_option = e_encode_literals){ return convert_to_wstring(string8_to_ustring(str), literals_option); }
inline u32string	string8_to_u32string(const string8 &str, literals_decoding literals_option = e_decode_literals){ return convert_to_u32string(string8_to_ustring(str), literals_option); }


//!	Преобразование строки в верхний регистр in-place
template<class T>
void	toupper(basic_string<T> &str)
{
	//	Пример взят отсюда: https://stackoverflow.com/questions/17991431/convert-a-unicode-string-in-c-to-upper-case
	//	В оригинале присутствовала инициализация глобальной локали:
	//	std::locale::global(std::locale(""));
	//
	//	Она приводила к ошибкам открытия дайкомов.
	//	Если ее не делать, некоторые языки преобразуются некорректно
	//
	//	Как оказалось, все дело в разнице между объектами, создаваемыми вызовами locale::classic() и locale("").
	//	По умолчанию используется locale::classic (она же locale::("C"), она корректно работает со многими кодовыми таблицами,
	//	но искажает расширенную латиницу (тестировалось на русской и английской системах). Возможно, на английской будет передавать его правильно.
	//	locale("") почти корректно работает почти со всеми языками, но вносит локализацию туда, где она не нужна
	//	(например, десятичную точку при вводе выводе заменяет на запятую).
	//
	//	Опыты на русской Windows 7 и английской Windows 10:
	//
	//	Расширенная латиница:
	//	Исходный текст		"Zoë Saldaña played in La maldición del padre Cardona. Blumenstraße"
	//	locale::classic()	"ZOë SALDAñA PLAYED IN LA MALDICIóN DEL PADRE CARDONA. BLUMENSTRAßE" (плохо)
	//	std::locale("")		"ZOË SALDAÑA PLAYED IN LA MALDICIÓN DEL PADRE CARDONA. BLUMENSTRAßE" (корректно, не считая эсцет)
	//
	//	Русский:
	//	Исходный текст			L"жил бы цитрус в чащах юга? да, но фальшивый экземпляр"
	//	Все локали				L"ЖИЛ БЫ ЦИТРУС В ЧАЩАХ ЮГА? ДА, НО ФАЛЬШИВЫЙ ЭКЗЕМПЛЯР" (корректно)
	//
	//	Древнегреческий с ударениями:
	//	Исходный текст					L"'Ότι μὲν ὑμει̃σ, ὠ̃ ἄνδρες 'Αθηναι̃οι, πεπόνθατε ὑπὸ τω̃ν ἐμω̃ν"
	//	locale::classic()				L"'ΌΤΙ ΜῈΝ ὙΜΕΙ̃Σ, Ὠ̃ ἌΝΔΡΕς 'ΑΘΗΝΑΙ̃ΟΙ, ΠΕΠΌΝΘΑΤΕ ὙΠῸ ΤΩ̃Ν ἘΜΩ̃Ν" (ошибка в концевой сигме (ς), меняется длина)
	//	std::locale("")				L"'ΌΤΙ ΜῈΝ ὙΜΕΙ̃Σ, Ὠ̃ ἌΝΔΡΕς 'ΑΘΗΝΑΙ̃ΟΙ, ΠΕΠΌΝΘΑΤΕ ὙΠῸ ΤΩ̃Ν ἘΜΩ̃Ν" (то же)
	//
	//	Новогреческий
	//	Исходный текст		L"Οι σημαντικότερες μαρτυρίες που πιστοποιούν την ύπαρξη ζωής στον ελλαδικό χώρο από την Λίθινη εποχή"
	//	locale::classic()	L"ΟΙ ΣΗΜΑΝΤΙΚΌΤΕΡΕς ΜΑΡΤΥΡΊΕς ΠΟΥ ΠΙΣΤΟΠΟΙΟΎΝ ΤΗΝ ΎΠΑΡΞΗ ΖΩΉς ΣΤΟΝ ΕΛΛΑΔΙΚΌ ΧΏΡΟ ΑΠΌ ΤΗΝ ΛΊΘΙΝΗ ΕΠΟΧΉ" ((ошибка в концевой сигме (ς))
	//	std::locale("")	L"ΟΙ ΣΗΜΑΝΤΙΚΌΤΕΡΕς ΜΑΡΤΥΡΊΕς ΠΟΥ ΠΙΣΤΟΠΟΙΟΎΝ ΤΗΝ ΎΠΑΡΞΗ ΖΩΉς ΣΤΟΝ ΕΛΛΑΔΙΚΌ ΧΏΡΟ ΑΠΌ ΤΗΝ ΛΊΘΙΝΗ ΕΠΟΧΉ" (то же)

	//	Армянский
	//	Исходный текст		L"Չնայած այն հանգամանքին, որ մայրցամաքային Հունաստանի և Էգեյան ծովում գտնվող կղզիների"
	//	locale::classic()	L"ՉՆԱՅԱԾ ԱՅՆ ՀԱՆԳԱՄԱՆՔԻՆ, ՈՐ ՄԱՅՐՑԱՄԱՔԱՅԻՆ ՀՈՒՆԱՍՏԱՆԻ և ԷԳԵՅԱՆ ԾՈՎՈՒՄ ԳՏՆՎՈՂ ԿՂԶԻՆԵՐԻ" (похоже на правду,но почему другая длина?)
	//	std::locale("")	L"ՉՆԱՅԱԾ ԱՅՆ ՀԱՆԳԱՄԱՆՔԻՆ, ՈՐ ՄԱՅՐՑԱՄԱՔԱՅԻՆ ՀՈՒՆԱՍՏԱՆԻ և ԷԳԵՅԱՆ ԾՈՎՈՒՄ ԳՏՆՎՈՂ ԿՂԶԻՆԵՐԻ" (то же)
	//

	//	Простейшее решение -- создавать нужную локаль каждый раз заново.
	auto	loc = std::locale("");
	auto& facet = std::use_facet<std::ctype<T>>(loc);
	facet.toupper(&str[0], &str[0] + str.size());

	//	Возможное альтернативное решение -- использовать бэкап глобальной локали:
	// 	auto	backup = std::locale();
	//	или auto	backup = std::locale::classic();
	// 	std::locale::global(std::locale(""));
	// 	auto& facet = std::use_facet<std::ctype<T>>(std::locale());
	// 	facet.toupper(&str[0], &str[0] + str.size());
	// 	std::locale::global(backup);
}

//!	Преобразование строки в нижний регистр in-place
template<class T>
void	tolower(basic_string<T> &str)
{
	auto	loc = std::locale("C");
	auto& facet = std::use_facet<std::ctype<T>>(loc);
	facet.tolower(&str[0], &str[0] + str.size());
}

//!	Преобразование строки в верхний регистр
template<class T>
basic_string<T> 	get_upper(const basic_string<T> &str)
{
	basic_string<T> result(str);
	toupper(result);
	return result;
}

//!	Преобразование строки в нижний регистр
template<class T>
basic_string<T> 	get_lower(const basic_string<T> &str)
{
	basic_string<T> result(str);
	tolower(result);
	return result;
}



XRAD_END

#endif // StringEncode_h__

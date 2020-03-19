/*!
	\file
	\date 2015-12-22 15:23
	\author kns
*/
#include "pre.h"
#include "StringEncode.h"

#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)
#include <XRADBasic/Sources/PlatformSpecific/MSVC/Internal/StringConverters_MS.h>
#endif

#include "HexChar.h"
#include <vector>
#include <locale>
#include <algorithm>

XRAD_BEGIN

//--------------------------------------------------------------

#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)

//--------------------------------------------------------------

namespace
{

inline wstring internal_string_to_wstring(const string &str, bool decode_literals)
{
	return string_to_wstring_MS(str, get_code_page_MS(), decode_literals);
}

inline string internal_wstring_to_string(const wstring &str, bool encode_literals,
		int default_char)
{
	return wstring_to_string_MS(str, get_code_page_MS(), encode_literals, default_char);
}

inline u16string internal_wstring_to_u16string(const wstring &str)
{
	return u16string(reinterpret_cast<const char16_t*>(str.c_str()), str.length());
}

inline wstring internal_u16string_to_wstring(const u16string &str)
{
	return wstring(reinterpret_cast<const wchar_t*>(str.c_str()), str.length());
}

} // namespace

//--------------------------------------------------------------

#elif !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_CHAR_UTF8_WCHAR_UTF32_VERSION)

//--------------------------------------------------------------

/*
	Полагаем следующее соответствие:
	- char - 8 бит, UTF-8
	- wchar_t - 32 бита, UTF-32
*/

namespace
{

static_assert(sizeof(wchar_t) == sizeof(char32_t), "Invalid wchar_t size.");

inline wstring internal_string_to_wstring(const string &str, bool decode_literals)
{
	auto s32 = ustring_to_u32string(
			ustring(uchar_t::pointer_from_char(str.c_str()), str.length()));
	return wstring(reinterpret_cast<const wchar_t*>(s32.c_str()), s32.length());
}

inline string internal_wstring_to_string(const wstring &str, bool encode_literals,
		int default_char)
{
	auto s8 = u32string_to_ustring(
			u32string(reinterpret_cast<const char32_t*>(str.c_str()), str.length()));
	return string(uchar_t::pointer_to_char(s8.c_str()), s8.length());
}

inline u16string internal_wstring_to_u16string(const wstring &str)
{
	return u32string_to_u16string(
			u32string(reinterpret_cast<const char32_t*>(str.c_str()), str.length()));
}

inline wstring internal_u16string_to_wstring(const u16string &str)
{
	auto s32 = u16string_to_u32string(str);
	return wstring(reinterpret_cast<const wchar_t*>(s32.c_str()), s32.length());
}

} // namespace

//--------------------------------------------------------------

#elif 0

// Пока не удаляем этот код ("defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)").

//--------------------------------------------------------------

namespace
{

// В этой реализации полагаем, что коды ASCII-символов одинаковы во всех кодировках.

inline wstring internal_string_to_wstring(const string &str, bool decode_literals)
{
	wstring result(str.length(), 0);
	auto res_it = result.begin();
	for (auto src_it = str.begin(); src_it != str.end(); ++src_it, ++res_it)
	{
		auto c = *src_it;
		if (c >= 0 && c <= 127)
		{
			*res_it = c;
		}
		else
		{
			*res_it = L'?';
		}
	}
	return result;
}

inline string internal_wstring_to_string(const wstring &str, bool encode_literals,
		int default_char)
{
	string result(str.length(), 0);
	auto res_it = result.begin();
	for (auto src_it = str.begin(); src_it != str.end(); ++src_it, ++res_it)
	{
		auto c = *src_it;
		if (c >= 0 && c <= 127)
		{
			*res_it = c;
		}
		else
		{
			*res_it = default_char;
		}
	}
	return result;
}

inline u16string internal_wstring_to_u16string(const wstring &str)
{
	// Кодировки wchar и char16_t могут не совпадать.
	u16string result(str.length(), 0);
	auto res_it = result.begin();
	for (auto src_it = str.begin(); src_it != str.end(); ++src_it, ++res_it)
	{
		auto c = *src_it;
		if (c >= 0 && c <= 127)
		{
			*res_it = c;
		}
		else
		{
			*res_it = u'?';
		}
	}
	return result;
}

inline wstring internal_u16string_to_wstring(const u16string &str)
{
	wstring result(str.length(), 0);
	auto res_it = result.begin();
	for (auto src_it = str.begin(); src_it != str.end(); ++src_it, ++res_it)
	{
		auto c = *src_it;
		if (c >= 0 && c <= 127)
		{
			*res_it = c;
		}
		else
		{
			*res_it = L'?';
		}
	}
	return result;
}

} // namespace

//--------------------------------------------------------------

#else

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

template <class InternT, class ExternT>
class string_converter
{
	public:
		using intern_string_t = InternT;
		using extern_string_t = ExternT;
		using intern_char_t = typename intern_string_t::value_type;
		using extern_char_t = typename extern_string_t::value_type;
		using state_type = mbstate_t;

	public:
		static extern_string_t convert_fwd(const intern_string_t& str, const locale& loc, bool *error);
		static intern_string_t convert_rev(const extern_string_t& str, const locale& loc, bool *error);

	private:
		template <class CodeCvt, class FromString, class ToString, class Functor>
		static FromString convert(const ToString& src_str, const locale& loc, bool *error,
				Functor f);
};

//--------------------------------------------------------------

template <class InternT, class ExternT>
auto string_converter<InternT, ExternT>::convert_fwd(const intern_string_t& str, const locale& loc,
		bool *error) -> extern_string_t
{
	using codecvt_t = codecvt<intern_char_t, extern_char_t, state_type>;
	return convert<codecvt_t, extern_string_t>(str, loc, error,
			[](const codecvt_t &cvt,
					state_type &state,
					const intern_char_t *from,
					const intern_char_t *from_end,
					const intern_char_t *&from_next,
					extern_char_t *to,
					extern_char_t *to_end,
					extern_char_t *&to_next)
			{
				return cvt.out(state, from, from_end, from_next, to, to_end, to_next);
			});
}

//--------------------------------------------------------------

template <class InternT, class ExternT>
auto string_converter<InternT, ExternT>::convert_rev(const extern_string_t& str, const locale& loc,
		bool *error) -> intern_string_t
{
	using codecvt_t = codecvt<intern_char_t, extern_char_t, state_type>;
	return convert<codecvt_t, intern_string_t>(str, loc, error,
			[](const codecvt_t &cvt,
					state_type &state,
					const extern_char_t *from,
					const extern_char_t *from_end,
					const extern_char_t *&from_next,
					intern_char_t *to,
					intern_char_t *to_end,
					intern_char_t *&to_next)
			{
				return cvt.in(state, from, from_end, from_next, to, to_end, to_next);
			});
}

//--------------------------------------------------------------

template <class InternT, class ExternT>
template <class CodeCvt, class FromString, class ToString, class Functor>
auto string_converter<InternT, ExternT>::convert(const ToString& from_str,
		const locale& loc, bool *error, Functor f) -> FromString
{
	using codecvt_t = CodeCvt;
	using from_string_t = ToString;
	using to_string_t = FromString;
	using from_char_t = typename from_string_t::value_type;
	using to_char_t = typename to_string_t::value_type;
	using state_type = typename codecvt_t::state_type;

	if (from_str.empty())
		return to_string_t();

	constexpr size_t max_initial_length = 4096;
	constexpr to_char_t default_char = to_char_t('?');

	const auto& cvt = use_facet<codecvt_t>(loc);
	// Преобразование может дать длину строки как больше, так и меньше исходной
	// (UTF-8 -> UTF-16, UTF-16 -> UTF-8).
	// Приблизительно оценить максимальную длину выходной строки для cvt.out(),
	// не выполняя конвертацию, можно с помощью вызова cvt.max_length().
	// Для cvt.in() такую оценку получить можно только с помощью cvt "обратного" типа.
	// Пока обойдемся грубой оценкой по длине исходной строки.
	size_t initial_length = from_str.length();
	if (initial_length > max_initial_length)
		initial_length = max_initial_length;
	vector<to_char_t> to_buf(initial_length, to_char_t(0));

	const auto *from = from_str.c_str();
	const auto *from_end = from + from_str.length();
	decltype(from) from_next = nullptr;
	auto *to = to_buf.data();
	auto *to_end = to + to_buf.size();
	decltype(to) to_next = nullptr;

	state_type state = {};
	size_t to_size = 0;
	for (;;)
	{
		auto status = f(cvt, state, from, from_end, from_next, to, to_end, to_next);
		switch (status)
		{
			case codecvt_t::noconv:
				if (to_buf.size() < from_str.length())
					to_buf.resize(from_str.length());
				to_size = from_str.length();
				copy(from_str.c_str(), from_str.c_str() + from_str.length(), to_buf.data());
				status = codecvt_t::ok;
				break;

			case codecvt_t::ok:
				to_size += to_next - to;
				if (from_next != from_end)
				{
					// Ошибка в библиотеках MSVC2015 для codecvt<u16char, u32char, ...>?
					// status == codecvt_t::ok, но преобразован не весь буфер, как в случае
					// codecvt_t::partial.
					from = from_next;
					status = codecvt_t::partial;
				}
				break;

			case codecvt_t::partial:
				to_size += to_next - to;
				if (from_next != from_end)
				{
					from = from_next;
				}
				else
				{
					// Входной буфер содержит в конце неполный символ.
					if (to_buf.size() < to_size + 1)
						to_buf.resize(to_size + 1);
					to_buf[to_size++] = default_char;
					if (error)
						*error = true;
					status = codecvt_t::ok;
				}
				break;

			case codecvt_t::error:
				to_size += to_next - to;
				if (to_buf.size() < to_size + 1)
					to_buf.resize(max(2*to_buf.size(), to_size + 1 + initial_length));
				to_buf[to_size++] = default_char;
				if (error)
					*error = true;
				if (from != from_end)
					from = from_next + 1;
				else
					status = codecvt_t::ok;
				break;
		}
		if (status == codecvt_t::ok)
			break;
		if (to_buf.size() < to_size + initial_length)
			to_buf.resize(2*to_buf.size());
		to = to_buf.data() + to_size;
		to_end = to_buf.data() + to_buf.size();
	}
	return to_string_t(to_buf.data(), to_size);
}

//--------------------------------------------------------------

auto cvt_loc = locale("");

//--------------------------------------------------------------
/*
	Замечания по реализации в MSVC

	В MSVC 2015, 2017 и, вероятно, 2019 не все варианты codecvt работают корректно, а некоторые
	(из описанных в стандарте C++11-17) даже не компилируются. Корректно работают следующие codecvt:
	- codecvt<wchar_t, char>: преобразование из UTF-16 (wchar_t) в ANSI (char) и обратно;
	- codecvt<wchar_t, char16_t>: преобразование типа без преобразования кодировки UTF-16
	  (при оно не является преобразованием codecvt<>::noconv [MSVC 2015, x64, Win10.1909 x64]).

	Преобразование codecvt<wchar_t, char> при использовании специально созданных локалей
	(".1251", ".1252", ".936") позволяют также использовать заданную кодировку для char.
	При этом UTF-8 не поддерживается. [MSVC 2015, x64, Win10.1909 x64, экспериментальная поддержка
	UTF-8 в качестве системной кодовой страницы выключена.]
*/
//--------------------------------------------------------------

inline wstring internal_string_to_wstring(const string &str, bool decode_literals)
{
	return string_converter<wstring, string>::convert_rev(str, cvt_loc, nullptr);
}

inline string internal_wstring_to_string(const wstring &str, bool encode_literals,
		int default_char)
{
	return string_converter<wstring, string>::convert_fwd(str, cvt_loc, nullptr);
}

inline u16string internal_wstring_to_u16string(const wstring &str)
{
	return string_converter<wstring, u16string>::convert_fwd(str, cvt_loc, nullptr);
}

inline wstring internal_u16string_to_wstring(const u16string &str)
{
	return string_converter<wstring, u16string>::convert_rev(str, cvt_loc, nullptr);
}

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

#endif



//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

namespace utf8_encoder
{

const char32_t utf8_length1_max = 0x7F;
const char32_t utf8_length2_max = 0x7FF;
const char32_t utf8_length3_max = 0xFFFF;
	// При обработке данных используется тот факт, что коды длины 3 и менее кодируют символы,
	// для представления которых достаточно 16 бит.

const char32_t utf8_length4_max = 0x10FFFF;
	// Теоретическое максимальное значение для 4-байтовой кодировки равно 0x1FFFFF (см. устаревший RFC 2279),
	// но значения более 0x10FFFF запрещены стандартом.
	// По устаревшему RFC 2279 были возможны также кодировки с 5 и 6 байтами, позволявшие закодировать числа до 0x7FFFFFFF,
	// но они сейчас запрещены.

const uint_fast8_t utf8_length1_prefix_mask = 0x80; // Сам префикс равен 0.
const uint_fast8_t utf8_length2_prefix = 0xC0;
const uint_fast8_t utf8_length2_prefix_mask = 0xE0;
const uint_fast8_t utf8_length2_value_mask = 0x1F;
const uint_fast8_t utf8_length3_prefix = 0xE0;
const uint_fast8_t utf8_length3_prefix_mask = 0xF0;
const uint_fast8_t utf8_length3_value_mask = 0x0F;
const uint_fast8_t utf8_length4_prefix = 0xF0;
const uint_fast8_t utf8_length4_prefix_mask = 0xF8;
const uint_fast8_t utf8_length4_value_mask = 0x07;
const uint_fast8_t utf8_tail_prefix = 0x80;
const uint_fast8_t utf8_tail_prefix_mask = 0xC0;
const unsigned int utf8_tail_value_bit_count = 6;
const uint_fast8_t utf8_tail_value_mask = 0x3F; // Маска 6 младших битов, (1 << 6) - 1.

const char32_t utf16_surrogate_min = 0xD800;
const char32_t utf16_surrogate_max = 0xDFFF;

const size_t code_point_buffer_size = 4;

/*!
	\brief Закодировать один символ

	\return
		- количество записанных в буфер символов (1 ... code_point_buffer_size);
		- 0 в случае ошибки кодирования (недопустимый символ Юникода).
*/
size_t encode_code_point(char32_t c32, char c8_buffer[code_point_buffer_size])
{
	if (c32 <= utf8_length1_max)
	{
		// Код длины 1.
		c8_buffer[0] = static_cast<char>(c32);
		return 1;
	}
	else if (c32 <= utf8_length2_max)
	{
		// Код длины 2.
		c8_buffer[0] = static_cast<char>(utf8_length2_prefix | (c32 >> utf8_tail_value_bit_count));
		c8_buffer[1] = static_cast<char>(utf8_tail_prefix | (c32 & utf8_tail_value_mask));
		return 2;
	}
	else if (c32 <= utf8_length3_max)
	{
		static_assert(utf16_surrogate_min > utf8_length2_max && utf16_surrogate_min <= utf8_length3_max, "Invalid UTF-8 constants.");
		static_assert(utf16_surrogate_max > utf8_length2_max && utf16_surrogate_max <= utf8_length3_max, "Invalid UTF-8 constants.");
		if (c32 >= utf16_surrogate_min && c32 <= utf16_surrogate_max)
		{
			// Неверное значение: диапазон суррогатных пар UTF-16.
			return 0;
		}
		// Код длины 3.
		c8_buffer[0] = utf8_length3_prefix |
				static_cast<char>(c32 >> (2 * utf8_tail_value_bit_count));
		c8_buffer[1] = utf8_tail_prefix |
				static_cast<char>((c32 >> utf8_tail_value_bit_count) & utf8_tail_value_mask);
		c8_buffer[2] = utf8_tail_prefix |
				static_cast<char>(c32 & utf8_tail_value_mask);
		return 3;
	}
	else if (c32 <= utf8_length4_max)
	{
		// Код длины 4.
		c8_buffer[0] = utf8_length4_prefix |
				static_cast<char>(c32 >> (3 * utf8_tail_value_bit_count));
		c8_buffer[1] = utf8_tail_prefix |
				static_cast<char>((c32 >> (2 * utf8_tail_value_bit_count)) & utf8_tail_value_mask);
		c8_buffer[2] = utf8_tail_prefix |
				static_cast<char>((c32 >> utf8_tail_value_bit_count) & utf8_tail_value_mask);
		c8_buffer[3] = utf8_tail_prefix |
				static_cast<char>(c32 & utf8_tail_value_mask);
		return 4;
	}
	// Неверное значение: слишком большое число.
	return 0;
}

/*!
	\brief Раскодировать один символ

	\param src Текст UTF-8, содержит не меньше 1 элемента.
	\param src_length Длина текста в src. Значение не меньше 1.
	\param src_code_length [out] Количество прочитанных значений из src.
	\param error [out] Если при декодировании произошла ошибка, сюда записывается true. Иначе не изменяется.
*/
char32_t decode_code_point(const char *src, size_t src_length, size_t *src_code_length,
		int default_ascii_character, bool *error = nullptr)
{
	uint_fast8_t code0 = (unsigned char)*src;
	if (!(code0 & utf8_length1_prefix_mask))
	{
		// Код длины 1.
		*src_code_length = 1;
		return code0;
	}
	else if ((code0 & utf8_length2_prefix_mask) == utf8_length2_prefix)
	{
		// Код длины 2.
		if (src_length < 2)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code1 = (unsigned char)src[1];
		if ((code1 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение второго элемента кода.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		*src_code_length = 2;
		static_assert(utf8_length2_max <= 0xFFFFu, "Invalid UTF-8 constants."); // Для представления символа достаточно 16 бит.
		auto c32 = ((uint_fast16_t)(code0 & utf8_length2_value_mask) << utf8_tail_value_bit_count) |
				(code1 & utf8_tail_value_mask);
		if (c32 <= utf8_length1_max)
		{
			// Неверное закодированное значение: не минимальный код (значение из диапазона кодов меньшей длины).
			if (error)
				*error = true;
			return default_ascii_character;
		}
		return c32;
	}
	else if ((code0 & utf8_length3_prefix_mask) == utf8_length3_prefix)
	{
		// Код длины 3.
		if (src_length < 2)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code1 = (unsigned char)src[1];
		if ((code1 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение второго элемента кода.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		if (src_length < 3)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 2;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code2 = (unsigned char)src[2];
		if ((code2 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение третьего элемента кода.
			*src_code_length = 2;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		*src_code_length = 3;
		static_assert(utf8_length3_max <= 0xFFFFu, "Invalid UTF-8 constants."); // Для представления символа достаточно 16 бит.
		auto c32 = ((uint_fast16_t)(code0 & utf8_length3_value_mask) << (2 * utf8_tail_value_bit_count)) |
				((uint_fast16_t)(code1 & utf8_tail_value_mask) << utf8_tail_value_bit_count) |
				(code2 & utf8_tail_value_mask);
		if (c32 <= utf8_length2_max)
		{
			// Неверное закодированное значение: не минимальный код (значение из диапазона кодов меньшей длины).
			if (error)
				*error = true;
			return default_ascii_character;
		}
		static_assert(utf16_surrogate_min > utf8_length2_max && utf16_surrogate_min <= utf8_length3_max, "Invalid UTF-8 constants.");
		static_assert(utf16_surrogate_max > utf8_length2_max && utf16_surrogate_max <= utf8_length3_max, "Invalid UTF-8 constants.");
		if (c32 >= utf16_surrogate_min && c32 <= utf16_surrogate_max)
		{
			// Неверное закодированное значение: код из диапазона суррогатных пар.
			if (error)
				*error = true;
			return default_ascii_character;
		}
		return c32;
	}
	else if ((code0 & utf8_length4_prefix_mask) == utf8_length4_prefix)
	{
		// Код длины 4.
		if (src_length < 2)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code1 = (unsigned char)src[1];
		if ((code1 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение второго элемента кода.
			*src_code_length = 1;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		if (src_length < 3)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 2;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code2 = (unsigned char)src[2];
		if ((code2 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение третьего элемента кода.
			*src_code_length = 2;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		if (src_length < 4)
		{
			// Недостаточная длина входного массива данных.
			*src_code_length = 3;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		uint_fast8_t code3 = (unsigned char)src[3];
		if ((code3 & utf8_tail_prefix_mask) != utf8_tail_prefix)
		{
			// Неверное значение четвертого элемента кода.
			*src_code_length = 3;
			if (error)
				*error = true;
			return default_ascii_character;
		}
		*src_code_length = 4;
		auto c32 = ((uint_fast32_t)(code0 & utf8_length3_value_mask) << (3 * utf8_tail_value_bit_count)) |
				((uint_fast32_t)(code1 & utf8_tail_value_mask) << (2 * utf8_tail_value_bit_count)) |
				((uint_fast32_t)(code2 & utf8_tail_value_mask) << utf8_tail_value_bit_count) |
				(code3 & utf8_tail_value_mask);
		if (c32 <= utf8_length3_max ||
				c32 > utf8_length4_max)
		{
			// 1: Неверное закодированное значение: не минимальный код (значение из диапазона кодов меньшей длины).
			// 2: Неверное закодированное значение: код символа больше допустимого стандартом.
			if (error)
				*error = true;
			return default_ascii_character;
		}
		return c32;
	}
	// Недопустимый код.
	*src_code_length = 1;
	if (error)
		*error = true;
	return default_ascii_character;
}

} // namespace utf8_encoder



//--------------------------------------------------------------



namespace utf16_encoder
{

const char32_t utf16_single_max = 0xFFFF;
const char32_t utf16_max = 0x10FFFF;
const char32_t utf16_surrogate_min = 0xD800;
const char32_t utf16_surrogate_max = 0xDFFF;
const unsigned int utf16_surrogate_bit_shift = 10;
const uint_fast32_t utf16_surrogate_bit_mask = 0x3FF; // Маска 10 младших битов.
const uint_fast32_t utf16_surrogate_code_bias = 0x10000;
const uint_fast32_t utf16_surrogate_high_bias = 0xD800;
const uint_fast32_t utf16_surrogate_low_bias = 0xDC00;

// Проверки согласованности значений констант.
// Реальной необходимости в проверках нет, т.к. значения констант прямо или косвенно прописаны в стандарте UTF-16 и быть другими не могут.
// Скорее здесь показано, как эти значения связаны друг с другом.
static_assert(utf16_max == utf16_surrogate_code_bias + ((uint_fast32_t)1 << (utf16_surrogate_bit_shift * 2u)) - 1u, "utf16 constants are invalid.");
static_assert(utf16_surrogate_bit_mask == ((uint_fast32_t)1 << utf16_surrogate_bit_shift) - 1u, "utf16 constants are invalid.");
static_assert(utf16_surrogate_high_bias == utf16_surrogate_min, "utf16 constants are invalid.");
static_assert(utf16_surrogate_low_bias == utf16_surrogate_high_bias + utf16_surrogate_bit_mask + 1u, "utf16 constants are invalid.");
static_assert(utf16_surrogate_max == utf16_surrogate_low_bias + utf16_surrogate_bit_mask, "utf16 constants are invalid.");

const size_t code_point_buffer_size = 2;

/*!
	\brief Закодировать один символ

	\return
		- количество записанных в буфер символов (1 ... code_point_buffer_size);
		- 0 в случае ошибки кодирования (недопустимый символ Юникода).
*/
size_t encode_code_point(char32_t c32, char16_t c16_buffer[code_point_buffer_size])
{
	if (c32 < utf16_surrogate_min)
	{
		c16_buffer[0] = static_cast<char16_t>(c32);
		return 1;
	}
	// Здесь c32 >= utf16_surrogate_min.
	if (c32 <= utf16_single_max)
	{
		if (c32 <= utf16_surrogate_max)
		{
			// Неверное значение кода символа Юникода (диапазон суррогатных пар).
			return 0;
		}
		c16_buffer[0] = static_cast<char16_t>(c32);
		return 1;
	}
	if (c32 > utf16_max)
	{
		// Неверное значение кода символа Юникода (слишком большое число).
		return 0;
	}
	// Запись суррогатной пары:
	uint_fast32_t biased_value = c32 - utf16_surrogate_code_bias;
	c16_buffer[0] = static_cast<char16_t>((biased_value >> utf16_surrogate_bit_shift) +
			utf16_surrogate_high_bias);
	c16_buffer[1] = static_cast<char16_t>((biased_value & utf16_surrogate_bit_mask) +
			utf16_surrogate_low_bias);
	return 2;
}

/*!
	\brief Раскодировать один символ

	\param src Текст UTF-8, содержит не меньше 1 элемента.
	\param src_length Длина текста в src. Значение не меньше 1.
	\param src_code_length [out] Количество прочитанных значений из src.
	\param error [out] Если при декодировании произошла ошибка, сюда записывается true. Иначе не изменяется.
*/
char32_t decode_code_point(const char16_t *src, size_t src_length, size_t *src_code_length,
		int default_ascii_character, bool *error = nullptr)
{
	uint_fast32_t code0 = *src;
	if (code0 < utf16_surrogate_min || code0 > utf16_surrogate_max)
	{
		// Обычный код.
		*src_code_length = 1;
		return code0;
	}
	// Суррогатная пара.
	if (code0 >= utf16_surrogate_low_bias ||
			src_length < 2)
	{
		// 1: неверное значение первой части кода.
		// 2: отсутствует вторая часть кода.
		*src_code_length = 1;
		if (error)
			*error = true;
		return default_ascii_character;
	}
	uint_fast32_t code1 = src[1];
	if (code1 < utf16_surrogate_low_bias || code1 > utf16_surrogate_max)
	{
		// Неверное значение второй части кода.
		*src_code_length = 1;
		if (error)
			*error = true;
		return default_ascii_character;
	}
	*src_code_length = 2;
	return (((code0 - utf16_surrogate_high_bias) << utf16_surrogate_bit_shift) |
			(code1 - utf16_surrogate_low_bias)) +
			utf16_surrogate_code_bias;
}

} // namespace utf16_encoder

} //namespace



//--------------------------------------------------------------

u32string ustring_to_u32string(const ustring &str, int default_character, bool *error)
{
	return ustring_to_u32string(str.c_str(), str.length(), default_character, error);
}

u32string ustring_to_u32string(const uchar_t *str, size_t src_length, int default_character, bool *error)
{
	const char *src = uchar_t::pointer_to_char(str);
	size_t out_buffer_capacity = 16;
	vector<char32_t> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	for (size_t i = 0; i < src_length;)
	{
		size_t src_code_length = 1;
		char32_t c32 = utf8_encoder::decode_code_point(src + i, src_length - i, &src_code_length, default_character, error);
		i += src_code_length;
		if (out_buffer_capacity < out_buffer.size() + 1)
		{
			out_buffer_capacity = (out_buffer.size() + 1) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		out_buffer.push_back(c32);
	}
	return u32string(out_buffer.data(), out_buffer.size());
}

ustring u32string_to_ustring(const u32string &str, int default_character, bool *error)
{
	const char32_t *src = str.c_str();
	size_t src_length = str.length();
	size_t out_buffer_capacity = 16;
	vector<char> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	char c8_buffer[utf8_encoder::code_point_buffer_size];
	for (size_t i = 0; i < src_length; ++i)
	{
		char32_t c32 = src[i];
		size_t out_code_length = utf8_encoder::encode_code_point(c32, c8_buffer);
		if (!out_code_length)
		{
			// Недопустимое значение Юникода.
			if (error)
				*error = true;
			c8_buffer[0] = static_cast<char>(default_character);
			out_code_length = 1;
		}
		if (out_buffer_capacity < out_buffer.size() + out_code_length)
		{
			out_buffer_capacity = (out_buffer.size() + out_code_length) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		for (size_t j = 0; j < out_code_length; ++j)
			out_buffer.push_back(c8_buffer[j]);
	}
	return ustring(uchar_t::pointer_from_char(out_buffer.data()), out_buffer.size());
}

u32string u16string_to_u32string(const u16string &str, int default_character, bool *error)
{
	const char16_t *src = str.c_str();
	size_t src_length = str.length();
	size_t out_buffer_capacity = 16;
	vector<char32_t> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	for (size_t i = 0; i < src_length;)
	{
		size_t src_code_length = 1;
		char32_t c32 = utf16_encoder::decode_code_point(src + i, src_length - i, &src_code_length, default_character, error);
		i += src_code_length;
		if (out_buffer_capacity < out_buffer.size() + 1)
		{
			out_buffer_capacity = (out_buffer.size() + 1) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		out_buffer.push_back(c32);
	}
	return u32string(out_buffer.data(), out_buffer.size());
}

u16string u32string_to_u16string(const u32string &str, int default_character, bool *error)
{
	const char32_t *src = str.c_str();
	size_t src_length = str.length();
	size_t out_buffer_capacity = 16;
	vector<char16_t> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	char16_t c16_buffer[utf16_encoder::code_point_buffer_size];
	for (size_t i = 0; i < src_length; ++i)
	{
		char32_t c32 = src[i];
		size_t out_code_length = utf16_encoder::encode_code_point(c32, c16_buffer);
		if (!out_code_length)
		{
			// Недопустимое значение Юникода.
			if (error)
				*error = true;
			c16_buffer[0] = static_cast<char16_t>(default_character);
			out_code_length = 1;
		}
		if (out_buffer_capacity < out_buffer.size() + out_code_length)
		{
			out_buffer_capacity = (out_buffer.size() + out_code_length) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		for (size_t j = 0; j < out_code_length; ++j)
			out_buffer.push_back(c16_buffer[j]);
	}
	return u16string(out_buffer.data(), out_buffer.size());
}

u16string ustring_to_u16string(const ustring &str, int default_character, bool *error)
{
	const char *src = uchar_t::pointer_to_char(str.c_str());
	size_t src_length = str.length();
	size_t out_buffer_capacity = 16;
	vector<char16_t> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	char16_t c16_buffer[utf16_encoder::code_point_buffer_size];
	for (size_t i = 0; i < src_length;)
	{
		size_t src_code_length = 1;
		char32_t c32 = utf8_encoder::decode_code_point(src + i, src_length - i, &src_code_length, default_character, error);
		i += src_code_length;
		size_t out_code_length = utf16_encoder::encode_code_point(c32, c16_buffer);
		if (!out_code_length)
		{
			// Недопустимое значение Юникода.
			if (error)
				*error = true;
			c16_buffer[0] = static_cast<char16_t>(default_character);
			out_code_length = 1;
		}
		if (out_buffer_capacity < out_buffer.size() + out_code_length)
		{
			out_buffer_capacity = (out_buffer.size() + out_code_length) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		for (size_t j = 0; j < out_code_length; ++j)
			out_buffer.push_back(c16_buffer[j]);
	}
	return u16string(out_buffer.data(), out_buffer.size());
}

ustring u16string_to_ustring(const u16string &str, int default_character, bool *error)
{
	const char16_t *src = str.c_str();
	size_t src_length = str.length();
	size_t out_buffer_capacity = 16;
	vector<char> out_buffer;
	out_buffer.reserve(out_buffer_capacity);
	char c8_buffer[utf8_encoder::code_point_buffer_size];
	for (size_t i = 0; i < src_length;)
	{
		size_t src_code_length = 1;
		char32_t c32 = utf16_encoder::decode_code_point(src + i, src_length - i, &src_code_length,
				default_character, error);
		i += src_code_length;
		size_t out_code_length = utf8_encoder::encode_code_point(c32, c8_buffer);
		if (!out_code_length)
		{
			// Недопустимое значение Юникода.
			if (error)
				*error = true;
			c8_buffer[0] = static_cast<char>(default_character);
			out_code_length = 1;
		}
		if (out_buffer_capacity < out_buffer.size() + out_code_length)
		{
			out_buffer_capacity = (out_buffer.size() + out_code_length) * 3 / 4;
			out_buffer.reserve(out_buffer_capacity);
		}
		for (size_t j = 0; j < out_code_length; ++j)
			out_buffer.push_back(c8_buffer[j]);
	}
	return ustring(uchar_t::pointer_from_char(out_buffer.data()), out_buffer.size());
}



//--------------------------------------------------------------



wstring string_to_wstring(const string &str, literals_decoding literals_option)
{
	return internal_string_to_wstring(str, literals_option == e_decode_literals);
}

ustring string_to_ustring(const string &str, literals_decoding e_decode_literals)
{
	return u16string_to_ustring(string_to_u16string(str, e_decode_literals));
}

u16string string_to_u16string(const string &str, literals_decoding literals_option)
{
	return internal_wstring_to_u16string(string_to_wstring(str, literals_option));
}

u32string string_to_u32string(const string &str, literals_decoding literals_option)
{
	return u16string_to_u32string(string_to_u16string(str, literals_option));
}



//--------------------------------------------------------------



string wstring_to_string(const wstring &str, literals_encoding literals_option)
{
	return internal_wstring_to_string(str, literals_option == e_encode_literals, default_ascii_character());
}

ustring wstring_to_ustring(const wstring &wstr)
{
	return u16string_to_ustring(internal_wstring_to_u16string(wstr));
}

u16string wstring_to_u16string(const wstring &str)
{
	return internal_wstring_to_u16string(str);
}

u32string wstring_to_u32string(const wstring &wstr)
{
	return u16string_to_u32string(internal_wstring_to_u16string(wstr));
}



//--------------------------------------------------------------



string ustring_to_string(const ustring &ustr, literals_encoding literals_option)
{
	return internal_wstring_to_string(internal_u16string_to_wstring(ustring_to_u16string(ustr)),
			literals_option == e_encode_literals, default_ascii_character());
}

wstring	ustring_to_wstring(const ustring &str)
{
	return internal_u16string_to_wstring(ustring_to_u16string(str));
}



//--------------------------------------------------------------



string u16string_to_string(const u16string &str16, literals_encoding literals_option)
{
	return internal_wstring_to_string(internal_u16string_to_wstring(str16),
			literals_option == e_encode_literals, default_ascii_character());
}

wstring u16string_to_wstring(const u16string &str16)
{
	return internal_u16string_to_wstring(str16);
}



//--------------------------------------------------------------



string u32string_to_string(const u32string &str32, literals_encoding literals_option)
{
	return internal_wstring_to_string(internal_u16string_to_wstring(u32string_to_u16string(str32)),
			literals_option == e_encode_literals, default_ascii_character());
}

wstring	u32string_to_wstring(const u32string &str)
{
	return internal_u16string_to_wstring(u32string_to_u16string(str));
}



//--------------------------------------------------------------

XRAD_END

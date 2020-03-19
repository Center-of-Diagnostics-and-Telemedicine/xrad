#ifndef md5_h__
#define md5_h__

/*!
	\file
	\date 2019/09/23 14:20
	\author kulberg

	\brief  
*/

#include <XRADBasic/Sources/Containers/DataArray.h>
#include <array>
#include <string>


XRAD_BEGIN

//!	\brief Тип для хранения контрольной суммы md5 в численном виде
using md5_digest_t = std::array<uint8_t, 16>;

//!	\brief Вычисление контрольной суммы для любой последовательности байт
md5_digest_t md5(const void *bytes, size_t n_bytes);

//!	\brief Вычисление контрольной суммы для std::string
md5_digest_t md5(const string &data);

//!	\brief Вычисление контрольной суммы для std::wstring.
//!	Прежде вычисления строка преобразуется в utf-8
md5_digest_t md5(const wstring &s);

//!	\brief Вычисление контрольной суммы от диапазона итераторов.
template<class IT>
md5_digest_t md5(IT i0, IT ie)
{
	size_t	n_elements = ie-i0;
	size_t	element_size = sizeof(iterator_traits<IT>::value_type);

	//TODO процедура использует избыточный буфер, надо сделать потом через append каждого следующего элемента
	DataArray<char> buffer(element_size*n_elements);

	auto	buffer_it = buffer.begin();
	for(auto it = i0; it != ie; ++it)
	{
		const char	*element = reinterpret_cast<const char*>(&*it);
		for(size_t i = 0; i < element_size; ++i)
		{
			*buffer_it = *element;
			++buffer_it;
			++element;
		}
	}
	return md5(buffer.data(), buffer.size());
}

string	md5_digest_string(const md5_digest_t &digest, bool caps);
wstring	md5_digest_wstring(const md5_digest_t &digest, bool caps);

template<typename ... Args>
string	md5s(Args ... args)
{
	return md5_digest_string(md5(args...), false);
}

template<typename ... Args>
string	md5S(Args ... args)
{
	return md5_digest_string(md5(args...), true);
}

template<typename ... Args>
wstring	md5ws(Args ... args)
{
	return md5_digest_wstring(md5(args...), false);
}

template<typename ... Args>
wstring	md5WS(Args ... args)
{
	return md5_digest_wstring(md5(args...), true);
}

XRAD_END

#endif // md5_h__

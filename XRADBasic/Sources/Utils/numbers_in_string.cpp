/*!
	\file
	\date 2018/04/02 14:11
	\author kulberg
*/
#include "pre.h"
#include "numbers_in_string.h"

XRAD_BEGIN


namespace
{
void	inc(wchar_t &c)
{
	switch(c)
	{
		case '0': c='1';
			break;
		case '1': c='2';
			break;
		case '2': c='3';
			break;
		case '3': c='4';
			break;
		case '4': c='5';
			break;
		case '5': c='6';
			break;
		case '6': c='7';
			break;
		case '7': c='8';
			break;
		case '8': c='9';
			break;
		default:
			throw invalid_argument("EraseDummyDigits, invalid digit");
	}
}

auto	zero = [](wchar_t c) {return c == L'0';};
auto	nine = [](wchar_t c) {return c == L'9';};
auto	is_sign = [](wchar_t c) {return c == L'+' || c == L'-';};
auto	is_e = [](wchar_t c) {return c == L'e' || c == L'E';};

// удалить нули до первой значащей цифры
void	erase_leading_zeroes(wstring &s)
{
	auto	first_position = s.begin() + (is_sign(s[0]) ? 1:0);
	auto	search = first_position;
	while(search < s.end() && *search == L'0') ++search;

	if(search > first_position)// если нули в начале числа обнаружены
	{
		if(search < s.end())
		{
			if(*search==L'.')--search;//последний 0 перед точкой оставить
		}
		if(search == s.end()) --search;// единственный оставшийся ноль в числе оставить

		s.erase(first_position, search);
	}
}
}//namespace


void	smart_round(wstring &s, size_t count)
{
	erase_leading_zeroes(s);

	size_t	n_dots(0), n_nondigits(0);
	//size_t	numbers_start(0);


	auto	e_position = find_if(s.begin(), s.end(), is_e);
	auto	first_position = s.begin() + (is_sign(s[0]) ? 1:0);
	size_t	epos = e_position - s.begin();


	for(auto c = first_position; c < e_position; ++c)
	{
		if(*c == L'.') ++n_dots; if(!isdigit(*c) && *c != L'.') ++n_nondigits;
	}

	if (n_dots>1 || n_nondigits)
	{
		throw invalid_argument(ssprintf("smart_round(wstring &s, size_t count), invalid string = %s",
				convert_to_string(s).c_str()));
	}


	size_t	dot_position(0);
	for(auto c: s)
	{
		if(c==L'.') break;
		++dot_position;
	}

	size_t	zeroes(0), nines(0);

	auto i = dot_position + 1;
	for(; i < epos; ++i)
	{
		zeroes = nines = 0;

		while(zero(s[i+zeroes]) && i+zeroes < epos) ++zeroes;
		if(zeroes >= count) break;

		while(nine(s[i+nines]) && i+nines < epos) ++nines;
		if(nines >= count) break;
	}

	if(i==dot_position+1)--i;
	if(zeroes>=count)
	{
		if(i>1) s.erase(s.begin() + i, e_position);
		else if(s[0] != '0') s.erase(s.begin() + i, e_position);
		// Удаляем длинную цепочку нулей но только в том случае, если результат не превращается в 0. Иначе оставляем как было, числа вида 0.000000001 имеют смысл
	}
	if(nines>=count)
	{
		if(i==0)
		{
			s = L"1";//.999999
		}
		else
		{
			s.erase(s.begin() + i, e_position);
			inc(s[i-1]);
		}
	}

}



XRAD_END

/*!
	\file
	\date 22:12:2016 12:26
	\author KNS
*/
#include "pre.h"
#include "SelectSeriesInteractive.h"

#include <XRADGUI.h>
#include <set>

XRAD_BEGIN

/*
Пожелание по работе следующей функции. Сейчас на кнопках выбора выводятся параметры "id" (series_id, patient_id и т.д.)
Они довольно безликие и для выбора неудобные. Особенно это относится к выбору серии, где id имеет вид
"1.2.232.20036...", и только где-то в конце последовательности различаются какие-то 6 цифр.
Применительно к серии, есть параметр series_description, который наглядно описывает, что это за серия.
Именно он и выводится во вьюверах.

Предлагаю во всех классах иерархии, помимо функции id_string сделать такую же под названием (например)
description(), которая будет выводить требуемую информацию для соответствующего объекта.
Функцию можно сделать "умной". Например, для series предпочтительным является параметр series_description.
Но если в dicom файле этот параметр пустой или отсутствует, то будет выводиться series_id.

И еще. Сейчас дополнительная информация о составе серии вычисляется в функции select_list и сообщает о
количестве items следующего уровня (без объяснения, что это за item). Можно это заложить в ту же функцию
description(), и сделать ее такой, чтобы, например, для исследований, состоящих из одной серии выводилось
не тривиальное [1 series], а информация о следующем нетривиальном случае. Например, [238 instances]

*/
template <class T, class R>
bool select_list(const T &list, R &it, size_t &root_level, size_t this_root_level, size_t next_root_level)
{
	vector<wstring> elements;
	vector<wstring> elements_with_amount;
	for (auto el : *list)
	{
		elements.push_back(el.id_string());
		//elements_with_amount.push_back(/*wstring(L"\"") + el.id_string() + L"\" (" + /*/el.description()/* + L")"*/);// [" + ssprintf(L"%d", int(el.size())) + L" items]");
		elements_with_amount.push_back(el.description());
	}

	if (elements_with_amount.size() == 1)
	{
		it = list->begin();
		if ((root_level == 0) || (root_level == this_root_level))
			root_level = next_root_level;
	}
	else
	{
		if (root_level == 0)
			root_level = this_root_level;

		elements_with_amount.push_back(L"Exit");
		size_t decision = GetButtonDecision(L"Select " + list->begin()->class_name(), elements_with_amount);
		if (decision == elements_with_amount.size()-1)
			return false;//exit pressed

		auto lambda = [&elements, decision](const decltype(list->front())& t) { return t.id_string() == elements[decision]; };
		it = std::find_if(list->begin(), list->end(), lambda);
	}

	if (it == list->end())
		throw logic_error("User canceled selection of " + convert_to_string(list->begin()->class_name()) );
	else return true;
}

void return_from_series_selection(size_t root_level, size_t current_root_level)
{
	if (root_level == current_root_level)
		throw canceled_operation("Series selection canceled or is impossible");
}

/*!
	Возвращает ссылку на выбранную сборку, с которой в последствии можно работать.
	Возвращает именно такое значение, чтобы пользователь функции в результате сам решал что ему делать с данными: использовать то, что находится в структуре, или делать копию для себя и работать с ней.

	\note в случае отказа бросает исключение
*/
Dicom::acquisition_loader &SelectSeriesInteractive(Dicom::patients_loader &studies_heap) //todo (Kovbas) убрал все const, т.к. теперь планируем работать с acquisition и будет при необходимости через неё менять. Возможно, как-то ещё.
{
	while (true)
	{
		size_t root_level = 0;
		auto it = studies_heap.begin();
		if (select_list(&studies_heap, it, root_level, 1, 2))
		{
			auto it1 = it->begin();
			if (select_list(it, it1, root_level, 2, 3))
			{
				auto it2 = it1->begin();
				if (select_list(it1, it2, root_level, 3, 4))
				{
					auto it3 = it2->begin();
					if (select_list(it2, it3, root_level, 4, 5))
					{
						auto it4 = it3->begin();
						if (select_list(it3, it4, root_level, 5, 6))
						{
							return (*it4);
						}
						else
							return_from_series_selection(root_level, 5);
					}
					else
						return_from_series_selection(root_level, 4);
				}
				else
					return_from_series_selection(root_level, 3);
			}
			else
				return_from_series_selection(root_level, 2);
		}
		else
			return_from_series_selection(root_level, 1);
	}
}


XRAD_END

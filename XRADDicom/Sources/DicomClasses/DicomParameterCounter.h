/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_DicomParameterCounter_h__
#define XRAD__File_DicomParameterCounter_h__
/*!
	\file
	\date 2018/02/28 10:38
	\author nicholas

	\brief  Средство учета того, какие значения заданного параметра встречаются в наборе Dicom данных

	\details Работа реализована через вспомогательный класс DicomParameterCounterCore, который не следует создавать напрямую
	Для анализа величин, имеющих тип parameter_f нужно:

	1. Объявить функцию parameter_t	func(const Dicom::instance &). Также возможно использование лямбды и создание класса-функтора, имеющего перегруженный оператор () с требуемыми параметрами.
	2. Объявить параметр типа map<parameter_t, size_t>, в который будет помещен результат подсчета
	3. Созодать объект-анализатор одной instance, вызвав функцию DicomParameterCounter
	4. Передать этот объект рекурсивному обработчику более высокого уровня.

	Пример использования (учет модальностей, содержащихся в одном исследовании):

	\code
	wstring	modf_global(const Dicom::instance &data) { return	data.modality(); }

	void test(Dicom::study &study)
	{
		map<wstring, size_t>	result;
		// можно использовать глобальную функцию
		Dicom::StudyProcessorRecursive	processor(DicomParameterCounter(result, modf_global));
		// или лямбда-выражение
		Dicom::StudyProcessorRecursive	processor(DicomParameterCounter(result, [](const Dicom::instance &data){ return	data.modality();}));
		processor.Apply(study, VoidProgressProxy());
	}
	\endcode

	После выполнения объект result содержит список всех модальностей, найденных в исследовании, и количество instances для каждой найденной модальности
*/

#include "DicomProcessors.h"
#include <map>

XRAD_BEGIN

//! Вспомогательный класс, отвечающий за сбор данных из одной instance
template<class P, class F>
class DicomParameterCounterCore : public Dicom::InstanceProcessor<Dicom::instance_ptr>
{
public:
	//! тип параметра, чаще всего строка или число
	typedef	P param_type;
	typedef	F function_type;
	typedef	std::map<param_type, size_t>	counter_t;

private:
	//! Ссылка на внешний контейнер, который будет содержать результаты подсчета. Объект создается прежде создания DicomParameterCounterCore и передается ему в качестве первого аргумента конструктора
	counter_t	&counter;

	//! Ссылка на функцию, которая извлекает из Dicom конкретный параметр
	function_type	&&f;

	virtual	void Apply(Dicom::instance_ptr &data, ProgressProxy /*pp*/) override
	{
		param_type	param = f(*data);
		auto	result = counter.insert(make_pair(param, 1));//Пытаемся добавить параметр будто бы в первый раз
		if(result.second==false) ++counter[param];//Добавление не удалось, значит, это значение уже встречалось. Увеличиваем счетчик.
	}

public:
	DicomParameterCounterCore(counter_t &c, function_type &&in_f) : counter(c), f(in_f){}
	DicomParameterCounterCore(DicomParameterCounterCore &c) : counter(c.counter), f(c.f){}
};

//! Основная функция, которую следует вызывать для создания объекта-анализатора
template<class P, class F>
shared_ptr<DicomParameterCounterCore<P,F>> DicomParameterCounter(std::map<P, size_t> &c, F &&f){ return make_shared<DicomParameterCounterCore<P,F>>(c,f); }

typedef	map<wstring, size_t> wstring_counter_t;
typedef	map<ptrdiff_t, size_t> integral_counter_t;

//!	Примеры функций, передаваемых в качестве второго аргумента DicomParameterCounterCore. Список можно расширять здесь или дописывать аналогичные функции в своем коде
inline wstring	modality_f(const Dicom::instance &data) { return	data.modality(); }
inline wstring	protocol_f(const Dicom::instance &data) { return	data.get_wstring(Dicom::e_protocol_name); }
inline size_t	age_f(const Dicom::instance &/*data*/){ return 0;/*пример использования, сортировка выборки по возрасту пациента. сделать, когда будет нормально реализовано преобразование возра*/};
inline double	weight_f(const Dicom::instance &/*data*/){ return 0;/*пример использования, сортировка выборки по весу пациента. в этой функции можно сделать "умное" округление веса чтобы не получать слишком много значений*/ };

XRAD_END

#endif // XRAD__File_DicomParameterCounter_h__

/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_ReferenceOwner_h
#define XRAD__File_ReferenceOwner_h
/*!
	\file
	\date 2014-10-01 17:06
	\author kns

	\brief Реализация ReferenceOwner
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\brief Контейнер, ссылающийся на внешние данные, без копирования данных

	Существует для создания ссылок на данные без копирования (например, на действительную
	часть комплексной функции или на одну из цветовых компонент цветного изображения).

	Объекты должны в точности повторять свойства аргумента шаблона, с той только
	разницей, что копирование объекта (кроме явного MakeCopy etc.) копирует ссылки
	на имеющиеся данные в другом объекте.

	Например, чтобы быстро получать доступ к любому срезу в многомерном массиве, можно
	создавать и хранить в памяти все возможные комбинации этих срезов, как это сделано
	в DataArray2D с rows. Однако с увеличением размерности объем памяти под эти срезы
	сильно возрастает.

	Вместо этого создается набор функций, создающий внутри себя объект ReferenceOwner
	с нужными ссылками и возвращающий его.

	\par Константность данных

	- ReferenceOwner &lt;container&lt;T&gt;&gt; - модифицируемые данные
	- ReferenceOwner &lt;container&lt;const T&gt;&gt; - немодифицируемые данные

	\par Способы копирования данных

	Конструктор копии и оператор присваивания ссылки ссылке передают ссылку на те же данные,
	что и в оригинальном контейнере. Это необходимо для возвращения ссылок из функций.

	Вызовы MakeCopy и CopyData запрещены.
*/
template<class OT>
class ReferenceOwner : public OT
{
	public:
		typedef OT owner_type;
		// базовый набор
	private:
		// здесь объявлены те методы, которые использовать нельзя.
		// эти объявления скрывают запретные методы родительского класса

		using owner_type::realloc;
		using owner_type::resize;
		using owner_type::MakeCopy;
		//using owner_type::ImportData;

	public:
		ReferenceOwner() {}

		//! \brief Только для константных данных
		ReferenceOwner(const ReferenceOwner<OT> &r)
		{
			owner_type::UseData(r);
		}
		ReferenceOwner(ReferenceOwner<OT> &&r)
		{
			owner_type::UseData(r);
		}

		//! \brief Только для константных данных
		ReferenceOwner &operator = (const ReferenceOwner<OT> &r)
		{
			owner_type::UseData(r);
			return *this;
		}
		ReferenceOwner &operator = (ReferenceOwner<OT> &&r)
		{
			owner_type::UseData(r);
			return *this;
		}

		/*
		// Определение этих методов плохо соотносится с методами от const ReferenceOwner &.
		ReferenceOwner(ReferenceOwner<OT> &r)
		{
			owner_type::UseData(r);
		}

		ReferenceOwner &operator = (ReferenceOwner<OT> &r)
		{
			owner_type::UseData(r);
			return *this;
		}
		*/

		ReferenceOwner &ref()
		{
			return *this;
		}
};

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_ReferenceOwner_h

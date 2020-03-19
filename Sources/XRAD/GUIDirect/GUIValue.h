#ifndef GUIValue_h__
#define GUIValue_h__

/*!
	\file
	\date 2018/03/16 13:25
	\author kulberg

	\brief
*/

XRAD_BEGIN

enum class default_value_t
{
	saved,
	fixed
};

constexpr default_value_t saved_default_value = default_value_t::saved;
constexpr default_value_t fixed_default_value = default_value_t::fixed;

template<class T>
struct GUIValue
{
	default_value_t	is_stored;
	//! \brief Значение по умолчанию. Если is_stored == saved_default_value,
	//! используется, в частности, когда сохраненного значения ещё нет
	T	value;
	/*!
		\brief Признак того, что value задано

		Этот признак нужен в тех случаях, когда значение по умолчанию
		отличается от T(). Например, в GetButtonDecision, если кнопка
		по умолчанию (default button) на задана явно, кнопкой по умолчанию
		должна стать первая кнопка, а не та, у которой идентификатор равен 0.
	*/
	bool value_valid;

//	operator T() const { return value; }

	GUIValue &operator = (const default_value_t &s){ is_stored = s; return *this; }
	GUIValue &operator = (const T &v){ value = v; return *this; }

	GUIValue(): is_stored(saved_default_value), value(), value_valid(false) {}
	GUIValue(default_value_t s): is_stored(s), value(), value_valid(false) {}
//	GUIValue(T v): is_stored(default_value_t::fixed), value(v) {}

	//! \brief Конструктор для автоматического преобразования из значений типа T
	template <class T2>
	GUIValue(const T2 &v): is_stored(default_value_t::fixed), value(v), value_valid(true) {}

	//! \brief Конструктор для автоматического преобразования из значений типа
	//! GUIValue<T> с другим типом данных
	template <class T2>
	GUIValue(const GUIValue<T2> &v): is_stored(v.is_stored), value(v.value), value_valid(v.value_valid) {}
};

template <class T>
GUIValue<T> MakeGUIValue(T v, default_value_t iss)
{
	GUIValue<T> result;
	result.value = v;
	result.value_valid = true;
	result.is_stored = iss;
	return result;
}

template <class T>
GUIValue<T> SavedGUIValue(T v)
{
	GUIValue<T> result;
	result.value = v;
	result.value_valid = true;
	result.is_stored = default_value_t::saved;
	return result;
}

//! \brief Конвертировать тип значения с сохранением остальных признаков
template <class T1, class T2>
GUIValue<T1> ConvertGUIValue(const GUIValue<T2> &gui_value, T1 v)
{
	GUIValue<T1> result;
	result.is_stored = gui_value.is_stored;
	result.value = v;
	result.value_valid = gui_value.value_valid;
	return result;
}

XRAD_END

#endif // GUIValue_h__

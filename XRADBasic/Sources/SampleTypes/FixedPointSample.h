#ifndef XRAD__File_fixed_point_sample_h
#define XRAD__File_fixed_point_sample_h

#include <XRADBasic/Core.h>
#include <cstdint>

//TODO: FixedPointSample, собранные здесь материалы нуждаются в существенной доработке

//--------------------------------------------------------------
//
//	здесь описаны средства, обеспечивающие
//	работу алгоритмов RASP в режимах
//	с плавающей и с фиксированной запятой
//
//	существует три типа данных: "внешний отсчет", "внутренний отсчет"
//	и "всегда плавающий"
//
//	особенности импорта входных значений:
//	плавающие только в плавающие.
//	целочисленные либо в целые большей разрядности (со сдвигом),
//	либо в плавающие (без сдвига).
//	!!!величина сдвига зависит от разрядности
//	!!!обеих величин!
//
//	особенности импорта множителей из плавающей во внутренние отсчеты.
//	во-первых, величины, идущие в масштабе значений данных (например,
//	пороговые значения усиления).
//	во-вторых, масштабные множители (например, border_enhance_factor,
//	peak_enhance_factor). их следует преобразовать, чтобы умножение в дальнейшем
//	осуществлялось между однородными величинами
//




//	ранее это была структура с внутренним полем int,
//	которая отвечала за действия с фиксированной запятой.
//	оказалось, что элементарные действия, например конструктор от void
//	в CodeComposer занимают несоразмерно много времени. явно ошибка
//	оптимизации их компилятора! но пока откажемся от класса и будем
//	использовать просто int32

XRAD_BEGIN

//--------------------------------------------------------------
//
//	информация о целочисленных типах, нужная
//	при действиях с фиксированной запятой: явно заданные константы
//


enum
{
	//	ширина поля в битах
	sample_width_int32 = 32,
	sample_width_int16 = 16,
	sample_width_int8 = 8,

	//	положение десятичной точки справа
	//	(ранее было равно width/4 - 1
	//	сейчас все вычисления производятся в контейнере int, поэтому
	//	точность 16-битных данных можно повысить
	point_position_32 = 7,
	point_position_16 = 6,//3
	point_position_8 = 1,

	//	сдвиг при импорте из меньшего целого контейнера
	//  в больший

	offset_8_to_32 = 7,
	offset_16_to_32 = 3,
	offset_32_to_32 = 1,

	offset_8_to_16 = 6,//3,
	offset_16_to_16 = 1,

	offset_8_to_8 = 1,

	//	дозволенные диапазоны значений
	//	для основных целочисленных типов

	//	можно было бы обойтись без задания констант
	//	(вычислять через сдвиг), но тогда пришлось бы
	//	вычислять этот сдвиг на каждом отсчете
	//	обрабатываемых данных
	min_ui8 = 0,
	max_ui8 = 255,
	min_si8 = -128,
	max_si8 = 127,

	min_ui16 = 0,
	max_ui16 = 65535,
	min_si16 = -32768,
	max_si16 = 32767,

	min_ui32 = 0,
	max_ui32 = 0xFFFFFFFFu,
	min_si32 = -0x7FFFFFFF,
	max_si32 = 0x7FFFFFFF
};

//--------------------------------------------------------------
//
//	следующая функция проверяет, чтобы размер целочисленных типов
//	соответствовал требуемым в данном случае. если на каком-либо
//	компиляторе здесь возникнет ошибка, это явится поводом для
//	использования типов с явным заданием разрядности (__int32 например)
//

static_assert(sample_width_int32 == sizeof(int32_t)*CHAR_BIT, "Invalid sample width");
static_assert(sample_width_int16 == sizeof(int16_t)*CHAR_BIT, "Invalid sample width");
static_assert(sample_width_int8 == sizeof(int8_t)*CHAR_BIT, "Invalid sample width");

//--------------------------------------------------------------
//
//	ширина отсчета в битах
//

inline	int	sample_width(int32_t){return sample_width_int32;}
inline	int	sample_width(int16_t){return sample_width_int16;}
inline	int	sample_width(int8_t){return sample_width_int8;}

inline	int	sample_width(uint32_t){return sample_width_int32;}
inline	int	sample_width(uint16_t){return sample_width_int16;}
inline	int	sample_width(uint8_t){return sample_width_int8;}

//--------------------------------------------------------------
//
//	сдвиг при целочисленном умножении
//	или (что то же самое) положение фиксированной
//	точки в числе
//

template<class T> inline int fixed_point_position() = delete;
template<>
inline	int	fixed_point_position<int32_t>(){return point_position_32;}
template<>
inline	int	fixed_point_position<uint32_t>(){return point_position_32;}
template<>
inline	int	fixed_point_position<int16_t>(){return point_position_16;}
template<>
inline	int	fixed_point_position<uint16_t>(){return point_position_16;}

// эти две для совместимости, еще не протестировано, как с ними будет
template<>
inline	int	fixed_point_position<float>(){return 0;}
template<>
inline	int	fixed_point_position<double>(){return 0;}


//--------------------------------------------------------------
//
//	корректный диапазон для целочисленных типов
//

template<class T>
inline	void	proper_range(uint8_t& x, const T& y){x = range(y, min_ui8, max_ui8);}
template<class T>
inline	void	proper_range(int8_t& x, const T& y){x = range(y, min_si8, max_si8);}

template<class T>
inline	void	proper_range(uint16_t& x, const T& y){x = range(y, min_ui16, max_ui16);}
template<class T>
inline	void	proper_range(int16_t& x, const T& y){x = range(y, min_si16, max_si16);}

template<class T>
inline	void	proper_range(uint32_t& x, const T& y){x = range(y, min_ui32, max_ui32);}
template<class T>
inline	void	proper_range(int32_t &x, const T& y){x = range(y, min_si32, max_si32);}

template<class T>
inline	void	proper_range(float& x, const T& y){x = y;}
template<class T>
inline	void	proper_range(double& x, const T& y){x = y;}


//--------------------------------------------------------------
//
//	корректный сдвиг для конкретной пары типов
//	(выписываю только для тех пар, что будут использоваться)
//
//	!!! наличие warning при компиляции этих функций означает, что
//	!!! они вызываются неправильно! нужно проверять вызывающую процедуру
//

template <class T1, class T2>
inline int conversion_offset() = delete;

template<>
inline int	conversion_offset<int32_t, uint8_t>(){return offset_8_to_32;}
template<>
inline int	conversion_offset<int32_t, int8_t>(){return offset_8_to_32;}
template<>
inline int	conversion_offset<uint32_t, uint8_t>(){return offset_8_to_32;}
template<>
inline int	conversion_offset<uint32_t, int8_t>(){return offset_8_to_32;}

template<>
inline int	conversion_offset<int32_t, uint16_t>(){return offset_16_to_32;}
template<>
inline int	conversion_offset<int32_t, int16_t>(){return offset_16_to_32;}
template<>
inline int	conversion_offset<uint32_t, uint16_t>(){return offset_16_to_32;}
template<>
inline int	conversion_offset<uint32_t, int16_t>(){return offset_16_to_32;}

template<>
inline int	conversion_offset<int16_t, uint8_t>(){return offset_8_to_16;}
template<>
inline int	conversion_offset<int16_t, int8_t>(){return offset_8_to_16;}
template<>
inline int	conversion_offset<uint16_t, uint8_t>(){return offset_8_to_16;}
template<>
inline int	conversion_offset<uint16_t, int8_t>(){return offset_8_to_16;}

//--------------------------------------------------------------
//
//	нормировочный множитель, который вычисляется в тех случаях, когда требуется
//	умножить текущий отсчет на число с плавающей запятой. процедура:
//
//	float	factor;
//	DataArray<sample_t>	data;
//	sample_t	intermediate_factor = factor*multiply_normalizer();
//	for(int i = 0; i < data.size(); ++i)
//		{
//		data[i] = multiply(data[i], intermediate_factor);
//		}
//

inline float	multiply_normalizer(float){return 1;}
inline double	multiply_normalizer(double){return 1;}

template<class T>
inline T	multiply_normalizer(const T&){return 1<<fixed_point_position<T>();}


//--------------------------------------------------------------
//
//	алгоритмы умножения с плавающей и фиксированной запятой
//

inline float&	multiply(float &z, const float& x, const float& y){return z = x*y;}
inline double&	multiply(double& z, const double& x, const double& y){return z = x*y;}

template<class T>
inline T	&multiply(T& z, const T& x, const T& y){return z=(x*y)>>fixed_point_position<T>();}



template<class sample_t>
class	shift_multiplier_base
{
	protected:
		int	correction_shift;
		double	factor;
		static double	inv_log2(){return 1./log(2.0);}

		static int	calculate_correction_shift(double x)
		{
			if(!x) return fixed_point_position<sample_t>();
			return range(fixed_point_position<sample_t>() - log(fabs(x))*inv_log2(), 0, sample_width(sample_t(0))-2);
			// поправочный сдвиг при целочисленном умножении
			// не должен превышать (width-2) бит, поскольку, помимо знакового,
			// должен остаться хотя бы один значащий бит
		}
};


template<class sample_t>
struct shift_multiplier : private shift_multiplier_base<sample_t>
{
private:
	using shift_multiplier_base<sample_t>::correction_shift;
	using shift_multiplier_base<sample_t>::factor;
public:
	shift_multiplier()
	{
		correction_shift = fixed_point_position<sample_t>();
		factor = 1./double(1<<correction_shift);
	}
	shift_multiplier(int cs)
	{
		correction_shift = cs;
		factor = 1./double(1<<correction_shift);
	}

	shift_multiplier(int cs, double f)
	{
		correction_shift = cs;
		factor = 1./f;
	}

	inline double	&operator() (double &x, const double &y) const
	{
		return ((x*=y)*=factor);
	}

	inline float	&operator() (float &x, const float &y) const
	{
		return ((x*=y)*=factor);
	}

	inline int32_t &operator() (int32_t &x, const int32_t &y) const
	{
		return (x*=y)>>=correction_shift;
	}

	inline int16_t &operator() (int16_t &x, const int16_t &y) const
	{
		// очень важное изменение (см. ниже). потеря точности была фатальная, разницы в быстродействии никакой
		//return (x*=y)>>=correction_shift;
		return x=(int32_t(x)*int32_t(y))>>correction_shift;
	}

	//	тернарные действия
	inline double	&operator() (double &result, const double &x, const double &y) const
	{
		return ((result=(x*y))*=factor);
	}

	inline float	&operator() (float &result, const float &x, const float &y) const
	{
		return ((result=(x*y))*=factor);
	}

	inline int32_t	&operator() (int32_t &result, const int32_t &x, const int32_t &y) const
	{
		return (result=((int64_t(x)*int64_t(y))>>correction_shift));
	}

	inline int16_t	&operator() (int16_t &result, const int16_t &x, const int16_t &y) const
	{
		return (result=((int32_t(x)*int32_t(y))>>correction_shift));
	}
};



template<class sample_t>
struct shift_divider : private shift_multiplier_base<sample_t>
{
private:
	using shift_multiplier_base<sample_t>::correction_shift;
	using shift_multiplier_base<sample_t>::factor;
public:
	shift_divider()
	{
		correction_shift = fixed_point_position<sample_t>();
		factor = double(1<<correction_shift);
	}
	shift_divider(int cs)
	{
		correction_shift = cs;
		factor = double(1<<correction_shift);
	}

	shift_divider(int cs, double f)
	{
		correction_shift = cs;
		factor = f;
	}

	inline double	&operator() (double &x, const double &y) const
	{
		return (x*=factor)/=y;
	}

	inline float	&operator() (float &x, const float &y) const
	{
		return (x*=factor)/=y;
	}

	inline int32_t &operator() (int32_t &x, const int32_t &y) const
	{
		return (x<<=correction_shift)/=y;
	}

	inline int16_t &operator() (int16_t &x, const int16_t &y) const
	{
		// очень важное изменение (см. ниже). потеря точности была фатальная, разницы в быстродействии никакой
		//return (x*=y)>>=correction_shift;
		return x=((int32_t(x)<<correction_shift)/y);
	}

	//	тернарные действия
	inline double	&operator() (double &result, const double &x, const double &y) const
	{
		return (result = x*factor)/=y;
	}

	inline float	&operator() (float &result, const float &x, const float &y) const
	{
		return (result = x*factor)/=y;
	}

	inline int32_t	&operator() (int32_t &result, const int32_t &x, const int32_t &y) const
	{
		return (result = x<<correction_shift)/=y;
	}

	inline int16_t	&operator() (int16_t &result, const int16_t &x, const int16_t &y) const
	{
		return (result = int32_t(x)<<correction_shift)/=y;
	}
};

//	импорт данных в целочисленный контейнер
//	каждая пара входных/внутренних данных расписывается особо.
//	вариант, который не выписан, считается запрещенным


//--------------------------------------------------------------
//
//	входные и внутренние данные целочисленные
//


template<class T1, class T2>
inline T1&	ImportOriginalSample(T1& result, const T2& x)
{
	return result = T1(x)<<conversion_offset<T1, T2>();
}

template<class T1, class T2>
inline void	ExportProcessedSample(T2& result, const T1& x)
{
	proper_range(result, x>>conversion_offset<T1, T2>());
}


//--------------------------------------------------------------
//
//	внутренние данные плавающие, входные любые
//

template<class T>
inline float&	ImportOriginalSample(float& result, const T& x){return result = x;}
template<class T>
inline void ExportProcessedSample(T& result, const float& x){proper_range(result, x);}

template<class T>
inline double&	ImportOriginalSample(double& result, const T& x){return result = x;}
template<class T>
inline void ExportProcessedSample(T& result, const double& x){proper_range(result, x);}



//--------------------------------------------------------------
//
//	нахождение максимального допустимого значения внутреннего
//	отсчета, которое не приведет к переполнению при экспорте
//	в формат внешнего отсчета. это значение прописывается в
//	объект типа TextureProcessor<> сразу после создания.
//	далее в процессе обработки возможен учет и коррекция
//

template<class T>
inline	T MaxAllowedSampleValue(uint8_t)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<uint8_t>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(uint16_t)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<uint16_t>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(int16_t)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<int16_t>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(int32_t)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<int32_t>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(uint32_t)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<uint32_t>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(float)
{
	T	result;
	return ImportOriginalSample(result, std::numeric_limits<float>::max());
}

template<class T>
inline	T MaxAllowedSampleValue(double)
{
	T	result;
	return ImportOriginalSample<T, double>(result, std::numeric_limits<double>::max());
}



//--------------------------------------------------------------
//
//	копирование данных из исходного пользовательского массива в массив обработки
//  функция копирования данных
//
//--------------------------------------------------------------

template<class T>
T	InterceptCorrection(T intercept)
{
#ifndef XRAD_KT_INTERCEPT
	return intercept;
#else
	return intercept - 1024;
#endif
}




//--------------------------------------------------------------
//
//--------------------------------------------------------------

template<class PROC_BUF, class IN_BUF>
void	ImportOriginalImage(PROC_BUF &a1, const IN_BUF &a2, typename IN_BUF::value_type intercept, float slope)
{
//	(с учетом выравнивания) массивы могут быть различных размеров.

	Apply_AA_2D_Different_F2(a1, a2,
			[](typename PROC_BUF::value_type &x, const typename IN_BUF::value_type &y)
			{
				ImportOriginalSample(x,y);
			},
			extrapolation::by_last_value);

	typename IN_BUF::value_type	applied_intercept = InterceptCorrection(intercept);

	if(applied_intercept)
	{
		// величина intercept прибавляется к данным до импорта,
		// и вычитается из данных после экспорта. поэтому она должна
		// быть в одно с ними масштабе. значит, на импорте вычитаем ее
		// после преобразования основных значений, а на экспорте напротив,
		// сначала прибавляем интерсепт, а потом масштабируем значения
		typename PROC_BUF::value_type	imported_intercept;
		ImportOriginalSample(imported_intercept, applied_intercept);
		a1 -= imported_intercept;
	}
	if(slope != 1 && slope != 0) a1 *= slope;
}


template<class IN_BUF, class PROC_BUF>
void	ExportProcessedImage(IN_BUF &a1, PROC_BUF &a2, typename IN_BUF::value_type intercept, float slope)
{
//	здесь экспортируемый аргумент не const, он разрушается в процессе импорта
//	if(a1.vsize() != a2.vsize() || a1.hsize() != a2.hsize()) return;
//	теперь (с учетом выравнивания) массивы могут быть различных размеров.

	if(slope != 1 && slope != 0) a2 *= (1./slope);

	typename IN_BUF::value_type	applied_intercept = InterceptCorrection(intercept);
	if(applied_intercept)
	{
		typename PROC_BUF::value_type	imported_intercept;
		ImportOriginalSample(imported_intercept, applied_intercept);
		a2 += imported_intercept;
	}

	Apply_AA_2D_Different_F2(a1, a2,
			[](typename IN_BUF::value_type &x, const typename PROC_BUF::value_type &y)
			{
				ExportProcessedSample(x,y);
			},
			extrapolation::by_last_value);
}

XRAD_END

#endif //XRAD__File_fixed_point_sample_h

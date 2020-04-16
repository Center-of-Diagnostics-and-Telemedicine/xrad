//	file ImageUtils.hh
#ifndef __image_utils_cc
#define __image_utils_cc

#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/Containers/DataArray2D.h>

XRAD_BEGIN
//--------------------------------------------------------------
//
//	AddNoise()
//	прибавляет к каждому пикселю изображения img равномерно распределенный шум
//	с амплитудой AM.
//
//
template<class ROW_T, class AM>
void	AddNoise(DataArray2D<ROW_T> &img, const AM &amplitude)
	{
	const size_t n_data_components = n_components((typename ROW_T::value_type)0);
	const size_t n_amplitude_components = n_components(amplitude);

	// если на последующей строке возникла ошибка компилятора,
	// значит, заданы заведомо несовместимые варианты
	// (например, цветной или комплексный шум по действительному сигналу)
	const typename ROW_T::value_type	adduced_amplitude(amplitude);
	// приводим амплитуду к типу элемента изображения.
	// для таких случаев, если, например, цветной шум по картине RGB задан в формате Lab.


	if(n_data_components == n_amplitude_components)
		{
		// каждая компонента изображения имеет свой шум
		// со своей отдельно заданной амплитудой
		// то есть, например, для цветных изображений шум получается цветным,
		// амплитуда его по каналам определяется величиной amplitude

		for(int k = 0; k < n_data_components; ++k)
			{
			double	noise_amplitude = norma(component(adduced_amplitude, k));
			for(int i = 0; i < img.vsize(); ++i)
				{
				typename ROW_T::iterator	it = img.row(i).begin(), ie = img.row(i).end();
				for(; it<ie; ++it)
					{
					double	noise = RandomUniformF64(-noise_amplitude/2, noise_amplitude/2);
					component(*it, k) += noise;
					}
				}
			}
		}
	else if(n_amplitude_components == 1)
		{
		// ко всем компонентам прибавляется одинаковое шумовое число
		// на цветных изображениях шум получается нейтральный (серый)
		double	noise_amplitude = norma(adduced_amplitude);
		for(int i = 0; i < img.vsize(); ++i)
			{
			typename ROW_T::iterator	it = img.row(i).begin(), ie = img.row(i).end();
			for(; it<ie; ++it)
				{
				double	noise = RandomUniformF64(-noise_amplitude/2, noise_amplitude/2);
				*it += noise;
				}
			}
		}
	else
		{
		// если формат шума несовместим с форматом исходных данных,
		// но компилятор почему-то пропустил проверку на этапе задания adduced_amplitude
		throw invalid_argument("AddNoise(DataArray2D<ROW_T> &img, const AM &amplitude), invalid noise format");
		}
	}

//--------------------------------------------------------------
//
//	логарифмическое преобразование изображения
//


template<class ROW_T>
void	LogCompress(DataArray2D<ROW_T> &img)
	{
	//	логарифмирование безо всякой обработки, выход в децибелах
	const size_t n_data_components = n_components((typename ROW_T::value_type)0);
	double	maxval = MaxComponentValue(img);
	if(maxval <= 0)
		{
		img.fill(typename ROW_T::value_type(0));
		return;
		}

	// поиск минимально положительного значения
	double	min_positive_value = maxval; // на начало алгоритма равна максимальному значению
	for(size_t i = 0; i < img.vsize(); ++i)
		{
		for(size_t j = 0; j < img.hsize(); ++j)
			{
			for(size_t k = 0; k < n_data_components; ++k)
				{
				double value = component(img.at(i,j), k);
				if(value > 0) min_positive_value = min(value, min_positive_value);
				}
			}
		}

	double	min_db = 20.*log10(min_positive_value);
	// значения <= 0 превращаются в логарифм минимального положительного значения:

	for(size_t i = 0; i < img.vsize(); ++i)
		{
		for(size_t j = 0; j < img.hsize(); ++j)
			{
			for(size_t k = 0; k < n_data_components; ++k)
				{
				double value = component(img.at(i,j), k);
				component(img.at(i,j), k) = (value <= 0) ? min_db : 20.*log10(value);
				}
			}
		}
	}


template<class ROW_T>
void	LogCompressRangeHistogram(DataArray2D<ROW_T> &img, double cut_shadows, double cut_lights)
	{
	//	логарифмирование с оптимизацией динамического диапазона по квантилям.
	//	после логарифмирования находит N*cut_xx максимальных и минимальных значений
	//	и приравнивает их минимальному (максимальному) из них. изображение становится
	//	более контрастным
	//	по умолчанию оба cut_xx = 0.001

	LogCompress(img);
	CutComponentHistogramEdges(img, range1_F64(cut_shadows, cut_lights));
	typedef typename ROW_T::value_type value_type;

	Apply_AS_2D_F2(img, value_type(MinComponentValue(img)), Functors::minus_assign());
	}



//--------------------------------------------------------------
//
//	логарифмирование с прямым заданием динамического диапазона
//	в силу сложившихся традиций это делается не совсем обычно
//	логарифмированные данные считаются отрицательными
//	величины dyn_range_shadows > dyn_range_lights устанавливается положительными,
//	свое истинное значение они обретают только после того, как в теле функции их
//	берут с обратным знаком (см. последний цикл). это все очень нелогично и должно
//	быть переделано. ставлю в план. кнс

template<class ROW_T>
void	LogCompressRangeDB(DataArray2D<ROW_T> &img, double dyn_range_shadows, double dyn_range_lights)
	{
	const size_t n_data_components = n_components((typename ROW_T::value_type)0);

	if(!dyn_range_shadows){ //auto range
		LogCompressRangeHistogram(img, 0.001, 0.001);
		return;
		}

	if(dyn_range_shadows <= dyn_range_lights ||
		dyn_range_shadows < 0 ||
		dyn_range_lights < 0)
		{
		img.fill(0);
		return;
		}

	LogCompress(img);
	double	maxvalue = MaxComponentValue(img);
	for(size_t i = 0; i < img.vsize(); ++i)
		{
		for(size_t j = 0; j < img.hsize(); ++j)
			{
			for(size_t k = 0; k < n_data_components; ++k)
				{
				double	value = component(img.at(i,j), k) - maxvalue;
				if(value > -dyn_range_lights) value = -dyn_range_lights;
				else if(value < -dyn_range_shadows) value = -dyn_range_shadows;

				component(img.at(i,j), k) = value + dyn_range_shadows;
				}
			}
		}
	}



//--------------------------------------------------------------
//
//	нахождение значений изображения по квантилям гистограммы
//


template<class ROW_T>
double	ComputeQuantile(const ROW_T &pdf, double probability);
/*
	TODO: Разобраться с нормировкой.
	{
	// возвращает значение порога для квантиля p. результат безразмерный, значения от 0 до 1,
	if(!in_range(probability, 0, 1))
		{
		throw invalid_argument(ssprintf("FindQuantile(GrayImage, double, const ROW_T &) -- index out of range: %g<>(%d,%d)", probability, 0, 1));
		}

	ROW_T cdf(pdf);
	typename ROW_T::iterator it = cdf.begin(), ie = cdf.end(), it1 = it+1;
	for(; it1 < ie; ++it1, ++it) *it1 += *it;

	size_t i = 0;
	while(i < pdf.size() && cdf[i] < probability) {++i;}

	return double(i)/s;
	}*/


template<class ROW_T>
range1_F64	ComputeQuantilesRange(const ROW_T &pdf, const range1_F64 &data_range, const range1_F64 &probability_range)
	{
	// находит значения порогов для двух квантилей. результат безразмерный, значения от 0 до 1,
	// 0 соответствует минимальному значению данных, 1 максимальному
	if(probability_range.p1() > probability_range.p2() || probability_range.p1() < 0 || probability_range.p2() > 1)
		{
		// если диапазон задан некорректно
		ForceDebugBreak();
		throw invalid_argument("ComputeDataQuantiles, invalid data ranges");
		}

	// от функции плотности переходим к функции распределения
	ROW_T cdf(pdf);

	cdf[0] = cdf[cdf.size()-1] = 0;
	// исключаем из рассмотрения крайние значения. они часто приходятся на фоновую засветку

	typename ROW_T::iterator it = cdf.begin(), it1 = it + 1;
	const typename ROW_T::iterator ie = cdf.end();

	for(; it1 < ie; ++it1, ++it) *it1 += *it;

	// находим крайние значения диапазона:
	range1_F64	result;
	if(cdf[cdf.size()-1])
		{
		// для обычной гистограммы
		cdf /= cdf[cdf.size()-1];

		double	d_range = data_range.delta()/pdf.size();
		double	bound = data_range.p1();
		result.x1() = result.x2() = bound;
			//когда result не был проинициализирован, в редких случаях x1 так и оставался без изменений
		it = cdf.begin();//, ie = cdf.end();

		for(; it<ie; ++it, bound+=d_range)
			{
			if(*it < probability_range.p1()) result.x1() = bound;
			if(*it < probability_range.p2()) result.x2() = bound;
			}
		}
	else
		{
		// для вырожденной гистограммы (такое может произойти, если
		// исходное изображение содержит nan)
		result = probability_range;
		result *= data_range.delta();
		result += data_range.p1();
		}
	return result;
	}

//--------------------------------------------------------------
//
//	линейная коррекция контрастности по гистограмме:
//	удаление редко встречающихся всплесков/провалов яркости
//
template<class ROW_T>
void	CutHistogramEdges(DataArray2D<ROW_T> &image, const range1_F64 &probability_range)
	{
	double	maxval = MaxValue(image);
	double	minval = MinValue(image);

	if(maxval==minval)
		{
		// изображение заполнено константой, нечего менять
		return;
		}
	// размер гистограммы для этой процедуры много больше, чем тот, какой используется
	// для визуального анализа. при таком значении точность задания диапазона вероятностей
	// составляет 0,01%
	size_t histogram_size = 10000;
	RealFunctionF64	histogram(histogram_size);
	ComputeHistogram(image, histogram, range1_F64(minval, maxval));

	// посчитаны квантили
	range1_F64	tresholds = ComputeQuantilesRange(histogram, range1_F64(minval, maxval), probability_range);

	ApplyFunction(image, [&tresholds](double v) { return range(v, tresholds.p1(), tresholds.p2()); });
	}

// покомпонентно
template<class ROW_T>
void	CutComponentHistogramEdges(DataArray2D<ROW_T> &image, const range1_F64 &probability_range)
	{
	const size_t n_data_components = n_components((typename ROW_T::value_type)0);
	double	maxval = MaxComponentValue(image);
	double	minval = MinComponentValue(image);

	// изображение заполнено константой, всплесков и провалов нет
	if(maxval==minval) return;

	// размер гистограммы для этой процедуры много больше, чем тот, какой используется
	// для визуального анализа. при таком значении точность задания диапазона вероятностей
	// составляет 0,01%
	size_t histogram_size = 10000;
	RealFunction2D_F64	component_histogram(n_data_components, histogram_size);
	RealFunctionF64	histogram(histogram_size);

	ComputeComponentsHistogram(image, component_histogram, range1_F64(minval, maxval));
	for(size_t i = 0; i < histogram_size; ++i)
		{
		histogram[i] = AverageValue(component_histogram.col(i));
		}

	range1_F64	tresholds = ComputeQuantilesRange(histogram, range1_F64(minval, maxval), probability_range);

	for(size_t i = 0; i < image.vsize(); i ++)
		{
		typename ROW_T::iterator	it = image.row(i).begin(), ie = image.row(i).end();
		for(; it<ie; ++it)
			{
			for(size_t k = 0; k < n_data_components; ++k)
				{
				//double c = component(img.at(i,j), k);
				//c = max(c,low_edge);
				component(*it, k) = range(component(*it, k), tresholds.p1(), tresholds.p2());
				}
			}
		}
	}

//--------------------------------------------------------------
//
//	нормализация яркости изображения:
//	принудительно устанавливаем значения черного и белого.
//	значения black_point и white_point могут быть скалярными (для любых изображений)
//	или векторными(только для изображений с векторным пикселом)
//	(иначе ошибка компилятора)
//

template<class ROW_T, class AM>
void	NormalizeImage(DataArray2D<ROW_T> &img, const AM &black_point, const AM &white_point)
	{
	const size_t	n_data_components = n_components((typename ROW_T::value_type)0);
	const size_t	n_ranges_components = n_components(black_point);
	// если на последующей строке возникла ошибка компилятора,
	// значит, заданы заведомо несовместимые варианты
	// (например, цветной или комплексный диапазон по GrayImage)
	typedef typename ROW_T::value_type value_type;

	const value_type	adduced_black = value_type(black_point);
	const value_type	adduced_white = value_type(white_point);
	// приводим амплитуду к типу элемента изображения.

	double	m = MinComponentValue(img);
	double	M = MaxComponentValue(img);
	if(m==M) ++M, --m;

	if(n_data_components == n_ranges_components && n_data_components>1)
		{
		// каждая компонента изображения имеет диапазон
		// со своми отдельно заданными границами
		for(size_t k = 0; k < n_data_components; ++k)
			{
			double	factor = (component(adduced_white, k) - component(adduced_black, k))/(M-m);
			double	offset = component(adduced_black, k) - m*factor;

			for(size_t i = 0; i < img.vsize(); ++i)
				{
				for(size_t j = 0; j < img.hsize(); ++j)
					{
					component(img.at(i,j), k) *= factor;
					component(img.at(i,j), k) += offset;
					}
				}
			}
		}
	else if(n_ranges_components == 1)
		{
		// ко всем компонентам прибавляется одинаковая нормализация,
		// черная/белая точки имеют нейтральный тон
		double	factor = (component(adduced_white, 0) - component(adduced_black, 0))/(M-m);
		double	shift = component(adduced_black, 0) - m*factor;

		Apply_AS_2D_F2(img, factor, Functors::multiply_assign());
		Apply_AS_2D_F2(img, shift, Functors::plus_assign());
		}
	}

//--------------------------------------------------------------
//
//	коррекция белого/черного. если в качестве границ диапазонов даны
//	скалярные величины, то линейное изменение контрастности
//

template<class ROW_T, class AM>
void	TruncateImageValues(DataArray2D<ROW_T> &img, const AM &black_point, const AM &white_point)
	{
	const size_t n_data_components = n_components(typename ROW_T::value_type());
//	const size_t n_ranges_components = n_components(black_point);
	// если на последующей строке возникла ошибка компилятора,
	// значит, заданы заведомо несовместимые варианты
	// (например, цветной или комплексный диапазон по GrayImage)
	const typename ROW_T::value_type	adduced_black(black_point);
	const typename ROW_T::value_type	adduced_white(white_point);

	for(size_t i = 0; i < img.vsize(); ++i)
		{
		for(size_t j = 0; j < img.hsize(); ++j)
			{
			for(size_t k = 0; k < n_data_components; ++k)
				{
				component(img.at(i,j), k) = range(component(img.at(i,j), k), component(adduced_black, k), component(adduced_white, k));
				}
			}
		}
	}

//--------------------------------------------------------------

XRAD_END

#endif	// __image_utils_cc

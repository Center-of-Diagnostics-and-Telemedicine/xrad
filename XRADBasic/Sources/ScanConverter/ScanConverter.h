#ifndef	XRAD__File_scan_converter_h
#define	XRAD__File_scan_converter_h

#include "TableInterpolator.h"
#include "ScanConverterOptions.h"
#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

struct	ray_sample_coord
	{
	double ray_f;
	double sample_f;

	ray_sample_coord(double r, double s) : ray_f(r), sample_f(s){}
	};

struct	row_col_coord
	{
	double row_f;
	double col_f;

	row_col_coord(double r, double c) : row_f(r), col_f(c){}
	};

/*!
	\brief Класс для преобразования системы координат изображения

	Конвертер по умолчанию работает в предположении,
	что координата h ("горизонтальная") входных данных при отображении на экране
	должна расти по вертикали сверху вниз.
	Т.е. растры в прямоугольной системе координат транспонируются,
	растры в полярной системе координат имеют центр сектора вверху по центру.

	Это обусловлено тем, что конвертер изначально разрабатывался для отображения
	ультразвуковых данных, в представлении которых в памяти самой быстрой координатой
	являлось время прихода сигнала, которое при отображении на экране традиционно откладывалось сверху вниз.

	По умолчанию предполагается, что в преобразованном растре [?количество разрядов данных?] имеет меньшую важность,
	чем занимаемое место. Поэтому [для сконвертированного изображения] выбирается аналогичный тип с наименьшей допустимой точностью.

	Исторически второй параметр шаблона имел вид: `class CS_T = typename ReducedWidth<typename IM_T::value_type>::type`.
	В данный момент этот параметр шаблона не используется, для сконвертированного растра всегда берется
	тип из ReducedWidth.

	\todo Второй параметр шаблона не используется.

	2014_10_01 Удалены из класса все зависимости от интерфейсных функции (методы DisplayRaster и его "помощники").
	Класс предназначен только для создания преобразованного растра. Отображение делается извне.
*/
template <class IM_T, class = void>
class	ScanConverter : public IM_T, public ScanConverterOptions
	{
	PARENT(IM_T);
public:
	using parent::hsize;
private:

	// для целочисленных типов предпочительно целое умножение со сдвигом.
	// для плавающих умножение на double.
	template<class T, class V = void>
		struct	FactorTypeSelector{typedef double factor_type;};
	template<class V>
		struct	FactorTypeSelector<int32_t, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<uint32_t, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<uint8_t, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<uint16_t, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<int16_t, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<ColorPixel, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<complexI32, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<complexI16, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<complexI32F, V>{typedef int factor_type;};
	template<class V>
		struct	FactorTypeSelector<complexI16F, V>{typedef int factor_type;};


	public:
		typedef	IM_T original_image_type;
		typedef typename original_image_type::value_type original_sample_type;

#if 0
		// 2021_01_14 КНС. Обнаружил, что из-за следующей строчки некорректно отображались данные ЦДК от Сономед-500.
		// Происходило сжатие complexI32 до complexI16, происходила потеря значащих разрядов.
		// Удаление его может привести к большой нагрузке на память для многомерных данных
		// Возможно, стоит перенести действия по сжатию данных в саму процедуру показа.
		typedef typename ReducedWidth<typename original_image_type::value_type>::type converted_sample_type;
#else
		typedef original_sample_type converted_sample_type;
#endif
		typedef typename FactorTypeSelector<converted_sample_type>::factor_type factor_type;
		typedef DataArray2D<DataArray<converted_sample_type> > converted_image_type;

	private:

	typedef	TableInterpolator<original_image_type, converted_sample_type, factor_type> interpolator_t;
	typedef	ScanConverter<original_image_type> self;

	converted_sample_type background_color;
	converted_sample_type grid_color;

	size_t	n_rows, n_cols;

	converted_image_type converted_image;
	DataArray2D<DataArray<interpolator_t> >	interpolator;

// 	string	image_title;
	bool	inited;
	bool	flip;

	// вспомогательные константы (небольшая экономия вычислений)
	ptrdiff_t	first_row;
	ptrdiff_t	first_col, last_col, middle_col;
	ptrdiff_t	first_sample, last_sample;
	double	start_angle_ctg, end_angle_ctg;


public:
	void	realloc(size_t vSize, size_t hSize){inherited::realloc(vSize, hSize); inited = false;}
	void	resize(size_t vSize, size_t hSize){inherited::resize(vSize, hSize);inited=false;}

	void	transpose(){parent::transpose(); if(inited) InitScanConverter(n_rows, n_cols);}

	void	UseData(original_sample_type *new_data, size_t v, size_t h){inherited::UseData(new_data,v,h);inited=false;}
	void	UseData(original_sample_type *new_data, size_t v, size_t h, ptrdiff_t st_v, ptrdiff_t st_h){inherited::UseData(new_data,v,h,st_v,st_h);inited=false;}
	void	UseData(original_image_type &new_data){inherited::UseData(new_data);inited=false;}
	void	MakeCopy(const original_image_type &original){inherited::MakeCopy(original);inited=false;}

public:

	size_t	n_rays() const {return parent::vsize();}
	size_t	n_samples() const {return parent::hsize();}

	ScanConverter(size_t x = 0, size_t y = 0);
	virtual ~ScanConverter();


// 	void	SetImageTitle(const string &new_title);

	void	DrawPalette();
	void	DrawGrid();
	void	BuildConvertedImage();

// 	void	DisplayRaster(const char *title = NULL);

	void	InitScanConverter(size_t nr = 0, size_t nc = 0);
	void	SetFlip(bool);
	void	SetGrid(bool gr, converted_sample_type c = 0, physical_length step = cm(2)){draw_grid = gr; grid_color = c;grid_step = step;};
	void	SetBackground(converted_sample_type c);

	// в нижеобъявленных функциях так и должно быть: n_rays()-1, n_samples()-1.
	// иначе появляется лишняя строка развертки, заполненная фоновым цветом
	physical_angle	d_angle() const {return angle_range()/(n_rays()-1);}
	physical_length	dx() const {return	scanning_trajectory_length()/(n_rays()-1);}
	physical_length dr() const {return	(r_max()-r_min())/(n_samples()-1);}

	size_t	get_n_rows() const {return n_rows;}
	size_t	get_n_cols() const {return n_cols;};

	ray_sample_coord GetRaySampleCoords(double row_f, double col_f) const;
	ray_sample_coord GetRaySampleCoords(row_col_coord rc) const;

	row_col_coord	GetRowColCoords(double ray_f, double sample_f) const;
	row_col_coord	GetRowColCoords(ray_sample_coord rs) const;

	void	GetRasterDimensions(physical_length &vmin, physical_length &vmax, physical_length &hmin, physical_length &hmax) const;

	const	converted_image_type &GetConvertedImage(){return converted_image;}
	};



typedef ScanConverter<RealFunction2D_F32> GrayScanConverter;
typedef ScanConverter<ColorImageF32> ColorScanConverter;

typedef ScanConverter<DataArray2D<DataArray<unsigned char> > > GrayPixelScanConverter;
typedef ScanConverter<DataArray2D<DataArray<ColorPixel> > > ColorPixelScanConverter;


//--------------------------------------------------------------
//
//	копирование данных в серошкальный пиксельный массив.
//	у макоси (ppc) однобайтный серый перевернут: 255 черное, 0 белое
//
//--------------------------------------------------------------

/*
__APPLE__
	This macro is defined in any Apple computer.
__APPLE_CC__
	This macro is set to an integer that represents the version number of
	the compiler. This lets you distinguish, for example, between compilers
	based on the same version of GCC, but with different bug fixes or features.
	Larger values denote later compilers.
__OSX__
	Presumabley the OS is a particular variant of OS X
*/
#ifdef __APPLE__

	struct assign_gray_pixel
		{
		template<class T2>
		void operator()(unsigned char &x, const T2 &y) const { x=~(unsigned char)y; }
		};
#else

	struct assign_gray_pixel
		{
		template<class T2>
		void operator()(unsigned char &x, const T2 &y) const { x=(unsigned char)y; }
		};

#endif//__APPLE__



template <class F2> void	MakeCopy(GrayPixelScanConverter &destination, const DataArray2D<F2> &original)
	{
	destination.realloc(original.vsize(), original.hsize());
	Apply_AA_2D_F2(destination, original, assign_gray_pixel());
	}

template <class F2> void	CopyData(GrayPixelScanConverter &destination, const DataArray2D<F2> &original)
	{
	if( (void*)(&original) == (const void*)(&destination)) return;
	if(!destination.vsize() || !destination.hsize()) return;

	Apply_AA_2D_Different_F2(destination, original, assign_gray_pixel());
	}



XRAD_END

#include "ScanConverter.hh"

#endif //XRAD__File_scan_converter_h

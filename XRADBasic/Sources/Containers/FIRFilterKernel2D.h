#ifndef __short_filter_h
#define __short_filter_h
//--------------------------------------------------------------

#include "FIRFilterKernel.h"
#include "DataArray2D.h"
#include "BooleanFunction2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

enum FIRFilter2DType
{
	cross3,
	cross5,
	cross7,
	cross9,

	square3,
	square5,
	square7,
	square9,

	diamond3,
	diamond5,
	diamond7,
	diamond9
};


//--------------------------------------------------------------

//! \brief Логическая маска-окно для двумерного фильтра порядковых статистик
template<class BOOLEAN>
class	FIRFilterKernel2DMask :
	private BooleanFunctionLogical2D<BOOLEAN>,
	public FIRFilterKernelBasic
{
		typedef	FIRFilterKernel2DMask self;
		PARENT(BooleanFunctionLogical2D<BOOLEAN>);
		typedef typename parent::value_type value_type;

		using parent::ready;
		using row_type = typename parent::row_type;

		size_t	n_order_statistics;
		size_t	fractile_no;

		/*!
			\brief Буфер для сортировки данных

			Поскольку тип элемента фильтруемого массива сразу неизвестен,
			объявляется как массив char, который аллокируется размером n_elements*sizeof(element),
			после чего указатель на нулевой элемент преобразуется к требуемому типу.
		*/
		DataArray<char>	*order_buffer;

	private:

		// Унаследованные свойства (по требованию GCC).
		// Все они перечисляются в приватной области,
		// чтобы нельзя было их просто так вызывать.
		using parent::fill;
		using parent::realloc;
		using parent::at;
		using parent::vsize;
		using parent::hsize;
		using parent::row;
		using parent::col;
		using FIRFilterKernelBasic::m_extrapolation_method;

		void CountOrderStatistics();

		void	InitBuffer(double fp)
		{
			CountOrderStatistics();
			SetFractile(fp);
			// может вызываться неоднократно, поэтому буфер
			// каждый раз удаляем и создаем заново
			DestroyObject(order_buffer);
			order_buffer = new DataArray<char>;
		}

	public:
		FIRFilterKernel2DMask() :order_buffer(NULL){};
		FIRFilterKernel2DMask(const parent &mask, double fp) :parent(mask), order_buffer(NULL){ InitBuffer(fp); }
		FIRFilterKernel2DMask(FIRFilter2DType ft, double fp) :order_buffer(NULL){ InitFilter(ft, fp); }
		~FIRFilterKernel2DMask(){ DestroyObject(order_buffer); };

		// В текущей реализации операции копирования (и перемещения) по умолчанию будут работать неверно.
		// При переделке нужно order_buffer переделать из указателя на просто DataArray<char>.
		// Тогда деструктор станет ненужным, операции по умолчанию должны стать корректными
		// (проверить, добавить тест в TestFunctions).
		FIRFilterKernel2DMask(const FIRFilterKernel2DMask &) = delete;
		FIRFilterKernel2DMask &operator=(const FIRFilterKernel2DMask &) = delete;

		//! \brief Получение const маски
		//!
		//! Разрешается просматривать маску const, изменение только через методы инициализации.
		const parent& GetMask(){ return static_cast<const parent&>(*this); }

		void	InitFilter(FIRFilter2DType ft, double fp);
		void	InitFilter(const parent &mask, double fp)
		{
			MakeCopy(mask);
			InitBuffer(fp);
		}

		/*!
			\brief Рассчитать фильтрованное значение на массиве data в координатах (v, h)

			При выходе за границы массива функция использует экстраполированные значения
			в соответствии с политикой extrapolation::by_last_value.

			\note
			Эта операция не const, модифицирует order_buffer. Для const и безопасной многопоточной работы
			нужно передавать буфер одним из параметров.
		*/
		template<class	A2D2>
		typename A2D2::value_type Apply(const A2D2 &data, ptrdiff_t v, ptrdiff_t h);

		size_t	GetNOrderStatistics() const { return n_order_statistics; };
		size_t	GetOrderStatisticNo() const { return fractile_no; };
		void SetOrderStatisticNo(size_t n){ fractile_no = range(n, 0, n_order_statistics-1); }

		void SetFractile(double p);
};



//--------------------------------------------------------------



/*!
	\brief Двумерный линейный ких-фильтр

	Важное условие: FIRFilter ничего не знает об объектах MathFunction,
	он наследуется от Algebra2D. Это наследование обеспечивает
	возможность алгебраических манипуляций над фильтрами.
	Менее громоздкое наследование от MathFunction недопустимо, т.к. в таком виде
	возникает опасность круговой зависимости, разрешать которую приходится искусственными
	приемами.
*/
template<class FF1D>
class	FIRFilterKernel2DConvolve :
	public Algebra2D<FIRFilterKernel2DConvolve<FF1D>, FF1D, typename FF1D::value_type, typename FF1D::scalar_type, typename FF1D::field_tag>,
	public FIRFilterKernelBasic
{
		PARENT(Algebra2D<FIRFilterKernel2DConvolve, FF1D, typename FF1D::value_type, typename FF1D::scalar_type, typename FF1D::field_tag>);

	public:
		typedef typename FF1D::value_type	value_type;
		typedef typename FF1D::scalar_type	scalar_type;
		typedef	FIRFilterKernel2DConvolve<FF1D> self;

	private:
		//! \brief Вспомогательная структура-функтор, используется для импорта
		//! логической маски при инициализации
		template<class B>
		struct auxiliary_import_boolean
			{
			value_type &operator()(value_type &x, const BooleanLogical<B> y) const { return x=y.is_true(); };
			};

	public:

		// Унаследованные свойства (по требованию GCC)
		using parent::fill;
		using parent::realloc;
		using parent::at;
		using parent::vsize;
		using parent::hsize;
		using parent::row;
		using parent::col;
		using FIRFilterKernelBasic::m_extrapolation_method;

		FIRFilterKernel2DConvolve(){};
		FIRFilterKernel2DConvolve(size_t size) :parent(size, size){};
		FIRFilterKernel2DConvolve(size_t v, size_t h) :parent(v, h){};


		void	IFilterGaussian(double dispersionInSamples, double gaussFilterEdge = 0.3);
		void	IFilterUnsharp(double dispersionInSamples, double strength);

		void	InitFilter(FIRFilter2DType f);

		//! \note Для фильтра нулевого размера возвращает 0. Возможно, следует кидать исключение.
		template<class	A2D2>
		floating64_type<typename A2D2::value_type> Apply(const A2D2 &, ptrdiff_t v, ptrdiff_t h) const;
};

//--------------------------------------------------------------

XRAD_END

#include "FIRFilterKernel2D.hh"

//--------------------------------------------------------------
#endif //__short_filter_h

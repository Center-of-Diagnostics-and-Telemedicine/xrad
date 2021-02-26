//	file MathFunction.h
#ifndef XRAD__File_mathfunction_h
#define XRAD__File_mathfunction_h

#include "DataArray.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>

XRAD_BEGIN

//--------------------------------------------------------------

#define XRAD__MathFunction_template class T, class ST, class FIELD_TAG
#define XRAD__MathFunction_template_args T, ST, FIELD_TAG
#define XRAD__MathFunction_template1 class T1, class ST1, class FIELD_TAG1
#define XRAD__MathFunction_template_args1 T1, ST1, FIELD_TAG1

template<XRAD__MathFunction_template>
class	MathFunction : public Algebra1D<MathFunction<XRAD__MathFunction_template_args>, T, ST, FIELD_TAG>
{
		PARENT(Algebra1D<MathFunction, T, ST, FIELD_TAG>);

	public:
		typedef typename parent::value_type value_type;
		typedef	typename parent::value_type_variable value_type_variable;
		typedef	typename parent::value_type_invariable value_type_invariable;
		typedef ST scalar_type;

		typedef MathFunction<value_type, ST, FIELD_TAG> self;
		typedef MathFunction<value_type_invariable, ST, FIELD_TAG> invariable;
		typedef MathFunction<value_type_variable, ST, FIELD_TAG> variable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

		//! \brief Тип, благодаря которому родительская алгебра узнает наследника без динамической типизации,
		//! что было бы вредно для производительности
		typedef MathFunction<XRAD__MathFunction_template_args> child_type;

		child_type &child_ref(){ return (*this); }
		const child_type &child_ref() const { return (*this); }



		//! \name Конструкторы. См. \ref pg_CopyContructorOperatorEq, \ref pg_MoveOperations
		//! @{

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		MathFunction() = default;
		MathFunction(const parent &p): parent(p) {}
		MathFunction(parent &&p): parent(std::move(p)) {}
		// Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип:
		// using parent::operator=;

		//! \brief См. \ref pg_CopyContructorOperatorEq
		//!
		//! Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип.
		template<class T2> child_type	&operator = (const DataArray<T2> &original){ parent::operator=(original); return child_ref(); }

		template<class T2> child_type	&operator = (DataArray<T2> &&original){ parent::operator=(std::move(original)); return child_ref(); }
		//! @}

		//! \brief Интерполяция
		template<class INTERPOLATOR_T>
		auto	in(double x, const INTERPOLATOR_T *interpolator) const
		{
			// Из-за сложности определения типа возвращаемого значения реализация метода расположена в теле класса,
			// чтобы можно было воспользоваться автоматическим выведением типа возвращаемого значения.
			interpolator->ApplyOffsetCorrection(x);
			const	typename INTERPOLATOR_T::filter_type	*filter = interpolator->GetNeededFilter(x);

			return filter->Apply((*this), integral_part(x));
		}

		//! \name Фильтрация
		//! @{
	private:
		enum fir_filter_direction
		{
			iir_filter_forward,
			iir_filter_reverse
		};
	// два ких-фильтра для сравнения, прокомментировать
		template<class FILTER_KERNEL_T>
		void	FilterBuiltIn(const MathFunction<T, ST, FIELD_TAG> &original, const FILTER_KERNEL_T &);

		template<class FILTER_KERNEL_T>
		void	FilterScanData(const MathFunction<T, ST, FIELD_TAG> &original, const FILTER_KERNEL_T &);

		template<class FILTER_KERNEL_T>
		void	FilterScanFilter(const MathFunction<T, ST, FIELD_TAG> &original, const FILTER_KERNEL_T &);

	// рекурсивный фильтр, тестировать; следует также провести реорганизацию (скрытая функция без буфера, буфер создается вне, если нужно)
		template<class FILTER_KERNEL_T>
		void	FilterRecursive(const FILTER_KERNEL_T &, fir_filter_direction direction);
	// слегка оптимизированная версия рекурсивного фильтра 2-го порядка
		template<class FILTER_KERNEL_T>
		void	FilterRecursive2(const FILTER_KERNEL_T &, fir_filter_direction direction);

		template<class FILTER_KERNEL_T>
		void	FilterRecursiveBidirectional(const FILTER_KERNEL_T &filter);

	public:
		template<class FILTER_KERNEL_T>
		void	Filter(const FILTER_KERNEL_T &filter);

		void	FilterMedian(size_t ws);


		// на переделку:
		void	FilterGauss(double dispersion, double value_at_edge = 0.3, extrapolation::method ex = extrapolation::by_last_value);
		// @}

		//! \name Inherited typedefs
		//! @{

		typedef typename parent::iterator iterator;
		typedef typename parent::const_iterator const_iterator;
		typedef typename parent::reverse_iterator reverse_iterator;
		typedef typename parent::const_reverse_iterator const_reverse_iterator;
		// !@}

		// Inherited methods
		using parent::step;
		using parent::at;

		using parent::realloc;
		using parent::resize;
		using parent::size;

		using parent::begin;
		using parent::end;
		using parent::rbegin;
		using parent::rend;
		using parent::uses_external_data;
		using parent::CopyData;
};



//--------------------------------------------------------------

//! \brief Поэлементная инверсия
template <XRAD__MathFunction_template> void negate(MathFunction<XRAD__MathFunction_template_args> &function)
{
	Apply_A_1D_F1(function, Functors::unary_minus_inplace());
}

//--------------------------------------------------------------

check_container_1d(MathFunction, int, int, AlgebraicStructures::FieldTagScalar)



//--------------------------------------------------------------

XRAD_END

#include "MathFunction.hh"

//--------------------------------------------------------------
#endif //XRAD__File_mathfunction_h

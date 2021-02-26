#ifndef XRAD__File_filter_kernel_h
#define XRAD__File_filter_kernel_h
//--------------------------------------------------------------

#include "DataArray.h"
#include <XRADBasic/Sources/SampleTypes/HomomorphSamples.h>
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>

XRAD_BEGIN

//--------------------------------------------------------------

enum filtering_algorithm
{
	fir_scan_data,
	fir_scan_filter,
	fir_built_in,
	iir_forward,
	iir_reverse,
	iir_bidirectional
};

class	FIRFilterKernelBasic
{
	protected:
		extrapolation::method	m_extrapolation_method;
		filtering_algorithm	m_filtering_algorithm;

		FIRFilterKernelBasic(): m_extrapolation_method(extrapolation::by_last_value), m_filtering_algorithm(fir_scan_data) {}
		FIRFilterKernelBasic(extrapolation::method m): m_extrapolation_method(m), m_filtering_algorithm(fir_scan_data) {}
	public:
		void	SetExtrapolationMethod(extrapolation::method m){ m_extrapolation_method = m; }
		void	SetFilteringAlgorithm(filtering_algorithm m){ m_filtering_algorithm = m; }
		extrapolation::method	ExtrapolationMethod() const { return m_extrapolation_method; }
		filtering_algorithm	FilteringAlgorithm() const { return m_filtering_algorithm; }
};

#define XRAD__FilterKernel_template class T, class ST, class FIELD_TAG
#define XRAD__FilterKernel_template_args T, ST, FIELD_TAG

/*!
	\brief Ядро фильтра

	FIRFilterKernel не должен ничего знать об объектах типа MathFunction, потому что
	в этом случае возникает круговая зависимость, разрушать которую приходилось бы
	искусственными приемами. Чтобы иметь возможность совершать алгебраические
	операции над различными фильтрами (например, складывать их между собой или
	умножать на скалярный коэффициент) фильтр наследуется от объектов, определенных
	в AlgebraicStructures.h.

	\todo Внести в Apply() процедуры ApplyUsingIterators* вместо циклов. Проверить. Для этого сохранить самую примитивную форму MathFunction::FilterBuiltIn().
	\todo Положение центра или начальной точки фильтра сделать параметром ядра. Все вычисления в процедурах свертки исключить!
*/
template<XRAD__FilterKernel_template>
class	FIRFilterKernel : public FIRFilterKernelBasic, public Algebra1D<FIRFilterKernel<XRAD__FilterKernel_template_args>, XRAD__FilterKernel_template_args>
{
	public:
		PARENT(Algebra1D<FIRFilterKernel, XRAD__FilterKernel_template_args>);

		typedef	T value_type;
		typedef	ST scalar_type;
		typedef	FIRFilterKernel<XRAD__FilterKernel_template_args>	self;

#ifndef XRAD_DEBUG
		// попробуем так оптимизироваться.
		typedef typename parent::value_type *iterator;
		typedef const typename parent::value_type *const_iterator;
		iterator	begin(){ return data(); }
		iterator	end(){ return begin() + size(); }
		const_iterator	begin() const { return data(); }
		const_iterator	end() const { return begin() + size(); }
		const_iterator	cbegin() const { return data(); }
		const_iterator	cend() const { return cbegin() + size(); }
#else
		using typename parent::iterator;
		using typename parent::const_iterator;
		using parent::begin;
		using parent::end;
		using parent::cbegin;
		using parent::cend;
#endif

		//typedef typename parent::reverse_iterator reverse_iterator;
		//typedef typename parent::const_reverse_iterator const_reverse_iterator;

		using parent::data;
		using parent::at;
		using parent::size;
		using parent::fill;

	private:
		//	Эти две функции запрещены, т.к. шаг данных должен быть всегда равен 1.
		//
		using parent::UseData;
		using parent::UseDataFragment;
		double	divisor;

	public:

		void	SetSmoothingKernelNormalizer(){ divisor = ElementSum(*this); }
		double	GetNormalizer()const { return divisor; }

		// Конструктор без параметра size создает фильтр нулевого размера.
		// Это поведение соответствует поведению двумерного фильтра.
		// Функция Apply для фильтра нулевого размера возвращает 0.
		// Можно пересмотреть такое поведение конструктора, но тогда
		// нужно разобраться с работой operator= (и тестами на move-семантику).
		FIRFilterKernel(): divisor(1) {}
		FIRFilterKernel(extrapolation::method m): FIRFilterKernelBasic(m), divisor(1) {}
		FIRFilterKernel(size_t s): parent(s), divisor(1) { if (s) fill(value_type(1./size())); }
		FIRFilterKernel(size_t s, extrapolation::method m):
			FIRFilterKernelBasic(m), parent(s), divisor(1)
		{
			if (s)
				fill(value_type(1./size()));
		}

	public:
		//! \note Для фильтра нулевого размера возвращает 0. Возможно, следует кидать исключение.
		template<class	ARR2>
		floating64_type<typename ARR2::value_type>	Apply(const ARR2 &data, ptrdiff_t x) const;

	public:
	// 		template<class	RESULT_T, class ORIGINAL_IT, class FIT>
	// 			void	ApplyUsingIteratorsIncremental(RESULT_T &result, ORIGINAL_IT original_it, FIT it, FIT ie) const;
		template<class ORIGINAL_IT, class FIT>
		floating64_type<typename ORIGINAL_IT::value_type>	ApplyUsingIterators(ORIGINAL_IT original_it, FIT it, FIT ie) const;
};



//--------------------------------------------------------------

XRAD_END

#include "FIRFilterKernel.hh"

//--------------------------------------------------------------
#endif //XRAD__File_filter_kernel_h

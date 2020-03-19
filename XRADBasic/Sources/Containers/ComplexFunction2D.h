//	file ComplexFunction2D.h
//--------------------------------------------------------------
#ifndef __complex_function_2D_h
#define __complex_function_2D_h

#include "MathFunction2D.h"
#include "ComplexFunction.h"
#include "ComplexContainer.h"

XRAD_BEGIN

//--------------------------------------------------------------

template<class T, class ST>
class	ComplexFunction2D : public ComplexContainer<
		MathFunction2D<ComplexFunction<T,ST>>,
		MathFunction2D<typename ComplexFunction<T,ST>::part_type>>
{
		PARENT(ComplexContainer<
				MathFunction2D<ComplexFunction<T,ST>>,
				MathFunction2D<typename ComplexFunction<T,ST>::part_type>>);

		// унаследованные функции интерполяции и дифференцирования использовать не разрешается,
		// потому что для комплексных данных тип осцилляции очень важен (см. комментарий в ComplexFunction.h)
	private:
		using parent::in;

	public:
		// про invariable см. подробный комментарий в DataOwner.h

		using typename parent::value_type;
		using typename parent::value_type_invariable;
		using typename parent::value_type_variable;

		using self = ComplexFunction2D;
		using invariable = ComplexFunction2D<value_type_invariable, ST>;
		using variable = ComplexFunction2D<value_type_variable, ST>;

		using ref = ReferenceOwner<self>;
		using ref_invariable = ReferenceOwner<invariable>;
		using ref_variable = ReferenceOwner<variable>;

		using row_type = ComplexFunction<T, ST>;

		using row_ref = ReferenceOwner<row_type>;
		using row_ref_invariable = ReferenceOwner<typename row_type::invariable>;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		ComplexFunction2D() = default;
		ComplexFunction2D(const parent &p): parent(p) {}
		ComplexFunction2D(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

//	проверка корректности задания атрибутов контейнера
//	см. ContainerCheck.h
check_complex_container(ComplexFunction2D, complexF64, double)
check_container_2d(ComplexFunction2D, complexF64, double)

//--------------------------------------------------------------

XRAD_END

#include "ComplexFunction2D.hh"

//--------------------------------------------------------------
#endif //__complex_function_2D_h

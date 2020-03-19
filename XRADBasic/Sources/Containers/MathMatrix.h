#ifndef __math_matrix_h
#define __math_matrix_h

#include "DataArray2D.h"
#include "LinearVector.h"
#include <XRADBasic/Sources/Algebra/AlgebraicAlgorithms2D.h>

XRAD_BEGIN


//--------------------------------------------------------------
//
//	MathMatrix
//
//	класс должен соответствовать матрице как элементу линейной алгебры
//	над полем ST; то есть, заданы операция сложения и умножения
//	на скаляр, а также некоммутативная операция умножения матриц
//	задавать матричное умножение через operator* не следует, т.к.
//	при этом должен создаваться массив, содержащий результат.
//	это неоправданно снижает производительность.
//


#define XRAD__MathMatrix_template class T, class ST, class FIELD_TAG
#define XRAD__MathMatrix_template_args T, ST, FIELD_TAG
#define XRAD__MathMatrix_template1 class T1, class ST1, class FIELD_TAG1
#define XRAD__MathMatrix_template_args1 T1, ST1, FIELD_TAG1
#define XRAD__MathMatrix_template2 class T2, class ST2, class FIELD_TAG2
#define XRAD__MathMatrix_template_args2 T2, ST2, FIELD_TAG2

#define	XRAD__Field2D(child_name, row_name, VT, ST, FIELD_TAG) AlgebraicStructures::FieldElement<DataArray2D<row_name<T, ST, FIELD_TAG>>, child_name<VT, ST, FIELD_TAG>, VT, ST, AlgebraicStructures::AlgebraicAlgorithmsDataArray2D, FIELD_TAG>
template <XRAD__MathMatrix_template>
class	MathMatrix: public XRAD__Field2D(MathMatrix, LinearVector, T, ST, FIELD_TAG)
// наследование от "алгебры" недопустимо, так как там в качестве умножения задано
// поэлементное действие, т.е. относящееся только к пространству функций
	{
	public:
		PARENT(XRAD__Field2D(MathMatrix, LinearVector, T, ST, FIELD_TAG));
#undef XRAD__Field2D

		typedef MathMatrix<T, ST, FIELD_TAG> self;
		typedef MathMatrix<const T, ST, FIELD_TAG> invariable;
			// про invariable см. подробный комментарий в DataOwner.h
		typedef T value_type;
		typedef ST scalar_type;

	//	создание
		MathMatrix(){}
		explicit MathMatrix(size_t vs, size_t hs) : parent(vs,hs){}
		MathMatrix(size_t vs, size_t hs, const value_type &default_value) : parent(vs, hs,default_value){}

		template<class ROW_T>
			MathMatrix(const DataArray2D<ROW_T> &original) : parent(original.vsize(), original.hsize()){Apply_AA_2D_F2(*this, original, Functors::assign());}

		// пустой деструктор объявлять не нужно: его вызов приводит
		// к неоправданной потере быстродействия на dsp
		// virtual ~MathMatrix(){}

		// транспонирование другой матрицы
		template<XRAD__MathMatrix_template1>
			self	&transpose(const MathMatrix<XRAD__MathMatrix_template_args1> &m1);
		// след матрицы. узнать получше, как это по-английски называется
		value_type tr() const;

		// матричное умножение. во избежание путаницы не объявляем через operator*,
		// но делаем функцию с таким названием.
		template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
			self	&matrix_multiply(const MathMatrix<XRAD__MathMatrix_template_args1> &m1, const MathMatrix<XRAD__MathMatrix_template_args2> &m2);
		template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
			self	&matrix_multiply(const MathMatrix<XRAD__MathMatrix_template_args1> &m1, const LinearVector<XRAD__MathMatrix_template_args2> &m2);
		template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
			self	&matrix_multiply(const LinearVector<XRAD__MathMatrix_template_args1> &m1, const MathMatrix<XRAD__MathMatrix_template_args2> &m2);
		template<XRAD__MathMatrix_template1>
			self	&diag(const LinearVector<XRAD__MathMatrix_template_args1> &v);

	private:
		// "деление" (умножение на обратную матрицу) в общем случае неопределено.

	public:
	//	inherited
		using parent::operator +=;
		using parent::operator -=;
		using parent::operator *=;
		using parent::operator /=;

		using parent::operator +;
		using parent::operator -;
		using parent::operator *;
		using parent::operator /;

		using parent::realloc;
		using parent::at;
		using parent::vsize;
		using parent::hsize;
		using parent::row;
		using parent::col;
	};


class matrix_algorithm_error : public logic_error
	{
	public:
		explicit matrix_algorithm_error(const char* what_arg) : logic_error(what_arg) {}
		explicit matrix_algorithm_error(const string& what_arg) : logic_error(what_arg) {}
	};


// TODO потом создать отдельный файл с объявлением типов матриц и векторов, как сделано с функциями


XRAD_END

#include "MathMatrix.hh"

#endif //__math_matrix_h

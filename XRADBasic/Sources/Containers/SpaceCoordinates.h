#ifndef XRAD__File_SpaceCoordinates_h
#define XRAD__File_SpaceCoordinates_h
/*!
	\file
	\date 2014-03-12 17:20
	\author kns

	Описанные ниже объекты представляют собой вектора и диапазоны
	в двумерном и трехмерном пространстве. Все они наследуются от
	LinearVectorFixedDimensions, поэтому к ним полностью применимы все действия
	элементов линейного векторного пространства.

	N - размерность. Наиболее употребимые будут 2,3 (вектора) или 4,6 (диапазоны)

	Тип компонента T и тип скаляра ST различаются.
	Например, если необходимо умножать целочисленные значения на дробные
	и быстродействие при этом некритично, нужно давать пару &lt;int,double&gt; и т.п.
	Если имеет значение скорость и не требуется сохранение точности,
	следует задавать пару &lt;int,int&gt;.

	\todo
	NB: Возможно, из поля (FieldElement) нужно перенести операцию сложения с числом в алгебру?
	(Она там вроде бы для удообства помещена, несмотря на алгебраическую некорректность.
	Но используется ли оно на деле?)
*/
//--------------------------------------------------------------

#include "LinearVectorFixedDimensions.h"

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\brief Двумерный вектор

	\todo Порядок подачи аргументов во всех векторах заменить с x,y,z на z,y,x (подумать).UPD: кажется, это давно уже сделано
*/
template<class T, class ST, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	point_2 : public LinearVectorFixedDimensions<point_2<T,ST,FIELD_TAG>,T,ST,FIELD_TAG,2>
{
	private:
		PARENT(LinearVectorFixedDimensions<point_2<T,ST,FIELD_TAG>, T,ST,FIELD_TAG,2>);
		using self = point_2<T,ST,FIELD_TAG>;

		enum
		{
			y_position = 0,
			x_position = 1
		};

	public:
		using parent::at;

		point_2(){}
		explicit point_2(const T& default_value) : parent(default_value){}
		point_2(const T& in_y, const T& in_x) {x()=in_x; y()=in_y;}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		point_2(const point_2<T2,ST2,FIELD_TAG2>& original) : parent(original){}
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		point_2 &operator=(const point_2<T2,ST2,FIELD_TAG2> &p){parent::operator=(p); return *this;}

		T	&x(){return at(x_position);}
		const T	&x() const{return at(x_position);}
		T	&y(){return at(y_position);}
		const T	&y() const{return at(y_position);}
};



//--------------------------------------------------------------
//! \brief Трехмерный вектор
template<class T, class ST, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	point_3 : public LinearVectorFixedDimensions<point_3<T,ST,FIELD_TAG>, T,ST,FIELD_TAG,3>
{
	private:
		PARENT(LinearVectorFixedDimensions<point_3<T,ST,FIELD_TAG>, T,ST,FIELD_TAG,3>);
		using self = point_3<T,ST,FIELD_TAG>;
		enum
		{
			z_position = 0,
			y_position = 1,
			x_position = 2
		};
	public:
		using parent::at;

		point_3(){}
		explicit point_3(const T& default_value) : parent(default_value){}
		point_3(const T& in_z, const T& in_y, const T& in_x){x()=in_x;y()=in_y;z()=in_z;}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		point_3(const point_3<T2,ST2,FIELD_TAG2>& original) : parent(original){}
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		point_3 &operator=(const point_3<T2,ST2,FIELD_TAG2> &p){parent::operator=(p); return *this;}

		T	&x(){return at(x_position);}
		const T	&x() const{return at(x_position);}
		T	&y(){return at(y_position);}
		const T	&y() const{return at(y_position);}
		T	&z(){return at(z_position);}
		const T	&z() const{return at(z_position);}
};

template<class T, class ST, size_t N, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	point_N : public LinearVectorFixedDimensions<point_N<T, ST, N, FIELD_TAG>, T, ST, FIELD_TAG, N>
{
	private:
		PARENT(LinearVectorFixedDimensions<point_N<T, ST, N, FIELD_TAG>, T, ST, FIELD_TAG, N>);
		using self = point_N<T, ST, N, FIELD_TAG>;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		point_N() = default;
		point_N(const parent &p): parent(p) {}
		point_N(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};


//--------------------------------------------------------------
/*!
	\brief Одномерный диапазон

	В качестве T по умолчанию используется скалярный тип данных, но допустим любой другой (например, цветовой диапазон RGB).
*/
template<class T, class ST, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	range_1 : public LinearVectorFixedDimensions<range_1<T,ST>, T,ST,FIELD_TAG,2>
{
	private:
		PARENT(LinearVectorFixedDimensions<range_1<T,ST>, T,ST,AlgebraicStructures::FieldTagScalar,2>);
		using self = range_1<T,ST>;

		enum
		{
			x1_position = 0,
			x2_position = 1,
		};

	public:
		using parent::at;

		range_1(){}
		explicit range_1(const T& default_value) : parent(default_value){}
		range_1(const T& in_x1, const T& in_x2){x1()=in_x1;x2()=in_x2;}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_1(const range_1<T2,ST2,FIELD_TAG2>& original) : parent(original){}
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_1 &operator=(const range_1<T2,ST2,FIELD_TAG2> &p){parent::operator=(p); return *this;}

		//T	&x1, &x2;
		T	&x1(){return at(x1_position);}
		const T	&x1() const {return at(x1_position);}
		T	&x2(){return at(x2_position);}
		const T	&x2() const {return at(x2_position);}

		T	&p1() {return x1();}
		const T	&p1() const {return x1();}

		T	&p2(){return x2();}
		const T	&p2() const {return x2();}

		T	delta() const { return p2()-p1(); }
		T	center() const { return (p2()+p1())/2; }

		void set_p1(const T& in_p1){x1=in_p1;}
		void set_p2(const T& in_p2){x2=in_p2;}

		//! \brief Проверка, лежит ли точка p в заданном диапазоне
		bool contains(const T& p) const { return in_range(p, x1(), x2()); }
		T	force(const T& p) const { return range(p, x1(), x2()); }
};



//--------------------------------------------------------------
/*!
	\brief Двумерный диапазон

	В качестве T по умолчанию используется скалярный тип данных, но допустим любой другой (например, цветовой диапазон RGB).
*/
template<class T, class ST, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	range_2 : public LinearVectorFixedDimensions<range_2<T,ST>, T,ST,FIELD_TAG,4>
{
	private:
		PARENT(LinearVectorFixedDimensions<range_2<T,ST>, T,ST,AlgebraicStructures::FieldTagScalar,4>);
		using self = range_2<T,ST>;
		using point_2_t = point_2<T,ST,typename range_2::field_tag>;
		using range_1_t = range_1<T,ST>;

		enum
		{
			y1_position = 0,
			x1_position = 1,
			y2_position = 2,
			x2_position = 3,
		};

	public:
		using parent::at;
		using parent::operator +;
		using parent::operator -;
		using parent::operator +=;
		using parent::operator -=;

		range_2(){}
		explicit range_2(const T& default_value) : parent(default_value){}
		range_2(const T& in_y1, const T& in_x1, const T& in_y2, const T& in_x2){x1()=in_x1;y1()=in_y1;x2()=in_x2;y2()=in_y2;}
		range_2(const point_2_t& p1, const point_2_t& p2){x1()=p1.x();y1()=p1.y();x2()=p2.x();y2()=p2.y();}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_2(const range_2<T2,ST2,FIELD_TAG2>& original) : parent(original){}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_2 &operator=(const range_2<T2,ST2,FIELD_TAG2> &p){parent::operator=(p); return *this;}

		T	&x1(){return at(x1_position);}
		T	&x2(){return at(x2_position);}
		T	&y1(){return at(y1_position);}
		T	&y2(){return at(y2_position);}

		const T	&x1() const {return at(x1_position);}
		const T	&x2() const {return at(x2_position);}
		const T	&y1() const {return at(y1_position);}
		const T	&y2() const {return at(y2_position);}

		point_2_t p1() const {return point_2_t(y1(), x1());}
		point_2_t p2() const {return point_2_t(y2(), x2());}
		point_2_t delta() const {return p2()-p1();}
		point_2_t center() const { return (p2()+p1())/2; }

		T	width() const {return norma(x2()-x1());}
		T	height() const {return norma(y2()-y1());}

		range_1_t range_x() const {return range_1_t(x1(),x2());}
		range_1_t range_y() const {return range_1_t(y1(),y2());}

		//! \name Смещение диапазона на вектор
		//! @{
		template<class T2, class ST2, class FIELD_TAG2>
		self &operator+=(const point_2<T2,ST2,FIELD_TAG2> &p){x1()+=p.x();x2()+=p.x();y1()+=p.y();y2()+=p.y(); return *this;}
		template<class T2, class ST2, class FIELD_TAG2>
		self &operator-=(const point_2<T2,ST2,FIELD_TAG2> &p){x1()-=p.x();x2()-=p.x();y1()-=p.y();y2()-=p.y(); return *this;}

		template<class T2, class ST2, class FIELD_TAG2>
		self operator+(const point_2<T2,ST2,FIELD_TAG2> &p) const{self result(*this); return result+=p;}
		template<class T2, class ST2, class FIELD_TAG2>
		self operator-(const point_2<T2,ST2,FIELD_TAG2> &p) const{self result(*this); return result-=p;}
		//! @}

		void set_p1(const point_2_t& p1){x1()=p1.x();y1()=p1.y();}
		void set_p2(const point_2_t& p2){x2()=p2.x();y2()=p2.y();}
		//! \brief Проверка, лежит ли точка p в заданном диапазоне
		bool contains(point_2_t& p){ return in_range(p.x(), x1(), x2())&&in_range(p.y(), y1(), y2()); }
		self force(point_2_t& p){ return self(range(p.y(), y1(), y2()), range(p.x(), x1(), x2())); }
};



//--------------------------------------------------------------
/*!
	\brief Трехмерный диапазон

	В качестве T по умолчанию используется скалярный тип данных, но допустим любой другой (например, цветовой диапазон RGB).
*/
template<class T, class ST, class FIELD_TAG = AlgebraicStructures::FieldTagScalar>
class	range_3 : public LinearVectorFixedDimensions<range_3<T,ST>, T,ST,FIELD_TAG,6>
{
	private:
		PARENT(LinearVectorFixedDimensions<range_3<T,ST>, T,ST,AlgebraicStructures::FieldTagScalar,6>);
		using self = range_3<T,ST>;
		using point_3_t = point_3<T,ST,typename range_3::field_tag> ;
		using range_1_t = range_1<T,ST>;
		using range_2_t = range_2<T,ST>;

		enum
		{
			z1_position = 0,
			y1_position = 1,
			x1_position = 2,
			z2_position = 3,
			y2_position = 4,
			x2_position = 5,
		};

	public:
		using parent::at;
		using parent::operator +;
		using parent::operator -;
		using parent::operator +=;
		using parent::operator -=;

		range_3(){}
		explicit range_3(const T& default_value) : parent(default_value){}
		range_3(const T& in_z1, const T& in_y1, const T& in_x1, const T& in_z2, const T& in_y2, const T& in_x2){x1()=in_x1;y1()=in_y1;z1()=in_z1;x2()=in_x2;y2()=in_y2;z2()=in_z2;}
		range_3(const point_3_t& p1, const point_3_t& p2){x1()=p1.x();y1()=p1.y();z1()=p1.z();x2()=p2.x();y2()=p2.y();z2()=p2.z();}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_3(const range_3<T2,ST2, FIELD_TAG2>& original) : parent(original){}
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2, class ST2, class FIELD_TAG2>
		range_3 &operator=(const range_3<T2,ST2,FIELD_TAG2> &p){parent::operator=(p); return *this;}

		T	&x1(){return at(x1_position);}
		T	&x2(){return at(x2_position);}
		T	&y1(){return at(y1_position);}
		T	&y2(){return at(y2_position);}
		T	&z1(){return at(z1_position);}
		T	&z2(){return at(z2_position);}

		const T	&x1() const {return at(x1_position);}
		const T	&x2() const {return at(x2_position);}
		const T	&y1() const {return at(y1_position);}
		const T	&y2() const {return at(y2_position);}
		const T	&z1() const {return at(z1_position);}
		const T	&z2() const {return at(z2_position);}

		point_3_t p1() const {return point_3_t(z1(),y1(),x1());}
		point_3_t p2() const {return point_3_t(z2(),y2(),x2());}
		point_3_t delta() const {return p2()-p1();}
		point_3_t center() const { return (p2()+p1())/2; }

		//! \name Проекции диапазона на оси
		//! @{
		range_1_t range_x() const {return range_1_t(x1(),x2());}
		range_1_t range_y() const {return range_1_t(y1(),y2());}
		range_1_t range_z() const {return range_1_t(z1(),z2());}
		//! @}

		//! \name Проекции диапазона на плоскости
		//! @{
		range_2_t range_yx() const {return range_2_t(y1(),x1(),y2(),x2());}
		range_2_t range_zy() const {return range_2_t(z1(),y1(),z2(),y2());}
		range_2_t range_zx() const {return range_2_t(z1(),x1(),z2(),x2());}
		//! @}

		//! \name Смещение диапазона на вектор
		//! @{
		self &operator+=(const point_3_t &p){x1()+=p.x();x2()+=p.x();y1()+=p.y();y2()+=p.y();z1()+=p.z;z2()+=p.z; return *this;}
		self &operator-=(const point_3_t &p){x1()-=p.x();x2()-=p.x();y1()-=p.y();y2()-=p.y();z1()-=p.z;z2()-=p.z; return *this;}
		self operator+(const point_3_t &p) const {self result(*this); return result+=p;};
		self operator-(const point_3_t &p) const {self result(*this); return result-=p;};
		//! @}

		void set_p1(const point_3_t& p1){x1()=p1.x();y1()=p1.y();z1()=p1.z;}
		void set_p2(const point_3_t& p2){x2()=p2.x();y2()=p2.y();z2()=p2.z;}

		void	set_range_z(const range_1_t &r) { z1()=r.p1(); z2()=r.p2(); }
		void	set_range_y(const range_1_t &r) { y1()=r.p1(); y2()=r.p2(); }
		void	set_range_x(const range_1_t &r) { x1()=r.p1(); x2()=r.p2(); }

		//! \brief Проверка, лежит ли точка p в заданном диапазоне
		bool contains(const point_3_t& p){ return in_range(p.x(), x1(), x2())&&in_range(p.y(), y1(), y2())&&in_range(p.z(), z1(), z2()); }
		self force(const point_3_t& p){ return self(range(p.z(), z1(), z2()), range(p.y(), y1(), y2()), range(p.x(), x1(), x2())); }
};



//--------------------------------------------------------------
//	Основные рабочие типы: данные целые или double,
//	множитель только double.
//	2017_04_12 Добавлены определения векторов из size_t и ptrdiff_t.
//	Их основное назначение -- навигация по массивам. Поэтому множители только целочисленные.

using	point2_I32 = point_2<int32_t, double, AlgebraicStructures::FieldTagScalar>;
using	point2_UI32 = point_2<uint32_t, double, AlgebraicStructures::FieldTagScalar>;
using	point2_F32 = point_2<float, double, AlgebraicStructures::FieldTagScalar>;
using	point2_F64 = point_2<double, double, AlgebraicStructures::FieldTagScalar>;
using	point2_ST = point_2<size_t, size_t, AlgebraicStructures::FieldTagScalar>;
using	point2_PDT = point_2<ptrdiff_t, ptrdiff_t, AlgebraicStructures::FieldTagScalar>;

using	point3_I32 = point_3<int32_t, double, AlgebraicStructures::FieldTagScalar>;
using	point3_UI32 = point_3<uint32_t, double, AlgebraicStructures::FieldTagScalar>;

using	point3_F32 = point_3<float, double, AlgebraicStructures::FieldTagScalar>;
using	point3_F64 = point_3<double, double, AlgebraicStructures::FieldTagScalar>;
using	point3_ST = point_3<size_t, size_t, AlgebraicStructures::FieldTagScalar>;
using	point3_PDT = point_3<ptrdiff_t, ptrdiff_t, AlgebraicStructures::FieldTagScalar>;

using	range1_I32 = range_1<int32_t, double>;
using	range1_F32 = range_1<float, double>;
using	range1_F64 = range_1<double, double>;
using	range1_ST = range_1<size_t, size_t>;
using	range1_PDT = range_1<ptrdiff_t, ptrdiff_t>;

using	range2_I32 = range_2<int32_t,double>;
using	range2_F32 = range_2<float, double>;
using	range2_F64 = range_2<double, double>;
using	range2_ST = range_2<size_t, size_t>;
using	range2_PDT = range_2<ptrdiff_t, ptrdiff_t>;

using	range3_I32 = range_3<int32_t, double>;
using	range3_UI32 = range_3<uint32_t, double>;
using	range3_F32 = range_3<float, double>;
using	range3_F64 = range_3<double, double>;
using	range3_ST = range_3<size_t, size_t>;
using	range3_PDT = range_3<ptrdiff_t, ptrdiff_t>;

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_SpaceCoordinates_h

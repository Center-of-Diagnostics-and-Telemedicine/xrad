#ifndef __boolean_algebra_element_h
#define __boolean_algebra_element_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Специфические действия для булевой алгебры

	@}
*/
//--------------------------------------------------------------

#include <XRADBasic/Sources/SampleTypes/BooleanSample.h>
#include <XRADBasic/Sources/Core/Functors.h>

// для булевой алгебры
// третий аргумент шаблона VT -- целочисленный тип внутреннего datum объекта Boolean*
// порождается тип Boolean*<VT>
#define XRAD__template_1B class CONTAINER_T, class CHILD_T, class VT, class ALG_T
#define XRAD__template_2B class CONTAINER_T2, class CHILD_T2, class VT2, class ALG_T2
#define XRAD__template_1B_args CONTAINER_T,CHILD_T,VT,ALG_T
#define XRAD__template_2B_args CONTAINER_T2,CHILD_T2,VT2,ALG_T2
#define XRAD__template_1B_args_logical CONTAINER_T,CHILD_T, BooleanLogical<VT>, ALG_T
#define XRAD__template_2B_args_logical CONTAINER_T2,CHILD_T2, BooleanLogical<VT2>, ALG_T2
#define XRAD__template_1B_args_bitwise CONTAINER_T,CHILD_T, BooleanBitwise<VT>, ALG_T
// для битовых масок все действия разрешены только с аргументом точно того же типа.
// шаблоны для операндов с другой разрядностью не предусмотрены

XRAD_BEGIN
namespace	AlgebraicStructures
{

//--------------------------------------------------------------

template<XRAD__template_1B>
class BooleanAlgebraElement : public CONTAINER_T
{
	public:
		PARENT(CONTAINER_T);
		typedef BooleanAlgebraElement<XRAD__template_1B_args> self;
		typedef VT value_type;
		typedef ALG_T algorithms_type;

	private:
		typedef	CHILD_T child_type;
		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	private:
		struct internal_xor_assign
		{
			void operator()(value_type &x, const value_type &y) const { x^=y; }
		};

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanAlgebraElement() = default;
		BooleanAlgebraElement(const parent &p): parent(p) {}
		BooleanAlgebraElement(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		template<XRAD__template_2B>
			bool	operator == (const BooleanAlgebraElement<XRAD__template_2B_args> &f2) const
			{
				if (!algorithms_type::AA_EqSize(child_ref(), f2))
					return false;
				return !algorithms_type::AA_Any(child_ref(), f2, Functors::not_equal());
			}

		template<XRAD__template_2B>
			bool	operator != (const BooleanAlgebraElement<XRAD__template_2B_args> &f2) const
			{
				if (algorithms_type::AA_EqSize(child_ref(), f2))
					return false;
				return algorithms_type::AA_Any(child_ref(), f2, Functors::not_equal());
			}
};



//--------------------------------------------------------------



template<XRAD__template_1B>
class BooleanAlgebraElementLogical : public BooleanAlgebraElement<XRAD__template_1B_args_logical>
{
	private:
		typedef	CHILD_T child_type;
		typedef BooleanAlgebraElementLogical<XRAD__template_1B_args_logical > self;
		PARENT(BooleanAlgebraElement<XRAD__template_1B_args_logical >);

		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	public:
		typedef BooleanLogical<VT> value_type;
		using algorithms_type = typename parent::algorithms_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanAlgebraElementLogical() = default;
		BooleanAlgebraElementLogical(const parent &p): parent(p) {}
		BooleanAlgebraElementLogical(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		template<XRAD__template_2B>
		child_type	&operator &= (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::logical_and_assign());}
		template<XRAD__template_2B>
		child_type	&operator |= (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::logical_or_assign());}
		template<XRAD__template_2B>
		child_type	&operator ^= (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::logical_xor_assign());}

		template<class T>
		child_type	&operator &= (const BooleanLogical<T> &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::logical_and_assign());}
		template<class T>
		child_type	&operator |= (const BooleanLogical<T> &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::logical_or_assign());}
		template<class T>
		child_type	&operator ^= (const BooleanLogical<T> &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::logical_xor_assign());}

		template<XRAD__template_2B>
		child_type	&operator & (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_logical_and());}
		template<XRAD__template_2B>
		child_type	&operator | (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_logical_or());}
		template<XRAD__template_2B>
		child_type	&operator ^ (const BooleanAlgebraElement<XRAD__template_2B_args_logical > &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_logical_xor());}

		template<class T>
		child_type	&operator & (const BooleanLogical<T> &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_logical_and());}
		template<class T>
		child_type	&operator | (const BooleanLogical<T> &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_logical_or());}
		template<class T>
		child_type	&operator ^ (const BooleanLogical<T> &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_logical_xor());}

		child_type	operator!() const {return algorithms_type::A_Op_New(child_ref(), Functors::assign_logical_not());}
};

//--------------------------------------------------------------

template<XRAD__template_1B>
class BooleanAlgebraElementBitwise : public BooleanAlgebraElement<XRAD__template_1B_args_bitwise>
{
	private:
		typedef	CHILD_T child_type;
		typedef BooleanAlgebraElementBitwise<XRAD__template_1B_args_bitwise > self;

		PARENT(BooleanAlgebraElement<XRAD__template_1B_args_bitwise >);

		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	public:
		typedef BooleanBitwise<VT> value_type;
		using algorithms_type = typename parent::algorithms_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanAlgebraElementBitwise() = default;
		BooleanAlgebraElementBitwise(const parent &p): parent(p) {}
		BooleanAlgebraElementBitwise(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		// для битовых масок все действия разрешены только с аргументом точно того же типа.
		// шаблоны для операндов с другой разрядностью не предусмотрены
		child_type	&operator &= (const self &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::bitwise_and_assign());}
		child_type	&operator |= (const self &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::bitwise_or_assign());}
		child_type	&operator ^= (const self &f2){return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::bitwise_xor_assign());}

		child_type	&operator &= (const value_type &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::bitwise_and_assign());}
		child_type	&operator |= (const value_type &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::bitwise_or_assign());}
		child_type	&operator ^= (const value_type &y){return algorithms_type::AS_Op_Assign(child_ref(), y, Functors::bitwise_xor_assign());}

		child_type	&operator & (const self &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_bitwise_and());}
		child_type	&operator | (const self &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_bitwise_or());}
		child_type	&operator ^ (const self &f2){return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_bitwise_xor());}

		child_type	&operator & (const value_type &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_bitwise_and());}
		child_type	&operator | (const value_type &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_bitwise_or());}
		child_type	&operator ^ (const value_type &y){return algorithms_type::AS_Op_New(child_ref(), y, Functors::assign_bitwise_xor());}

		child_type	&operator <<= (int n) {return algorithms_type::AS_Op_Assign(child_ref(), n, Functors::shl_assign());}
		child_type	&operator >>= (int n) {return algorithms_type::AS_Op_Assign(child_ref(), n, Functors::shr_assign());}

		child_type	operator << (int n) const {return algorithms_type::AS_Op_New(child_ref(), n, Functors::assign_shl());}
		child_type	operator >> (int n) const {return algorithms_type::AS_Op_New(child_ref(), n, Functors::assign_shr());}
		child_type	operator ~ () const {return algorithms_type::A_Op_New(child_ref(), Functors::assign_bitwise_not());}
};

//--------------------------------------------------------------

}//namespace AlgebraicStructures
XRAD_END

#undef XRAD__template_1B
#undef XRAD__template_2B
#undef XRAD__template_1B_args
#undef XRAD__template_2B_args
#undef XRAD__template_1B_args_logical
#undef XRAD__template_2B_args_logical
#undef XRAD__template_1B_args_bitwise

//--------------------------------------------------------------
#endif //__boolean_algebra_element_h

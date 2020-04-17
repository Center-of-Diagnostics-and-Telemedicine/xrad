// file ComplexContainer.h
//--------------------------------------------------------------
#ifndef __ComplexContainer_h
#define __ComplexContainer_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
class ComplexContainer: public Container
{
		PARENT(Container);

	public:
		using typename parent::value_type;
		using typename parent::value_type_invariable;
		using typename parent::value_type_variable;
		using parent::GetDataComponent;

		using self = ComplexContainer;
		using invariable = ComplexContainer<typename Container::invariable,
				typename ComponentContainer::invariable>;
		using variable = ComplexContainer<typename Container::variable,
				typename ComponentContainer::variable>;
		using ref = ReferenceOwner<self>;

		using plane_type = ComponentContainer;
		using plane_type_invariable = typename ComponentContainer::invariable;
		using plane_ref = ReferenceOwner<plane_type>;
		using plane_ref_invariable = ReferenceOwner<plane_type_invariable>;

		using value_type_ = value_type; // MSVC2015 fix: value_type is not recognized for ::part_type.
		using component_type = typename value_type_::part_type;
		using component_type_invariable = std::add_const_t<component_type>;
		using component_type_variable = std::remove_const_t<component_type>;

		// Псевдонимы типов в терминологии ComplexFunction: part вместо plane.
		using value_part_type = component_type;
		using value_part_type_invariable = component_type_invariable;
		using value_part_type_variable = component_type_variable;

		using part_type = plane_type;
		using part_type_invariable = plane_type_invariable;
		using part_ref = plane_ref;
		using part_ref_invariable = plane_ref_invariable;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		ComplexContainer() = default;
		ComplexContainer(const parent &p): parent(p) {}
		ComplexContainer(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

		//! \name Доступ к компонентам real, imag
		//! @{
	public:
		template <class Plane = plane_ref>
		Plane real();
		template <class Plane = plane_ref_invariable>
		Plane real() const;

		template <class Plane = plane_ref>
		Plane imag();
		template <class Plane = plane_ref_invariable>
		Plane imag() const;

		template <class Plane>
		void GetReal(Plane &result);
		template <class Plane>
		void GetReal(Plane &result) const;

		template <class Plane>
		void GetImag(Plane &result);
		template <class Plane>
		void GetImag(Plane &result) const;

		//! @}
};

//--------------------------------------------------------------

template <class CT, class CCT>
auto real(ComplexContainer<CT, CCT> &f)
{
	return f.real();
}

template <class CT, class CCT>
auto real(const ComplexContainer<CT, CCT> &f)
{
	return f.real();
}

template <class CT, class CCT>
auto imag(ComplexContainer<CT, CCT> &f)
{
	return f.imag();
}

template <class CT, class CCT>
auto imag(const ComplexContainer<CT, CCT> &f)
{
	return f.imag();
}

//--------------------------------------------------------------

//TODO две следующие набросок, хорошо обдумать
template<class ARR3_T, class CT, class CCT>
void	GetReal(ARR3_T &part, ComplexContainer<CT, CCT> &arr)
{
	arr.GetReal(part);
}

template<class ARR3_T, class CT, class CCT>
void	GetReal(ARR3_T &part, const ComplexContainer<CT, CCT> &arr)
{
	arr.GetReal(part);
}

template<class ARR3_T, class CT, class CCT>
void	GetImag(ARR3_T &part, ComplexContainer<CT, CCT> &arr)
{
	arr.GetImag(part);
}

template<class ARR3_T, class CT, class CCT>
void	GetImag(ARR3_T &part, const ComplexContainer<CT, CCT> &arr)
{
	arr.GetImag(part);
}

//--------------------------------------------------------------

XRAD_END

#include "ComplexContainer.hh"

//--------------------------------------------------------------
#endif // __ComplexContainer_h

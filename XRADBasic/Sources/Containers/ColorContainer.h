/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ColorContainer.h
//--------------------------------------------------------------
#ifndef XRAD__File_ColorContainer_h
#define XRAD__File_ColorContainer_h
//--------------------------------------------------------------

#include "ReferenceOwner.h"

XRAD_BEGIN

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
class ColorContainer: public Container
{
		PARENT(Container);

	public:
		using typename parent::value_type;
		using typename parent::value_type_invariable;
		using typename parent::value_type_variable;
		using parent::GetDataComponent;

		using self = ColorContainer;
		using invariable = ColorContainer<typename Container::invariable,
				typename ComponentContainer::invariable>;
		using variable = ColorContainer<typename Container::variable,
				typename ComponentContainer::variable>;
		using ref = ReferenceOwner<self>;

		using plane_type = ComponentContainer;
		using plane_type_invariable = typename ComponentContainer::invariable;
		using plane_ref = ReferenceOwner<plane_type>;
		using plane_ref_invariable = ReferenceOwner<plane_type_invariable>;

	public:
		using value_type_ = value_type;
		using component_type = typename value_type_::component_type;
		using component_type_invariable = std::add_const_t<component_type>;
		using component_type_variable = std::remove_const_t<component_type>;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		ColorContainer() = default;
		ColorContainer(const parent &p): parent(p) {}
		ColorContainer(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

		//! \name Доступ к цветовым плоскостям
		//! @{
	public:
		plane_ref red();
		plane_ref green();
		plane_ref blue();

		plane_ref_invariable	red() const;
		plane_ref_invariable	green() const;
		plane_ref_invariable	blue() const;

		template <class Plane>
		void GetRed(Plane &plane);
		template <class Plane>
		void GetGreen(Plane &plane);
		template <class Plane>
		void GetBlue(Plane &plane);

		template <class Plane>
		void GetRed(Plane &plane) const;
		template <class Plane>
		void GetGreen(Plane &plane) const;
		template <class Plane>
		void GetBlue(Plane &plane) const;

		//! @}
};

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
auto red(ColorContainer<Container, ComponentContainer> &image)
{
	return image.red();
}

template <class Container, class ComponentContainer>
auto green(ColorContainer<Container, ComponentContainer> &image)
{
	return image.green();
}

template <class Container, class ComponentContainer>
auto blue(ColorContainer<Container, ComponentContainer> &image)
{
	return image.blue();
}

template <class Container, class ComponentContainer>
auto red(const ColorContainer<Container, ComponentContainer> &image)
{
	return image.red();
}

template <class Container, class ComponentContainer>
auto green(const ColorContainer<Container, ComponentContainer> &image)
{
	return image.green();
}

template <class Container, class ComponentContainer>
auto blue(const ColorContainer<Container, ComponentContainer> &image)
{
	return image.blue();
}

//--------------------------------------------------------------

//! \brief Вычисление негативного цвета
template <class Container, class ComponentContainer>
void inverse(ColorContainer<Container, ComponentContainer> &image)
{
	ApplyAction(image, Functors::bitwise_not_inplace());
}

//--------------------------------------------------------------

XRAD_END

#include "ColorContainer.hh"

//--------------------------------------------------------------
#endif // XRAD__File_ColorContainer_h

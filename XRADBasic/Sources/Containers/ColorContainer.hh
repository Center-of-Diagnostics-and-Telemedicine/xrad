/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
XRAD_BEGIN

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::red() -> plane_ref
{
	return parent::template GetDataComponent<plane_ref>([](value_type *data) { return &data->red(); });
}

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::green() -> plane_ref
{
	return parent::template GetDataComponent<plane_ref>([](value_type *data) { return &data->green(); });
}

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::blue() -> plane_ref
{
	return parent::template GetDataComponent<plane_ref>([](value_type *data) { return &data->blue(); });
}

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::red() const -> plane_ref_invariable
{
	return parent::template GetDataComponent<plane_ref_invariable>([](value_type_invariable *data)
			{ return &data->red(); });
}

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::green() const -> plane_ref_invariable
{
	return parent::template GetDataComponent<plane_ref_invariable>([](value_type_invariable *data)
			{ return &data->green(); });
}

template <class Container, class ComponentContainer>
auto ColorContainer<Container, ComponentContainer>::blue() const -> plane_ref_invariable
{
	return parent::template GetDataComponent<plane_ref_invariable>([](value_type_invariable *data)
			{ return &data->blue(); });
}

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetRed(Plane &plane)
{
	GetDataComponent(plane, [](value_type *data) { return &data->red(); });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetGreen(Plane &plane)
{
	GetDataComponent(plane, [](value_type *data) { return &data->green(); });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetBlue(Plane &plane)
{
	GetDataComponent(plane, [](value_type *data) { return &data->blue(); });
}

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetRed(Plane &plane) const
{
	GetDataComponent(plane, [](value_type_invariable *data) { return &data->red(); });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetGreen(Plane &plane) const
{
	GetDataComponent(plane, [](value_type_invariable *data) { return &data->green(); });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ColorContainer<Container, ComponentContainer>::GetBlue(Plane &plane) const
{
	GetDataComponent(plane, [](value_type_invariable *data) { return &data->blue(); });
}

//--------------------------------------------------------------

XRAD_END

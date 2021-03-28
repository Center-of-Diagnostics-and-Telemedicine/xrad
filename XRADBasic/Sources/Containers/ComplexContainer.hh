/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ComplexContainer.hh
//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
template <class Plane>
Plane ComplexContainer<Container, ComponentContainer>::real()
{
	return parent::template GetDataComponent<Plane>([](value_type *data) { return &data->re; });
}

template <class Container, class ComponentContainer>
template <class Plane>
Plane ComplexContainer<Container, ComponentContainer>::real() const
{
	return parent::template GetDataComponent<Plane>([](const value_type *data) { return &data->re; });
}

template <class Container, class ComponentContainer>
template <class Plane>
Plane ComplexContainer<Container, ComponentContainer>::imag()
{
	return parent::template GetDataComponent<Plane>([](value_type *data) { return &data->im; });
}

template <class Container, class ComponentContainer>
template <class Plane>
Plane ComplexContainer<Container, ComponentContainer>::imag() const
{
	return parent::template GetDataComponent<Plane>([](const value_type *data) { return &data->im; });
}

//--------------------------------------------------------------

template <class Container, class ComponentContainer>
template <class Plane>
void ComplexContainer<Container, ComponentContainer>::GetReal(Plane &plane)
{
	GetDataComponent(plane, [](value_type *data) { return &data->re; });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ComplexContainer<Container, ComponentContainer>::GetReal(Plane &plane) const
{
	GetDataComponent(plane, [](const value_type *data) { return &data->re; });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ComplexContainer<Container, ComponentContainer>::GetImag(Plane &plane)
{
	GetDataComponent(plane, [](value_type *data) { return &data->im; });
}

template <class Container, class ComponentContainer>
template <class Plane>
void ComplexContainer<Container, ComponentContainer>::GetImag(Plane &plane) const
{
	GetDataComponent(plane, [](const value_type *data) { return &data->im; });
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

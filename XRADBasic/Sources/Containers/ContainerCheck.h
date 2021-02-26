/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_ContainerCheck_h
#define XRAD__File_ContainerCheck_h
/*!
	\file
	\date 2014-10-01 11:06
	\author kns

	\brief Макро, проверяющие, заданы ли обязательные атрибуты в одномерном контейнере

	После определения любого контейнера должна стоять эта проверка.
	При появлении новых обязательных атрибутов вносить поправки сюда.
*/
//--------------------------------------------------------------

//__VA_ARGS__


#define check_container_1d(C, ...)\
inline void	check_container_1d_##C()\
{\
	typedef C<__VA_ARGS__> container_t;\
	\
	container_t::value_type	v;\
	v = zero_value(container_t::value_type());/*нулевое значение для элемента контейнера*/\
	make_zero(v);\
	\
	container_t c1, c2;\
	c1=zero_value(c2);\
	make_zero(c1);\
	\
	container_t::value_type_variable	vnc; vnc;\
	extern container_t::value_type_invariable	vni; vni;\
	\
	container_t::self	s;\
	container_t::invariable	inv;\
	container_t::ref	r;\
	container_t::ref_invariable	ri;\
	\
	container_t::iterator	it;\
	container_t::const_iterator	cit;\
	container_t::reverse_iterator	rit;\
	container_t::const_reverse_iterator	crit;\
}



#define check_container_2d(C, ...)\
inline void	check_container_2d_##C()\
{\
	typedef C<__VA_ARGS__> container_t;\
	container_t c1, c2;\
	\
	c1=zero_value(c2);\
	make_zero(c1);\
	\
	container_t::value_type	v; v;\
	container_t::value_type_variable	vnc; vnc;\
	extern container_t::value_type_invariable	vni; vni;\
	\
	container_t::self	s;\
	container_t::invariable	inv;\
	container_t::ref	r;\
	container_t::ref_invariable	ri;\
	\
	container_t::row_iterator	it;\
	container_t::const_row_iterator	cit;\
	container_t::reverse_row_iterator	rit;\
	container_t::const_reverse_row_iterator	crit;\
	\
	container_t::row_type	rt;\
	container_t::row_type_invariable	rti;\
	container_t::row_ref	rref;\
	container_t::row_ref_invariable	rrefi;\
}


#define check_container_md(C, ...)\
inline void	check_container_md_##C()\
{\
	typedef C<__VA_ARGS__> container_t;\
	container_t c1, c2;\
	\
	c1=zero_value(c1);\
	make_zero(c1);\
	\
	container_t::value_type	v; v;\
	container_t::value_type_variable	vnc; vnc;\
	extern container_t::value_type_invariable	vni; vni;\
	\
	container_t::self	s;\
	container_t::invariable	inv;\
	container_t::ref	r;\
	container_t::ref_invariable	ri;\
	\
	container_t::row_iterator	it;\
	container_t::const_row_iterator	cit;\
	container_t::reverse_row_iterator	rit;\
	container_t::const_reverse_row_iterator	crit;\
	container_t::index_vector_iterator	iit;\
	container_t::const_index_vector_iterator	ciit;\
	container_t::steps_vector_iterator	svit;\
	container_t::const_steps_vector_iterator	csvit;\
	\
	container_t::row_type	rt;\
	typename container_t::row_type::invariable	rti;\
	container_t::row_ref	rref;\
	container_t::row_ref_invariable	rrefi;\
	\
	container_t::slice_type	st;\
	typename container_t::slice_type::invariable	sti;\
	container_t::slice_ref	sref;\
	container_t::slice_ref_invariable	srefi;\
}



#define check_complex_container(C, ...)\
inline void	check_complex_container_##C()\
{\
	typedef C<__VA_ARGS__> container_t;\
	\
	container_t::value_type	vt;\
	container_t::value_type_variable	vnc; vnc;\
	extern container_t::value_type_invariable	vni; vni;\
	\
	container_t::value_part_type	vpt; vpt;\
	container_t::value_part_type_variable	vptnc; vptnc;\
	extern container_t::value_part_type_invariable	vptni; vptni;\
	\
	container_t::scalar_type	sc; sc;\
	container_t::part_type	pt;\
	container_t::part_type_invariable	pti;\
	container_t::part_ref	cr;\
	container_t::part_ref_invariable	pri;\
}



//--------------------------------------------------------------
#endif //XRAD__File_ContainerCheck_h

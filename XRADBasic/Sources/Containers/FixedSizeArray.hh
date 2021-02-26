/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
XRAD_BEGIN



template<class VT, size_t N>
const VT &FixedSizeArray<VT,N>::at(size_t i ) const
{
	check_data_and_index(i);
	return m_data[i];
}

template<class VT, size_t N>
VT	&FixedSizeArray<VT,N>::at(size_t i )
{
	check_data_and_index(i);
	return m_data[i];
}



//--------------------------------------------------------------
//
//	Методы проверки границ массива и поиска "плохих чисел"
//

//	В общем случае только проверка границ
template<class VT, size_t N>
inline	void FixedSizeArray<VT, N>::check_data_and_index(size_t index) const
{
	check_array_boundaries(index);
}



template<class VT, size_t N>
#if !XRAD_CHECK_ARRAY_BOUNDARIES
void FixedSizeArray<VT, N>::check_array_boundaries(size_t ) const{}
#else
void FixedSizeArray<VT, N>::check_array_boundaries(size_t index) const
{
	if(/*index < 0 || */index >= size())
	{
		string problem_description = typeid(self).name() + ssprintf("::at(size_t): m_data==%X, index = %d, size = %d", (size_t)m_data, index, size());
		ForceDebugBreak();
		throw(out_of_range(problem_description));
	}
}
#endif



XRAD_END

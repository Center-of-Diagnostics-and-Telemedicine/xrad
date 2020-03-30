#ifndef __unitmatrix_hh
#define __unitmatrix_hh

XRAD_BEGIN

using namespace std;

template<class result_type>
result_type  unit_matrix(size_t size)
{
	result_type	 result(size, size, 0);

	for (size_t i = 0; i < size; i++)
	{
		result.at(i, i) = 1;
	}
	
	return result;
}


XRAD_END

#endif //#define __unitmatrix_hh


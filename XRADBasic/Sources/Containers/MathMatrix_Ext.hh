/*!
	\file
	\brief Функции умножения матриц и векторов
	\author    Prokudaylo
*/
XRAD_BEGIN

using namespace std;

/*!
	\brief Generation of unit matrix of size \a size.

	\tparam size The size of unit matrix.
	\return  The unit matrix of given size.
	The type of result should be explicetely specified when the function is called like r = unit_matrix<return type> (size).

*/

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

/*!
	\brief Multiplication of two rectangular matrixes \a m1 and \a m2.

	\tparam m1 First member of matrix product.
	\tparam m2 Second member of matrix product.
	\return  The result of matrix product. The type of result should be explicetely specified when
	the function is called like r = matrix_multiply<return type> (m1,m2).

	Horizontal size of m1 should be the same as vertical size of m2, otherwise exeption is thrown.
*/

template<class result_type, XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
result_type   matrix_multiply(
	const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
	const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
{
	result_type	 result(m1.vsize(), m2.hsize());
	result.matrix_multiply(m1, m2);
	return result;
}

/*!
	\brief Multiplication of two rectangular matrixes \a m1 and \a m2.
	\tparam m1 First member of matrix product.
	\tparam m2 Second member of matrix product.
	\return  The result of matrix product. the type of result is taken from type m1 which is the same as m2. Function is called like r = matrix_multiply(m1,m2).

	Horizontal size of m1 should be the same as vertical size of m2, otherwise exeption is thrown.
*/

template< XRAD__MathMatrix_template>
MathMatrix<XRAD__MathMatrix_template_args>    matrix_multiply(
	const MathMatrix<XRAD__MathMatrix_template_args> &m1,
	const MathMatrix<XRAD__MathMatrix_template_args> &m2)
{
	return  matrix_multiply<MathMatrix<XRAD__MathMatrix_template_args>>(m1, m2);
}


/*!
	\brief Multiplication of rectangular matrix \a m1 and vector\a v, the vector comes to products from right.
	\tparam m1 First member of product.
	\tparam v Second member of product.
	\return  The result of matrix to vector product. The type of result should be explicetely specified when the function is called like r = vector_multiply_R<return type> (m1,v).

	Horizontal size of m1 should be the same as size of v, otherwise exeption is thrown.
*/

template<class result_type, XRAD__MathMatrix_template1, XRAD__LinearVector_template>
result_type vector_multiply_R(
	const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
	const LinearVector<XRAD__LinearVector_template_args> &v)
{
	result_type result(m1.vsize());


	if (v.size() != m1.hsize())
	{
		string problem_description = __func__ +// typeid(self).name() +
			ssprintf("::matrix_multiply(m1,v) -- invalid arrays sizes: m1.hsize() = %zu, v.size() = %zu",
				EnsureType<size_t>(m1.hsize()),
				EnsureType<size_t>(v.size())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}

	typename result_type::iterator it = result.begin();
	for (size_t i = 0; i<m1.vsize(); ++i, ++it)
	{
		*it = sp(m1.row(i), v);
	}

	return result;

}

/*!
	\brief Multiplication of rectangular matrix \a m1 and vector\a v, the vector comes to products from right.
	\tparam m1 First member of product.
	\tparam v Second member of product.
	\return  The result of matrix to vector product. The type of result is taken from type \a v. function is called like r = vector_multiply_R(m1,v).

	Horizontal size of m1 should be the same as size of v, otherwise exeption is thrown.
*/

template<XRAD__MathMatrix_template1, XRAD__LinearVector_template>
LinearVector<XRAD__LinearVector_template_args>	vector_multiply_R(
	const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
	const LinearVector<XRAD__LinearVector_template_args> &v)
{
	return vector_multiply_R<LinearVector<XRAD__LinearVector_template_args>>(m1, v);
}


/*!
	\brief Multiplication of vector \a v and matrix\a m1, the vector comes to products from left
	\tparam v First member of product.
	\tparam m1 Second member of product.
	\return  The result of vector to matrix product. The type of result should be explicetely specified when the function is called like r = vector_multiply_L<return type> (v,m1).

	Size of v should be the same as vertical size of m, otherwise exeption is thrown.
*/

template<class result_type, XRAD__LinearVector_template, XRAD__MathMatrix_template1>
result_type vector_multiply_L(
	const LinearVector<XRAD__LinearVector_template_args> &v,
	const MathMatrix<XRAD__MathMatrix_template_args1> &m1)
{
	result_type result(m1.hsize());


	if (v.size() != m1.vsize())
	{
		string problem_description = __func__ +// typeid(self).name() +
			ssprintf(" -- invalid arrays sizes: v.size()  = %zu, m1.vsize() = %zu",
				EnsureType<size_t>(v.size()),
				EnsureType<size_t>(m1.vsize())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}

	typename result_type::iterator it = result.begin();
	for (size_t i = 0; i<m1.hsize(); ++i, ++it)
	{
		*it = sp(m1.col(i), v);
	}

	return result;

}


/*!
	\brief Multiplication of vector \a v and matrix \a m1, the vector comes to products from left.
	\tparam v First member of product.
	\tparam m1 Second member of product.
	\return  The result of vector to matrix product. The type of result taken from \a v when the function is called like r = vector_multiply_L(v,m1).

	Size of v should be the same as vertical size of m, otherwise exeption is thrown.
*/

template<XRAD__LinearVector_template, XRAD__MathMatrix_template1>
LinearVector<XRAD__LinearVector_template_args>	vector_multiply_L(
	const LinearVector<XRAD__LinearVector_template_args> &v,
	const MathMatrix<XRAD__MathMatrix_template_args1> &m1)
{
	return vector_multiply_L<LinearVector<XRAD__LinearVector_template_args>>(v, m1);
}



XRAD_END

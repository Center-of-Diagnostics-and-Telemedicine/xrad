/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/

XRAD_BEGIN


//--------------------------------------------------------------
//
//	элементарные действия
//

//	транспонирование другой матрицы (в дополнение к транспонированию себя, объявленному в DataArray2D

/*!
	\brief Transposition of matrix \a m1 with result written to itself.
	\tparam m1 The input matrix to be transposed.
	\return  The result of input matrix transposition. 

	Horizontal size of m1 should be the same as vertical size of itself, 
	the vertical size of m1 should be the same as horizontal size of itself, 
	otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1>
MathMatrix<XRAD__MathMatrix_template_args>	& MathMatrix<XRAD__MathMatrix_template_args>::transpose(const MathMatrix<XRAD__MathMatrix_template_args1> &m1)
	{
	if(	vsize() != m1.hsize() || hsize() != m1.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::transpose(m1) -- invalid arrays sizes: this(%zu,%zu), m1(%zu,%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize()),
				EnsureType<size_t>(m1.vsize()), EnsureType<size_t>(m1.hsize())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}
	for(size_t i = 0; i < vsize(); ++i)
		{
		row(i).CopyData(m1.col(i));
		}
	return *this;
	}

/*!
	\brief Calculation of the trace of matrix.
	\return  The trace of the matrix is returned.

	Horizontal size of the matrix should be the same as vertcal one, otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
T MathMatrix<XRAD__MathMatrix_template_args>::tr() const
	{
	if(	vsize() != hsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::tr() -- invalid matrix sizes: this(%zu,%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}

// 	size_t n = min(vsize(), hsize());
	value_type	result(value_type(0));

	for(size_t i = 0; i < vsize(); ++i)
		{
		result += at(i,i);
		}

	return result;
	}



//--------------------------------------------------------------
//
//	матричное умножение
//

// матрицы на матрицу

/*!
	\brief Multiplication of two rectangular matrixes \a m1 and \a m2 as member of MathMatrix class.
	\tparam m1 First member of matrix product.
	\tparam m2 Second member of matrix product.
	\return  The result of matrix product. 

	Vertical sizes of matrix itself should be equal to vertical size of m1, 
	horizontal size of matrix itself should be equal to horizontal size of m2, 
	horizontal size of m1 should be equal to vertical size of m2,
	otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
		const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
	{
	if(	vsize() != m1.vsize()|| hsize() != m2.hsize()||m1.hsize() != m2.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(m1,m2) -- invalid arrays sizes: this(%zu,%zu), m1(%zu,%zu), m2(%zu,%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize()),
				EnsureType<size_t>(m1.vsize()), EnsureType<size_t>(m1.hsize()),
				EnsureType<size_t>(m2.vsize()), EnsureType<size_t>(m2.hsize())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}

	for(size_t i=0; i<m1.vsize(); i++)
		{
		auto it = row(i).begin();
		for(size_t j=0; j<m2.hsize(); j++, ++it)
			{
			//*it = m1.row(i) | m2.col(j);
			m1.row(i).scalar_product(*it, m2.col(j));
			// скалярное умножение строки на столбец
			}
		}

	return *this;
	}

// матрицы на вектор(столбец), результат -- столбец в MathMatrix

/*!
	\brief Multiplication of rectangular matrix \a m1 and vector \a v as member of MathMatrix class, 
	the vector comes to product from right.
	\tparam m1 First member of product.
	\tparam v Second member of product.
	\return  The result of matrix to vector product.

	Vertical sizes of matrix itself and m1 should be equal, 
	horizontal size of matrix itself should be 1, 
	horizontal size of m1 should be the same as size of v, 
	otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
		const LinearVector<XRAD__MathMatrix_template_args2> &v)
	{
	if(	vsize() != m1.vsize()|| hsize() != 1 || m1.hsize() != v.size())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(m1,v) -- invalid arrays sizes: this(%zu,%zu), m1(%zu,%zu), v(%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize()),
				EnsureType<size_t>(m1.vsize()), EnsureType<size_t>(m1.hsize()),
				EnsureType<size_t>(v.size())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}

	auto it = col(0).begin();
	for(size_t i=0; i<m1.vsize(); ++i, ++it)
		{
		*it = m1.row(i) | v;
// 		at(i,0) = m1.row(i) | v;
		}

	return *this;
	}


// вектор (строка) на матрицу, результат -- строка в MathMatrix

/*!
	\brief Multiplication of vector \a v and rectangular matrix \a m2 as member of MathMatrix class, 
	the vector comes to product from left.
	\tparam v First member of product.
	\tparam m2 Second member of product.
	\return  The result of vector to matrix  product.

	Vertical size of the matrix itself should be 1, 
	horizontal sizes of matrix itself and horizontal size of m2 should be equal, 
	the size of v should be the same as vertical size of m2, 
	otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	& MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const LinearVector<XRAD__MathMatrix_template_args1> &v,
		const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
	{
	if(	vsize() != 1 || hsize() != m2.hsize()||v.size() != m2.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(v,m2) -- invalid arrays sizes: this(%zu,%zu), v(%zu), m2(%zu,%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize()),
				EnsureType<size_t>(v.size()),
				EnsureType<size_t>(m2.vsize()), EnsureType<size_t>(m2.hsize())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}

	auto it = row(0).begin();
	for(size_t j=0; j<m2.hsize(); j++, ++it)
		{
		//*it = v | m2.col(j);
		v.scalar_product(*it, m2.col(j));
		// скалярное умножение строки на столбец
		}

	return *this;
	}


// TODO Декларация этих методов находится в LinearVector, что неправильно. Переделать. Сделать эти методы не членами класса.
#if 0
// матрицы на вектор (в результате столбец в LinearVector)
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
LinearVector<XRAD__MathMatrix_template_args>	&LinearVector<XRAD__MathMatrix_template_args>::matrix_multiply(
		const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
		const LinearVector<XRAD__MathMatrix_template_args2> &v)
	{
	if(size() != m1.vsize() || m1.hsize() != v.size())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(m1,v) -- invalid arrays sizes: this(%d), m1(%d,%d), v(%d)",
			size(),
			m1.vsize(), m1.hsize(),
			v.size());
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}
	iterator it = begin();
	for(size_t i=0; i<m1.vsize(); ++i, ++it)
		{
//		*it = m1.row(i) | v;
		m1.row(i).scalar_product(*it, v);
		}

	return *this;
	}

// вектора на матрицу (в результате строка в LinearVector)

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
LinearVector<XRAD__MathMatrix_template_args>	& LinearVector<XRAD__MathMatrix_template_args>::matrix_multiply(
		const LinearVector<XRAD__MathMatrix_template_args1> &v,
		const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
	{
	if(size() != m2.hsize()||v.size() != m2.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(v,m2) -- invalid arrays sizes: this(%d), v(%d), m2(%d,%d)",
			size(),
			v.size(),
			m2.vsize(), m2.hsize());
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}

	iterator it = begin();
	for(size_t j=0; j<m2.hsize(); j++, ++it)
		{
		//*it = v | m2.col(j);
		// скалярное умножение строки на столбец
		v.scalar_product(*it, m2.col(j));
		}

	return *this;
	}
#endif

//	делает из вектора диагональную матрицу

/*!
	\brief The matrix is constructed from vector \a v as member of MathMatrix class.
	\tparam v Vector which elements are put to diagonal elements of matrix.
	\return  The matrix constructed.

	Vertical size of the matrix itself should be equal to horizontal one and to the size of vector,
	otherwise exeption is thrown.
*/

template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::diag(const LinearVector<XRAD__MathMatrix_template_args1> &v)
	{
	if(vsize() != hsize() || vsize() != v.size())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::diag(v) -- invalid arrays sizes: this(%zu,%zu), v(%zu)",
				EnsureType<size_t>(vsize()), EnsureType<size_t>(hsize()),
				EnsureType<size_t>(v.size())
			);
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}
	fill(value_type(0));
	auto it = v.begin();
	for(size_t i = 0; i < vsize(); ++i, ++it)
		{
// 		at(i,i) = value_type(v[i]);
		at(i,i) = value_type(*it);
		}
	return *this;
	}

//--------------------------------------------------------------

template<XRAD__MathMatrix_template>
void	PrintMatrix(const string title, const MathMatrix<XRAD__MathMatrix_template_args> &rf)
	{
	printf("\n%s\n", title.c_str());
	for(size_t i = 0; i < rf.hsize(); ++i) printf("----------");
	printf("\n");

	for(size_t i = 0; i < rf.vsize(); ++i)
		{
		for(size_t j = 0; j < rf.hsize(); ++j)
			{
			double v = rf.at(i,j);
			if(v) printf("%.2e, ", v);
			else printf("0,        ");
			}
		printf("\n");
		}
	for(size_t i = 0; i < rf.hsize(); ++i) printf("----------");
	printf("\n");
	fflush(stdout);
	}



// TODO: Эта функция должна быть в LinearVector.
template<XRAD__MathMatrix_template>
void	PrintVector(const LinearVector<XRAD__MathMatrix_template_args> &rf)
	{
	printf("\n");
	for(size_t i = 0; i < rf.size(); ++i) printf("----------------");
	printf("\n");

	for(size_t i = 0; i < rf.size(); ++i)
		{
		double v = rf[i];
		if(v) printf("%.2e,\t", v);
		else printf("0,       \t");
		}
	printf("\n");
	for(size_t i = 0; i < rf.size(); ++i) printf("----------------");
	printf("\n");
	fflush(stdout);
	}






XRAD_END

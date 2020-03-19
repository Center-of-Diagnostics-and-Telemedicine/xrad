
XRAD_BEGIN


//--------------------------------------------------------------
//
//	элементарные действия
//

//	транспонирование другой матрицы (в дополнение к транспонированию себя, объявленному в DataArray2D
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1>
MathMatrix<XRAD__MathMatrix_template_args>	& MathMatrix<XRAD__MathMatrix_template_args>::transpose(const MathMatrix<XRAD__MathMatrix_template_args1> &m1)
	{
	if(	vsize() != m1.hsize() || hsize() != m1.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::transpose(m1) -- invalid arrays sizes: this(%d,%d), m1(%d,%d)",
			vsize(), hsize(),
			m1.vsize(), m1.hsize());
		ForceDebugBreak();
		throw invalid_argument(problem_description);
		}
	for(size_t i = 0; i < vsize(); ++i)
		{
		row(i).CopyData(m1.col(i));
		}
	return *this;
	}

template <XRAD__MathMatrix_template>
T MathMatrix<XRAD__MathMatrix_template_args>::tr() const
	{
	if(	vsize() != hsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::tr() -- invalid matrix sizes: this(%d,%d)",
			vsize(), hsize());
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
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
		const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
	{
	if(	vsize() != m1.vsize()|| hsize() != m2.hsize()||m1.hsize() != m2.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(m1,m2) -- invalid arrays sizes: this(%d,%d), m1(%d,%d), m2(%d,%d)",
			vsize(), hsize(),
			m1.vsize(), m1.hsize(),
			m2.vsize(), m2.hsize());
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
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const MathMatrix<XRAD__MathMatrix_template_args1> &m1,
		const LinearVector<XRAD__MathMatrix_template_args2> &v)
	{
	if(	vsize() != m1.vsize()|| hsize() != 1 || m1.hsize() != v.size())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(m1,v) -- invalid arrays sizes: this(%d,%d), m1(%d,%d), v(%d)",
			vsize(), hsize(),
			m1.vsize(), m1.hsize(),
			v.size());
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
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
MathMatrix<XRAD__MathMatrix_template_args>	& MathMatrix<XRAD__MathMatrix_template_args>::matrix_multiply(
		const LinearVector<XRAD__MathMatrix_template_args1> &v,
		const MathMatrix<XRAD__MathMatrix_template_args2> &m2)
	{
	if(	vsize() != 1 || hsize() != m2.hsize()||v.size() != m2.vsize())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::matrix_multiply(v,m2) -- invalid arrays sizes: this(%d,%d), v(%d), m2(%d,%d)",
			vsize(), hsize(),
			v.size(),
			m2.vsize(), m2.hsize());
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
template <XRAD__MathMatrix_template>
template<XRAD__MathMatrix_template1>
MathMatrix<XRAD__MathMatrix_template_args>	&MathMatrix<XRAD__MathMatrix_template_args>::diag(const LinearVector<XRAD__MathMatrix_template_args1> &v)
	{
	if(vsize() != hsize() || vsize() != v.size())
		{
		string problem_description = typeid(self).name() +
			ssprintf("::diag(v) -- invalid arrays sizes: this(%d,%d), v(%d)",
			vsize(), hsize(),
			v.size());
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

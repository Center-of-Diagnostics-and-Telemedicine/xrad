#ifndef __SolveLinearSystem_h
#define __SolveLinearSystem_h

#include <XRADBasic/MathMatrixTypes.h>

//--------------------------------------------------------------
//
//	created:	2014/06/26
//	created:	26.6.2014   14:08
//	author:		kns
//
//--------------------------------------------------------------

XRAD_BEGIN

namespace SolveLinearSystemNS
{

template<class MM>
inline void PrepareSolutionMMImpl(MM &, AlgebraicStructures::FieldTagScalar *)
{
}

template<class MM>
inline void PrepareSolutionMMImpl(MM &m, AlgebraicStructures::FieldTagComplex *)
{
	m.conjugate();
}

template<XRAD__MathMatrix_template>
inline void PrepareSolution(MathMatrix<XRAD__MathMatrix_template_args> &m)
{
	PrepareSolutionMMImpl(m, (typename MathMatrix<XRAD__MathMatrix_template_args>::field_tag*)nullptr);
}

template<XRAD__MathMatrix_template, XRAD__MathMatrix_template1>
	void	SolveLinearSystemDestructive(MathMatrix<XRAD__MathMatrix_template_args> &solution, MathMatrix <XRAD__MathMatrix_template_args1> &matrix)
	{
	// простейший алгоритм решения линейной системы по методу гаусса:
	// получает матрицу размером (n,n+k);
	// на входе k правых столбцов содержат правую часть
	// на выходе в столбцах матрицы solution размером n x k находятся k векторов решений.
	// как-то учитывается возможное наличие нулевых коэффициентов на главной диагонали;
	// при несовместности системы дается исключение
	//
	// входная матрица разрушается.
	// эта функция используется в качестве основы для "недеструктивных" нижеобъявленных функций.
	// также ее можно использовать самостоятельно в тех случаях, когда сохранение
	// исходной матрицы не требуется, а дополнительное выделение памяти под буферы нежелательно.

	typedef	T value_type;

	size_t order = matrix.vsize();
	size_t	n_equations = solution.hsize();
	if(matrix.hsize() - order != n_equations || solution.vsize() != order)
		{
		string problem_description = ssprintf("SolveLinearSystemDestructive -- Invalid matrix dimensions (%d:%d) or solution size (%d,%d)", order, matrix.hsize(), solution.vsize(), solution.hsize());
		ForceDebugBreak();
		throw matrix_algorithm_error(problem_description);
		}

	for(size_t i = 0; i < order; ++i)
		{
		// проходим по очереди все столбцы и зануляем все элементы
		// кроме лежащего на главной диагонали.
		// i - номер очищаемого столбца

		if(!fast_norma(matrix.at(i,i)))
			{
			// учет нулевого значения на главной диагонали: находим наибольшее по модулю значение
			// в i-м столбце; соответствующую ему строку прибавляем к i-й строке.
			size_t max_position = 0;
			bool	inconsistent = false;
			if(i==order-1) inconsistent = true;
			else
				{
// 				MathFunctionF64	maxfinder;
				LinearVector<XRAD__MathMatrix_template_args1>	maxfinder;
				maxfinder.UseDataFragment(matrix.col(i), i+1, order, 1);
				double	max_abs = MaxAbsoluteValue(maxfinder, &max_position);
				if(!max_abs) inconsistent = true;
				}
			if(inconsistent)
					{
					// несовместность системы уравнений. кидается особое исключение
					string problem_description = ssprintf("SolveLinearSystemDestructive -- Linear equations system is inconsistent, matrix range is %d (order is %d)", i, order);
					ForceDebugBreak();
					throw matrix_algorithm_error(problem_description);
					}

			// прибавляем строку, содержащую наибольшее значение
			matrix.row(i) += matrix.row(max_position+i+1);
			}

		MathMatrix<XRAD__MathMatrix_template_args1>	sub;
		sub.UseDataFragment(matrix, 0, i+1, order, matrix.hsize());
		LinearVector<XRAD__MathMatrix_template_args1>	sub_decrement(matrix.hsize()-(i+1));

		for(size_t j = 0; j < order; ++j)
			{
			if(j!=i)
				{
				sub_decrement.CopyData(sub.row(i));
//				double factor = matrix[j,i]/matrix[i,i];
				value_type factor = matrix.at(j,i)/matrix.at(i,i);
				sub_decrement *= factor;
				sub.row(j) -= sub_decrement;
				matrix.at(j,i) = 0;
				}
			}
		}
	for(size_t i = 0; i < order; ++i)
		{
		value_type	factor = value_type(1.) / matrix.at(i,i);
		for(size_t j = 0; j < n_equations; ++j)
			{
			solution.at(i,j) = matrix.at(i,j+order) * factor;
			}
		}
	//для комплексных матриц получается сопряженное решение. "выпрямляем" его
	PrepareSolution(solution);
	}

template<XRAD__LinearVector_template, XRAD__MathMatrix_template1>
void	SolveLinearSystemDestructive(LinearVector<XRAD__LinearVector_template_args> &solution,
		MathMatrix <XRAD__MathMatrix_template_args1> &in_matrix)
	{
	// решается 1 линейная система из n уравнений.
	// in_matrix размером n x n+1 содержит матрицу n x n и столбец, задающий правую часть
	// в вектор solution размером n помещается решение

	MathMatrix<XRAD__MathMatrix_template_args1> matrix_solution;
	size_t	order = solution.size();
	matrix_solution.UseData(&solution.at(0), order, 1, solution.step(), order*solution.step());
	SolveLinearSystemDestructive(matrix_solution, in_matrix);
	}

}//namespace SolveLinearSystemNS



template<XRAD__MathMatrix_template, XRAD__MathMatrix_template1>
void	SolveLinearSystem(MathMatrix<XRAD__MathMatrix_template_args> &solution,
		const MathMatrix <XRAD__MathMatrix_template_args1> &in_matrix)
	{
	// решаются k линейных систем по n уравнений с одной матрицей.
	// in_matrix размером n x n+k содержит матрицу n x n и k столбцов, задающих правые части
	// в матрицу solution размером n x k помещаются столбцы с решениями
	MathMatrix <XRAD__MathMatrix_template_args1> matrix(in_matrix);
	SolveLinearSystemNS::SolveLinearSystemDestructive(solution, matrix);
	}


template<XRAD__LinearVector_template, XRAD__MathMatrix_template1>
void	SolveLinearSystem(LinearVector<XRAD__LinearVector_template_args> &solution,
		const MathMatrix <XRAD__MathMatrix_template_args1> &in_matrix)
	{
	// решается 1 линейная система из n уравнений.
	// in_matrix размером n x n+1 содержит матрицу n x n и столбец, задающий правую часть
	// в вектор solution размером n помещается решение

	MathMatrix <XRAD__MathMatrix_template_args1> matrix(in_matrix);
	SolveLinearSystemNS::SolveLinearSystemDestructive(solution, matrix);
	}

template<XRAD__MathMatrix_template, XRAD__MathMatrix_template1, XRAD__MathMatrix_template2>
void	SolveLinearSystem(MathMatrix<XRAD__MathMatrix_template_args> &solution,
		const MathMatrix <XRAD__MathMatrix_template_args1> &in_matrix,
		const MathMatrix <XRAD__MathMatrix_template_args2> &right_part)
	{
	// решаются k линейных систем по n уравнений с одной матрицей.
	// in_matrix размером n x n содержит матрицу n x n
	// right_part размером n x k и содержит k столбцов, задающих правые части
	// в матрицу solution размером n x k помещаются столбцы с решениями
	size_t order = in_matrix.vsize();
	size_t	n_equations = solution.hsize();
	if(in_matrix.hsize() != in_matrix.vsize() ||
			right_part.vsize() != order ||
			solution.vsize() != order ||
			right_part.hsize() != n_equations ||
			solution.hsize() != n_equations)
		{
		string problem_description = ssprintf("SolveLinearSystem -- Invalid matrix dimensions (%d:%d)  right part (%d,%d) or solution (%d,%d) size",
			in_matrix.vsize(), in_matrix.hsize(),
			right_part.vsize(), right_part.hsize(),
			solution.vsize(), solution.hsize());
		ForceDebugBreak();
		throw matrix_algorithm_error(problem_description);
		}
	// матрица и правая часть могут быть действительными или комплексными в любых сочетаниях,
	// единственное условие, чтобы контейнер решения был с ними совместим
	MathMatrix <XRAD__MathMatrix_template_args>	matrix(order, order+n_equations);

	for(size_t i = 0; i < order; ++i)
		{
		matrix.col(i).CopyData(in_matrix.col(i));
		}
	for(size_t i = 0; i < n_equations; ++i)
		{
		matrix.col(i+order).CopyData(right_part.col(i));
		}

	SolveLinearSystemNS::SolveLinearSystemDestructive(solution, matrix);
	}

template<XRAD__LinearVector_template, XRAD__MathMatrix_template1, XRAD__LinearVector_template2>
void	SolveLinearSystem(LinearVector<XRAD__LinearVector_template_args> &solution,
		const MathMatrix <XRAD__MathMatrix_template_args1> &in_matrix,
		const LinearVector <XRAD__LinearVector_template_args2> &right_part)
	{
	// решаются k линейных систем по n уравнений с одной матрицей.
	// in_matrix размером n x n содержит матрицу n x n
	// right_part размером n x k и содержит k столбцов, задающих правые части
	// в матрицу solution размером n x k помещаются столбцы с решениями
	size_t order = in_matrix.vsize();
	if(in_matrix.hsize() != in_matrix.vsize() ||
			right_part.size() != order ||
			solution.size() != order)
		{
		string problem_description = ssprintf("SolveLinearSystem -- Invalid matrix dimensions (%d:%d)  right part (%d) or solution (%d) size",
			in_matrix.vsize(), in_matrix.hsize(),
			right_part.size(), solution.size());
		ForceDebugBreak();
		throw matrix_algorithm_error(problem_description);
		}
	// матрица и правая часть могут быть действительными или комплексными в любых сочетаниях,
	// единственное условие, чтобы контейнер решения был с ними совместим
	MathMatrix <XRAD__MathMatrix_template_args>	matrix(order, order+1);

	for(size_t i = 0; i < order; ++i)
		{
		matrix.col(i).CopyData(in_matrix.col(i));
		}
	matrix.col(order).CopyData(right_part);

	MathMatrix<XRAD__MathMatrix_template_args> matrix_solution;
	matrix_solution.UseData(&solution.at(0), order, 1, solution.step(), order*solution.step());
	SolveLinearSystemNS::SolveLinearSystemDestructive(solution, matrix);
	}

XRAD_END

#endif //__SolveLinearSystem_h
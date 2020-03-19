#include "pre.h"
#include "ProgressIndicatorScheduler.h"


/*!
	\file
	\date 2019/02/05 17:32
	\author kulberg

	\brief
*/

XRAD_BEGIN

ProgressIndicatorScheduler::ProgressIndicatorScheduler(const vector<ProgressOperation> &ops, size_t n_displayed_steps) :
	m_operations_boundaries(ops.size() + 1, 0),
	m_operations(ops),
	m_n_displayed_steps(n_displayed_steps)
{
	ComputeBoundaries();
}


ProgressIndicatorScheduler::ProgressIndicatorScheduler(const vector<double> &costs, size_t n_displayed_steps) :
	m_operations_boundaries(costs.size() + 1, 0),
	m_operations(costs.size()),
	m_n_displayed_steps(n_displayed_steps)
{
	for (size_t i = 0; i < size(); ++i)
	{
		m_operations[i].cost = costs[i];
		m_operations[i].name = ssprintf(L"%d", int(i));
		m_operations[i].used = true;
	}
	ComputeBoundaries();
}


void ProgressIndicatorScheduler::ComputeBoundaries()
{
	m_operations_boundaries[0] = 0;
	for(size_t i = 1; i < m_operations_boundaries.size(); ++i)
	{
		if(m_operations[i-1].used)
		{
			m_operations_boundaries[i] = m_operations_boundaries[i-1] + m_operations[i-1].cost;
		}
		else
		{
			m_operations_boundaries[i] = m_operations_boundaries[i - 1];
		}
	}
	double	factor = double(m_n_displayed_steps)/m_operations_boundaries.back();
	for(size_t i = 1; i < m_operations_boundaries.size(); ++i)
	{
		m_operations_boundaries[i] *= factor;
		m_operations[i-1].cost *= factor;
	}
}


void ProgressIndicatorScheduler::SetUsage(size_t n, bool u)
{
	m_operations[n].used = u;
	ComputeBoundaries();
}


void ProgressIndicatorScheduler::set_operation_cost(size_t n, double new_c)
{
	m_operations[n].cost = new_c;
	ComputeBoundaries();
}


XRAD_END

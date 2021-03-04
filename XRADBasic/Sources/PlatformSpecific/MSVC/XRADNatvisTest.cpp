/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file XRADNatvisTest.cpp
// Created by ACS on 26.08.16
//--------------------------------------------------------------
#include "pre.h"
#include "XRADNatvisTest.h"

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/MathFunctionTypesMD.h>

#ifndef XRAD_COMPILER_MSC
	#error This file is for MS Visual Studio only
#endif

#pragma warning (disable:4305)
#pragma warning (disable:4189)

XRAD_BEGIN

//--------------------------------------------------------------

void XRADNatvisTest()
{
	// Sample: STL vector
	vector<int> values;
	values.push_back(22);
	values.push_back(33);

	// DataOwner<T> (1D)
	RealFunctionF32 rf_empty;
	RealFunctionF32 rf_10(10, 77);
	rf_10[0] = 10;
	rf_10[1] = 20;
	rf_10[2] = 30;
	rf_10[3] = 40;
	rf_10[4] = 50;
	RealFunctionF32 rf_1000(1000, 7);
	rf_1000[2] = 22;
	RealFunctionF32 rf_sparse;
	rf_sparse.UseData(rf_10.data() + rf_10.size() - 1, rf_10.size()/2, -2);
	RealFunctionF32 rf_ext_64bit_step;
// 	rf_ext_64bit_step.UseData(rf_1000.data(), 1, 0x1000000000ull);
	rf_ext_64bit_step.UseData(rf_1000.data(), 1, 1);

	// Iterators
	auto it = rf_10.begin();
	auto rev_it = rf_10.rbegin();
	auto it_sparse = rf_sparse.begin();
	auto it_sparse_next = it_sparse + 1;
	auto rev_it_sparse = rf_sparse.rbegin();

	// FixedSizeArray<T,N>
	FixedSizeArray<int, 4> fs4_empty;
	(void)fs4_empty;

	// Points and ranges
	point_2<float,double,AlgebraicStructures::FieldTagScalar> sample_point2(15,16);
	point_3<float,double,AlgebraicStructures::FieldTagScalar> sample_point3(15,16,17);
	range_1<float,double> sample_range1(1,2);
	range_2<float,double> sample_range2(-1,-2,3,4);
	range_3<float,double> sample_range3(1,2,3,4,5,6);

	// DataArray2D<T>
	RealFunction2D_F32 rf2_empty;
	RealFunction2D_F32 rf2_2_3(2, 3, 77);
	rf2_2_3.row(0)[0] = 1;
	rf2_2_3.row(0)[1] = 2;
	rf2_2_3.row(1)[0] = 11;
	rf2_2_3.row(1)[1] = 12;
	RealFunction2D_F32 rf2_ext;
	rf2_ext.UseData(&rf2_2_3.at(0,rf2_2_3.hsize()-1), rf2_2_3.vsize(), rf2_2_3.hsize(), rf2_2_3.hsize(), -1);

	// Многомерные массивы
	RealFunctionMD_F32 rfmd_empty;
	index_vector mdi4_dims(4);
	mdi4_dims[0] = 2;
	mdi4_dims[1] = 3;
	mdi4_dims[2] = 4;
	mdi4_dims[3] = 5;
	RealFunctionMD_F32 rfmd(mdi4_dims, 77);
	index_vector mdi4_index(4);
	mdi4_index.fill(0);
	rfmd.at(mdi4_index) = 1;
	mdi4_index[2] = 1;
	rfmd.at(mdi4_index) = 1.1;
	mdi4_index[2] = 2;
	rfmd.at(mdi4_index) = 1.2;
	mdi4_index[2] = 3;
	rfmd.at(mdi4_index) = 1.3;
	mdi4_index[0] = 1;
	mdi4_index[1] = 2;
	mdi4_index[2] = 3;
	mdi4_index[3] = 0;
	rfmd.at(mdi4_index) = 33;

	// Физические единицы
	physical_frequency freq(Hz(123));
	physical_length	len=cm(2.5);
	physical_speed	sp=cm_sec(1);
	physical_time	t = sec(1);
	physical_angle	a = degrees(180);

	// Место для точки останова отладчика
	ForceDebugBreak();
	//int jjj=0;
}

//--------------------------------------------------------------

XRAD_END

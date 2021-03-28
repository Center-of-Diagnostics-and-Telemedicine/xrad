/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__sample_types
#define XRAD__sample_types
/*!
	\file
	\brief Определение типов для единичных отсчетов данных (действительные и комплексные числа, пикселы)
*/
//--------------------------------------------------------------

#include "Sources/Core/Limits.h"
#include "Sources/Core/NumberTraits.h"
#include "Sources/SampleTypes/HomomorphSamples.h"
#include "Sources/SampleTypes/ComplexSample.h"
#include "Sources/SampleTypes/BooleanSample.h"
#include "Sources/SampleTypes/ColorSample.h" //этого не следует здесь включать, т.к. зависит от вектора

#include <cstdint>

//--------------------------------------------------------------
#endif // XRAD__sample_types

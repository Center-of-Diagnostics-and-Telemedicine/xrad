﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file BooleanFunctionTypes.h
//--------------------------------------------------------------
#ifndef XRAD__File_BooleanFunctionTypes_h
#define XRAD__File_BooleanFunctionTypes_h
//--------------------------------------------------------------

#include "Sources/Containers/BooleanFunction.h"
#include "Sources/Containers/BooleanFunction2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

typedef BooleanFunctionLogical<int8_t> BooleanFunctionLogical8;
typedef BooleanFunctionLogical2D<int8_t> BooleanFunctionLogical2D8;

//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_BooleanFunctionTypes_h

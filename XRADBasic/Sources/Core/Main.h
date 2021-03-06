﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file Main.h
//--------------------------------------------------------------
#ifndef XRAD__Core_Main_h
#define XRAD__Core_Main_h
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"

XRAD_BEGIN

//! \brief Прототип главной функции приложения.
//! Аргументы и возвращаемое значение такие же, как у функции main
int xrad_main(int in_argc, char *in_argv[]);

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__Core_Main_h

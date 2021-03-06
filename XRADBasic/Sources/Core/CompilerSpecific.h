﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file CompilerSpecific.h
#ifndef CompilerSpecific_h__
#define CompilerSpecific_h__
//--------------------------------------------------------------
/*!
	\defgroup gr_CompilerSpecific Настройки, зависящие от компилятора
	@{

	\file
	\brief Настройки библиотеки XRAD, зависящие от компилятора (платформы)

	Внутренний файл библиотеки.

	Посредством подключения данного файла определяются параметры целевой платформы (компилятора),
	которые невозможно определить универсальным способом, а также настройки
	библиотеки под целевую платформу (компилятор).

	Файл состоит из нескольких секций, предназначенных для разных компиляторов
	(зависящих от компилятора),
	которые подключают заголовочные файлы с определениями под конкретные компиляторы.

	В конце включается файл <CompilerSpecificQC.h>, в котором проверяется, что все необходимые
	определения были сделаны в секции текущего используемого компилятора.

	Для нестандартных конфигураций можно определить на уровне опций компилятора
	макрос XRAD_CustomCompiler, тогда вместо секции текущего компилятора будет
	использована секция нестандартных конфигураций, которая включает файл
	<CustomCompiler.h>. Этот файл в библиотеку XRAD не входит.
	Он должен быть создан вне библиотеки XRAD по образу одного из входящих
	в XRAD файлов настроек для компилятора или с нуля, ориентируясь
	на проверяемые условия и комментарии в файле <CompilerSpecificQC.h>.
*/
//--------------------------------------------------------------

#include "Endian.h"

//--------------------------------------------------------------
// Начало блока секций, зависящих от компилятора
//--------------------------------------------------------------

#ifdef XRAD_CustomCompiler
//--------------------------------------------------------------
//
// Для нестандартных конфигураций библиотеки, обычно не используется
//
//--------------------------------------------------------------

#include <CustomCompiler.h>



#elif defined(_MSC_VER)
//--------------------------------------------------------------
//
// Настройки для Visual C++ (targets: Win32 x86, x64)
//
//--------------------------------------------------------------

#if defined(_M_IX86) || defined(_M_X64)
#include <XRADBasic/Sources/PlatformSpecific/MSVC/Internal/MSVCCompilerSpecific.h>
#else
#error "Current platform in not supported for Visual C++."
#endif



#elif defined(__GNUC__)
//--------------------------------------------------------------
//
// Настройки для GNU C++
//
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/gcc/GCCCompilerSpecific.h>



#elif defined(__TI_COMPILER_VERSION__)
//--------------------------------------------------------------
//
// TI Code Composer Studio (targets: DSP)
//
//--------------------------------------------------------------

#include <TI_CCS_Specific.h>



#else
//--------------------------------------------------------------
//
// Unsupported compiler
//
//--------------------------------------------------------------

#if defined(__MWERKS__)
	// Metrowerks CodeWarrior (targets: MacOS)
	// Также делалась проверка: #if defined(__ppc__)
	// Использовался заголовочный файл: <Mac_CWSpecific.h>
	#error "Metrowerks CodeWarrior compiler no longer supported"
#elif defined(__SC__)
	// Mac - Symantec C++
	#error "Symantec C++ compiler no longer supported"
#elif defined(__BORLANDC__)
	// PC - Borland C++ Builder
	// Historical include file: <PCBcbSpecific.h>
	#error "Borland C++ Builder compiler no longer supported"
#else
	#error "Unknown compiler, not supported"
#endif



#endif

//--------------------------------------------------------------
// Конец блока секций, зависящих от компилятора
//--------------------------------------------------------------


//--------------------------------------------------------------
// Секция проверки наличия необходимых определений, зависящих от компилятора
//--------------------------------------------------------------

#include "CompilerSpecificQC.h"

//--------------------------------------------------------------
//! @} <!-- ^group gr_CompilerSpecific -->
#endif // CompilerSpecific_h__

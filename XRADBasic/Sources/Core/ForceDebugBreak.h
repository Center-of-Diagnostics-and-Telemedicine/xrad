/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef ForceDebugBreak_h__
#define ForceDebugBreak_h__
/*!
	\addtogroup gr_Debug
	@{

	\file
	\brief Определение макроса ForceDebugBreak()

	Внутренний файл библиотеки.
*/

//! \brief Макро-функция для выхода в отладчик в DEBUG-режиме
#if true //@@@def XRAD_DEBUG
	#define ForceDebugBreak()	XRAD_FORCE_DEBUG_BREAK_FUNCTION()
#else
//	#define ForceDebugBreak()
//	В случае "if(error_condition) ForceDebugBreak();" объявленная выше конструкция приводит к ненужным предупреждениям
//	warning C4390: ';': empty controlled statement found; is this the intent?
//	Заменяю на inline функцию
	inline void ForceDebugBreak(){}
#endif //XRAD_DEBUG

//! @} <!-- ^group gr_Debug -->
#endif // ForceDebugBreak_h__
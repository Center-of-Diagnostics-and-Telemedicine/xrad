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
#ifdef _DEBUG
	#define ForceDebugBreak()	XRAD_FORCE_DEBUG_BREAK_FUNCTION()
#else
//	#define ForceDebugBreak()
//	В случае "if(error_condition) ForceDebugBreak();" объявленная выше конструкция приводит к ненужным предупреждениям 
//	warning C4390: ';': empty controlled statement found; is this the intent?
//	Заменяю на inline функцию
	inline void ForceDebugBreak(){}
#endif //_DEBUG

//! @} <!-- ^group gr_Debug -->
#endif // ForceDebugBreak_h__
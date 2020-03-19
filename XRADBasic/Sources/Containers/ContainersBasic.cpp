#include "pre.h"
#include "ContainersBasic.h"

// в ContainerBasic не должно быть никаких
// обращений к интерфейсу вроде вызовов Error и т.п.
// должен быть только останов в отладчике.
// быть может, еще запись сообщения об ошибке в лог-файл.
// стоит еще обдумать это.

XRAD_BEGIN

namespace internal_ContainerChecksControl
{

//--------------------------------------------------------------
//
//	Нижеобъявленные функции входят в слинкованную библиотеку XRAD.
//	Настоящие названия этих функций переопределены с помощью макросов,
//	которые зависят от заданных опций проверки.
//	Их вызывает inline функция InitFlowControl. Если опции компилятора
//	в библиотеке и в приложении не совпали, линкер не может найти соответствующую функцию.
//
//--------------------------------------------------------------

int	InitNANCheck()
{
	return XRAD_CHECK_NAN_FLOAT;
}

int	InitArrayBoundariesCheck()
{
	return XRAD_CHECK_ARRAY_BOUNDARIES << 8;
}

int	InitIteratorBoundariesCheck()
{
	return XRAD_CHECK_ITERATOR_BOUNDARIES << 16;
}

//--------------------------------------------------------------
} // namespace internal_ContainerChecksControl
XRAD_END

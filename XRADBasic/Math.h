/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef Math_h__
#define Math_h__

/*!
	\file
	\date 2018/02/15 10:13
	\author kulberg

	\brief
*/

/*
Общие соображения при добавлении этого файла. Предлагаю обсудить.

Думаю, что пользовательский код не должен знать пути далее	XRADBasic/.
Всем подпапкам XRAD должно соответствовать одноименные включения.
Подпапки, для которых включений не предусмотрено, таким образом, можно
запрятать еще глубже.

С этой точки зрения получается такая картина. Возможно использование контейнеров
без математики, это XRADBasic/Containers(.h)

Контейнеры с математикой, это XRADBasic/MathFunction(.h). Разные заголовки для размерностей слить в один.

Algebra напрямую не включается, она подчинена MathFunction, значит, поместить ее внутрь каталога.

По структуре. В Math поместить еще StatisticUtils
*/

#include "Sources/Math/SpecialFunctions.h"

XRAD_BEGIN

XRAD_END

#endif // Math_h__

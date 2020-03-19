/*!
	\file
	\date 2018/03/02 15:31
	\author kulberg

	\brief
*/
#ifndef CreateInstance_h__
#define CreateInstance_h__


#include "instance.h"

#include <XRADDicom/DicomClasses/Instances/LoadGenericClasses.h>

XRAD_BEGIN

namespace Dicom
{

	/*!
		Создание наследников полиморфного контейнера Dicom::instance_ptr с опорой на информацию о модальности в файле Dicom
		В случае успеха возвращает instance соответствующего типа с данными для однозначной идентификации и открытым файлом. Далее пользователь сам решает что с этим делать.
		В противном случае возвращает NULL.
	*/
	instance_ptr	CreateInstance(const instancestorage_ptr &inst_src, const dicom_instance_predicate &instance_predicate_p);
	instance_ptr	CreateInstancePreIndexed(const instancestorage_ptr &inst_src, const dicom_instance_predicate &instance_predicate_p);

}//namespace Dicom

XRAD_END

#endif // CreateInstance_h__

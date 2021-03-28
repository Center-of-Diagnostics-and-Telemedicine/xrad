/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef FileSaveUtils_h__
#define FileSaveUtils_h__
/********************************************************************
	created:	2015/05/13
	created:	13:5:2015   22:41
	author:		kns
*********************************************************************/

#include "XRADGUIAPIDefs.h"
#include <QtCore/QString>

namespace XRAD_GUI
{

void	SaveDefaultPath(file_dialog_mode flag, QString path);
QString GetDefaultPath(file_dialog_mode flag);

QString	GetSaveFileName(const QString &prompt, const QString &types);

bool IsRasterImageFormat(const QString &format);
bool IsVectorImageFormat(const QString &format);
QString FormatByFileExtension(const QString &file_name);

}//namespace XRAD_GUI

#endif // FileSaveUtils_h__

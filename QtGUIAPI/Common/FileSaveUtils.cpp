#include "pre_GUI.h"
#include "FileSaveUtils.h"
#include "SavedSettings.h"

/********************************************************************
	created:	2015/05/13
	created:	13:5:2015   22:40
	author:		kns
*********************************************************************/

namespace XRAD_GUI
{



QString	GetSaveFileName(const QString &prompt, const QString &type)
	{
	QString currentPath = GetDefaultPath(file_save_dialog);

	QString file_name = QFileDialog::getSaveFileName(0, prompt, currentPath, type);

	if (!file_name.isEmpty())
	{
		SaveDefaultPath(file_save_dialog, QFileInfo(file_name).absolutePath());
	}
	return file_name;
	}


QString FormatByFileExtension(const QString &file_name)
	{
	int	len = file_name.length();
	int	dot_position(0);
	for(int i = len-1; i>=0 && file_name[i]!='.'; --i)
		{
		dot_position = i;
		}

	if(!dot_position) return QString();// нету расширения

	QString	result(len-dot_position, '0');

	std::copy(file_name.begin() + dot_position, file_name.end(), result.begin());
	return result;
	}


bool IsVectorImageFormat(const QString &format)
	{
	if(format == "svg") return true;
	if(format == "pdf") return true;
	if(format == "ps") return true;
//	if(format == "eps") return true;

	return false;
	}


bool IsRasterImageFormat(const QString &format)
	{
	if(format == "png") return true;
	if(format == "jpg") return true;
	if(format == "jpeg") return true;
	if(format == "tif") return true;
	if(format == "tiff") return true;
	if(format == "bmp") return true;

	return false;
	}



}//namespace XRAD_GUI

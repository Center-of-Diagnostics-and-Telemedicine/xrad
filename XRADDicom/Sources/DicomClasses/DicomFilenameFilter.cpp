#include "pre.h"
#include "DicomFilenameFilter.h"

XRAD_BEGIN


wstring	extension(const wstring &name)
{
	ptrdiff_t	i = name.size()-1;
	size_t	dot_position = i;
	bool	dot_found(false);
	bool	slash_found(false);

	while(i>=0 && !dot_found && !slash_found)
	{
		switch(name[i])
		{
			case L'.':
				dot_found = true;
				slash_found = true;
				dot_position = i;
				break;

			case L'\\':
			case L'/':
				slash_found = true;
				dot_found = false;
				break;
		}
		--i;
	}
	if(dot_found) return wstring(name.begin() + dot_position, name.end());
	else return wstring();
}

bool	may_be_dicom_filename(const wstring &name)
{
	// не рискую исключать все кроме ".DCM" и ".", поэтому пишется список того, что встречалось в dicom каталогах,
	// не являясь при этом dicom файлом. если их открывать и анализировать содержимое, это замедляет поиск.
	static const vector<wstring>	dicom_extensions =
	{
		L".dcm"
	};
	static const vector<wstring>	non_dicom_extensions =
	{
		L".avi",
		L".bat",
		L".bin",//?
		L".bmp",
		L".cab",
		L".chm",
		L".cmd",
		L".command",
		L".c",
		L".cpp",
		L".css",
		L".dat",//?
		L".dic",
		L".dll",
		L".gz",
		L".doc",
		L".docx",
		L".exe",
		L".h",
		L".hdr",
		L".hpp",
		L".gif",
		L".htm",
		L".html",
		L".inf",
		L".ini",
		L".jar",
		L".jpg",
		L".log",
		L".mkv",
		L".mpg",
		L".note",
		L".par",
		L".pct",
		L".pdf",
		L".png",
		L".ppt",
		L".pptx",
		L".reg",
		L".res",
		L".raw",
		L".rtf",
		L".sh",
		L".ssm",
		L".tif",
		L".tiff",
		L".txt",
		L".tmp",
		L".temp",
		L".xml",
		L".xls",
		L".xlsx",
		L".wmv",
		L".zip"
	};
	wstring	ext = extension(name);

	if(ext.length() == 0)
	{
		return true;
	}

	tolower(ext);

	for(auto de: dicom_extensions) if(de==ext)
	{
		return true;
	}
	for(auto nde: non_dicom_extensions) if(nde==ext)
	{
		return false;
	}

	return true;
}

XRAD_END

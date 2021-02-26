/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_DicomFilenameFilters_h
#define XRAD__File_DicomFilenameFilters_h

#include <XRADBasic/Core.h>

XRAD_BEGIN

/*
фильтр, исключающий из рассмотрения файлы, заведомо не являющиеся dicom.
сохраняются файлы без расширения и с расширением .dcm.

NB ремарка отсюда: http://www.saravanansubramanian.com/Saravanan/Articles_On_Software/Entries/2014/9/28_DICOM_Basics_-_Making_Sense_of_the_DICOM_File.html

The DICOM standard restricts the file names/identifiers contained within to 8 characters 
(either uppercase alphabetic characters and numbers only) to keep in conformity with legacy/historical 
requirements. It also states that no information must be inferred/extracted from these names. The file
names usually don’t have a .dcm extension when they are stored as part of a media such as CD or DVD. I use
longer names to keep these details from being a distraction right now, but I still want to mention what 
the standard states here so that no confusion arises as a result.

*/

bool	may_be_dicom_filename(const wstring &name);

XRAD_END

#endif //XRAD__File_DicomFilenameFilters_h

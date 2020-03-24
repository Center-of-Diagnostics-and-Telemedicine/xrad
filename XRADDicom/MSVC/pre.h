#ifndef XRAD__File_XRADDicom_pre_h
#define XRAD__File_XRADDicom_pre_h
//--------------------------------------------------------------

// XRAD
#include <XRADBasic/Core.h>
#include <XRADSystem/System.h>

//--------------------------------------------------------------

// DCMTK

#ifdef UNICODE
	#error You shoud undefine this macro in project settings
	// dcmtk не работает с определённым UNICODE:
	// http://support.dcmtk.org/docs/file_install.html
	// http://forum.dcmtk.org/viewtopic.php?f=3&t=3882
#endif

// общий для заголовков dcmtk заголовок
#include <dcmtk/config/osconfig.h>

// DcmFileFormat
#include <dcmtk/dcmdata/dcfilefo.h>
// теги
#include <dcmtk/dcmdata/dcdeftag.h>
// нужен для извлечения изображения из dicom файла
#include <dcmtk/dcmimgle/dcmimage.h>

// for compressed image

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcpxitem.h>
// jpeg
#include <dcmtk/dcmjpeg/djdecode.h>
// JPEG-LS
#include <dcmtk/dcmjpls/djdecode.h>

// for compress image
#include <dcmtk/dcmjpeg/djencode.h>
#include <dcmtk/dcmjpeg/djrplol.h>

// словарь тегов
#include <dcmtk/dcmdata/dcdict.h>

// network part

// Covers most common dcmdata classes
// https://support.dcmtk.org/redmine/projects/dcmtk/wiki/Howto_DcmSCU
#include <dcmtk/dcmnet/scu.h>
// StoreSCP receive data from PACS
#include <dcmtk/dcmnet/dstorscp.h>
// StoreSCU send data to PACS
#include <dcmtk/dcmnet/dstorscu.h>
// generate UUID
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmnet/diutil.h>

#ifndef XRAD_NO_LIBRARIES_LINKS
#ifdef XRAD_COMPILER_MSC
#include <XRADDicom/Sources/PlatformSpecific/MSVC/MSVC_DCMTKLink.h>
#endif // XRAD_COMPILER_MSC
#endif // XRAD_NO_LIBRARIES_LINKS

//--------------------------------------------------------------

// openjpeg for decode/encode JPEG2000

extern "C"
{
#include <openjpeg.h>
}

//--------------------------------------------------------------
#endif // XRAD__File_XRADDicom_pre_h

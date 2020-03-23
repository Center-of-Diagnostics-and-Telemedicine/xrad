#ifndef pre_XRADDicom_h__
#define pre_XRADDicom_h__

#include <XRADBasic/Core.h>
#include <XRADSystem/System.h>

/**************************************************************
DCMTK, OpenJPEG
**************************************************************/
/**************
libs
***************/
#ifdef XRAD_COMPILER_MSC
	#include <XRAD/PlatformSpecific/MSVC/PC_DCMTKLink.h>
#endif

/*********
headers
**********/
#ifdef UNICODE
	#error You shoud undefine this macro in project settings
//#undef UNICODE // dcmtk не работает с определённым UNICODE http://support.dcmtk.org/docs/file_install.html  http://forum.dcmtk.org/viewtopic.php?f=3&t=3882
// поэтому отключаем
#endif

#include "dcmtk/config/osconfig.h" // общий для заголовков dcmtk заголовок

#include "dcmtk/dcmdata/dcfilefo.h"  // DcmFileFormat
#include "dcmtk/dcmdata/dcdeftag.h" // теги
#include "dcmtk/dcmimgle/dcmimage.h" // нужен для извлечения изображения из dicom файла

//for compressed image
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcpxitem.h"
#include "dcmtk/dcmjpeg/djdecode.h" //jpeg
#include "dcmtk/dcmjpls/djdecode.h" //JPEG-LS

//for compress image
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"

//словарь тегов
#include "dcmtk/dcmdata/dcdict.h"

//network part
#include <dcmtk/dcmnet/scu.h>     /* Covers most common dcmdata classes */ // https://support.dcmtk.org/redmine/projects/dcmtk/wiki/Howto_DcmSCU
#include <dcmtk/dcmnet/dstorscp.h> //StoreSCP receive data from PACS
#include <dcmtk/dcmnet/dstorscu.h> //StoreSCU send data to PACS
#include <dcmtk/dcmdata/dcuid.h> //generate UUID
#include <dcmtk/dcmnet/diutil.h>

//openjpeg for decode/encode JPEG2000
extern "C" {
#include "openjpeg.h"
}

#endif //pre_XRADDicom_h__

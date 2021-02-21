#include "pre.h"

#include "xrad_fstream.h"
#include "FileNameOperations.h"

XRAD_BEGIN

//--------------------------------------------------------------

#if defined(XRAD_USE_FILENAMES_WIN32_VERSION)
wstring xrad_fstream_ConvertFilename(const string &filename)
{
	return GetPathSystemRawFromAutodetect(convert_to_wstring(filename));
}
#elif defined(XRAD_USE_FILENAMES_UNIX_VERSION)
string xrad_fstream_ConvertFilename(const string &filename)
{
	return convert_to_string(GetPathSystemRawFromAutodetect(convert_to_wstring(filename)));
}
#else
#error Unknown XRAD filename configuration.
#endif

//--------------------------------------------------------------

namespace
{
void type_check()
{
	fstream fs("");
	fstream fs2("", ios_base::in);
	fs.open("", ios_base::in);
	fs.open("");
}
}

//--------------------------------------------------------------

XRAD_END

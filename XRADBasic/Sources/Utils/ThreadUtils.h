// file ThreadUtils.h
//--------------------------------------------------------------
#ifndef XRAD__ThreadUtils_h
#define XRAD__ThreadUtils_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <string>
#include <thread>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

string ToString(std::thread::id id);

string DebugThreadIdStr();

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__ThreadUtils_h

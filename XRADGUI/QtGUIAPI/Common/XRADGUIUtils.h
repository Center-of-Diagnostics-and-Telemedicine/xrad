// file XRADGUIUtils.h
//--------------------------------------------------------------
#ifndef __XRADGUIUtils_h
#define __XRADGUIUtils_h
//--------------------------------------------------------------

#include <string>

namespace XRAD_GUI
{

using namespace std;

//! \brief Получить строку сообщения исключения в блоке catch
//!
//! Допускается вызывать только изнутри блока catch.
//! В противном случае программа аварийно завершится.
string GetGUIExceptionString();

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // __XRADGUIUtils_h

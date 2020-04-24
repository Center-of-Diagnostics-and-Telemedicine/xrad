#include "pre.h"
#include "Keyboard_Win32.h"

#ifdef XRAD_USE_KEYBOARD_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <XRADGUI/Sources/GUI/ModifierKeys.h>
#include <windows.h>

XRAD_BEGIN

//--------------------------------------------------------------

int	ModifierKeysState_MS()
{
	static const unsigned short pressed_mask = 1u<<15;
	static const unsigned short toggled_mask = 0x01;

	int	result = 0;

	if (GetKeyState(VK_SHIFT) & pressed_mask) result |= modkeyShift;
	if (GetKeyState(VK_CONTROL) & pressed_mask) result |= modkeyControl;
	if (GetKeyState(VK_MENU) & pressed_mask) result |= modkeyAlt;
	if (GetKeyState(VK_CAPITAL)& toggled_mask) result |= modkeyLock;

	if (
		GetKeyState(VK_LWIN) & pressed_mask ||
		GetKeyState(VK_RWIN) & pressed_mask
		)
		result |= modkeyCommand;

	return result;
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_KEYBOARD_WIN32_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_Keyboard_Win32() {}
XRAD_END

#endif // XRAD_USE_KEYBOARD_WIN32_VERSION

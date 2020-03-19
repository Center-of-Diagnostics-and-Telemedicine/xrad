#include "pre_GUI.h"
#include "Keyboard.h"
#include "GUIConfig.h"

#if defined(XRAD_USE_KEYBOARD_WIN32_VERSION)

#include <XRAD/PlatformSpecific/MSVC/Internal/Keyboard_Win32.h>

#elif defined(XRAD_USE_KEYBOARD_QT_VERSION)

#include <XRAD/PlatformSpecific/Qt/Internal/SystemUtils_Qt.h>

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)
#else
#error No std implementation for Keyboard.
#endif

XRAD_BEGIN

//--------------------------------------------------------------

#if defined(XRAD_USE_KEYBOARD_WIN32_VERSION)

namespace
{
auto api_ModifierKeysState = ModifierKeysState_MS;
} // namespace

#elif defined(XRAD_USE_KEYBOARD_QT_VERSION)

namespace
{
auto api_ModifierKeysState = ModifierKeysState_Qt;
} // namespace

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)

namespace
{
int api_ModifierKeysState()
{
	return 0;
}
} // namespace

#else

#error No std implementation for Keyboard.

#endif

//--------------------------------------------------------------

//	TODO: комментарий привести в порядок и поставить на место
//	int	api_ModifierKeysState();
// 	/// считывает состояние клавиш control, shift, alt, win, capslock
// 	/// и возвращает их в виде маски ModifierKeys
// bool api_ModifierKeysPressed( int keys_mask, bool exact);
// 	/// используются значения modkeyXXX из ModifierKeys, объединённые побитовым "или"
// 	///	exact = true
// 	/// возвращает true, если заданная комбинация нажата в точности
// 	///	exact = false
// 	/// возвращает true, если заданная комбинация содержится
// 	/// в числе активных кнопок регистра
// 	///
// 	/// пример формирования параметра keys:
// 	/// keys_mask = modkeyCmd | modkeyOption
// 	///
// 	/// реализация на основе Qt реагирует только на кнопки Shift, Control, Alt
// 	///
// 	/// некоторые реализации (например, универсальная консольная) могут всегда
// 	/// возвращать 0. если конкретная реализация не осуществляет опрос какой-либо из
// 	/// этих клавиш, соответствующий бит в возвращаемом значении должен устанавливаться в 0.

//--------------------------------------------------------------

int	ModifierKeys()
{
	// анализ сочетаний регистровых клавиш реализован в Qt очень плохо.
	// из 5 клавиш 2 никак не учитываются (CapsLock, LWin/RWin).
	// хотя кроссплатформенная реализация также сделана,
	// по возможности следует использовать полноценные версии, реализованные
	// для конкретных платформ
	return api_ModifierKeysState();
}

bool ModifierKeysPressed(int modifiers)
{
	int	keys_pressed = api_ModifierKeysState();

	return ((modifiers&keys_pressed) == modifiers);

	//	return api_ModifierKeysPressed(modifiers, false);
}

bool ExactModifierKeyPressed(int modifiers)
{
	int	keys_pressed = api_ModifierKeysState();
	return (keys_pressed == modifiers);

}

//--------------------------------------------------------------

XRAD_END

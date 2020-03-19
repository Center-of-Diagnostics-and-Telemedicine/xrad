#ifndef XRAD__File_Keyboard_h
#define XRAD__File_Keyboard_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "ModifierKeys.h"

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Возвращает битовую маску, соответствующую нажатым клавишам регистров
//! (modkeyCommand и т.д.)
int	ModifierKeys();

//! \brief Эта функция определяет, нажата ли комбинация из перечисленных клавиш,
//! возвращает true, если нажаты указанные клавиши и, возможно, какие-либо другие
bool ModifierKeysPressed(int modifiers);

//! \brief Определяет, нажата ли в точности указанная комбинация. При нажатых избыточных
//! клавишах возвращает false
bool ExactModifierKeyPressed(int modifiers);

// для удобства вызова следующие функции
inline bool CapsLock() { return ModifierKeysPressed(modkeyLock); }
inline bool ShiftPressed() { return ModifierKeysPressed(modkeyShift); }
inline bool ControlPressed() { return ModifierKeysPressed(modkeyControl); }
inline bool CommandPressed() { return ModifierKeysPressed(modkeyCommand); }
inline bool WinPressed() { return ModifierKeysPressed(modkeyWin); }
inline bool OptionPressed() { return ModifierKeysPressed(modkeyOption); }
inline bool AltPressed() { return ModifierKeysPressed(modkeyAlt); }

//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_Keyboard_h

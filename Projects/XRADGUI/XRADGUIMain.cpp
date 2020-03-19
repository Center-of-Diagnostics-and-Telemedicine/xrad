/*!
	\mainpage XRADGUIMain.cpp
	\brief  модуль, из которого запускается процедура main().
	данная процедура нужна для запуска Qt, из нее вызывается xrad_main(), которая содержит основные пользовательские действия
*/

#include "pre_GUI.h"
#include <GUIController.h>

int main(int argc, char *argv[])
{
	// есть два кода завершения, один из xrad_main(), другой из QApplication::Run().
	// один определяется работой вычислительных алгоритмов, другой корректностью
	// работы интерфейсных модулей. по смыслу возвращать надо тот, который возвращается
	// из xrad_main()

	int	main_result;
	int	interface_result;

	//XRAD_GUI::GUIGlobals::Initialize(&main_result, argc, argv);

	XRAD_GUI::global_gui_controller = make_shared<XRAD_GUI::GUIController>(&main_result, argc, argv);
	interface_result = XRAD_GUI::global_gui_controller->Run();

	return main_result;
}

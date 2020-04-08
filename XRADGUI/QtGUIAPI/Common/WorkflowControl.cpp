// file WorkflowControl.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "WorkflowControl.h"

#include "GUIController.h"

namespace XRAD_GUI
{

#pragma message плохо продуманный исходник, переделать как можно быстрее

//--------------------------------------------------------------
void	ScheduleApplicationQuit()
{
	global_gui_controller->quit_scheduled = true;
	global_gui_controller->workthread_pause_scheduled = false;
}

void	ScheduleWorkthreadPause(bool flag)
{
	global_gui_controller->workthread_pause_scheduled = flag;
}

bool	WorkthreadPauseScheduled()
{
	return global_gui_controller->workthread_pause_scheduled;
}

void	ScheduleCurrentOperationCancel()
{
	global_gui_controller->cancel_scheduled = true;
	global_gui_controller->workthread_pause_scheduled = false;
}

//--------------------------------------------------------------

} // namespace XRAD_GUI

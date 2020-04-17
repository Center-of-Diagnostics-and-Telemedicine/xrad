#ifndef WorkflowControl_h__
#define WorkflowControl_h__
/********************************************************************
	created:	2015/01/01
	created:	1:1:2015   17:49
	author:		kns
*********************************************************************/

namespace XRAD_GUI
{

void	ScheduleApplicationQuit();
void	ScheduleWorkthreadPause(bool flag);
bool	WorkthreadPauseScheduled();

void	ScheduleCurrentOperationCancel();

}//namespace XRAD_GUI

#endif // WorkflowControl_h__

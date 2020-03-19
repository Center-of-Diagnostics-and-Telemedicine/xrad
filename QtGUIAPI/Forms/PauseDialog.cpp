#include "pre_GUI.h"

#include "PauseDialog.h"
#include "WorkflowControl.h"
#include "GUIController.h"

namespace XRAD_GUI
{

void PauseDialog::QuitClicked()
{
	ScheduleApplicationQuit();
	accept();
}

void PauseDialog::ContinueClicked()
{
	accept();
}

PauseDialog::PauseDialog()
{
	setupUi(this);
	connect(buttonOK, &QPushButton::clicked, this, &PauseDialog::ContinueClicked);
	connect(buttonQuit, &QPushButton::clicked, this, &PauseDialog::QuitClicked);
}

}//namespace XRAD_GUI

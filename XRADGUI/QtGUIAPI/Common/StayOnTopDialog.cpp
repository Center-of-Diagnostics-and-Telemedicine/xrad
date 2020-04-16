// file StayOnTopDialog.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "StayOnTopDialog.h"

#include "GUIController.h"

//--------------------------------------------------------------

namespace XRAD_GUI
{

//--------------------------------------------------------------

StayOnTopDialog::StayOnTopDialog()
{
	cached_allow_stay_on_top = GUIController::GetStayOnTopAllowed();
	UpdateWindowFlags();
	GUIController::AddGUISettingsChangedTarget(this);
}

//--------------------------------------------------------------

StayOnTopDialog::~StayOnTopDialog()
{
	GUIController::RemoveGUISettingsChangedTarget(this);
}

//--------------------------------------------------------------

void StayOnTopDialog::SetStayOnTop(bool stay_on_top)
{
	if (stay_on_top == stay_on_top_flag)
		return;
	stay_on_top_flag = stay_on_top;
	UpdateWindowFlags();
}

//--------------------------------------------------------------

void StayOnTopDialog::GUISettingsChanged()
{
	bool allow = GUIController::GetStayOnTopAllowed();
	if (allow == cached_allow_stay_on_top)
		return;
	cached_allow_stay_on_top = allow;
	UpdateWindowFlags();
}

//--------------------------------------------------------------

void StayOnTopDialog::UpdateWindowFlags()
{
	bool visible = isVisible();
	WindowFlags wf = windowFlags();

	wf |= CustomizeWindowHint | WindowTitleHint;
	wf &= ~WindowContextHelpButtonHint;
	if (stay_on_top_flag && cached_allow_stay_on_top)
		wf |= WindowStaysOnTopHint;
	else
		wf &= ~WindowStaysOnTopHint;

	setWindowFlags(wf);
	if (visible)
		show(); // Qt может сделать окно невидимым при вызове setWindowFlags().
}

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------

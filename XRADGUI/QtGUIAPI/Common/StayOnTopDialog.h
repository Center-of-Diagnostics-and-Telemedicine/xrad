// file StayOnTopDialog.h
//--------------------------------------------------------------
#ifndef __StayOnTopDialog_h
#define __StayOnTopDialog_h
//--------------------------------------------------------------

#include "IGUISettingsChanged.h"
#include <QDialog.h>

//--------------------------------------------------------------

namespace XRAD_GUI
{

class StayOnTopDialog: public QDialog, public IGUISettingsChanged
{
		Q_OBJECT
	public:
		StayOnTopDialog();
		~StayOnTopDialog();

		void SetStayOnTop(bool stay_on_top);

		//! \name IGUISettingsChanged
		//! @{
	public:
		virtual void GUISettingsChanged() override;
		//! @}

	private:
		virtual void UpdateWindowFlags();
	private:
		bool stay_on_top_flag = false;
		bool cached_allow_stay_on_top = false;
};

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // __StayOnTopDialog_h

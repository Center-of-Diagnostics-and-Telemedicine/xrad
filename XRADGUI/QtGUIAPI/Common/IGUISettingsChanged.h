// file IGUISettingsChanged.h
//--------------------------------------------------------------
#ifndef __IGUISettingsChanged_h
#define __IGUISettingsChanged_h
//--------------------------------------------------------------

namespace XRAD_GUI
{

class IGUISettingsChanged
{
	public:
		virtual void GUISettingsChanged() = 0;
};

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // __IGUISettingsChanged_h

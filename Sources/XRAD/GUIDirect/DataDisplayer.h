#ifndef DataDisplayer_h__
#define DataDisplayer_h__

/*!
	\file
	\date 2019/02/19 14:58
	\author kulberg

	\brief  
*/

#include <XRAD/GUICore.h>


XRAD_BEGIN

class DataDisplayer
{
protected:
	unique_ptr<DataWindowContainer>	window;

public:
	bool	SetWindowTitle(const wstring &new_title);
	bool	SetWindowTitle(const string &new_title);

	bool	SetPersistent(bool persistent);
	bool	SetStayOnTop(bool stay_on_top);
	bool	Display(bool is_stopped = true);
	bool	Close();
	bool	Hide();
};


XRAD_END

#endif // DataDisplayer_h__

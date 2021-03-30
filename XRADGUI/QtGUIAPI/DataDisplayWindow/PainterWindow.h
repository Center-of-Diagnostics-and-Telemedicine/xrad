//	Created by IRD on 07.2013
//  Version 3.0.5
//--------------------------------------------------------------

#ifndef __XRAD_PainterWindow
#define __XRAD_PainterWindow

//--------------------------------------------------------------

#include <QGraphicsView>

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>
#include <ui_PainterWindow.h>
#include <XRADBasic/ContainersAlgebra.h>
#include <XRADGUI/QTGUIAPI/DataDisplayWindow/DataDisplayWindow.h>
#include <QSpinBox>

#include "ColorPanel.h"
#include "SpinBoxWidget.h"
#include "ToolsMenuWidget.h"
#include "PaintWidget.h"

XRAD_USING

namespace XRAD_GUI
{



class PainterWindow : public DataDisplayWindow
{
	Q_OBJECT

public:
	PainterWindow(const QString& in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc);
	~PainterWindow();



private:



	void closeEvent(QCloseEvent* event);
	void keyPressEvent(QKeyEvent* event);

	bool eventFilter(QObject* target, QEvent* event);

	void addToolsMenuItem(size_t w, size_t h, int drawer, const QString& path);


private: // fields


	SpinBoxWidget* spin_box_;
	ColorPanel* color_panel_;
	ToolsMenuWidget* tools_menu_;
	QFrame* painter_frame_;

	QString title_;

	size_t height_, width_;

	shared_ptr<QImage> presult_;
	PaintWidget* paint_widget_;

	Ui::Dialog ui;


public slots:
	//void slotSavePicture();
	//void slotSaveRawData();

signals:
	void signal_esc();
};



}//namespace XRAD_GUI

#endif // __XRAD_PainterWindow

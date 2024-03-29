﻿//	Created by IRD on 07.2013
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
#include <XRADGUI/QtGUIAPI/DataDisplayWindow/DataDisplayWindow.h>
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
	PainterWindow(const QString& in_title,  shared_ptr<QImage> in_result, GUIController& gc);
	//PainterWindow(const QString& in_title, const QImage& image, shared_ptr<QImage> in_result, GUIController& gc);

	~PainterWindow();



private:



	void closeEvent(QCloseEvent* event);
	void keyPressEvent(QKeyEvent* event);

	bool eventFilter(QObject* target, QEvent* event);

	void addToolsMenuItem(size_t w, size_t h, int drawer, const QString& path);


private: // fields


	SpinBoxWidget* m_sb_brush_size;
	ColorPanel* m_color_panel;
	ToolsMenuWidget* m_tools_menu;
	QFrame* m_painter_frame;

	QString m_title;

	size_t m_height, m_width;

	shared_ptr<QImage> m_presult;
	PaintWidget* m_paint_widget;

	Ui::Dialog ui;


public slots:
	//void slotSavePicture();
	//void slotSaveRawData();

signals:
	void signal_esc();
};



}//namespace XRAD_GUI

#endif // __XRAD_PainterWindow

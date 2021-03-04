/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__CONSOLE_H
#define XRAD__CONSOLE_H

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>
#include <QtGui/QtGui>

namespace XRAD_GUI
{



class stdio_console : public QPlainTextEdit
{
		Q_OBJECT
	public:
		explicit stdio_console(QWidget *parent = 0);
		void ShowSpecialCharacters(bool show);
		void output(QString string, QTextCharFormat format);
		void scrollDown();
	protected:
		void keyPressEvent(QKeyEvent *);
		void mousePressEvent(QMouseEvent *);
		void mouseDoubleClickEvent(QMouseEvent *);
		void contextMenuEvent(QContextMenuEvent *);
	private:
		bool showSpecialCharacters = false;
		QString prompt;
		bool isLocked;
		QStringList history;
		int historyPos;

		void onEnter();
		void insertPrompt(bool insertNewBlock = true);
		void historyAdd(QString);
		void historyBack();
		void historyForward();
	signals:
		void onCommand(QString);
		void onChange(QString);
	//public slots:    void output(QString);
};



}//namespace XRAD_GUI

#endif // XRAD__CONSOLE_H

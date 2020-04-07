#include "pre_GUI.h"
#include "stdio_console.h"
#include <XRADBasic/Core.h>

namespace XRAD_GUI
{

XRAD_USING



stdio_console::stdio_console(QWidget *parent) :
	QPlainTextEdit(parent)
{
	//prompt = "redis> ";
	prompt = "> ";

	QPalette p = palette();
	p.setColor(QPalette::Base, Qt::black);
	p.setColor(QPalette::Text, Qt::green);
	setPalette(p);

//	history = new QStringList;
	historyPos = 0;
	//  insertPrompt(false);
	isLocked = false;
}

void stdio_console::ShowSpecialCharacters(bool show)
{
	showSpecialCharacters = show;
}

void stdio_console::keyPressEvent(QKeyEvent *event)
{
	QPlainTextEdit::keyPressEvent(event);

	if(!isLocked)
	{
		if(event->key() >= 0x20 && event->key() <= 0x7e && (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier))
		{
			QPlainTextEdit::keyPressEvent(event);
		}
		if(event->key() == Qt::Key_Backspace && event->modifiers() == Qt::NoModifier && textCursor().positionInBlock() > prompt.length())
		{
			QPlainTextEdit::keyPressEvent(event);
		}
		if(event->key() == Qt::Key_Return && event->modifiers() == Qt::NoModifier)
			onEnter();
		if(event->key() == Qt::Key_Up && event->modifiers() == Qt::NoModifier)
			historyBack();
		if(event->key() == Qt::Key_Down && event->modifiers() == Qt::NoModifier)
			historyForward();

		QString cmd = textCursor().block().text().mid(prompt.length());
		emit onChange(cmd);
	}
}

void stdio_console::mousePressEvent(QMouseEvent *event)
{
	setFocus();
	QPlainTextEdit::mousePressEvent(event);
}

void stdio_console::mouseDoubleClickEvent(QMouseEvent *event)
{
	QPlainTextEdit::mouseDoubleClickEvent(event);
}

void stdio_console::contextMenuEvent(QContextMenuEvent *menu)
{
	QPlainTextEdit::contextMenuEvent(menu);
}

void stdio_console::onEnter()
{
	if(textCursor().positionInBlock() == prompt.length())
	{
		insertPrompt();
		return;
	}
	QString cmd = textCursor().block().text().mid(prompt.length());
	isLocked = true;
	historyAdd(cmd);
	emit onCommand(cmd);
}

void stdio_console::output(QString s, QTextCharFormat format)
{
	//isLocked = true;

	const int MaxBlockCount = 1000;
	auto doc = this->document();

	QTextCursor	cursor = textCursor();
	cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	for(QString::iterator it = s.begin(); it < s.end(); ++it)
	{
		if(*it == QChar('\r'))
		{
			cursor.select(QTextCursor::LineUnderCursor);
			cursor.removeSelectedText();
		}
		else
		{
			QChar c = *it;
			if (!showSpecialCharacters)
			{
				cursor.insertText(QString(c), format);
			}
			else
			{
				if (c == '\\')
				{
					cursor.insertText(QString("\\\\"), format);
				}
				else if (c != '\n' && c.unicode() < 0x20)
				{
					cursor.insertText(ssprintf("\\x%02X", (int)c.unicode()).c_str(), format);
				}
				else
				{
					cursor.insertText(QString(c), format);
				}
			}

			while (doc->blockCount() > MaxBlockCount)
			{
				auto block = doc->begin();
				QTextCursor	cursor(block);
				cursor.select(QTextCursor::BlockUnderCursor);
				cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
				cursor.removeSelectedText();
			}
		}
		//cursor.select(QTextCursor::LineUnderCursor);
	}

	ensureCursorVisible();

	//insertPrompt();
	isLocked = false;
}

void stdio_console::insertPrompt(bool insertNewBlock)
{
	if(insertNewBlock)
		textCursor().insertBlock();
	QTextCharFormat format;
	format.setForeground(Qt::green);
	textCursor().setBlockCharFormat(format);
	textCursor().insertText(prompt);
	scrollDown();
}

void stdio_console::scrollDown()
{
	QScrollBar *vbar = verticalScrollBar();
	vbar->setValue(vbar->maximum());
}

void stdio_console::historyAdd(QString cmd)
{
	history.append(cmd);
	historyPos = history.length();
}

void stdio_console::historyBack()
{
	if(!historyPos)
		return;
	QTextCursor cursor = textCursor();
	cursor.movePosition(QTextCursor::StartOfBlock);
	cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();
	cursor.insertText(prompt + history.at(historyPos-1));
	setTextCursor(cursor);
	--historyPos;
}

void stdio_console::historyForward()
{
	if(historyPos == history.length())
		return;
	QTextCursor cursor = textCursor();
	cursor.movePosition(QTextCursor::StartOfBlock);
	cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();

	if(historyPos == history.length() - 1)
		cursor.insertText(prompt);
	else
		cursor.insertText(prompt + history.at(historyPos + 1));

	setTextCursor(cursor);
	++historyPos;
}



}//namespace XRAD_GUI

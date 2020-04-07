#include "pre_GUI.h"
#include "TextWindow.h"

#include "ThreadGUI.h"
#include "FileSaveUtils.h"
#include <XRADBasic/Core.h>
#include <XRAD/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>

#include <time.h>

#include <XRADSystem/TextFile.h>

namespace XRAD_GUI
{

XRAD_USING



//--------------------------------------------------------------
//
//	конструкторы и деструктор
//

void TextWindow::init(const QString &title)
{
	try
	{
		setupUi(this);
		// назначаем обработчик событий (фильтр событий)
	// 	installEventFilter(this); в родителя

		cursor_position_label->setVisible(false);
		text_size_label->setVisible(false);
		line_down->setVisible(false);

		SetEditable(false);

		text->setEnabled(true);
		text->setAcceptRichText(false);
		font_size_box->setValue(10);
		text->setFontPointSize(font_size_box->value());

		text->setText("");

		setWindowTitle(title);

		WrapMode();
		UpdateFontStyle();


		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		QObject::connect(copy_button, SIGNAL(clicked()), text, SLOT(copy()));
		QObject::connect(cut_button, SIGNAL(clicked()), text, SLOT(cut()));
		QObject::connect(paste_button, SIGNAL(clicked()), text, SLOT(paste()));

		QObject::connect(font_size_box, SIGNAL(valueChanged(double)), this, SLOT(UpdateFontStyle()));

		QObject::connect(cb_word_wrap, SIGNAL(stateChanged(int)), this, SLOT(WrapMode()));
		QObject::connect(cb_fixed_width, SIGNAL(stateChanged(int)), this, SLOT(UpdateFontStyle()));

		QObject::connect(save_text_button, SIGNAL(clicked()), this, SLOT(SaveTextFile()));

		installEventFilter(this);
	}
	catch(...)
	{
	}
}

TextWindow::~TextWindow()
{
}

void	TextWindow::SetText(const QString &in_text)
{
	text->setText(in_text);
}

void	TextWindow::SetWindowTitle(const QString &title)
{
	setWindowTitle(title);
}

QString	TextWindow::GetText() const
{
	return text->toPlainText();
}

void	TextWindow::SetEditable(bool editable)
{
	text->setReadOnly(!editable);
	paste_button->setEnabled(editable);
	cut_button->setEnabled(editable);
}

void	TextWindow::SetFixedWidth(bool fixed_width)
{
	cb_fixed_width->setChecked(fixed_width);
//	UpdateFontStyle();
}

void	TextWindow::SetFontSize(double new_size)
{
	font_size_box->setValue(new_size);
}

void	TextWindow::WrapMode()
{
	text->setLineWrapMode(cb_word_wrap->isChecked() ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
	text->setFocus();
}

void	TextWindow::UpdateFontStyle()
{
	QTextCursor	cursor_init = text->textCursor();
	QTextCursor	cursor_all = cursor_init;
	cursor_all.select(QTextCursor::Document);
	text->setTextCursor(cursor_all);

	text->setFontFamily(cb_fixed_width->isChecked() ? "Courier" : "Helvetica");
	text->setFontPointSize(font_size_box->value());

	text->setTextCursor(cursor_init);
	text->setFocus();
}

// Обработчик всех событий
bool TextWindow::eventFilter(QObject *target, QEvent *event)
{
	//Следующая строка важна. Иначе при ForceUpdateGui вид текста сбрасывался к настройкам по умолчанию
	UpdateFontStyle();
	return QObject::eventFilter(target, event);
}

void TextWindow::closeEvent(QCloseEvent *event)
{
	//if(b_window_persistent)
	//{
	//	event->ignore();
	//	MakeWindowPersistent(!b_window_persistent);
	//	return;
	//}

	QDialog::closeEvent(event);
}

void TextWindow::SaveTextFile()
{
	char *prompt = "Save text file";
	char *type = "Text file (*.txt)";
	QString qfile_name = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));
	if(qfile_name.isEmpty()) return;
	wstring	file_name = qstring_to_wstring(qfile_name);//.toStdWString();

	try
	{
		//Сохранение в utf-8 работает плохо. Нужно радикально переделывать соответствующую часть text_file_writer::printf_
// 		text_file_writer	file_container(file_name, text_encoding::utf8);
		text_file_writer	file_container(file_name, text_encoding::utf16_le);
		file_container.printf_(L"%Ls", qstring_to_wstring(text->toPlainText()).c_str());
	}
	catch(...)
	{
	}
}

void TextWindow::keyPressEvent(QKeyEvent *event)
{
	if(event->type()==QEvent::KeyPress)
	{
		switch(event->key())
		{
				break;
			case Qt::Key_S:
				if(event->modifiers() == Qt::ControlModifier) SaveTextFile();
				break;
		};
	}
	//TODO если не вызвать выше signal_esc, то закрытие окна по умолчанию происходит неправильно. Надо бы понять, почему
	return parent::keyPressEvent(event);
}



}//namespace XRAD_GUI

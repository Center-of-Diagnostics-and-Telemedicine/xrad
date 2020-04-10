// file DynamicDialogQt.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "DynamicDialogQt.h"

#include "GUIController.h"
#include "SavedSettings.h"
#include "XRADGUIAPI.h"
#include <XRADGUI/Sources/GUI/XRADGUI.h>
#include <XRADGUI/Sources/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <XRADSystem/System.h>

//--------------------------------------------------------------

namespace XRAD_GUI
{

XRAD_USING

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

constexpr int sub_margin = 16;

// Выравнивание: по горизонтали растягиваем на всю ширину, поэтому без Qt::AlignLeft.
const Qt::Alignment default_alignment = Qt::AlignTop;

//--------------------------------------------------------------

int StretchFromLayout(QBoxLayout *layout, bool is_stretch = false)
{
	switch (layout->direction())
	{
		case QBoxLayout::LeftToRight:
		case QBoxLayout::RightToLeft:
			if (is_stretch)
				return 100;
			return 1;
		default:
			if (is_stretch)
				return 1;
			return 0;
	}
}

//--------------------------------------------------------------

QBoxLayout *WrapLayout(QBoxLayout *layout, QDialog *dialog,
		DynamicDialogLayout control_layout = DynamicDialogLayout::Vertical,
		bool use_widget = false)
{
	auto box_layout_direction = control_layout == DynamicDialogLayout::Horizontal?
			QBoxLayout::LeftToRight: QBoxLayout::TopToBottom;
	if (use_widget)
	{
		// Использование одного только layout дает побочный эффект:
		// контролы отрываются друг от друга при растягивании родительского layout.
		// Поэтому создаем QWidget.
		auto widget = new QWidget(dialog);
		layout->addWidget(widget, StretchFromLayout(layout), default_alignment);
		auto sub_layout = new QBoxLayout(box_layout_direction, widget);
		sub_layout->setContentsMargins(QMargins(0, 0, 0, 0));
		return sub_layout;
	}
	else
	{
		auto sub_layout = new QBoxLayout(box_layout_direction);
		layout->addLayout(sub_layout, StretchFromLayout(layout));
		return sub_layout;
	}
}

//--------------------------------------------------------------

template <class Functor>
class QtSlotSafeExecuteHelper
{
	public:
		QtSlotSafeExecuteHelper(Functor f): functor(f) {}
		template <class... Args>
		void operator()(Args&&... args)
		{
			try
			{
				functor(std::forward<Args>(args)...);
			}
			catch (...)
			{
				fprintf(stderr, "Exception in Qt slot:\n%s\n", GetExceptionString().c_str());
			}
		}
	private:
		Functor functor;
};

//--------------------------------------------------------------

template <class Functor>
QtSlotSafeExecuteHelper<Functor> QtSlotSafeExecute(Functor f)
{
	return QtSlotSafeExecuteHelper<Functor>(f);
}

//--------------------------------------------------------------

template <class Functor>
class QtEventSafeExecuteHelper
{
	public:
		QtEventSafeExecuteHelper(Functor f): functor(f) {}
		template <class... Args>
		bool operator()(Args&&... args)
		{
			try
			{
				return functor(std::forward<Args>(args)...);
			}
			catch (...)
			{
				fprintf(stderr, "Exception in Qt event:\n%s\n", GetExceptionString().c_str());
			}
			return false;
		}
	private:
		Functor functor;
};

//--------------------------------------------------------------

template <class Functor>
QtEventSafeExecuteHelper<Functor> QtEventSafeExecute(Functor f)
{
	return QtEventSafeExecuteHelper<Functor>(f);
}

//--------------------------------------------------------------

wstring NormalizeSettingsName(const wstring &name)
{
	if (!name.length())
		return L"_";
	wstring result(name);
	for (size_t i = 0; i < result.size(); ++i)
	{
		wchar_t c = result[i];
		if (c < 0x20 || wcschr(L"/\\:?*\"<>|", c))
		{
			result[i] = L'_';
		}
	}
	return result;
}

//--------------------------------------------------------------

wstring CombineSettingsPath(const wstring &path, const wstring &name)
{
	return path + L"/" + NormalizeSettingsName(name);
}

//--------------------------------------------------------------

QMenu *CreateResetDefaultMenu(QWidget *widget,
		const wstring &default_value_str, function<void ()> set_default_value,
		const wstring &stored_value_str, function<void ()> set_stored_value)
{
	auto menu = new QMenu(widget);
	auto ac_value_default = new QAction(wstring_to_qstring(ssprintf(L"Default: %ls",
					EnsureType<const wchar_t*>(default_value_str.c_str()))),
			widget);
	QObject::connect(ac_value_default, &QAction::triggered, QtSlotSafeExecute(
			[set_default_value](bool)
			{
				set_default_value();
			}));
	menu->addAction(ac_value_default);
	auto ac_value_reset = new QAction(wstring_to_qstring(ssprintf(L"Reset: %ls",
					EnsureType<const wchar_t*>(stored_value_str.c_str()))),
			widget);
	QObject::connect(ac_value_reset, &QAction::triggered, QtSlotSafeExecute(
			[set_stored_value](bool)
			{
				set_stored_value();
			}));
	menu->addAction(ac_value_reset);
	return menu;
}

//--------------------------------------------------------------

void ConnectMenu(QWidget *widget, QMenu *menu)
{
	widget->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(widget, &QCheckBox::customContextMenuRequested, QtSlotSafeExecute(
			[w_widget = QPointer<QWidget>(widget), menu](const QPoint &pos)
			{
				if (!w_widget)
					return;
				menu->exec(w_widget->mapToGlobal(pos));
			}));
}

//--------------------------------------------------------------

void ConnectResetDefaultMenu(QWidget *widget,
		const wstring &default_value_str, function<void ()> set_default_value,
		const wstring &stored_value_str, function<void ()> set_stored_value)
{
	auto menu = CreateResetDefaultMenu(widget,
			default_value_str, set_default_value,
			stored_value_str, set_stored_value);
	ConnectMenu(widget, menu);
}

//--------------------------------------------------------------

void SetupMiscButton(QPushButton *button)
{
	button->setFocusPolicy(NoFocus);
}

//--------------------------------------------------------------

// Настройки для фильтра клавиатуры, единообразные для всех контролов.

const Qt::KeyboardModifiers KeyFilterModifiersMask =
		Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier;

const Qt::KeyboardModifiers KeyFilterCursorModifiers = 0;

constexpr int KeyFilterMaxKey = Qt::Key_PageUp;
constexpr int KeyFilterMinKey = Qt::Key_PageDown;
constexpr int KeyFilterIncrementKey = Qt::Key_Up;
constexpr int KeyFilterDecrementKey = Qt::Key_Down;

const Qt::KeyboardModifiers KeyFilterAlphaModifiers = Qt::ControlModifier;

constexpr int KeyFilterBrowseKey = Qt::Key_Space;
constexpr int KeyFilterDefaultKey = Qt::Key_D;
constexpr int KeyFilterHistoryKey = Qt::Key_H;

//--------------------------------------------------------------

} // namespace



//--------------------------------------------------------------
//
//--------------------------------------------------------------



ContainerApi::ContainerApi(PrivateTag, bool use_frame, const wstring &caption,
		Layout control_layout):
	use_frame(use_frame),
	caption(caption),
	control_layout(control_layout)
{
}

shared_ptr<ContainerApi> ContainerApi::Create(bool use_frame, const wstring &caption,
		Layout control_layout)
{
	return make_shared<ContainerApi>(PrivateTag(), use_frame, caption, control_layout);
}

void ContainerApi::AddControl(shared_ptr<ControlApi> control)
{
	controls.push_back(control);
}

void ContainerApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	for (auto &control: controls)
	{
		control->SetDialogApiInterface(dialog_api);
	}
}

void ContainerApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	auto sub_layout_up = make_unique<QBoxLayout>(
			control_layout == Layout::Horizontal? QBoxLayout::LeftToRight: QBoxLayout::TopToBottom);
	auto sub_layout = sub_layout_up.get();
	if (use_frame)
	{
		auto ui_group_box = new QGroupBox(wstring_to_qstring(caption), dialog);
		layout->addWidget(ui_group_box, StretchFromLayout(layout), default_alignment);
		ui_group_box->setLayout(sub_layout_up.release());
	}
	else
	{
		layout->addLayout(sub_layout_up.release(), StretchFromLayout(layout));
	}
	wstring sub_path;
	if (use_frame)
		sub_path = CombineSettingsPath(settings_path, caption);
	else
		sub_path = settings_path;
	for (auto &control: controls)
	{
		control->AddToDialog(dialog, sub_layout, ddi, sub_path);
	}
}

void ContainerApi::ReleaseDialog()
{
	for (auto &control: controls)
	{
		control->ReleaseDialog();
	}
}

bool ContainerApi::TryAccept(QString *err_message)
{
	for (auto &control: controls)
	{
		if (!control->TryAccept(err_message))
			return false;
	}
	return true;
}

void ContainerApi::Accept()
{
	for (auto &control: controls)
		control->Accept();
}

void ContainerApi::Enable(bool enable)
{
	for (auto &control: controls)
	{
		control->Enable(enable);
	}
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



SeparatorApi::SeparatorApi(PrivateTag)
{
}

shared_ptr<SeparatorApi> SeparatorApi::Create()
{
	return make_shared<SeparatorApi>(PrivateTag());
}

void SeparatorApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
}

void SeparatorApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &)
{
	XRAD_ASSERT_THROW(!ui_frame);
	ui_frame = new QFrame(dialog);
	Qt::Alignment ui_alignment = Qt::Alignment();
	switch (layout->direction())
	{
		case QBoxLayout::LeftToRight:
		case QBoxLayout::RightToLeft:
			ui_frame->setFrameShape(QFrame::VLine);
			ui_alignment = Qt::AlignLeft;
			break;
		case QBoxLayout::TopToBottom:
		case QBoxLayout::BottomToTop:
			ui_frame->setFrameShape(QFrame::HLine);
			ui_alignment = Qt::AlignTop;
			break;
		default:
			throw invalid_argument("SeparatorApi::AddToDialog: invalid layout.");
	}
	ui_frame->setFrameShadow(QFrame::Sunken);
	layout->addWidget(ui_frame, 0, ui_alignment);
}

void SeparatorApi::ReleaseDialog()
{
	ui_frame = nullptr;
}

bool SeparatorApi::TryAccept(QString *err_message)
{
	return true;
}

void SeparatorApi::Accept()
{
}

void SeparatorApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_frame);
	ui_frame->setEnabled(enable);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



StretchApi::StretchApi(PrivateTag)
{
}

shared_ptr<StretchApi> StretchApi::Create()
{
	return make_shared<StretchApi>(PrivateTag());
}

void StretchApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
}

void StretchApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &)
{
	XRAD_ASSERT_THROW(!ui_created);
	ui_created = true;
	layout->addStretch(StretchFromLayout(layout, true));
}

void StretchApi::ReleaseDialog()
{
	ui_created = false;
}

bool StretchApi::TryAccept(QString *err_message)
{
	return true;
}

void StretchApi::Accept()
{
}

void StretchApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_created);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



TextLabelApi::TextLabelApi(PrivateTag, const wstring &text):
	text(text)
{
}

shared_ptr<TextLabelApi> TextLabelApi::Create(const wstring &text)
{
	return make_shared<TextLabelApi>(PrivateTag(), text);
}

void TextLabelApi::SetText(const wstring &t)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), t]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (!obj->ui_label)
				return;
			obj->ui_label->setText(wstring_to_qstring(t));
		});
}

void TextLabelApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
}

void TextLabelApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &)
{
	XRAD_ASSERT_THROW(!ui_label);
	ui_label = new QLabel(dialog);
	ui_label->setText(wstring_to_qstring(text));
	layout->addWidget(ui_label, StretchFromLayout(layout), default_alignment);
}

void TextLabelApi::ReleaseDialog()
{
	ui_label = nullptr;
}

bool TextLabelApi::TryAccept(QString *err_message)
{
	return true;
}

void TextLabelApi::Accept()
{
}

void TextLabelApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_label);
	ui_label->setEnabled(enable);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



ButtonApi::ButtonApi(PrivateTag, const wstring &caption, DialogResultCode result_code,
		bool default_button,
		const function<void ()> &action,
		ActionPolicy action_policy):
	caption(caption), result_code(result_code), default_button(default_button),
	action(action),
	action_policy(action_policy)
{
}

shared_ptr<ButtonApi> ButtonApi::Create(const wstring &caption, DialogResultCode result_code,
		bool default_button,
		const function<void ()> &action,
		ActionPolicy action_policy)
{
	return make_shared<ButtonApi>(PrivateTag(), caption, result_code, default_button,
			action, action_policy);
}

void ButtonApi::SetValueDependencies(vector<ControlApi*> dependencies)
{
	value_dependencies = std::move(dependencies);
	use_value_dependencies = true;
}

void ButtonApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
}

void ButtonApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &)
{
	// См. также: setAutoDefault.
	XRAD_ASSERT_THROW(!ui_button);
	ui_ddi = ddi;
	ui_button = new QPushButton(dialog);
	ui_button->setText(wstring_to_qstring(caption));
	if (default_button)
		ui_button->setDefault(true);
	layout->addWidget(ui_button, StretchFromLayout(layout), default_alignment);
	QObject::connect(ui_button, &QPushButton::clicked, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				if (auto obj = w_obj.lock())
					obj->DoClicked();
			}));
}

void ButtonApi::DoClicked()
{
	if (action)
	{
		if (use_value_dependencies && !ui_ddi->TryAccept(value_dependencies))
			return;
		ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
			{
				// Worker thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (obj->action)
					obj->action();
			},
			action_policy);
	}
	else
	{
		ui_ddi->EndDialog(this, result_code,
				use_value_dependencies? &value_dependencies: nullptr);
	}
}

void ButtonApi::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_button = nullptr;
}

bool ButtonApi::TryAccept(QString *err_message)
{
	return true;
}

void ButtonApi::Accept()
{
}

void ButtonApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_button);
	ui_button->setEnabled(enable);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



CheckBoxApi::CheckBoxApi(PrivateTag, const wstring &caption,
		bool value,
		GUIValue<bool> default_value,
		bool history_value,
		const shared_ptr<ControlApi> &sub_control,
		function<void ()> on_value_changed):
	caption(caption),
	default_value(default_value),
	history_value(history_value),
	sub_control(sub_control),
	on_value_changed(on_value_changed),
	ui_value(value)
{
}

shared_ptr<CheckBoxApi> CheckBoxApi::Create(const wstring &caption,
		bool value,
		GUIValue<bool> default_value,
		bool history_value,
		const shared_ptr<ControlApi> &sub_control,
		function<void ()> on_value_changed)
{
	return make_shared<CheckBoxApi>(PrivateTag(), caption, value,
			default_value,
			history_value,
			sub_control, on_value_changed);
}

bool CheckBoxApi::UIValue() const
{
	return ui_value.load();
}

void CheckBoxApi::SetValue(bool v)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), v]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (!obj->ui_checkbox)
				return;
			obj->UIChangeEditValue(v);
		});
}

void CheckBoxApi::UISetValue(bool v)
{
	if (v == ui_value)
		return;
	ui_value.store(v);
	if (sub_control)
		sub_control->Enable(ui_enabled && v);
}

void CheckBoxApi::UIChangeEditValue(bool v)
{
	if (!ui_checkbox || !ui_ddi)
		return;
	ui_checkbox->setChecked(v);

	if (v == ui_value)
		return;

	ui_value.store(v);

	if (sub_control)
		sub_control->Enable(ui_enabled && v);

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void CheckBoxApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
	if (sub_control)
		sub_control->SetDialogApiInterface(dialog_api);
}

namespace
{

wstring FormatBoolValueStd(bool v)
{
	return v? L"true": L"false";
}

} // namespace

void CheckBoxApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_checkbox);

	if (sub_control)
		layout = WrapLayout(layout, dialog);
	ui_ddi = ddi;
	ui_checkbox = new QCheckBox(dialog);
	ui_checkbox->setText(wstring_to_qstring(caption));
	ui_checkbox->setChecked(ui_value);
	// Connect: подключаемся к событию clicked, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_checkbox, &QCheckBox::clicked, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_checkbox || !obj->ui_ddi)
					return;
				obj->UISetValue(obj->ui_checkbox->isChecked());
				// (*1544536937)
				// В callback текущее состояние не передаем.
				// Во время выполнения callback прочитает актуальное на тот момент состояние.
				// Использование в callback запомненного состояния может привести к зацикливанию.
				obj->ui_ddi->ProcessCallback([w_obj]()
					{
						// Worker thread
						auto obj = w_obj.lock();
						if (!obj)
							return;
						if (obj->on_value_changed)
							obj->on_value_changed();
					});
			}));

	layout->addWidget(ui_checkbox, StretchFromLayout(layout), default_alignment);

	ConnectResetDefaultMenu(ui_checkbox,
			FormatBoolValueStd(default_value.value),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatBoolValueStd(history_value),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});

	if (sub_control)
	{
		auto sub_layout = new QVBoxLayout();
		sub_layout->setContentsMargins(QMargins(sub_margin, 0, 0, 0));
		layout->addLayout(sub_layout, StretchFromLayout(layout));
		sub_control->AddToDialog(dialog, sub_layout, ddi, CombineSettingsPath(settings_path, caption));
		sub_control->Enable(ui_enabled && ui_value);
	}
}

void CheckBoxApi::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_checkbox = nullptr;
	if (sub_control)
		sub_control->ReleaseDialog();
}

bool CheckBoxApi::TryAccept(QString *err_message)
{
	if (!sub_control)
		return true;
	return sub_control->TryAccept(err_message);
}

void CheckBoxApi::Accept()
{
	if (!sub_control)
		return;
	sub_control->Accept();
}

void CheckBoxApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_checkbox);
	ui_checkbox->setEnabled(enable);
	ui_enabled = enable;
	if (sub_control)
		sub_control->Enable(enable && ui_value);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



RadioButtonApi::RadioButtonApi(PrivateTag, const wstring &caption, bool checked,
		const shared_ptr<ControlApi> &sub_control):
	caption(caption), sub_control(sub_control),
	ui_checked(checked)
{
}

shared_ptr<RadioButtonApi> RadioButtonApi::Create(const wstring &caption, bool checked,
		const shared_ptr<ControlApi> &sub_control)
{
	return make_shared<RadioButtonApi>(PrivateTag(), caption, checked, sub_control);
}

QRadioButton *RadioButtonApi::GetRadioButton()
{
	XRAD_ASSERT_THROW(ui_radiobutton);
	return ui_radiobutton;
}

void RadioButtonApi::Check()
{
	XRAD_ASSERT_THROW(ui_radiobutton);
	ui_radiobutton->setChecked(true);
}

void RadioButtonApi::Uncheck()
{
	XRAD_ASSERT_THROW(ui_radiobutton);
	ui_radiobutton->setChecked(false);
}

void RadioButtonApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	if (sub_control)
		sub_control->SetDialogApiInterface(dialog_api);
}

void RadioButtonApi::AddToGroup(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		QButtonGroup *ui_group, const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_radiobutton);
	if (sub_control)
		layout = WrapLayout(layout, dialog);

	ui_radiobutton = new QRadioButton(dialog);
	ui_group->addButton(ui_radiobutton);
	ui_radiobutton->setText(wstring_to_qstring(caption));
	ui_radiobutton->setChecked(ui_checked);
	// Connect: подключаемся к событию toggled, которое вызывается и при ручном,
	// и программном изменении состояния UI. Отслеживание состояния требуется
	// для блокировки sub_control.
	QObject::connect(ui_radiobutton, &QRadioButton::toggled, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_radiobutton)
					return;
				obj->UICheckedChanged(obj->ui_radiobutton->isChecked());
			}));
	layout->addWidget(ui_radiobutton, StretchFromLayout(layout), default_alignment);
	if (sub_control)
	{
		auto sub_layout = new QVBoxLayout();
		sub_layout->setContentsMargins(QMargins(sub_margin, 0, 0, 0));
		layout->addLayout(sub_layout, StretchFromLayout(layout));
		sub_control->AddToDialog(dialog, sub_layout, ddi, CombineSettingsPath(settings_path, caption));
		sub_control->Enable(ui_enabled && ui_checked);
	}
}

void RadioButtonApi::ReleaseDialog()
{
	ui_radiobutton = nullptr;
	if (sub_control)
		sub_control->ReleaseDialog();
}

bool RadioButtonApi::TryAccept(QString *err_message)
{
	if (!sub_control)
		return true;
	return sub_control->TryAccept(err_message);
}

void RadioButtonApi::Accept()
{
	if (!sub_control)
		return;
	sub_control->Accept();
}

void RadioButtonApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_radiobutton);
	ui_enabled = enable;
	ui_radiobutton->setEnabled(enable);
	if (sub_control)
		sub_control->Enable(enable && ui_checked);
}

void RadioButtonApi::UICheckedChanged(bool c)
{
	ui_checked = c;
	if (sub_control)
		sub_control->Enable(ui_enabled && ui_checked);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



RadioButtonContainerApi::RadioButtonContainerApi(PrivateTag, bool use_frame, const wstring &caption,
		size_t value,
		const GUIValue<size_t> &default_value,
		size_t history_value,
		Layout control_layout,
		function<void ()> on_value_changed):
	use_frame(use_frame),
	caption(caption),
	control_layout(control_layout),
	default_value(default_value),
	history_value(history_value),
	on_value_changed(on_value_changed),
	ui_value(value)
{
}

shared_ptr<RadioButtonContainerApi> RadioButtonContainerApi::Create(bool use_frame,
		const wstring &caption,
		size_t value,
		const GUIValue<size_t> &default_value,
		size_t history_value,
		Layout control_layout,
		function<void ()> on_value_changed)
{
	return make_shared<RadioButtonContainerApi>(PrivateTag(), use_frame, caption,
			value,
			default_value,
			history_value,
			control_layout,
			on_value_changed);
}

void RadioButtonContainerApi::AddControl(shared_ptr<RadioButtonApi> control)
{
	controls.push_back(control);
}

size_t RadioButtonContainerApi::UIValue() const
{
	return ui_value.load();
}

void RadioButtonContainerApi::SetValue(size_t v)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), v]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			obj->UIChangeEditValue(v);
		});
}

void RadioButtonContainerApi::UISetValue(size_t v)
{
	if (v == ui_value)
		return;
	ui_value.store(v);
}

void RadioButtonContainerApi::UIChangeEditValue(size_t v)
{
	if (!ui_group || !ui_ddi)
		return;
	UISetGroupIndex(v);

	if (v == ui_value)
		return;

	ui_value.store(v);

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void RadioButtonContainerApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
	for (auto &control: controls)
	{
		control->SetDialogApiInterface(dialog_api);
	}
}

void RadioButtonContainerApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_group);

	ui_ddi = ddi;

	ui_group = new QButtonGroup(dialog);
	auto sub_layout_up = make_unique<QBoxLayout>(
			control_layout == Layout::Horizontal? QBoxLayout::LeftToRight: QBoxLayout::TopToBottom);
	auto sub_layout = sub_layout_up.get();
	if (use_frame)
	{
		ui_group_box = new QGroupBox(wstring_to_qstring(caption), dialog);
		layout->addWidget(ui_group_box, StretchFromLayout(layout), default_alignment);
		ui_group_box->setLayout(sub_layout_up.release());
	}
	else
	{
		layout->addLayout(sub_layout_up.release(), StretchFromLayout(layout));
	}
	wstring sub_path;
	if (use_frame)
		sub_path = CombineSettingsPath(settings_path, caption);
	else
		sub_path = settings_path;
	for (auto &control: controls)
	{
		control->AddToGroup(dialog, sub_layout, ddi, ui_group, sub_path);
	}

	UISetGroupIndex(ui_value);
	// Connect: подключаемся к событию buttonClicked, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_group, (void (QButtonGroup::*)(QAbstractButton *button))
					&QButtonGroup::buttonClicked,
			QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](QAbstractButton *button)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_group || !obj->ui_ddi)
					return;
				size_t index = obj->UIButtonToIndex(button);
				if (obj->ui_value == index)
					return; // Событие может вызываться и в тех случаях, когда выбранный пункт не меняется.
				obj->UISetValue(index);
				// См. (*1544536937): В callback текущее состояние не передаем...
				obj->ui_ddi->ProcessCallback([w_obj]()
					{
						// Worker thread
						auto obj = w_obj.lock();
						if (!obj)
							return;
						if (obj->on_value_changed)
							obj->on_value_changed();
					});
			}));

	if (controls.size())
	{
		auto menu = CreateResetDefaultMenu(
				ui_group_box? (QWidget*)ui_group_box: (QWidget*)controls.front()->GetRadioButton(),
				FormatValueStd(default_value.value),
				[w_obj = to_weak_ptr(shared_from_this())]()
				{
					auto obj = w_obj.lock();
					if (!obj || !obj->ui_ddi)
						return;
					obj->UIChangeEditValue(obj->default_value.value);
				},
				FormatValueStd(history_value),
				[w_obj = to_weak_ptr(shared_from_this())]()
				{
					auto obj = w_obj.lock();
					if (!obj || !obj->ui_ddi)
						return;
					obj->UIChangeEditValue(obj->history_value);
				});
		if (ui_group_box)
		{
			ConnectMenu(ui_group_box, menu);
		}
		else
		{
			for (auto &c: controls)
			{
				ConnectMenu(c->GetRadioButton(), menu);
			}
		}
	}
}

void RadioButtonContainerApi::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_group = nullptr;
	ui_group_box = nullptr;
	for (auto &control: controls)
	{
		control->ReleaseDialog();
	}
}

bool RadioButtonContainerApi::TryAccept(QString *err_message)
{
	if (!ui_enabled)
		return true;

	if (!controls.size())
		return true;
	auto i = ui_value.load();
	if (i == (size_t)-1 || i >= controls.size())
	{
		if (ui_group_box)
		{
			ui_group_box->setFocus();
		}
		else
		{
			controls.front()->GetRadioButton()->setFocus();
		}
		auto *message_details = L"Не выбран вариант";
		wstring message;
		if (use_frame && caption.length())
		{
			message = ssprintf(L"%ls: %ls.",
					EnsureType<const wchar_t*>(caption.c_str()),
					EnsureType<const wchar_t*>(message_details));
		}
		else
		{
			message = ssprintf(L"%ls (%ls...).",
					EnsureType<const wchar_t*>(message_details),
					EnsureType<const wchar_t*>(controls.front()->Caption().c_str()));
		}
		*err_message = wstring_to_qstring(message);
		return false;
	}

	for (auto &control: controls)
	{
		if (!control->TryAccept(err_message))
			return false;
	}
	return true;
}

void RadioButtonContainerApi::Accept()
{
	for (auto &control: controls)
		control->Accept();
}

void RadioButtonContainerApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_group);
	ui_enabled = enable;
	for (auto &control: controls)
	{
		control->Enable(enable);
	}
}

void RadioButtonContainerApi::UISetGroupIndex(size_t index)
{
	if (index == (size_t)-1 || index >= controls.size())
	{
		ui_group->setExclusive(false);
		for (auto &c: controls)
		{
			c->Uncheck();
		}
		ui_group->setExclusive(true);
	}
	else
	{
		controls[index]->Check();
	}
}

size_t RadioButtonContainerApi::UIButtonToIndex(QAbstractButton *button)
{
	for (size_t i = 0; i < controls.size(); ++i)
	{
		if (controls[i]->GetRadioButton() == button)
			return i;
	}
	return (size_t)-1;
}

wstring RadioButtonContainerApi::FormatValueStd(size_t value)
{
	if (value == size_t(-1) || value > controls.size())
		return L"<значение не выбрано>";
	return controls[value]->Caption();
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



ComboBoxApi::ComboBoxApi(PrivateTag, bool use_caption, const wstring &caption,
		const vector<wstring> &items,
		size_t value,
		const GUIValue<size_t> &default_value,
		size_t history_value,
		function<void ()> on_value_changed):
	use_caption(use_caption),
	caption(caption),
	items(items),
	default_value(default_value),
	history_value(history_value),
	on_value_changed(on_value_changed),
	ui_value(value)
{
}

shared_ptr<ComboBoxApi> ComboBoxApi::Create(bool use_caption, const wstring &caption,
		const vector<wstring> &items,
		size_t value,
		const GUIValue<size_t> &default_value,
		size_t history_value,
		function<void ()> on_value_changed)
{
	return make_shared<ComboBoxApi>(PrivateTag(), use_caption, caption, items,
			value,
			default_value,
			history_value,
			on_value_changed);
}

size_t ComboBoxApi::UIValue() const
{
	return ui_value.load();
}

void ComboBoxApi::SetValue(size_t v)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), v]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			obj->UIChangeEditValue(v);
		});
}

void ComboBoxApi::UISetValue(size_t v)
{
	if (v == ui_value)
		return;
	ui_value.store(v);
}

void ComboBoxApi::UIChangeEditValue(size_t v)
{
	if (!ui_combobox || !ui_ddi)
		return;
	UISetComboIndex(v);

	if (v == ui_value)
		return;

	ui_value.store(v);

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void ComboBoxApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
}

void ComboBoxApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_combobox);

	ui_ddi = ddi;

	ui_combobox = new QComboBox(dialog);
	for (auto &item: items)
	{
		ui_combobox->addItem(wstring_to_qstring(item));
	}
	UISetComboIndex(ui_value);
	// Connect: подключаемся к событию activated, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_combobox, (void (QComboBox::*)(int))&QComboBox::activated,
			QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](int index)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_combobox || !obj->ui_ddi)
					return;
				if (obj->ui_value == index)
					return; // Событие может вызываться и в тех случаях, когда выбранный пункт не меняется.
				obj->UISetValue(index);
				// См. (*1544536937): В callback текущее состояние не передаем...
				obj->ui_ddi->ProcessCallback([w_obj]()
					{
						// Worker thread
						auto obj = w_obj.lock();
						if (!obj)
							return;
						if (obj->on_value_changed)
							obj->on_value_changed();
					});
			}));

	ConnectResetDefaultMenu(ui_combobox,
			FormatValueStd(default_value.value),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatValueStd(history_value),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});

	if (use_caption)
	{
		// Использование одного только layout дает побочный эффект: заголовок может отрываться
		// от комбобокса при растягивании родительского layout.
		// Поэтому нужно создать QWidget + QBoxLayout.
		auto sub_layout = WrapLayout(layout, dialog, DynamicDialogLayout::Vertical, true);
		sub_layout->setContentsMargins(QMargins(0, 0, 0, 0));
		ui_label = new QLabel(dialog);
		ui_label->setText(wstring_to_qstring(caption));
		sub_layout->addWidget(ui_label, StretchFromLayout(sub_layout), default_alignment);
		sub_layout->addWidget(ui_combobox, StretchFromLayout(sub_layout), default_alignment);
	}
	else
	{
		layout->addWidget(ui_combobox, StretchFromLayout(layout), default_alignment);
	}
}

void ComboBoxApi::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_combobox = nullptr;
	ui_label = nullptr;
}

bool ComboBoxApi::TryAccept(QString *err_message)
{
	if (!ui_enabled)
		return true;

	if (!items.size())
		return true;
	auto i = ui_value.load();
	if (i != (size_t)-1 && i < items.size())
		return true;
	ui_combobox->setFocus();
	auto *message_details = L"Не выбрано значение из списка";
	wstring message;
	if (use_caption && caption.length())
	{
		message = ssprintf(L"%ls: %ls.",
				EnsureType<const wchar_t*>(caption.c_str()),
				EnsureType<const wchar_t*>(message_details));
	}
	else
	{
		message = ssprintf(L"%ls (%ls...).",
				EnsureType<const wchar_t*>(message_details),
				EnsureType<const wchar_t*>(items.front().c_str()));
	}
	*err_message = wstring_to_qstring(message);
	return false;
}

void ComboBoxApi::Accept()
{
}

void ComboBoxApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_combobox);
	ui_enabled = enable;
	if (ui_label)
		ui_label->setEnabled(enable);
	ui_combobox->setEnabled(enable);
}

void ComboBoxApi::UISetComboIndex(size_t index)
{
	if (index == (size_t)-1 || index >= items.size())
		ui_combobox->setCurrentIndex(-1);
	else
		ui_combobox->setCurrentIndex((int)index);
}

wstring ComboBoxApi::FormatValueStd(size_t value)
{
	if (value == size_t(-1) || value > items.size())
		return L"<значение не выбрано>";
	return items[value];
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



StringEditBaseApi::StringEditBaseApi(const wstring &caption,
		const wstring &value,
		const GUIValue<wstring> &default_value,
		const wstring &history_value,
		Layout control_layout,
		function<void ()> on_value_changed):
	use_caption(!caption.empty()),
	caption(caption),
	default_value(default_value),
	history_value(history_value),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	ui_value(value)
{
}

wstring StringEditBaseApi::UIValue() const
{
	std::unique_lock<std::mutex> locker(ui_value_mutex);
	return ui_value;
}

void StringEditBaseApi::SetValue(const wstring &v)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(SharedFromThisBase()), v]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			obj->UIChangeEditValue(v);
		});
}

void StringEditBaseApi::UISetValue(const wstring &v)
{
	if (v == ui_value)
		return;
	std::unique_lock<std::mutex> locker(ui_value_mutex);
	ui_value = v;
}

void StringEditBaseApi::UIChangeEditValue(const wstring &v)
{
	if (!ui_ddi)
		return;
	SetEditText(v);

	if (v == ui_value)
		return;

	{
		std::unique_lock<std::mutex> locker(ui_value_mutex);
		ui_value = v;
	}

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(SharedFromThisBase())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void StringEditBaseApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
}

void StringEditBaseApi::ReleaseDialog()
{
	ui_ddi = nullptr;
}

bool StringEditBaseApi::TryAccept(QString *err_message)
{
	return true;
}

void StringEditBaseApi::Accept()
{
}

void StringEditBaseApi::InstallUIEditFilter(QObject *ui_edit)
{
	ui_edit_filter = new KeyPressEventFilter(QtEventSafeExecute(
			[w_obj = to_weak_ptr(SharedFromThisBase())](QObject *, QKeyEvent *event)
			{
				auto obj = w_obj.lock();
				if (!obj)
					return false;
				if (!obj->ui_edit_filter || !obj->ui_ddi)
					return false;
				return obj->ProcessUIEditKeyPress(event);
			}),
			ui_edit);
	ui_edit->installEventFilter(ui_edit_filter);
}

void StringEditBaseApi::RemoveUIEditFilter(QObject *ui_edit)
{
	if (!ui_edit || !ui_edit_filter)
		return;
	ui_edit->removeEventFilter(ui_edit_filter);
	ui_edit_filter = nullptr;
}

bool StringEditBaseApi::ProcessUIEditKeyPress(QKeyEvent *event)
{
	auto modifiers = event->modifiers();
	auto key = event->key();
	if ((modifiers & KeyFilterModifiersMask) == KeyFilterAlphaModifiers)
	{
		switch (key)
		{
			case KeyFilterDefaultKey:
				// default
				UIChangeEditValue(default_value.value);
				return true;
			case KeyFilterHistoryKey:
				// reset (history)
				UIChangeEditValue(history_value);
				return true;
		}
	}
	return false;
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



StringEditApi::StringEditApi(PrivateTag, const wstring &caption,
		const wstring &value,
		const GUIValue<wstring> &default_value,
		const wstring &history_value,
		Layout control_layout,
		function<void ()> on_value_changed):
	parent(caption, value, default_value, history_value, control_layout, on_value_changed)
{
}

shared_ptr<StringEditApi> StringEditApi::Create(const wstring &caption,
		const wstring &value,
		const GUIValue<wstring> &default_value,
		const wstring &history_value,
		Layout control_layout,
		function<void ()> on_value_changed)
{
	return make_shared<StringEditApi>(PrivateTag(), caption, value,
			default_value,
			history_value,
			control_layout, on_value_changed);
}

void StringEditApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_edit);

	layout = WrapLayout(layout, dialog, control_layout, true);
	auto alignment = control_layout == Layout::Vertical? default_alignment: Qt::AlignBaseline;
	ui_ddi = ddi;

	if (use_caption)
	{
		ui_label = new QLabel(dialog);
		layout->addWidget(ui_label, 0, alignment);
		ui_label->setText(wstring_to_qstring(caption));
	}

	ui_edit = new QLineEdit(dialog);
	ui_edit->setText(FormatValue(ui_value));
	// Заставляем показать начало строки, если вся строка не умещается по ширине поля:
	ui_edit->setCursorPosition(0);
	InstallUIEditFilter(ui_edit);
	// Connect: подключаемся к событию textEdited, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_edit, &QLineEdit::textEdited, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](const QString &)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_edit || !obj->ui_ddi)
					return;
				obj->UISetValue(qstring_to_wstring(obj->ui_edit->text()));
				// См. (*1544536937): В callback текущее состояние не передаем...
				obj->ui_ddi->ProcessCallback([w_obj]()
					{
						// Worker thread
						auto obj = w_obj.lock();
						if (!obj)
							return;
						if (obj->on_value_changed)
							obj->on_value_changed();
					});
			}));
	layout->addWidget(ui_edit, 1, alignment);

	ui_default_button = new QPushButton(dialog);
	SetupMiscButton(ui_default_button);
	ui_default_button->setText(QString::fromUtf8(default_value.is_stored == saved_default_value?
			u8"Reset": u8"Default"));

	auto menu = CreateResetDefaultMenu(ui_default_button,
			FormatValueStd(default_value.value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatValueStd(history_value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});
	ui_default_button->setMenu(menu);
	// QPushButton::clicked: Это событие не работает для кнопки с меню.

	if (control_layout == Layout::Vertical)
	{
		auto button_layout = new QBoxLayout(QBoxLayout::LeftToRight);
		layout->addLayout(button_layout, 0);
		button_layout->addStretch(1);
		button_layout->addWidget(ui_default_button, 0, alignment);
	}
	else
	{
		layout->addWidget(ui_default_button, 0, alignment);
	}
}

void StringEditApi::ReleaseDialog()
{
	parent::ReleaseDialog();
	ui_label = nullptr;
	RemoveUIEditFilter(ui_edit);
	ui_edit = nullptr;
	ui_default_button = nullptr;
}

void StringEditApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_edit);
	if (ui_label)
		ui_label->setEnabled(enable);
	ui_edit->setEnabled(enable);
	ui_default_button->setEnabled(enable);
}

shared_ptr<StringEditBaseApi> StringEditApi::SharedFromThisBase()
{
	return shared_from_this();
}

void StringEditApi::SetEditText(const wstring &v)
{
	if (!ui_edit)
		return;
	ui_edit->setText(FormatValue(v));
	// Заставляем показать начало строки, если вся строка не умещается по ширине поля:
	ui_edit->setCursorPosition(0);
}

wstring StringEditApi::FormatValueStd(const wstring &v, bool compact)
{
	if (!compact)
		return v;
	auto str_u32 = convert_to_u32string(v);
	constexpr size_t max_length = 32;
	if (str_u32.length() < max_length)
		return v;
	return convert_to_wstring(str_u32.substr(0, max_length) + U"...");
}

QString StringEditApi::FormatValue(const wstring &v, bool compact)
{
	return wstring_to_qstring(FormatValueStd(v, compact));
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



TextEditApi::TextEditApi(PrivateTag, const wstring &caption,
		const wstring &value,
		const GUIValue<wstring> &default_value,
		const wstring &history_value,
		Layout control_layout,
		function<void ()> on_value_changed):
	parent(caption, value, default_value, history_value, control_layout, on_value_changed)
{
}

shared_ptr<TextEditApi> TextEditApi::Create(const wstring &caption,
		const wstring &value,
		const GUIValue<wstring> &default_value,
		const wstring &history_value,
		Layout control_layout,
		function<void ()> on_value_changed)
{
	return make_shared<TextEditApi>(PrivateTag(), caption, value,
			default_value,
			history_value,
			control_layout, on_value_changed);
}

void TextEditApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_edit);

	layout = WrapLayout(layout, dialog, control_layout, true);
	auto alignment = control_layout == Layout::Vertical? default_alignment: Qt::AlignBaseline;
	ui_ddi = ddi;

	if (use_caption)
	{
		ui_label = new QLabel(dialog);
		layout->addWidget(ui_label, 0, alignment);
		ui_label->setText(wstring_to_qstring(caption));
	}

	ui_edit = new QTextEdit(dialog);
	ui_edit->setWordWrapMode(QTextOption::NoWrap);
	ui_edit->setPlainText(FormatValue(ui_value));
	InstallUIEditFilter(ui_edit);
	// Connect: подключаемся к событию textChanged, которое вызывается и при ручном,
	// и при программном редактировании. Для определения программного редактирования
	// используется флаг changing_text.
	QObject::connect(ui_edit, &QTextEdit::textChanged, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				obj->DoTextEditTextChanged();
			}));
	layout->addWidget(ui_edit, 1, alignment);

	auto menu = CreateResetDefaultMenu(ui_edit,
			FormatValueStd(default_value.value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatValueStd(history_value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});

	SetWordWrap(false);
	auto ac_word_wrap = new QAction(wstring_to_qstring(L"Автоматический перенос слов"), ui_edit);
	ac_word_wrap->setCheckable(true);
	ac_word_wrap->setChecked(false);
	QObject::connect(ac_word_wrap, &QAction::triggered, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool v)
			{
				if (auto obj = w_obj.lock())
					obj->SetWordWrap(v);
			}));
	menu->addAction(ac_word_wrap);

	SetFixedWidthFont(false);
	auto ac_fixed_width_font = new QAction(wstring_to_qstring(L"Моноширинный шрифт"), ui_edit);
	ac_fixed_width_font->setCheckable(true);
	ac_fixed_width_font->setChecked(false);
	QObject::connect(ac_fixed_width_font, &QAction::triggered, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool v)
			{
				if (auto obj = w_obj.lock())
					obj->SetFixedWidthFont(v);
			}));
	menu->addAction(ac_fixed_width_font);

	ConnectMenu(ui_edit, menu);
}

void TextEditApi::ReleaseDialog()
{
	parent::ReleaseDialog();
	ui_label = nullptr;
	RemoveUIEditFilter(ui_edit);
	ui_edit = nullptr;
}

void TextEditApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_edit);
	if (ui_label)
		ui_label->setEnabled(enable);
	ui_edit->setEnabled(enable);
}

shared_ptr<StringEditBaseApi> TextEditApi::SharedFromThisBase()
{
	return shared_from_this();
}

void TextEditApi::SetEditText(const wstring &v)
{
	if (!ui_edit)
		return;
	changing_text = true;
	try
	{
		ui_edit->setPlainText(FormatValue(v));
	}
	catch (...)
	{
		changing_text = false;
		throw;
	}
	changing_text = false;
}

void TextEditApi::DoTextEditTextChanged()
{
	if (changing_text)
		return;
	if (!ui_edit || !ui_ddi)
		return;
	UISetValue(qstring_to_wstring(ui_edit->toPlainText()));
	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void TextEditApi::SetWordWrap(bool v)
{
	if (!ui_edit)
		return;
	ui_edit->setWordWrapMode(v ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

void TextEditApi::SetFixedWidthFont(bool v)
{
	if (!ui_edit)
		return;
	QTextCursor	cursor_init = ui_edit->textCursor();
	QTextCursor	cursor_all = cursor_init;
	cursor_all.select(QTextCursor::Document);
	ui_edit->setTextCursor(cursor_all);
	ui_edit->setFontFamily(v ? "Courier" : "Helvetica");
	ui_edit->setTextCursor(cursor_init);
}

namespace
{

wstring FormatLineEndsQt(const wstring &text)
{
	// Нормировать концы строк: "\n".
	return normalize_line_ends(text, LineEndKind::N);
}

wstring EncodeMultilineString(const wstring &str)
{
	wstring result(str);
	replace(result.begin(), result.end(), U'\n', U'\u00A6');
	replace(result.begin(), result.end(), U'\r', U'\u00B6');
	return result;
}

} // namespace

wstring TextEditApi::FormatValueStd(const wstring &in_v, bool compact)
{
	auto v = FormatLineEndsQt(in_v);
	if (!compact)
		return v;
	auto str_u32 = convert_to_u32string(v);
	constexpr size_t max_length = 32;
	if (str_u32.length() < max_length)
	{
		return EncodeMultilineString(v);
	}
	return EncodeMultilineString(convert_to_wstring(str_u32.substr(0, max_length) + U"..."));
}

QString TextEditApi::FormatValue(const wstring &v, bool compact)
{
	return wstring_to_qstring(FormatValueStd(v, compact));
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



/*!
	\class NumberEditApi

	Допустимые значения типа T:
	- int
	- unsigned int
	- long long
	- unsigned long long
	- double
	- float
*/

// Явно инстанциируем реализации шаблона для поддерживаемых типов.
// Без этого будут ошибки линковки.
// Среди типов должны быть size_t и ptrdiff_t.
template class NumberEditApi<int>;
template class NumberEditApi<unsigned int>;
template class NumberEditApi<long long>;
template class NumberEditApi<unsigned long long>;
template class NumberEditApi<double>;
template class NumberEditApi<float>;

//--------------------------------------------------------------

template <class T>
NumberEditApi<T>::NumberEditApi(PrivateTag, const wstring &caption,
		value_t value,
		const GUIValue<value_t> &default_value,
		value_t history_value,
		value_t min_value,
		value_t max_value,
		out_of_range_control allow_out_of_range,
		Layout control_layout,
		function<void ()> on_value_changed):
	use_caption(!caption.empty()),
	caption(caption),
	default_value(default_value),
	history_value(history_value),
	allow_out_of_range(allow_out_of_range),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	ui_min_value(min_value),
	ui_max_value(max_value),
	ui_value(value),
	ui_value_valid(true)
{
	if (allow_out_of_range != out_of_range_allowed)
	{
		if (max_value < min_value)
			throw invalid_argument("NumberEditApi: Invalid min-max values.");
		if (!in_range(value, min_value, max_value)) // Проверка независимо от value_initialized.
			throw invalid_argument("NumberEditApi: Invalid initial value (out of range).");
		if (!in_range(default_value.value, min_value, max_value))
			throw invalid_argument("NumberEditApi: Invalid default value (out of range).");
	}
}

template <class T>
shared_ptr<NumberEditApi<T>> NumberEditApi<T>::Create(const wstring &caption,
		value_t value,
		const GUIValue<value_t> &default_value,
		value_t history_value,
		value_t min_value,
		value_t max_value,
		out_of_range_control allow_out_of_range,
		Layout control_layout,
		function<void ()> on_value_changed)
{
	return make_shared<NumberEditApi<T>>(PrivateTag(), caption,
			value,
			default_value,
			history_value,
			min_value, max_value,
			allow_out_of_range,
			control_layout, on_value_changed);
}

template <class T>
auto NumberEditApi<T>::UIValue() const -> value_t
{
	std::unique_lock<std::mutex> locker(ui_value_mutex);
	return ui_value;
}

template <class T>
void NumberEditApi<T>::SetValue(value_t v, value_t min_value_, value_t max_value_)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), v,
			min_value_, max_value_]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			obj->UIChangeEditValue(v, min_value_, max_value_);
		});
}

template <class T>
bool NumberEditApi<T>::UISetValue(value_t v)
{
	bool err = false;
	value_t validated_v = ValidateValue(v, &err);
	UISetValueValid(!err);
	if (validated_v == ui_value)
		return !err;
	std::unique_lock<std::mutex> locker(ui_value_mutex);
	ui_value = validated_v;
	return !err;
}

template <class T>
void NumberEditApi<T>::UISetValueValid(bool valid)
{
	ui_value_valid = valid;
	VisualizeValueValid(valid);
}

template <class T>
void NumberEditApi<T>::UIChangeEditValue(value_t v)
{
	UIChangeEditValue(v, ui_min_value, ui_max_value);
}

template <class T>
void NumberEditApi<T>::UIChangeEditValue(value_t v, value_t min_value_, value_t max_value_)
{
	if (!ui_edit || !ui_ddi)
		return;
	if (min_value_ != ui_min_value)
	{
		ui_min_value = min_value_;
		ui_min_button->setText(wstring_to_qstring(ssprintf(L"Min: %ls",
				EnsureType<const wchar_t*>(FormatValueStd(ui_min_value, true).c_str()))));
	}
	if (max_value_ != ui_max_value)
	{
		ui_max_value = max_value_;
		ui_max_button->setText(wstring_to_qstring(ssprintf(L"Max: %ls",
				EnsureType<const wchar_t*>(FormatValueStd(ui_max_value, true).c_str()))));
	}
	value_t validated_v = ValidateValue(v);

	ui_edit->setText(FormatValue(validated_v));
	// Заставляем показать начало числа, если всё число не умещается по ширине поля:
	ui_edit->setCursorPosition(0);
	UISetValueValid(true);

	if (validated_v == ui_value)
		return;

	{
		std::unique_lock<std::mutex> locker(ui_value_mutex);
		ui_value = validated_v;
	}

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

template <class T>
void NumberEditApi<T>::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
}

template <class T>
void NumberEditApi<T>::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_edit);

	layout = WrapLayout(layout, dialog, control_layout, true);
	auto alignment = control_layout == Layout::Vertical? default_alignment: Qt::AlignBaseline;
	ui_ddi = ddi;

	if (use_caption)
	{
		ui_label = new QLabel(dialog);
		layout->addWidget(ui_label, 0, alignment);
		ui_label->setText(wstring_to_qstring(caption));
	}

	ui_edit = new QLineEdit(dialog);
	ui_edit->setText(FormatValue(ui_value));
	// Заставляем показать начало числа, если всё число не умещается по ширине поля:
	ui_edit->setCursorPosition(0);
	ui_edit_filter = new KeyPressEventFilter(QtEventSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](QObject *, QKeyEvent *event)
			{
				auto obj = w_obj.lock();
				if (!obj)
					return false;
				if (!obj->ui_edit || !obj->ui_ddi)
					return false;
				return obj->ProcessUIEditKeyPress(event);
			}),
			ui_edit);
	ui_edit->installEventFilter(ui_edit_filter);
	// Connect: подключаемся к событию textEdited, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_edit, &QLineEdit::textEdited, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](const QString &)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (!obj->ui_edit || !obj->ui_ddi)
					return;
				value_t v;
				bool valid = ParseValue(&v, obj->ui_edit->text());
				if (valid)
				{
					valid = obj->UISetValue(v);
				}
				else
				{
					obj->UISetValueValid(false);
				}
				// См. (*1544536937): В callback текущее состояние не передаем...
				obj->ui_ddi->ProcessCallback([w_obj]()
					{
						// Worker thread
						auto obj = w_obj.lock();
						if (!obj)
							return;
						if (obj->on_value_changed)
							obj->on_value_changed();
					});
			}));
	layout->addWidget(ui_edit, 1, alignment);
	ui_edit_default_palette = ui_edit->palette();

	ui_min_button = new QPushButton(dialog);
	SetupMiscButton(ui_min_button);
	ui_min_button->setText(wstring_to_qstring(ssprintf(L"Min: %ls",
			EnsureType<const wchar_t*>(FormatValueStd(ui_min_value, true).c_str()))));
	QObject::connect(ui_min_button, &QPushButton::clicked, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->ui_min_value);
			}));

	ui_max_button = new QPushButton(dialog);
	SetupMiscButton(ui_max_button);
	ui_max_button->setText(wstring_to_qstring(ssprintf(L"Max: %ls",
			EnsureType<const wchar_t*>(FormatValueStd(ui_max_value, true).c_str()))));
	QObject::connect(ui_max_button, &QPushButton::clicked, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->ui_max_value);
			}));

	ui_default_button = new QPushButton(dialog);
	SetupMiscButton(ui_default_button);
	ui_default_button->setText(QString::fromUtf8(default_value.is_stored == saved_default_value?
			u8"Reset": u8"Default"));

	auto menu = CreateResetDefaultMenu(ui_default_button,
			FormatValueStd(default_value.value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatValueStd(history_value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});
	ui_default_button->setMenu(menu);
	// QPushButton::clicked: Это событие не работает для кнопки с меню.

	if (control_layout == Layout::Vertical)
	{
		auto button_layout = new QBoxLayout(QBoxLayout::LeftToRight);
		layout->addLayout(button_layout, 0);
		button_layout->addStretch(1);
		button_layout->addWidget(ui_min_button, 0, alignment);
		button_layout->addWidget(ui_max_button, 0, alignment);
		button_layout->addWidget(ui_default_button, 0, alignment);
	}
	else
	{
		layout->addWidget(ui_min_button, 0, alignment);
		layout->addWidget(ui_max_button, 0, alignment);
		layout->addWidget(ui_default_button, 0, alignment);
	}
}

template <class T>
void NumberEditApi<T>::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_label = nullptr;
	if (ui_edit && ui_edit_filter)
	{
		ui_edit->removeEventFilter(ui_edit_filter);
	}
	ui_edit_filter = nullptr;
	ui_edit = nullptr;
	ui_default_button = nullptr;
	ui_min_button = nullptr;
	ui_max_button = nullptr;
}

template <class T>
bool NumberEditApi<T>::TryAccept(QString *err_message)
{
	XRAD_ASSERT_THROW(ui_edit);
	if (!ui_enabled)
		return true;
	if (ui_value_valid)
		return true;

	ui_edit->setFocus();
	ui_edit->selectAll();
	auto *message_details = L"Недопустимое значение или значение вне допустимого диапазона";
	wstring message;
	if (caption.length())
	{
		message = ssprintf(L"%ls: %ls.",
				EnsureType<const wchar_t*>(caption.c_str()),
				EnsureType<const wchar_t*>(message_details));
	}
	else
	{
		message = ssprintf(L"%ls.",
				EnsureType<const wchar_t*>(message_details));
	}
	*err_message = wstring_to_qstring(message);
	return false;
}

template <class T>
void NumberEditApi<T>::Accept()
{
}

template <class T>
void NumberEditApi<T>::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_edit);
	ui_enabled = enable;
	if (ui_label)
		ui_label->setEnabled(enable);
	ui_edit->setEnabled(enable);
	ui_default_button->setEnabled(enable);
	ui_min_button->setEnabled(enable);
	ui_max_button->setEnabled(enable);
}

template <class T>
void NumberEditApi<T>::VisualizeValueValid(bool valid)
{
	if (valid)
	{
		ui_edit->setPalette(ui_edit_default_palette);
	}
	else
	{
		auto palette = ui_edit_default_palette;
		palette.setColor(QPalette::Highlight, Qt::red);
		palette.setColor(QPalette::HighlightedText, Qt::white);
#if QT_VERSION <= 0x50600
		palette.setColor(QPalette::Text, Qt::red);

#elif QT_VERSION > 0x50600 && QT_VERSION < 0x50905
#error Try this code:
		// Для версий Qt 5.6.1--5.9.4 код ниже не тестировался
		// При появлении версий из этого диапазона тестировать
		// и вносить исправление в диапазоны условий #if
#else
		// Этот код дает неправильное отображение при наведении мыши в Qt 5.6.0 и более ранних.
		// В версии 5.9.5 все работает корректно.
		// - https://bugreports.qt.io/browse/QTBUG-42575
		// - https://bugreports.qt.io/browse/QTBUG-40634
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::white);
		ui_edit->setAutoFillBackground(true);
#endif
		ui_edit->setPalette(palette);
	}
}

template <class T>
bool NumberEditApi<T>::ProcessUIEditKeyPress(QKeyEvent *event)
{
	auto modifiers = event->modifiers();
	auto key = event->key();
	if ((modifiers & KeyFilterModifiersMask) == KeyFilterCursorModifiers)
	{
		switch (key)
		{
			case KeyFilterMaxKey:
				UIChangeEditValue(ui_max_value);
				// max
				return true;
			case KeyFilterMinKey:
				// min
				UIChangeEditValue(ui_min_value);
				return true;
			case KeyFilterIncrementKey:
				// inc (+1)
				if (ui_value < ui_max_value)
					UIChangeEditValue(ui_value + 1);
				return true;
			case KeyFilterDecrementKey:
				// dec (-1)
				if (ui_value > ui_min_value)
					UIChangeEditValue(ui_value - 1);
				return true;
		}
	}
	else if ((modifiers & KeyFilterModifiersMask) == KeyFilterAlphaModifiers)
	{
		switch (key)
		{
			case KeyFilterDefaultKey:
				// default
				UIChangeEditValue(default_value.value);
				return true;
			case KeyFilterHistoryKey:
				// reset (history)
				UIChangeEditValue(history_value);
				return true;
		}
	}
	return false;
}

template <class T>
auto NumberEditApi<T>::ValidateValue(value_t v, bool *err) -> value_t
{
	if (allow_out_of_range == out_of_range_allowed)
		return v;
	if (v < ui_min_value)
	{
		if (err)
			*err = true;
		return ui_min_value;
	}
	if (v > ui_max_value)
	{
		if (err)
			*err = true;
		return ui_max_value;
	}
	return v;
}

template <>
wstring NumberEditApi<int>::FormatValueStd(value_t v, bool compact)
{
	return ssprintf(L"%i", EnsureType<int>(v));
}

template <>
wstring NumberEditApi<unsigned int>::FormatValueStd(value_t v, bool compact)
{
	return ssprintf(L"%u", EnsureType<unsigned int>(v));
}

template <>
wstring NumberEditApi<long long>::FormatValueStd(value_t v, bool compact)
{
	return ssprintf(L"%lli", EnsureType<long long>(v));
}

template <>
wstring NumberEditApi<unsigned long long>::FormatValueStd(value_t v, bool compact)
{
	return ssprintf(L"%llu", EnsureType<unsigned long long>(v));
}

template <>
wstring NumberEditApi<double>::FormatValueStd(value_t v, bool compact)
{
	if (compact)
		return ssprintf(L"%.6lg", EnsureType<double>(v));
	return ssprintf(L"%.16lg", EnsureType<double>(v));
}

template <>
wstring NumberEditApi<float>::FormatValueStd(value_t v, bool compact)
{
	if (compact)
		return ssprintf(L"%.6g", EnsureType<float>(v));
	return ssprintf(L"%.7g", EnsureType<float>(v));
}

template <class T>
QString NumberEditApi<T>::FormatValue(value_t v, bool compact)
{
	return wstring_to_qstring(FormatValueStd(v, compact));
}

namespace
{

// Внимание! Функция scanf работает некорректно, если в строке слишком большое число.
// Узнать о переполнении нельзя. Нужно использовать strtoll / strtoull и errno.

//! \param c_str Строка, длина на 1 больше length, c_str[length] = 0.
bool ParseValue_ull(unsigned long long *o_value, const char *c_str, size_t length)
{
	size_t start = 0;
	while (start < length)
	{
		if (!isspace(c_str[start]))
			break;
		++start;
	}
	if (start == length)
		return false;
	if (start + 1 <= length && c_str[start] == '-')
	{
		// Запрещаем использование знака, даже в виде "-0".
		return false;
	}
	int radix = 10;
	if (start + 2 <= length && c_str[start] == '0' && c_str[start+1] == 'x')
	{
		radix = 16;
		start += 2;
	}
	char *end = nullptr;
	errno = 0;
	unsigned long long value = strtoull(c_str + start, &end, radix);
	if (errno)
		return false;

	size_t pos = end - c_str;
	if (pos == start)
		return false;
	while (pos < length)
	{
		if (!isspace(c_str[pos]))
			return false;
		++pos;
	}
	*o_value = value;
	return true;
}

//! \param c_str Строка, длина на 1 больше length, c_str[length] = 0.
bool ParseValue_ll(long long *o_value, const char *c_str, size_t length)
{
	size_t start = 0;
	while (start < length)
	{
		if (!isspace(c_str[start]))
			break;
		++start;
	}
	if (start == length)
		return false;
	bool negative = false;
	if (start + 1 <= length && c_str[start] == '-')
	{
		negative = true;
		++start;
	}
	int radix = 10;
	if (start + 2 <= length && c_str[start] == '0' && c_str[start+1] == 'x')
	{
		radix = 16;
		start += 2;
	}
	char *end = nullptr;
	errno = 0;
	unsigned long long value_u = strtoull(c_str + start, &end, radix);
	if (errno)
		return false;

	long long value;
	if (!negative)
	{
		if (value_u > (unsigned long long)numeric_limits<long long>::max())
			return false;
		value = (long long)value_u;
	}
	else
	{
		// Здесь хитрый код смены знака, работающий в случае разных бинарных представлений
		// отрицательных чисел.
		// Сложность возникает, если numeric_limits<long long>::min() по модулю превышает
		// numeric_limits<long long>::max(), как в случае дополнительного кода.

		// Удостоверяемся, что min() по модулю равен max() или больше него на 1. Других представлений
		// знакопеременных целых чисел на практике не используется.
		constexpr long long max_min_sum =
				numeric_limits<long long>::max() + numeric_limits<long long>::min();
		static_assert(max_min_sum == 0 || max_min_sum == -1,
				"Unexpected signed number representation.");

		if (value_u <= (unsigned long long)numeric_limits<long long>::max())
		{
			value = -(long long)value_u;
		}
		else
		{
			unsigned long long value_u_x = value_u - (unsigned long long)numeric_limits<long long>::max();
			if (value_u_x > (unsigned long long)-max_min_sum)
				return false;
			value = -numeric_limits<long long>::max() - (long long)value_u_x;
		}
	}

	size_t pos = end - c_str;
	if (pos == start)
		return false;
	while (pos < length)
	{
		if (!isspace(c_str[pos]))
			return false;
		++pos;
	}
	*o_value = value;
	return true;
}

bool ParseValueD(double *o_value, const char *c_str, size_t length)
{
	char *end = nullptr;
	auto value = strtod(c_str, &end);
	size_t pos = end - c_str;
	if (!pos)
		return false;
	while (pos < length)
	{
		if (!isspace(c_str[pos]))
			return false;
		++pos;
	}
	*o_value = value;
	return true;
}

bool ParseValueF(float *o_value, const char *c_str, size_t length)
{
	char *end = nullptr;
	auto value = strtod(c_str, &end);
	size_t pos = end - c_str;
	if (!pos)
		return false;
	while (pos < length)
	{
		if (!isspace(c_str[pos]))
			return false;
		++pos;
	}
	if (value > numeric_limits<float>::max() || value < numeric_limits<float>::lowest())
		return false;
	*o_value = value;
	return true;
}

} // namespace

template <>
bool NumberEditApi<int>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	long long value_ll;
	if (!ParseValue_ll(&value_ll, ss.c_str(), ss.length()))
		return false;
	if (value_ll > numeric_limits<int>::max() ||
			value_ll < numeric_limits<int>::min())
		return false;
	*o_value = (int)value_ll;
	return true;
}

template <>
bool NumberEditApi<unsigned int>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	unsigned long long value_ull;
	if (!ParseValue_ull(&value_ull, ss.c_str(), ss.length()))
		return false;
	if (value_ull > numeric_limits<unsigned int>::max())
		return false;
	*o_value = (unsigned int)value_ull;
	return true;
}

template <>
bool NumberEditApi<long long>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	return ParseValue_ll(o_value, ss.c_str(), ss.length());
}

template <>
bool NumberEditApi<unsigned long long>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	return ParseValue_ull(o_value, ss.c_str(), ss.length());
}

template <>
bool NumberEditApi<double>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	return ParseValueD(o_value, ss.c_str(), ss.length());
}

template <>
bool NumberEditApi<float>::ParseValue(value_t *o_value, QString str)
{
	string ss = qstring_to_string(str);
	return ParseValueF(o_value, ss.c_str(), ss.length());
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



auto FileNameEditApi::CreateFileLoadValidator(const wstring &file_type) ->
		unique_ptr<FileNameValidator>
{
	return make_unique<FileLoadValidator>(file_type);
}

auto FileNameEditApi::CreateFileSaveValidator(const wstring &file_type) ->
		unique_ptr<FileNameValidator>
{
	return make_unique<FileSaveValidator>(file_type);
}

auto FileNameEditApi::CreateDirectoryReadValidator() ->
		unique_ptr<FileNameValidator>
{
	return make_unique<DirectoryReadValidator>();
}

auto FileNameEditApi::CreateDirectoryWriteValidator() ->
		unique_ptr<FileNameValidator>
{
	return make_unique<DirectoryWriteValidator>();
}

FileNameEditApi::FileNameEditApi(PrivateTag,
		unique_ptr<FileNameValidator> &&validator,
		const wstring &caption,
		const value_t &value,
		const GUIValue<value_t> &default_value,
		value_t history_value,
		Layout control_layout,
		function<void ()> on_value_changed):
	validator(std::move(validator)),
	use_caption(!caption.empty()),
	caption(caption),
	default_value(default_value),
	history_value(history_value),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	ui_value(value),
	ui_value_validity(ValueValidity::Unchecked)
{
	ui_value_validity = this->validator->Validate(ui_value);
}

shared_ptr<FileNameEditApi> FileNameEditApi::Create(
		unique_ptr<FileNameValidator> &&validator,
		const wstring &caption,
		const value_t &value,
		const GUIValue<value_t> &default_value,
		value_t history_value,
		Layout control_layout,
		function<void ()> on_value_changed)
{
	return make_shared<FileNameEditApi>(PrivateTag(),
			std::move(validator),
			caption,
			value,
			default_value,
			history_value,
			control_layout,
			on_value_changed);
}

auto FileNameEditApi::UIValue() const -> pair<value_t, ValueValidity>
{
	std::unique_lock<std::mutex> locker(ui_value_mutex);
	return make_pair(ui_value, ui_value_validity);
}

void FileNameEditApi::SetValue(value_t v)
{
	if (!dialog_api_interface)
		return;
	dialog_api_interface->ProcessUICallback([w_obj = to_weak_ptr(shared_from_this()), v]()
		{
			// UI thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			obj->UIChangeEditValue(v);
		});
}

void FileNameEditApi::DoEditTextEdited()
{
	if (!ui_edit || !ui_ddi)
		return;
	auto v = ParseValue(ui_edit->text());
	if (v == ui_value)
		return;
	auto validity = validator->Validate(v);
	if (validity != ui_value_validity)
		VisualizeValueValidity(validity);

	// Изменяем значения под мьютексом
	{
		std::unique_lock<std::mutex> locker(ui_value_mutex);
		ui_value = v;
		ui_value_validity = validity;
	}
	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
			{
				// Worker thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				if (obj->on_value_changed)
					obj->on_value_changed();
			});
}

void FileNameEditApi::UISetValueValidity(ValueValidity validity)
{
	if (validity == ui_value_validity)
		return;
	VisualizeValueValidity(validity);
	{
		std::unique_lock<std::mutex> locker(ui_value_mutex);
		ui_value_validity = validity;
	}
}

void FileNameEditApi::UIChangeEditValue(value_t in_v, ValueValidity i_validity)
{
	if (!ui_edit || !ui_ddi)
		return;
	auto v = ParseValue(in_v);
	auto validity = i_validity != ValueValidity::Unchecked? i_validity: validator->Validate(v);

	ui_edit->setText(FormatValue(v));
	VisualizeValueValidity(validity);

	if (v == ui_value && validity == ui_value_validity)
		return;

	{
		std::unique_lock<std::mutex> locker(ui_value_mutex);
		ui_value = v;
		ui_value_validity = validity;
	}

	// См. (*1544536937): В callback текущее состояние не передаем...
	ui_ddi->ProcessCallback([w_obj = to_weak_ptr(shared_from_this())]()
		{
			// Worker thread
			auto obj = w_obj.lock();
			if (!obj)
				return;
			if (obj->on_value_changed)
				obj->on_value_changed();
		});
}

void FileNameEditApi::SetDialogApiInterface(DialogApiInterface *dialog_api)
{
	dialog_api_interface = dialog_api;
}

void FileNameEditApi::AddToDialog(QDialog *dialog, QBoxLayout *layout,
		DynamicDialogInterface *ddi,
		const wstring &settings_path)
{
	XRAD_ASSERT_THROW(!ui_edit);

	layout = WrapLayout(layout, dialog, control_layout, true);
	auto alignment = control_layout == Layout::Vertical? default_alignment: Qt::AlignBaseline;
	ui_ddi = ddi;

	if (use_caption)
	{
		ui_label = new QLabel(dialog);
		layout->addWidget(ui_label, 0, alignment);
		ui_label->setText(wstring_to_qstring(caption));
	}

	ui_edit = new QLineEdit(dialog);
	ui_edit->setText(FormatValue(ui_value));
	ui_edit_filter = new KeyPressEventFilter(QtEventSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](QObject *, QKeyEvent *event)
			{
				auto obj = w_obj.lock();
				if (!obj)
					return false;
				if (!obj->ui_edit || !obj->ui_ddi)
					return false;
				return obj->ProcessUIEditKeyPress(event);
			}),
			ui_edit);
	ui_edit->installEventFilter(ui_edit_filter);
	// Connect: подключаемся к событию textEdited, которое вызывается только при ручном
	// редактировании, не вызывается при программном редактировании.
	QObject::connect(ui_edit, &QLineEdit::textEdited, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](const QString &)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj)
					return;
				obj->DoEditTextEdited();
			}));
	layout->addWidget(ui_edit, 1, alignment);
	ui_edit_default_palette = ui_edit->palette();

	ui_browse_button = new QPushButton(dialog);
	SetupMiscButton(ui_browse_button);
	ui_browse_button->setText(wstring_to_qstring(L"Обзор..."));
	QObject::connect(ui_browse_button, &QPushButton::clicked, QtSlotSafeExecute(
			[w_obj = to_weak_ptr(shared_from_this())](bool)
			{
				// UI thread
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->Browse();
			}));

	ConnectResetDefaultMenu(ui_edit,
			FormatValueStd(default_value.value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->default_value.value);
			},
			FormatValueStd(history_value, true),
			[w_obj = to_weak_ptr(shared_from_this())]()
			{
				auto obj = w_obj.lock();
				if (!obj || !obj->ui_ddi)
					return;
				obj->UIChangeEditValue(obj->history_value);
			});

	if (control_layout == Layout::Vertical)
	{
		auto button_layout = new QBoxLayout(QBoxLayout::LeftToRight);
		layout->addLayout(button_layout, 0);
		button_layout->addStretch(1);
		button_layout->addWidget(ui_browse_button, 0, alignment);
	}
	else
	{
		layout->addWidget(ui_browse_button, 0, alignment);
	}

	VisualizeValueValidity(ui_value_validity);
}

void FileNameEditApi::ReleaseDialog()
{
	ui_ddi = nullptr;
	ui_label = nullptr;
	if (ui_edit && ui_edit_filter)
	{
		ui_edit->removeEventFilter(ui_edit_filter);
	}
	ui_edit_filter = nullptr;
	ui_edit = nullptr;
	ui_browse_button = nullptr;
}

bool FileNameEditApi::TryAccept(QString *err_message)
{
	XRAD_ASSERT_THROW(ui_edit);
	if (!ui_enabled)
		return true;
	if (ui_value_validity == ValueValidity::Valid)
		return true;

	wstring validator_message;
	auto validity = validator->ValidateUI(ui_value, &validator_message, ui_ddi);
	UISetValueValidity(validity);
	if (validity == ValueValidity::Valid)
		return true;

	ui_edit->setFocus();
	ui_edit->selectAll();
	if (!validator_message.empty())
	{
		wstring message;
		if (caption.length())
		{
			message = ssprintf(L"%ls: %ls",
					EnsureType<const wchar_t*>(caption.c_str()),
					EnsureType<const wchar_t*>(validator_message.c_str()));
		}
		else
		{
			message = ssprintf(L"%ls",
					EnsureType<const wchar_t*>(validator_message.c_str()));
		}
		*err_message = wstring_to_qstring(message);
	}
	return false;
}

void FileNameEditApi::Accept()
{
}

void FileNameEditApi::Enable(bool enable)
{
	XRAD_ASSERT_THROW(ui_edit);
	ui_enabled = enable;
	if (ui_label)
		ui_label->setEnabled(enable);
	ui_edit->setEnabled(enable);
	ui_browse_button->setEnabled(enable);
}

void FileNameEditApi::VisualizeValueValidity(ValueValidity validity)
{
	if (validity != ValueValidity::Invalid)
	{
		ui_edit->setPalette(ui_edit_default_palette);
	}
	else
	{
		auto palette = ui_edit_default_palette;
		palette.setColor(QPalette::Highlight, Qt::red);
		palette.setColor(QPalette::HighlightedText, Qt::white);

#if QT_VERSION <= 0x50600
		palette.setColor(QPalette::Text, Qt::red);

#elif QT_VERSION > 0x50600 && QT_VERSION < 0x50905
#error Try this code:
		// Для версий Qt 5.6.1--5.9.4 код ниже не тестировался
		// При появлении версий из этого диапазона тестировать
		// и вносить исправление в диапазоны условий #if
#else
		// Этот код дает неправильное отображение при наведении мыши в Qt 5.6.0 и более ранних.
		// В версии 5.9.5 все работает корректно.
		// - https://bugreports.qt.io/browse/QTBUG-42575
		// - https://bugreports.qt.io/browse/QTBUG-40634
		palette.setColor(QPalette::Base, Qt::red);
		palette.setColor(QPalette::Text, Qt::white);
		ui_edit->setAutoFillBackground(true);
#endif
		ui_edit->setPalette(palette);
	}
}

bool FileNameEditApi::ProcessUIEditKeyPress(QKeyEvent *event)
{
	auto modifiers = event->modifiers();
	auto key = event->key();
	if ((modifiers & KeyFilterModifiersMask) == KeyFilterAlphaModifiers)
	{
		switch (key)
		{
			case KeyFilterBrowseKey:
				Browse();
				return true;
			case KeyFilterDefaultKey:
				// default
				UIChangeEditValue(default_value.value);
				return true;
			case KeyFilterHistoryKey:
				// reset (history)
				UIChangeEditValue(history_value);
				return true;
		}
	}
	return false;
}

void FileNameEditApi::Browse()
{
	if (!ui_ddi)
		return;
	auto filename = validator->Browse(caption, ui_value, ui_ddi->GetDialog());
	if (filename.empty())
		return;
	UIChangeEditValue(filename, ValueValidity::Valid);
}

wstring FileNameEditApi::FormatValueStd(value_t v, bool compact)
{
	if (!compact)
		return GetPathNativeFromGeneric(v);
	auto str_u32 = convert_to_u32string(v);
	constexpr size_t max_length = 64;
	if (str_u32.length() < max_length)
		return v;
	// Вырезать середину строки, при этом учесть разделители пути.
	size_t sep_pos = str_u32.find_last_of(U"/\\");
	if (sep_pos == str_u32.npos)
		sep_pos = 0;
	if (str_u32.length() - sep_pos >= max_length)
	{
		return convert_to_wstring(U"..." + str_u32.substr(str_u32.length() - max_length));
	}
	return GetPathNativeFromGeneric(
			convert_to_wstring(str_u32.substr(0, max_length - (str_u32.length() - sep_pos)) +
				U"..." +
				str_u32.substr(sep_pos)));
}

QString FileNameEditApi::FormatValue(value_t v, bool compact)
{
	return wstring_to_qstring(FormatValueStd(v, compact));
}

wstring FileNameEditApi::ParseValue(const wstring &v)
{
	return GetPathGenericFromAutodetect(v);
}

wstring FileNameEditApi::ParseValue(const QString &v)
{
	return ParseValue(qstring_to_wstring(v));
}

//--------------------------------------------------------------

wstring FileNameEditApi::FileLoadValidator::Browse(
		const wstring &caption, const wstring &current_filename, QWidget *parent)
{
	auto return_file_name = QFileDialog::getOpenFileName(parent, wstring_to_qstring(caption),
			wstring_to_qstring(current_filename), wstring_to_qstring(file_type));
	if (return_file_name.isEmpty())
		return wstring();
	return qstring_to_wstring(return_file_name);
}

auto FileNameEditApi::FileLoadValidator::Validate(
		const wstring &filename) -> ValueValidity
{
	return DoValidate(filename, nullptr);
}

auto FileNameEditApi::FileLoadValidator::ValidateUI(
		const wstring &filename, wstring *err_message, DynamicDialogInterface *ddi) -> ValueValidity
{
	return DoValidate(filename, err_message);
}

auto FileNameEditApi::FileLoadValidator::DoValidate(const wstring &filename, wstring *err_message)
		-> ValueValidity
{
	if (filename.empty())
	{
		if (err_message)
			*err_message = L"Имя файла не задано.";
		return ValueValidity::Invalid;
	}
	if (!FileExists(filename))
	{
		if (err_message)
		{
			*err_message = ssprintf(L"Файл с указанным именем не найден:\n\"%ls\".",
					EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename).c_str()));
		}
		return ValueValidity::Invalid;
	}
	return ValueValidity::Valid;
}

//--------------------------------------------------------------

wstring FileNameEditApi::FileSaveValidator::Browse(
		const wstring &caption, const wstring &current_filename, QWidget *parent)
{
	auto return_file_name = QFileDialog::getSaveFileName(parent, wstring_to_qstring(caption),
			wstring_to_qstring(current_filename), wstring_to_qstring(file_type));
	if (return_file_name.isEmpty())
		return wstring();
	return qstring_to_wstring(return_file_name);
}

auto FileNameEditApi::FileSaveValidator::Validate(
		const wstring &filename) -> ValueValidity
{
	ExtendedReason ex_reason = ExtendedReason::None;
	return DoValidate(filename, &ex_reason);
}

auto FileNameEditApi::FileSaveValidator::ValidateUI(
		const wstring &filename, wstring *err_message, DynamicDialogInterface *ddi) -> ValueValidity
{
	ExtendedReason ex_reason = ExtendedReason::None;
	wstring path;
	wstring filename_without_path;
	auto validity = DoValidate(filename, &ex_reason, &path, &filename_without_path);
	if (validity == ValueValidity::Valid)
		return validity;
	switch (ex_reason)
	{
		case ExtendedReason::EmptyFileName:
			*err_message = L"Имя файла не задано.";
			return ValueValidity::Invalid;

		case ExtendedReason::DirectoryNotFound:
			if (!ddi->AskYesNo(
					ssprintf(L"Папка \"%ls\" для файла \"%ls\" не существует.\nСоздать папку?",
							EnsureType<const wchar_t*>(GetPathNativeFromGeneric(path).c_str()),
							EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename_without_path).c_str()))))
			{
				// err_message = empty
				return ValueValidity::Invalid;
			}
			if (!CreatePath(path))
			{
				*err_message = ssprintf(L"Ошибка создания папки \"%ls\" для файла \"%ls\".",
						EnsureType<const wchar_t*>(GetPathNativeFromGeneric(path).c_str()),
						EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename_without_path).c_str()));
				return ValueValidity::Invalid;
			}
			return ValueValidity::Valid;

		case ExtendedReason::FileExists:
			if (!ddi->AskYesNo(
					ssprintf(L"Файл с указанным именем уже существует:\n\"%ls\".\nПерезаписать?",
							EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename).c_str()))))
			{
				// err_message = empty
				return ValueValidity::Invalid;
			}
			return ValueValidity::Valid;

		case ExtendedReason::ObjectIsDirectory:
			*err_message = ssprintf(L"Заданное имя ссылается на папку:\n\"%ls\".",
					GetPathNativeFromGeneric(filename).c_str());
			return ValueValidity::Invalid;

		default:
			*err_message = L"Внутренняя ошибка в FileSaveValidator::ValidateUI: неверный код ошибки.";
			return ValueValidity::Invalid;
	}
}

auto FileNameEditApi::FileSaveValidator::DoValidate(
		const wstring &filename,
		ExtendedReason *ex_reason,
		wstring *o_path,
		wstring *o_filename_without_path) -> ValueValidity
{
	if (filename.empty())
	{
		*ex_reason = ExtendedReason::EmptyFileName;
		return ValueValidity::Invalid;
	}
	if (FileExists(filename))
	{
		*ex_reason = ExtendedReason::FileExists;
		return ValueValidity::Invalid;
	}
	if (DirectoryExists(filename))
	{
		*ex_reason = ExtendedReason::ObjectIsDirectory;
		return ValueValidity::Invalid;
	}
	wstring path, filename_without_path;
	SplitFilename(filename, &path, &filename_without_path);
	if (path.length() > 1)
	{
		path.resize(path.length() - 1); // удаляем концевой '/'
		if (!DirectoryExists(path))
		{
			*ex_reason = ExtendedReason::DirectoryNotFound;
			if (o_path)
				*o_path = path;
			if (o_filename_without_path)
				*o_filename_without_path = filename_without_path;
			return ValueValidity::Invalid;
		}
	}
	return ValueValidity::Valid;
}

//--------------------------------------------------------------

wstring FileNameEditApi::DirectoryReadValidator::Browse(
		const wstring &caption, const wstring &current_filename, QWidget *parent)
{
	auto return_file_name = QFileDialog::getExistingDirectory(parent,
			wstring_to_qstring(caption),
			wstring_to_qstring(current_filename),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	return qstring_to_wstring(return_file_name);
}

auto FileNameEditApi::DirectoryReadValidator::Validate(
		const wstring &filename) -> ValueValidity
{
	return DoValidate(filename, nullptr);
}

auto FileNameEditApi::DirectoryReadValidator::ValidateUI(
		const wstring &filename, wstring *err_message, DynamicDialogInterface *ddi) -> ValueValidity
{
	return DoValidate(filename, err_message);
}

auto FileNameEditApi::DirectoryReadValidator::DoValidate(const wstring &filename, wstring *err_message)
		-> ValueValidity
{
	if (filename.empty())
	{
		if (err_message)
			*err_message = L"Имя папки не задано.";
		return ValueValidity::Invalid;
	}
	if (!DirectoryExists(filename))
	{
		if (err_message)
		{
			*err_message = ssprintf(L"Папка с указанным именем не найдена:\n\"%ls\".",
					EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename).c_str()));
		}
		return ValueValidity::Invalid;
	}
	return ValueValidity::Valid;
}

//--------------------------------------------------------------

wstring FileNameEditApi::DirectoryWriteValidator::Browse(
		const wstring &caption, const wstring &current_filename, QWidget *parent)
{
	auto return_file_name = QFileDialog::getExistingDirectory(parent,
			wstring_to_qstring(caption),
			wstring_to_qstring(current_filename),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	return qstring_to_wstring(return_file_name);
}

auto FileNameEditApi::DirectoryWriteValidator::Validate(
		const wstring &filename) -> ValueValidity
{
	ExtendedReason ex_reason = ExtendedReason::None;
	return DoValidate(filename, &ex_reason);
}

auto FileNameEditApi::DirectoryWriteValidator::ValidateUI(
		const wstring &filename, wstring *err_message, DynamicDialogInterface *ddi) -> ValueValidity
{
	ExtendedReason ex_reason = ExtendedReason::None;
	auto validity = DoValidate(filename, &ex_reason);
	if (validity == ValueValidity::Valid)
		return validity;
	switch (ex_reason)
	{
		case ExtendedReason::EmptyFileName:
			*err_message = L"Имя папки не задано.";
			return ValueValidity::Invalid;

		case ExtendedReason::DirectoryNotFound:
			if (!ddi->AskYesNo(
					ssprintf(L"Папка \"%ls\" не существует.\nСоздать папку?",
							EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename).c_str()))))
			{
				// err_message = empty
				return ValueValidity::Invalid;
			}
			if (!CreatePath(filename))
			{
				*err_message = ssprintf(L"Ошибка создания папки \"%ls\".",
						EnsureType<const wchar_t*>(GetPathNativeFromGeneric(filename).c_str()));
				return ValueValidity::Invalid;
			}
			return ValueValidity::Valid;

		case ExtendedReason::ObjectIsFile:
			*err_message = ssprintf(L"Заданное имя ссылается на файл:\n\"%ls\".",
					GetPathNativeFromGeneric(filename).c_str());
			return ValueValidity::Invalid;

		default:
			*err_message =
					L"Внутренняя ошибка в DirectoryReadValidator::ValidateUI: неверный код ошибки.";
			return ValueValidity::Invalid;
	}
}

auto FileNameEditApi::DirectoryWriteValidator::DoValidate(
		const wstring &filename,
		ExtendedReason *ex_reason) -> ValueValidity
{
	if (filename.empty())
	{
		*ex_reason = ExtendedReason::EmptyFileName;
		return ValueValidity::Invalid;
	}
	if (FileExists(filename))
	{
		*ex_reason = ExtendedReason::ObjectIsFile;
		return ValueValidity::Invalid;
	}
	if (!DirectoryExists(filename))
	{
		*ex_reason = ExtendedReason::DirectoryNotFound;
		return ValueValidity::Invalid;
	}
	return ValueValidity::Valid;
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



DynamicDialog::DynamicDialog(const wstring &caption,
		const weak_ptr<DynamicDialogInterface> &dialog_api,
		ThreadQueueWriter<DynamicDialogResult> &&thread_channel):
	w_dialog_api(dialog_api),
	thread_channel(std::move(thread_channel))
{
	setWindowTitle(wstring_to_qstring(caption));
	QObject::connect(this, &QDialog::finished, this, &DynamicDialog::do_finished);
}

void DynamicDialog::CancelDialog()
{
	if (auto dialog_api = w_dialog_api.lock())
	{
		dialog_api->EndDialog(nullptr, DialogResultCode::Rejected);
	}
	else
	{
		reject();
	}
}

ThreadQueueWriter<DynamicDialogResult> &DynamicDialog::GetThreadChannel()
{
	return thread_channel;
}

void DynamicDialog::do_finished()
{
	try
	{
		thread_channel.Stop();
		deleteLater();
	}
	catch (...)
	{
		fprintf(stderr, "Exception in DynamicDialog::do_finished():\n%s\n",
				GetExceptionString().c_str());
	}
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



DialogApi::DialogApi(PrivateTag, const wstring &caption):
	caption(caption)
{
}

shared_ptr<DialogApi> DialogApi::Create(const wstring &caption)
{
	return make_shared<DialogApi>(PrivateTag(), caption);
}

void DialogApi::AddControl(shared_ptr<ControlApi> control)
{
	controls.push_back(control);
}

//--------------------------------------------------------------

namespace
{

class ShowDynamicDialogGuard
{
	public:
		ShowDynamicDialogGuard(DialogApi *dialog_api):
			dialog_api(dialog_api)
		{
			dialog_api->SetDialogApiInterface(true);
		}

		~ShowDynamicDialogGuard()
		{
			global_gui_controller->dialogs_creator->WorkerUICallback(
					[dialog_api = this->dialog_api, id = this]()
					{
						// поток UI
						dialog_api->DeleteDialog();
						global_gui_controller->dialogs_creator->EndDialogLoop(id);
					});

			dialog_api->SetDialogApiInterface(false);
		}
	private:
		DialogApi *dialog_api;
};

} // namespace

//--------------------------------------------------------------

void DialogApi::Show()
{
	api_ForceUpdateGUI(sec(0));
	ShowDynamicDialogGuard dialog_guard(this);
	ThreadQueue<DynamicDialogResult> thread_channel;
	{
		auto writer = thread_channel.GetWriter();
		global_gui_controller->dialogs_creator->WorkerUICallback(
				[dialog_api = this, thread_channel = &writer, id = &dialog_guard]()
				{
					// поток UI
					DynamicDialog *dialog = dialog_api->CreateDialog(std::move(*thread_channel));
					try
					{
						global_gui_controller->dialogs_creator->ExecuteDialog(dialog);
						global_gui_controller->dialogs_creator->BeginDialogLoop(id, dialog,
								[dialog]()
								{
									dialog->CancelDialog();
								});
					}
					catch(...)
					{
						delete dialog;
						throw;
					}
				});
		// Переменная unlocker уничтожается до вызова Wait().
	}
	for (;;)
	{
		DynamicDialogResult result;
		if (!thread_channel.Read(&result))
			break;
		if (global_gui_controller->quit_scheduled)
		{
			api_ForceUpdateGUI(sec(0));
		}
		if (!result.action)
			continue;
		SafeExecute(result.action);
		if (global_gui_controller->quit_scheduled)
		{
			api_ForceUpdateGUI(sec(0));
		}
	}
	api_ForceUpdateGUI(global_gui_controller->GUI_update_interval);
}

//--------------------------------------------------------------

void DialogApi::SetDialogApiInterface(bool apply)
{
	this->thread_api = apply;
	auto *dialog_api = apply? this: nullptr;
	for (auto &control: controls)
	{
		control->SetDialogApiInterface(dialog_api);
	}
}

DynamicDialog *DialogApi::CreateDialog(ThreadQueueWriter<DynamicDialogResult> &&thread_channel)
{
	XRAD_ASSERT_THROW(!dialog);
	DynamicDialogInterface *ddi = this;
	dialog = new DynamicDialog(Caption(),
			shared_ptr<DynamicDialogInterface>(shared_from_this(), ddi),
			std::move(thread_channel));
	try
	{
		AddToDialog();
		return dialog;
	}
	catch (...)
	{
		delete dialog;
		throw;
	}
}

void DialogApi::AddToDialog()
{
	XRAD_ASSERT_THROW(dialog);
	auto layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
	for (auto &control: controls)
	{
		control->AddToDialog(dialog, layout, this, CombineSettingsPath(L"DynamicDialog", caption));
	}
}

void DialogApi::ReleaseDialog()
{
	if (dialog)
	{
		dialog = nullptr;
		for (auto &control: controls)
		{
			control->ReleaseDialog();
		}
	}
}

void DialogApi::DeleteDialog()
{
	if (dialog)
	{
		dialog->deleteLater();
	}
	ReleaseDialog();
}

bool DialogApi::TryAccept(QString *err_message, const vector<ControlApi*> *value_dependencies)
{
	if (value_dependencies)
	{
		for (auto control: *value_dependencies)
		{
			if (!control->TryAccept(err_message))
				return false;
		}
	}
	else
	{
		for (auto &control: controls)
		{
			if (!control->TryAccept(err_message))
				return false;
		}
	}
	return true;
}

void DialogApi::Accept()
{
	for (auto &control: controls)
		control->Accept();
}

void DialogApi::SetResult(ButtonApi *control)
{
	if (!dialog)
		return;
	result_button = control;
}

//--------------------------------------------------------------

void DialogApi::ProcessUICallback(function<void ()> callback)
{
	if (!thread_api)
		return;
	global_gui_controller->dialogs_creator->WorkerUICallback(std::move(callback));
}

//--------------------------------------------------------------

bool DialogApi::TryAccept(const vector<ControlApi*> &value_dependencies)
{
	QString err_message;
	if (!TryAccept(&err_message, &value_dependencies))
	{
		if (!err_message.isEmpty())
			QMessageBox::warning(dialog, wstring_to_qstring(Caption()), err_message);
		return false;
	}
	return true;
}

QWidget *DialogApi::GetDialog()
{
	return dialog;
}

bool DialogApi::AskYesNo(const wstring &question)
{
	return QMessageBox::Yes == QMessageBox::question(dialog, wstring_to_qstring(Caption()),
			wstring_to_qstring(question));
}

void DialogApi::EndDialog(ButtonApi *control, DialogResultCode code,
		const vector<ControlApi*> *value_dependencies)
{
	if (code != DialogResultCode::Rejected)
	{
		QString err_message;
		if (!TryAccept(&err_message, value_dependencies))
		{
			if (!err_message.isEmpty())
				QMessageBox::warning(dialog, wstring_to_qstring(Caption()), err_message);
			return;
		}
		Accept();
	}
	SetResult(control);
	if (dialog)
	{
		switch (code)
		{
			case DialogResultCode::Accepted:
				dialog->accept();
				break;
			case DialogResultCode::Rejected:
				dialog->reject();
				break;
		}
	}
}

void DialogApi::ProcessCallback(const function<void ()> &callback,
		ActionPolicy action_policy)
{
	if (!callback || !dialog)
		return;
	// TODO: Сделать вместо простой очереди более сложный объект. Что-то типа map, где ключом
	// будет пара из указателя на источник и тип операции. Предполагается, что
	// Тогда размер "очереди" никогда не станет больше
	// суммы количества типов операций для всех элементов диалога.
	// Или, может быть, сделать защиту от повторного помещения callback в очередь на уровне
	// классов, использующих эту функцию.
	// TODO: Очередь (но не сигнал закрытия Stop()) сделать общую для всех диалогов,
	// чтобы callback'и работали при вызове одного диалога из обработчика событий (callback) другого.
	shared_ptr<void> lock;
	switch (action_policy)
	{
		default:
		case ActionPolicy::Block:
			lock = LockDialog();
			break;
		case ActionPolicy::Hide:
			lock = HideDialog();
			break;
		case ActionPolicy::None:
			break;
	}
	dialog->GetThreadChannel().Push(DynamicDialogResult(callback, std::move(lock)));
}

//--------------------------------------------------------------

class DialogApi::DialogLocker
{
	public:
		DialogLocker(shared_ptr<DialogApi> dialog_api):
			w_dialog_api(dialog_api)
		{
			dialog_api->LockDialogSync();
		}
		~DialogLocker()
		{
			if (auto dialog_api = w_dialog_api.lock())
			{
				dialog_api->UnlockDialogAsync();
			}
		}
	private:
		weak_ptr<DialogApi> w_dialog_api;
};

//--------------------------------------------------------------

shared_ptr<void> DialogApi::LockDialog()
{
	if (!dialog)
		return nullptr;
	return make_shared<DialogLocker>(shared_from_this());
}

void DialogApi::LockDialogSync()
{
	if (!dialog_lock_count)
	{
		dialog->setCursor(Qt::WaitCursor);
		dialog->setEnabled(false);
	}
	++dialog_lock_count;
}

void DialogApi::UnlockDialogAsync()
{
	ProcessUICallback([dialog_api = shared_from_this()]()
			{
				// UI поток
				if (!dialog_api->dialog)
					return;
				if (dialog_api->dialog_lock_count)
				{
					--dialog_api->dialog_lock_count;
					if (!dialog_api->dialog_lock_count)
					{
						dialog_api->dialog->setEnabled(true);
						dialog_api->dialog->unsetCursor();
					}
				}
			});
}

//--------------------------------------------------------------

class DialogApi::DialogHider
{
	public:
		DialogHider(shared_ptr<DialogApi> dialog_api):
			w_dialog_api(dialog_api)
		{
			dialog_api->HideDialogSync();
		}
		~DialogHider()
		{
			if (auto dialog_api = w_dialog_api.lock())
			{
				dialog_api->UnhideDialogAsync();
			}
		}
	private:
		weak_ptr<DialogApi> w_dialog_api;
};

//--------------------------------------------------------------

shared_ptr<void> DialogApi::HideDialog()
{
	if (!dialog)
		return nullptr;
	return make_shared<DialogHider>(shared_from_this());
}

void DialogApi::HideDialogSync()
{
	if (!dialog_hide_count)
	{
		dialog->hide();
	}
	++dialog_hide_count;
}

void DialogApi::UnhideDialogAsync()
{
	ProcessUICallback([dialog_api = shared_from_this()]()
			{
				// UI поток
				if (!dialog_api->dialog)
					return;
				if (dialog_api->dialog_hide_count)
				{
					--dialog_api->dialog_hide_count;
					if (!dialog_api->dialog_hide_count)
					{
						// TODO: Восстановить координаты окна.
						dialog_api->dialog->show();
					}
				}
			});
}



//--------------------------------------------------------------

#ifdef _DEBUG
void ParseValue_ull_test()
{
	struct TestData
	{
		const char *str;
		size_t length;
		bool result;
		unsigned long long value;
	};
	static_assert(numeric_limits<unsigned long long>::max() == 18446744073709551615ull, "Modify test bounds.");
	const TestData test_data[] =
	{
		{"0", 0, true, 0},
		{"1", 0, true, 1},
		{"-0", 0, false, 0},
		{"-1", 0, false, 0},
		{"-0x1", 0, false, 0},
		{"  1", 0, true, 1},
		{"1  ", 0, true, 1},
		{"  1  ", 0, true, 1},
		{"a1", 0, false, 0},
		{"1a", 0, false, 0},
		{"1 a", 0, false, 0},
		{"1 1", 0, false, 0},
		{"18446744073709551615", 0, true, 18446744073709551615ull},
		{"18446744073709551616", 0, false, 0},
		{"100000000000000000000", 0, false, 0},
		{"1000000000000000000000000", 0, false, 0},
		{"0x1", 0, true, 1},
		{"0xFFFFFFFFFFFFFFFF", 0, true, 0xFFFFFFFFFFFFFFFFull},
		{"0x10000000000000000", 0, false, 0},
		{"0x100000000000000000", 0, false, 0},
		{"1\0 ", 3, false, 0},
		{"1\0""1", 3, false, 0},
		{nullptr, 0, false, 0}
	};
	for (auto &test: test_data)
	{
		if (!test.str)
			break;
		const unsigned long long bad_value = 274;
		unsigned long long value = bad_value;
		bool result = ParseValue_ull(&value, test.str, test.length? test.length: strlen(test.str));
		if (result != test.result ||
				value != (test.result? test.value: bad_value))
		{
			throw runtime_error(ssprintf(
					"ParseValue_ull_test failed for str=\"%s\".",
					EnsureType<const char*>(test.str)));
		}
	}
}

void ParseValue_ll_test()
{
	struct TestData
	{
		const char *str;
		size_t length;
		bool result;
		long long value;
	};
	static_assert(numeric_limits<long long>::max() == 9223372036854775807ll, "Modify test bounds.");
	static_assert(numeric_limits<long long>::min() == -9223372036854775808ll, "Modify test bounds.");
	const TestData test_data[] =
	{
		{"0", 0, true, 0},
		{"1", 0, true, 1},
		{"-1", 0, true, -1},
		{"  1", 0, true, 1},
		{"1  ", 0, true, 1},
		{"  1  ", 0, true, 1},
		{"a1", 0, false, 0},
		{"1a", 0, false, 0},
		{"1 a", 0, false, 0},
		{"1 1", 0, false, 0},
		{"9223372036854775807", 0, true, 9223372036854775807ll},
		{"-9223372036854775808", 0, true, -9223372036854775808ll},
		{"9223372036854775808", 0, false, 0},
		{"-9223372036854775809", 0, false, 0},
		{"10000000000000000000", 0, false, 0},
		{"-10000000000000000000", 0, false, 0},
		{"100000000000000000000", 0, false, 0},
		{"-100000000000000000000", 0, false, 0},
		{"1000000000000000000000000", 0, false, 0},
		{"-1000000000000000000000000", 0, false, 0},
		{"0x1", 0, true, 1},
		{"0x7FFFFFFFFFFFFFFF", 0, true, 0x7FFFFFFFFFFFFFFFll},
		{"0x8000000000000000", 0, false, 0},
		{"0xFFFFFFFFFFFFFFFF", 0, false, 0},
		{"0x10000000000000000", 0, false, 0},
		{"0x100000000000000000", 0, false, 0},
		{"-0x8000000000000000", 0, true, -0x8000000000000000ll},
		{"-0x8000000000000001", 0, false, 0},
		{"--1", 0, false, 0},
		{"0x-1", 0, false, 0},
		{"1\0 ", 3, false, 0},
		{"1\0""1", 3, false, 0},
		{nullptr, 0, false, 0}
	};
	for (auto &test: test_data)
	{
		if (!test.str)
			break;
		const long long bad_value = -274;
		long long value = bad_value;
		bool result = ParseValue_ll(&value, test.str, test.length? test.length: strlen(test.str));
		if (result != test.result ||
				value != (test.result? test.value: bad_value))
		{
			throw runtime_error(ssprintf(
					"ParseValue_ll_test failed for str=\"%s\".",
					EnsureType<const char*>(test.str)));
		}
	}
}

class ParseValue_tester
{
	public:
		ParseValue_tester()
		{
			ParseValue_ull_test();
			ParseValue_ll_test();
		}
};

ParseValue_tester ParseValue_tester_;
#endif

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------

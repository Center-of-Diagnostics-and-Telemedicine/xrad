// file DynamicDialog.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "DynamicDialog.h"

#include <DynamicDialogQt.h>
#include <XRADSystem/System.h>
#include <map>

//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

namespace DynamicDialog
{

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

XRAD_GUI::DynamicDialogLayout LayoutToUI(Layout control_layout)
{
	switch (control_layout)
	{
		case Layout::Horizontal:
			return XRAD_GUI::DynamicDialogLayout::Horizontal;
		case Layout::Vertical:
			return XRAD_GUI::DynamicDialogLayout::Vertical;
	}
	return XRAD_GUI::DynamicDialogLayout::Vertical;
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

wstring SelectHistoryCaption(const wstring &caption,
		const wchar_t *class_name, const wstring &generic_id)
{
	if (caption.length())
		return caption;
	return ssprintf(L"-%ls-%ls",
			EnsureType<const wchar_t*>(class_name),
			EnsureType<const wchar_t*>(generic_id.c_str()));
}

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

struct GuiControl
{
	shared_ptr<XRAD_GUI::ControlApi> control;

	template <class T>
	GuiControl(shared_ptr<T> &&control): control(control) {}
};

//--------------------------------------------------------------

struct GuiControlBindings
{
	vector<pair<Button*, XRAD_GUI::ButtonApi*>> buttons;
	map<Control*, XRAD_GUI::ControlApi*> controls;

	void clear()
	{
		buttons.clear();
		controls.clear();
	}
};



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



void Control::Perform(const function<bool (const Control*)> &action) const
{
	if (!action(this))
		return;
	PerformInternal(action);
}

void Control::Perform(const function<bool (Control*)> &action)
{
	if (!action(this))
		return;
	PerformInternal(action);
}

GuiControl Control::CreateGuiControl(GuiControlBindings *bindings)
{
	auto result = CreateGuiControlImpl(bindings);
	bindings->controls[this] = result.control.get();
	return result;
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



void ControlProxy::PerformInternal(const function<bool (const Control*)> &action) const
{
	ProxyControl()->Perform(action);
}

void ControlProxy::PerformInternal(const function<bool (Control*)> &action)
{
	ProxyControl()->Perform(action);
}

void ControlProxy::LoadValues()
{
	ProxyControl()->LoadValues();
}

void ControlProxy::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	ProxyControl()->LoadHistory(settings_path, generic_id);
}

void ControlProxy::SaveHistory()
{
	ProxyControl()->SaveHistory();
}

GuiControl ControlProxy::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	return ProxyControl()->CreateGuiControl(bindings);
}

void ControlProxy::BindGuiControls(const GuiControlBindings &bindings)
{
	ProxyControl()->BindGuiControls(bindings);
}

void ControlProxy::UpdateFromGuiControl()
{
	return ProxyControl()->UpdateFromGuiControl();
}

void ControlProxy::ReleaseGuiControl()
{
	return ProxyControl()->ReleaseGuiControl();
}

void ControlProxy::ApplyDialogResult(DialogResultCode code)
{
	return ProxyControl()->ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



void ControlContainer::PerformInternal(const function<bool (const Control*)> &action) const
{
	for (auto &control: controls)
	{
		const auto *const_control = control.get();
		const_control->Perform(action);
	}
}

void ControlContainer::PerformInternal(const function<bool (Control*)> &action)
{
	for (auto &control: controls)
	{
		control->Perform(action);
	}
}

void ControlContainer::LoadValues()
{
	for (auto &control: controls)
	{
		control->LoadValues();
	}
}

void ControlContainer::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	wstring sub_path;
	if (use_frame)
		sub_path = CombineSettingsPath(settings_path, caption);
	else
		sub_path = settings_path;
	size_t i = 0;
	for (auto &control: controls)
	{
		control->LoadHistory(sub_path, ssprintf(L"#%zu", i));
		++i;
	}
}

void ControlContainer::SaveHistory()
{
	for (auto &control: controls)
	{
		control->SaveHistory();
	}
}

GuiControl ControlContainer::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	auto gui_container = XRAD_GUI::ContainerApi::Create(use_frame, caption, LayoutToUI(control_layout));
	for (auto &control: controls)
	{
		gui_container->AddControl(control->CreateGuiControl(bindings).control);
	}
	return gui_container;
}

void ControlContainer::BindGuiControls(const GuiControlBindings &bindings)
{
	for (auto &control: controls)
	{
		control->BindGuiControls(bindings);
	}
}

void ControlContainer::UpdateFromGuiControl()
{
	for (auto &control: controls)
	{
		control->UpdateFromGuiControl();
	}
}

void ControlContainer::ReleaseGuiControl()
{
	for (auto &control: controls)
	{
		control->ReleaseGuiControl();
	}
}

void ControlContainer::ApplyDialogResult(DialogResultCode code)
{
	for (auto &control: controls)
	{
		control->ApplyDialogResult(code);
	}
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



void StaticControlBase::PerformInternal(const function<bool (const Control*)> &action) const
{
}

void StaticControlBase::PerformInternal(const function<bool (Control*)> &action)
{
}

void StaticControlBase::LoadValues()
{
}

void StaticControlBase::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
}

void StaticControlBase::SaveHistory()
{
}

void StaticControlBase::BindGuiControls(const GuiControlBindings &bindings)
{
}

void StaticControlBase::UpdateFromGuiControl()
{
}

void StaticControlBase::ApplyDialogResult(DialogResultCode code)
{
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



Separator::Separator()
{
}

GuiControl Separator::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (gui_in_use )
		throw runtime_error("Separator::CreateGuiControl: control already created.");
	return XRAD_GUI::SeparatorApi::Create();
}

void Separator::ReleaseGuiControl()
{
	gui_in_use = false;
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



Stretch::Stretch()
{
}

GuiControl Stretch::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (gui_in_use )
		throw runtime_error("Stretch::CreateGuiControl: control already created.");
	return XRAD_GUI::StretchApi::Create();
}

void Stretch::ReleaseGuiControl()
{
	gui_in_use = false;
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct TextLabel::PrivateStorage
{
	shared_ptr<XRAD_GUI::TextLabelApi> gui_text_label;
};

//--------------------------------------------------------------

TextLabel::TextLabel(const wstring &text):
	text(text),
	private_storage(make_shared<PrivateStorage>())
{
}

void TextLabel::SetText(const wstring &t)
{
	text = t;
	if (private_storage->gui_text_label)
	{
		private_storage->gui_text_label->SetText(t);
	}
}

GuiControl TextLabel::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_text_label)
		throw runtime_error("TextLabel::CreateGuiControl: control already created.");
	auto gui_text_label = XRAD_GUI::TextLabelApi::Create(text);
	private_storage->gui_text_label = gui_text_label;
	return gui_text_label;
}

void TextLabel::ReleaseGuiControl()
{
	private_storage->gui_text_label.reset();
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct Button::PrivateStorage
{
	shared_ptr<XRAD_GUI::ButtonApi> gui_button;
};

//--------------------------------------------------------------

Button::Button(const wstring &caption, DialogResultCode code):
	caption(caption), result_code(code),
	private_storage(make_shared<PrivateStorage>())
{
}

Button::Button(const wstring &caption, const function<void ()> &action,
		ActionPolicy action_policy):
	caption(caption), action(action),
	action_policy(action_policy),
	private_storage(make_shared<PrivateStorage>())
{
}

void Button::SetResultCode(DialogResultCode code)
{
	if (private_storage->gui_button)
		throw runtime_error("Button::SetResultCode: not allowed in UI loop.");
	result_code = code;
}

void Button::SetDefault(bool is_default_button)
{
	if (private_storage->gui_button)
		throw runtime_error("Button::SetDefault: not allowed in UI loop.");
	default_button = is_default_button;
}

void Button::SetValueDependencies(vector<weak_ptr<Control>> dependencies)
{
	if (private_storage->gui_button)
		throw runtime_error("Button::SetValueDependencies: not allowed in UI loop.");
	value_dependencies = std::move(dependencies);
	use_value_dependencies = true;
}

void Button::PerformInternal(const function<bool (const Control*)> &action) const
{
}

void Button::PerformInternal(const function<bool (Control*)> &action)
{
}

void Button::LoadValues()
{
}

void Button::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
}

void Button::SaveHistory()
{
}

GuiControl Button::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_button)
		throw runtime_error("Button::CreateGuiControl: control already created.");
	auto gui_code = XRAD_GUI::DialogResultCode::Accepted;
	switch (result_code)
	{
		case DialogResultCode::Accepted:
			gui_code = XRAD_GUI::DialogResultCode::Accepted;
			break;
		case DialogResultCode::Rejected:
			gui_code = XRAD_GUI::DialogResultCode::Rejected;
			break;
	}
	auto gui_action_policy = XRAD_GUI::DynamicDialogInterface::ActionPolicy::Block;
	switch (action_policy)
	{
		case ActionPolicy::Block:
			gui_action_policy = XRAD_GUI::DynamicDialogInterface::ActionPolicy::Block;
			break;
		case ActionPolicy::Hide:
			gui_action_policy = XRAD_GUI::DynamicDialogInterface::ActionPolicy::Hide;
			break;
		case ActionPolicy::None:
			gui_action_policy = XRAD_GUI::DynamicDialogInterface::ActionPolicy::None;
			break;
	}
	auto gui_button = XRAD_GUI::ButtonApi::Create(caption, gui_code, default_button, action,
			gui_action_policy);
	bindings->buttons.push_back(make_pair(this, &*gui_button));
	private_storage->gui_button = gui_button;
	return gui_button;
}

void Button::BindGuiControls(const GuiControlBindings &bindings)
{
	if (!private_storage->gui_button)
		throw runtime_error("Button::BindGuiControls: control is not created.");
	if (!use_value_dependencies)
		return;
	vector<XRAD_GUI::ControlApi*> gui_dependencies;
	gui_dependencies.reserve(value_dependencies.size());
	for (auto &w_c: value_dependencies)
	{
		if (auto c = w_c.lock())
		{
			auto it = bindings.controls.find(c.get());
			if (it == bindings.controls.end())
			{
				throw runtime_error(
						"Button::BindGuiControls: dialog bindings do not contain data for dependency control.");
			}
			gui_dependencies.push_back(it->second);
		}
	}
	private_storage->gui_button->SetValueDependencies(std::move(gui_dependencies));
}

void Button::UpdateFromGuiControl()
{
}

void Button::ReleaseGuiControl()
{
	private_storage->gui_button.reset();
}

void Button::ApplyDialogResult(DialogResultCode code)
{
}


//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct CheckBox::PrivateStorage
{
	shared_ptr<XRAD_GUI::CheckBoxApi> gui_checkbox;
};

//--------------------------------------------------------------

CheckBox::CheckBox(const wstring &caption, const GUIValue<bool> &default_value,
		const on_value_changed_t &on_checked_changed):
	caption(caption), default_value(default_value),
	on_value_changed(on_checked_changed),
	value(default_value.value),
	value_valid(default_value.value_valid && default_value.is_stored != saved_default_value),
	private_storage(make_shared<PrivateStorage>())
{
	if (default_value.is_stored != saved_default_value && !default_value.value_valid)
		throw invalid_argument("CheckBox: Default value is neither specified nor stored.");
}

void CheckBox::SetSubControl(shared_ptr<Control> control)
{
	if (private_storage->gui_checkbox)
		throw runtime_error("CheckBox::SetSubControl: not allowed in UI loop.");
	sub_control = control;
}

void CheckBox::SetOnCheckedChanged(const on_value_changed_t &on_checked_changed)
{
	this->on_value_changed = on_checked_changed;
}

void CheckBox::SetChecked(bool c)
{
	ApplyValue(c);
	if (private_storage->gui_checkbox)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_checkbox передаем не c, а value.
		private_storage->gui_checkbox->SetValue(value);
	}
}

void CheckBox::PerformInternal(const function<bool (const Control*)> &action) const
{
	if (sub_control)
	{
		const auto *const_sub_control = sub_control.get();
		const_sub_control->Perform(action);
	}
}

void CheckBox::PerformInternal(const function<bool (Control*)> &action)
{
	if (sub_control)
		sub_control->Perform(action);
}

void CheckBox::LoadValues()
{
	if (sub_control)
		sub_control->LoadValues();
}

void CheckBox::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = SelectHistoryCaption(caption, L"CheckBox", generic_id);
	bool history_value_loaded = false;
	history_value = GetSavedParameter(history_settings_path, history_caption, history_value,
			&history_value_loaded);
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetChecked(history_value);
	}

	if (sub_control)
		sub_control->LoadHistory(CombineSettingsPath(settings_path, history_caption), L"#sub");
}

void CheckBox::SaveHistory()
{
	SaveParameter(history_settings_path, history_caption, value);

	if (sub_control)
		sub_control->SaveHistory();
}

GuiControl CheckBox::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_checkbox)
		throw runtime_error("CheckBox::CreateGuiControl: control already created.");
	auto gui_sub_control = sub_control? sub_control->CreateGuiControl(bindings).control: nullptr;
	auto gui_checkbox = XRAD_GUI::CheckBoxApi::Create(caption, value,
			default_value,
			history_value,
			gui_sub_control,
			[this]()
			{
				if (!private_storage->gui_checkbox)
					throw runtime_error("CheckBox::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(private_storage->gui_checkbox->UIValue());
			});
	private_storage->gui_checkbox = gui_checkbox;
	return gui_checkbox;
}

void CheckBox::BindGuiControls(const GuiControlBindings &bindings)
{
}

void CheckBox::UpdateFromGuiControl()
{
	if (!private_storage->gui_checkbox)
		throw runtime_error("CheckBox::UpdateFromGuiControl: control is not created.");
	ApplyValue(private_storage->gui_checkbox->UIValue());
	if (sub_control)
		sub_control->UpdateFromGuiControl();
}

void CheckBox::ReleaseGuiControl()
{
	private_storage->gui_checkbox.reset();
	if (sub_control)
		sub_control->ReleaseGuiControl();
}

void CheckBox::ApplyDialogResult(DialogResultCode code)
{
	if (sub_control)
		sub_control->ApplyDialogResult(code);
}

void CheckBox::ApplyValue(bool v)
{
	if (value_valid && v == value)
		return;
	value = v;
	value_valid = true;
	if (on_value_changed)
		on_value_changed(v);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



ValueCheckBox::ValueCheckBox(const wstring &caption, const GUIValue<bool*> &p_checked,
		const on_value_changed_t &on_checked_changed):
	CheckBox(caption, ConvertGUIValue(p_checked, *CheckNotNull(p_checked.value)), on_checked_changed),
	p_value(p_checked.value)
{
}

void ValueCheckBox::Update()
{
	SetChecked(*p_value);
}

void ValueCheckBox::LoadValues()
{
	parent::LoadValues();
	SetChecked(*p_value);
}

void ValueCheckBox::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Checked();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct GuiControlRadioButton
{
	shared_ptr<XRAD_GUI::RadioButtonApi> control;

	GuiControlRadioButton(shared_ptr<XRAD_GUI::RadioButtonApi> &&control): control(control) {}
};

//--------------------------------------------------------------

struct RadioButton::PrivateStorage
{
	shared_ptr<XRAD_GUI::RadioButtonApi> gui_radiobutton;
};

//--------------------------------------------------------------

RadioButton::RadioButton(const wstring &caption,
		const function<void (bool checked)> &on_checked_changed):
	caption(caption), checked(false),
	on_checked_changed(on_checked_changed),
	private_storage(make_shared<PrivateStorage>())
{
}

RadioButton::RadioButton(const wstring &caption, bool default_checked,
		const function<void (bool checked)> &on_checked_changed):
	caption(caption), checked(default_checked),
	on_checked_changed(on_checked_changed),
	private_storage(make_shared<PrivateStorage>())
{
}

void RadioButton::SetSubControl(shared_ptr<Control> control)
{
	if (private_storage->gui_radiobutton)
		throw runtime_error("RadioButton::SetSubControl: not allowed in UI loop.");
	sub_control = control;
}

void RadioButton::SetOnCheckedChanged(const function<void (bool checked)> &on_checked_changed)
{
	this->on_checked_changed = on_checked_changed;
}

void RadioButton::CheckedChanged(bool c)
{
	if (c == checked)
		return;
	checked = c;
	if (on_checked_changed)
		on_checked_changed(c);
}

void RadioButton::Perform(const function<bool (const Control*)> &action) const
{
	// RadioButton не наследуется от Control. Можно сделать отдельный класс для Perform,
	// от которого унаследовать Control и RadioButton.
	//if (!action(this))
	//	return;
	PerformInternal(action);
}

void RadioButton::Perform(const function<bool (Control*)> &action)
{
	// См. Perform() const.
	//if (!action(this))
	//	return;
	PerformInternal(action);
}

void RadioButton::PerformInternal(const function<bool (const Control*)> &action) const
{
	if (sub_control)
	{
		const auto *const_sub_control = sub_control.get();
		const_sub_control->Perform(action);
	}
}

void RadioButton::PerformInternal(const function<bool (Control*)> &action)
{
	if (sub_control)
		sub_control->Perform(action);
}

void RadioButton::LoadValues()
{
	if (sub_control)
		sub_control->LoadValues();
}

void RadioButton::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	if (sub_control)
		sub_control->LoadHistory(CombineSettingsPath(settings_path,
				SelectHistoryCaption(caption, L"RadioButton", generic_id)), L"#sub");
}

void RadioButton::SaveHistory()
{
	if (sub_control)
		sub_control->SaveHistory();
}

GuiControlRadioButton RadioButton::CreateGuiControlRadioButton(GuiControlBindings *bindings)
{
	if (private_storage->gui_radiobutton)
		throw runtime_error("RadioButton::CreateGuiControl: control already created.");
	auto gui_sub_control = sub_control? sub_control->CreateGuiControl(bindings).control: nullptr;
	auto gui_radiobutton = XRAD_GUI::RadioButtonApi::Create(caption, checked, gui_sub_control);
	private_storage->gui_radiobutton = gui_radiobutton;
	return gui_radiobutton;
}

void RadioButton::BindGuiControls(const GuiControlBindings &bindings)
{
	if (sub_control)
		sub_control->BindGuiControls(bindings);
}

void RadioButton::UpdateFromGuiControl()
{
	if (!private_storage->gui_radiobutton)
		throw runtime_error("RadioButton::UpdateFromGuiControl: control is not created.");
	if (sub_control)
		sub_control->UpdateFromGuiControl();
}

void RadioButton::ReleaseGuiControl()
{
	private_storage->gui_radiobutton.reset();
	if (sub_control)
		sub_control->ReleaseGuiControl();
}

void RadioButton::ApplyDialogResult(DialogResultCode code)
{
	if (sub_control)
		sub_control->ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct RadioButtonContainer::PrivateStorage
{
	shared_ptr<XRAD_GUI::RadioButtonContainerApi> gui_radiobutton_container;
};

//--------------------------------------------------------------

RadioButtonContainer::RadioButtonContainer(
		default_value_t default_value_type,
		Layout control_layout,
		const on_value_changed_t &on_choice_changed,
		const on_value_apply_t &on_choice_apply):
	RadioButtonContainer(false, wstring(), {}, default_value_type, control_layout,
			on_choice_changed, on_choice_apply)
{
}

RadioButtonContainer::RadioButtonContainer(vector<shared_ptr<RadioButton>> controls,
		const GUIValue<const RadioButton*> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_choice_changed,
		const on_value_apply_t &on_choice_apply):
	RadioButtonContainer(false, wstring(), std::move(controls), default_value, control_layout,
			on_choice_changed, on_choice_apply)
{
}

RadioButtonContainer::RadioButtonContainer(const wstring &group_caption,
		default_value_t default_value_type,
		Layout control_layout,
		const on_value_changed_t &on_choice_changed,
		const on_value_apply_t &on_choice_apply):
	RadioButtonContainer(true, group_caption, {}, default_value_type, control_layout,
			on_choice_changed, on_choice_apply)
{
}

RadioButtonContainer::RadioButtonContainer(const wstring &group_caption,
		vector<shared_ptr<RadioButton>> controls,
		const GUIValue<const RadioButton*> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_choice_changed,
		const on_value_apply_t &on_choice_apply):
	RadioButtonContainer(true, group_caption, std::move(controls), default_value, control_layout,
			on_choice_changed, on_choice_apply)
{
}

//--------------------------------------------------------------

namespace
{
GUIValue<const RadioButton*> ComputeDefaultButton(const GUIValue<const RadioButton*> &gui_value,
		const vector<shared_ptr<RadioButton>> &controls)
{
	auto result = gui_value;
	for (auto &c: controls)
	{
		if (c->Checked())
		{
			result.value = c.get();
			result.value_valid = true;
		}
	}
	return result;
}
} // namespace

//--------------------------------------------------------------

RadioButtonContainer::RadioButtonContainer(bool use_frame, const wstring &group_caption,
		vector<shared_ptr<RadioButton>> controls,
		const GUIValue<const RadioButton*> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_choice_changed,
		const on_value_apply_t &on_choice_apply):
	use_frame(use_frame),
	caption(group_caption),
	controls(std::move(controls)),
	default_value(ComputeDefaultButton(default_value, this->controls)),
	control_layout(control_layout),
	on_value_changed(on_choice_changed),
	on_value_apply(on_choice_apply),
	value(ButtonToIndex(this->default_value.value)),
	value_valid(this->default_value.value_valid &&
			this->default_value.is_stored != saved_default_value),
	private_storage(make_shared<PrivateStorage>())
{
	// default_value.is_stored != saved_default_value && !default_value.value_valid:
	// Для RadioButtonContainer вполне возможна ситуация, когда в конструктор передается
	// признак fixed_default_value, но само значение задается в поле Checked() радиокнопки,
	// добавляемой позже функцией AddControl().
	// Поэтому проверок здесь не делаем.
}

void RadioButtonContainer::AddControl(shared_ptr<RadioButton> control)
{
	if (private_storage->gui_radiobutton_container)
	{
		throw runtime_error("RadioButtonContainer::AddControl() is not allowed in UI loop.");
	}
	bool checked = control->Checked();
	controls.push_back(std::move(control)); // далее переменная control не валидна
	if (checked)
	{
		value = controls.size() - 1;
		value_valid = true;
		default_value.value = IndexToButton(value);
		default_value.value_valid = true;
	}
}

void RadioButtonContainer::SetChoice(const RadioButton *v)
{
	ApplyValue(ButtonToIndex(v));
	if (private_storage->gui_radiobutton_container)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_checkbox передаем не item_index, а value.
		private_storage->gui_radiobutton_container->SetValue(value);
	}
}

void RadioButtonContainer::PerformInternal(const function<bool (const Control*)> &action) const
{
	for (auto &control: controls)
	{
		const auto *const_control = control.get();
		const_control->Perform(action);
	}
}

void RadioButtonContainer::PerformInternal(const function<bool (Control*)> &action)
{
	for (auto &control: controls)
	{
		control->Perform(action);
	}
}

void RadioButtonContainer::LoadValues()
{
	for (auto &control: controls)
	{
		control->LoadValues();
	}
}

void RadioButtonContainer::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = ComputeStoredCaption();
	bool history_value_loaded = false;
	if (!history_caption.empty())
	{
		history_value = DecodeEnumValue(GetSavedParameter(settings_path, history_caption, wstring(),
				&history_value_loaded),
				history_value);
	}
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetChoice(IndexToButton(history_value));
	}

	wstring sub_path;
	if (use_frame)
	{
		sub_path = CombineSettingsPath(settings_path,
				SelectHistoryCaption(history_caption, L"RadioButtonContainer", generic_id));
	}
	else
		sub_path = settings_path;
	size_t i = 0;
	for (auto &control: controls)
	{
		control->LoadHistory(sub_path, ssprintf(L"#%zu", i));
		++i;
	}
}

void RadioButtonContainer::SaveHistory()
{
	if (!history_caption.empty())
		SaveParameter(history_settings_path, history_caption, EncodeEnumValue(value));

	for (auto &control: controls)
	{
		control->SaveHistory();
	}
}

GuiControl RadioButtonContainer::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_radiobutton_container)
		throw runtime_error("RadioButtonContainer::CreateGuiControl: control already created.");
	auto gui_container = XRAD_GUI::RadioButtonContainerApi::Create(use_frame, caption,
			value,
			ConvertGUIValue(default_value, ButtonToIndex(default_value.value)),
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_radiobutton_container)
					throw runtime_error("RadioButtonContainer::DoUIChoiceChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(private_storage->gui_radiobutton_container->UIValue());
			});
	for (auto &control: controls)
	{
		gui_container->AddControl(control->CreateGuiControlRadioButton(bindings).control);
	}
	private_storage->gui_radiobutton_container = gui_container;
	return gui_container;
}

void RadioButtonContainer::BindGuiControls(const GuiControlBindings &bindings)
{
	if (!private_storage->gui_radiobutton_container)
		throw runtime_error("RadioButtonContainer::BindGuiControls: control is not created.");
	for (auto &control: controls)
	{
		control->BindGuiControls(bindings);
	}
}

void RadioButtonContainer::UpdateFromGuiControl()
{
	if (!private_storage->gui_radiobutton_container)
		throw runtime_error("RadioButtonContainer::UpdateFromGuiControl: control is not created.");
	ApplyValue(private_storage->gui_radiobutton_container->UIValue());
	for (auto &control: controls)
	{
		control->UpdateFromGuiControl();
	}
}

void RadioButtonContainer::ReleaseGuiControl()
{
	private_storage->gui_radiobutton_container.reset();
	for (auto &control: controls)
	{
		control->ReleaseGuiControl();
	}
}

void RadioButtonContainer::ApplyDialogResult(DialogResultCode code)
{
	for (auto &control: controls)
	{
		control->ApplyDialogResult(code);
	}
	if (on_value_apply)
		on_value_apply(IndexToButton(value), code);
}

void RadioButtonContainer::ApplyValue(size_t v)
{
	if (v >= controls.size())
		v = (size_t)-1;
	if (value_valid && v == value)
		return;
	if (value != (size_t)-1)
	{
		controls[value]->CheckedChanged(false);
	}
	value = v;
	if (value != (size_t)-1)
	{
		controls[value]->CheckedChanged(true);
	}
	value_valid = true;
	if (on_value_changed)
		on_value_changed(IndexToButton(v));
}

size_t RadioButtonContainer::ButtonToIndex(const RadioButton *b) const
{
	for (size_t i = 0; i < controls.size(); ++i)
	{
		if (controls[i].get() == b)
			return i;
	}
	return (size_t)-1;
}

const RadioButton *RadioButtonContainer::IndexToButton(size_t i) const
{
	if (i == (size_t)-1 || i >= controls.size())
		return nullptr;
	return controls[i].get();
}

wstring RadioButtonContainer::EncodeEnumValue(size_t value) const
{
	if (value == size_t(-1) || value > controls.size())
		return wstring();
	return controls[value]->Caption();
}

size_t RadioButtonContainer::DecodeEnumValue(const wstring &str, size_t default_value) const
{
	for (size_t i = 0; i < controls.size(); ++i)
	{
		if (controls[i]->Caption() == str)
			return i;
	}
	return default_value;
}

wstring RadioButtonContainer::ComputeStoredCaption() const
{
	if (!caption.empty())
		return caption;
	for (auto &c: controls)
	{
		auto caption = c->Caption();
		if (!caption.empty())
			return L"_radiogroup_" + caption;
	}
	return {};
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct ComboBox::PrivateStorage
{
	shared_ptr<XRAD_GUI::ComboBoxApi> gui_combobox;
};

//--------------------------------------------------------------

ComboBox::ComboBox(const vector<wstring> &items, const GUIValue<size_t> &default_value,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	ComboBox(false, {}, items, default_value, Layout::Horizontal, on_value_changed, on_value_apply)
{
}

ComboBox::ComboBox(const wstring &caption, const vector<wstring> &items,
		const GUIValue<size_t> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	ComboBox(true, caption, items, default_value, control_layout, on_value_changed, on_value_apply)
{
}

ComboBox::ComboBox(bool use_caption, const wstring &caption,
		const vector<wstring> &items,
		const GUIValue<size_t> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	use_caption(use_caption),
	caption(caption),
	items(items),
	default_value(default_value),
	control_layout(control_layout),
	on_value_changed(std::move(on_value_changed)),
	on_value_apply(std::move(on_value_apply)),
	value(default_value.value),
	value_valid(default_value.value_valid && default_value.is_stored != saved_default_value),
	private_storage(make_shared<PrivateStorage>())
{
	if (default_value.is_stored != saved_default_value && !default_value.value_valid)
		throw invalid_argument("ComboBox: Default value is neither specified nor stored.");
}

void ComboBox::SetChoice(size_t item_index)
{
	ApplyValue(item_index);
	if (private_storage->gui_combobox)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_checkbox передаем не item_index, а value.
		private_storage->gui_combobox->SetValue(value);
	}
}

void ComboBox::PerformInternal(const function<bool (const Control*)> &action) const
{
}

void ComboBox::PerformInternal(const function<bool (Control*)> &action)
{
}

void ComboBox::LoadValues()
{
}

void ComboBox::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = ComputeStoredCaption();
	bool history_value_loaded = false;
	if (!history_caption.empty())
	{
		history_value = DecodeEnumValue(GetSavedParameter(settings_path, history_caption, wstring(),
				&history_value_loaded),
				history_value);
	}
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetChoice(history_value);
	}
}

void ComboBox::SaveHistory()
{
	if (!history_caption.empty())
		SaveParameter(history_settings_path, history_caption, EncodeEnumValue(value));
}

GuiControl ComboBox::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_combobox)
		throw runtime_error("ComboBox::CreateGuiControl: control already created.");
	auto gui_combobox = XRAD_GUI::ComboBoxApi::Create(use_caption, caption, items,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_combobox)
					throw runtime_error("ComboBox::DoUIChoiceChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(private_storage->gui_combobox->UIValue());
			});
	private_storage->gui_combobox = gui_combobox;
	return gui_combobox;
}

void ComboBox::BindGuiControls(const GuiControlBindings &bindings)
{
}

void ComboBox::UpdateFromGuiControl()
{
	if (!private_storage->gui_combobox)
		throw runtime_error("ComboBox::UpdateFromGuiControl: control is not created.");
	ApplyValue(private_storage->gui_combobox->UIValue());
}

void ComboBox::ReleaseGuiControl()
{
	private_storage->gui_combobox.reset();
}

void ComboBox::ApplyDialogResult(DialogResultCode code)
{
	if (on_value_apply)
		on_value_apply(value, code);
}

void ComboBox::ApplyValue(size_t v)
{
	if (v >= items.size())
		v = (size_t)-1;
	if (value_valid && v == value)
		return;
	value = v;
	value_valid = true;
	if (on_value_changed)
		on_value_changed(v);
}

wstring ComboBox::EncodeEnumValue(size_t value) const
{
	if (value == size_t(-1) || value > items.size())
		return wstring();
	return items[value];
}

size_t ComboBox::DecodeEnumValue(const wstring &str, size_t default_value) const
{
	for (size_t i = 0; i < items.size(); ++i)
	{
		if (items[i] == str)
			return i;
	}
	return default_value;
}

wstring ComboBox::ComputeStoredCaption() const
{
	if (!caption.empty())
		return caption;
	for (auto &item: items)
	{
		if (!item.empty())
			return L"_combo_" + item;
	}
	return {};
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



template <class EditTag>
struct StringEditBase<EditTag>::PrivateStorage
{
	using EditApi_t = typename EditTag::EditApi_t;
	shared_ptr<EditApi_t> gui_edit;
};

//--------------------------------------------------------------

template <class EditTag>
StringEditBase<EditTag>::StringEditBase(const wstring &caption,
		const GUIValue<wstring> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	caption(caption), default_value(default_value),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	on_value_apply(on_value_apply),
	value(EditTag::FormatValue(default_value.value)),
	value_valid(default_value.value_valid && default_value.is_stored != saved_default_value),
	private_storage(make_shared<PrivateStorage>())
{
	if (default_value.is_stored != saved_default_value && !default_value.value_valid)
		throw invalid_argument("StringEdit: Default value is neither specified nor stored.");
}

template <class EditTag>
void StringEditBase<EditTag>::SetOnValueChanged(const on_value_changed_t &on_value_changed)
{
	this->on_value_changed = on_value_changed;
}

template <class EditTag>
void StringEditBase<EditTag>::SetValue(const wstring &v)
{
	ApplyValue(v);
	if (private_storage->gui_edit)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_edit передаем не v, а value.
		private_storage->gui_edit->SetValue(value);
	}
}

template <class EditTag>
void StringEditBase<EditTag>::PerformInternal(const function<bool (const Control*)> &action) const
{
}

template <class EditTag>
void StringEditBase<EditTag>::PerformInternal(const function<bool (Control*)> &action)
{
}

template <class EditTag>
void StringEditBase<EditTag>::LoadValues()
{
}

template <class EditTag>
void StringEditBase<EditTag>::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = SelectHistoryCaption(caption, L"StringEdit", generic_id);
	bool history_value_loaded = false;
	history_value = GetSavedParameter(history_settings_path, history_caption, history_value,
			&history_value_loaded);
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetValue(history_value);
	}
}

template <class EditTag>
void StringEditBase<EditTag>::SaveHistory()
{
	SaveParameter(history_settings_path, history_caption, value);
}

template <class EditTag>
GuiControl StringEditBase<EditTag>::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("StringEdit::CreateGuiControl: control already created.");
	auto gui_edit = PrivateStorage::EditApi_t::Create(caption,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("StringEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(private_storage->gui_edit->UIValue());
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}

template <class EditTag>
void StringEditBase<EditTag>::BindGuiControls(const GuiControlBindings &bindings)
{
}

template <class EditTag>
void StringEditBase<EditTag>::UpdateFromGuiControl()
{
	if (!private_storage->gui_edit)
		throw runtime_error("StringEdit::UpdateFromGuiControl: control is not created.");
	ApplyValue(private_storage->gui_edit->UIValue());
}

template <class EditTag>
void StringEditBase<EditTag>::ReleaseGuiControl()
{
	private_storage->gui_edit.reset();
}

template <class EditTag>
void StringEditBase<EditTag>::ApplyDialogResult(DialogResultCode code)
{
	if (on_value_apply)
		on_value_apply(value, code);
}

template <class EditTag>
void StringEditBase<EditTag>::ApplyValue(const wstring &in_v)
{
	auto v = EditTag::FormatValue(in_v);
	if (value_valid && v == value)
		return;
	value = v;
	value_valid = true;
	if (on_value_changed)
		on_value_changed(v);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



class StringEditTag
{
	public:
		using EditApi_t = XRAD_GUI::StringEditApi;
		static wstring FormatValue(const wstring &text) { return text; }
};

template class StringEditBase<StringEditTag>;

//--------------------------------------------------------------

class TextEditTag
{
	public:
		using EditApi_t = XRAD_GUI::TextEditApi;
		static wstring FormatValue(const wstring &text);
};

wstring TextEditTag::FormatValue(const wstring &text)
{
	// Нормировать концы строк: "\n".
	const wchar_t *source = text.c_str();
	size_t source_length = text.length();
	vector<wchar_t> result_buf(text.length(), 0);
	wchar_t *result = result_buf.data();
	size_t out_i = 0;
	bool prev_cr = false;
	for (size_t i = 0; i < source_length; ++i)
	{
		wchar_t c = source[i];
		if (c == L'\n')
		{
			if (!prev_cr)
				result[out_i++] = c;
			prev_cr = false;
		}
		else if (c == L'\r')
		{
			result[out_i++] = L'\n';
			prev_cr = true;
		}
		else
		{
			result[out_i++] = c;
			prev_cr = false;
		}
	}
	return wstring(result, out_i);
}

template class StringEditBase<TextEditTag>;



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



template <class EditTag>
ValueStringEditBase<EditTag>::ValueStringEditBase(const wstring &caption, const GUIValue<wstring*> &p_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed):
	parent(caption, ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

template <class EditTag>
void ValueStringEditBase<EditTag>::Update()
{
	SetValue(*p_value);
}

template <class EditTag>
void ValueStringEditBase<EditTag>::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

template <class EditTag>
void ValueStringEditBase<EditTag>::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}

//--------------------------------------------------------------

template class ValueStringEditBase<StringEditTag>;
template class ValueStringEditBase<TextEditTag>;



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



// Явно инстанциируем реализации шаблона для поддерживаемых типов.
// Поскольку реализация шаблона не в заголовочном файле, по техническим причинам она там быть
// не может (скрытые из заголовочных файлов зависимости от API),
// без этой конструкции будут ошибки линковки.
//
// Типы специализаций должны содержать size_t и ptrdiff_t для всех платформ.
template class NumberEdit<int>;
template class NumberEdit<unsigned int>;
template class NumberEdit<long>;
template class NumberEdit<unsigned long>;
template class NumberEdit<long long>;
template class NumberEdit<unsigned long long>;
template class NumberEdit<double>;
template class NumberEdit<float>;

//--------------------------------------------------------------

template <class T>
struct NumberEdit<T>::PrivateStorage
{
	shared_ptr<XRAD_GUI::NumberEditApi<value_t>> gui_edit;
};

//--------------------------------------------------------------

template <class T>
NumberEdit<T>::NumberEdit(const wstring &caption,
		const GUIValue<value_t> &default_value,
		value_t min_value,
		value_t max_value,
		out_of_range_control allow_out_of_range,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	caption(caption),
	default_value(default_value),
	allow_out_of_range(allow_out_of_range),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	on_value_apply(on_value_apply),
	min_value(min_value),
	max_value(max_value),
	value(GetValueFromDefault(default_value, min_value, max_value)),
	value_valid(default_value.value_valid && default_value.is_stored != saved_default_value),
	history_value(default_value.value_valid? default_value.value: value_t()),
	private_storage(make_shared<PrivateStorage>())
{
	if (default_value.is_stored != saved_default_value && !default_value.value_valid)
		throw invalid_argument("NumberEdit: Default value is neither specified nor stored.");
	if (allow_out_of_range != out_of_range_allowed)
	{
		if (max_value < min_value)
			throw invalid_argument("NumberEdit: Invalid min-max values.");
		if (default_value.value_valid && !in_range(default_value.value, min_value, max_value))
			throw invalid_argument("NumberEdit: Invalid default value (out of range).");
	}
}

template <class T>
void NumberEdit<T>::SetOnValueChanged(const on_value_changed_t &on_value_changed_)
{
	on_value_changed = on_value_changed_;
}

template <class T>
void NumberEdit<T>::SetValue(value_t v)
{
	ApplyValue(v);
	if (private_storage->gui_edit)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_edit передаем не v, а value.
		private_storage->gui_edit->SetValue(value, min_value, max_value);
	}
}

template <class T>
void NumberEdit<T>::SetMinMax(value_t min_value_, value_t max_value_)
{
	if (allow_out_of_range != out_of_range_allowed)
	{
		if (max_value < min_value)
			throw invalid_argument("NumberEdit: Invalid min-max values.");
	}
	ApplyMinMax(min_value_, max_value_);
	if (private_storage->gui_edit)
	{
		// В ApplyMinMax() могут быть вызваны обработчики, изменяющие состояние.
		// Поэтому в gui_edit передаем текущие значения из объекта.
		private_storage->gui_edit->SetValue(value, min_value, max_value);
	}
}

template <class T>
void NumberEdit<T>::PerformInternal(const function<bool (const Control*)> &action) const
{
}

template <class T>
void NumberEdit<T>::PerformInternal(const function<bool (Control*)> &action)
{
}

template <class T>
void NumberEdit<T>::LoadValues()
{
}

template <class T>
void NumberEdit<T>::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = SelectHistoryCaption(caption, L"NumberEdit", generic_id);
	bool history_value_loaded = false;
	history_value = GetSavedParameter(history_settings_path, history_caption, history_value,
			&history_value_loaded);
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetValue(history_value);
	}
}

template <class T>
void NumberEdit<T>::SaveHistory()
{
	SaveParameter(history_settings_path, history_caption, value);
}

template <class T>
GuiControl NumberEdit<T>::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("NumberEdit::CreateGuiControl: control already created.");

	auto gui_edit = XRAD_GUI::NumberEditApi<value_t>::Create(caption,
			value,
			default_value,
			history_value,
			min_value,
			max_value,
			allow_out_of_range,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("NumberEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(private_storage->gui_edit->UIValue());
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}

template <class T>
void NumberEdit<T>::BindGuiControls(const GuiControlBindings &bindings)
{
}

template <class T>
void NumberEdit<T>::UpdateFromGuiControl()
{
	if (!private_storage->gui_edit)
		throw runtime_error("NumberEdit::UpdateFromGuiControl: control is not created.");
	ApplyValue(private_storage->gui_edit->UIValue());
}

template <class T>
void NumberEdit<T>::ReleaseGuiControl()
{
	private_storage->gui_edit.reset();
}

template <class T>
void NumberEdit<T>::ApplyDialogResult(DialogResultCode code)
{
	if (on_value_apply)
		on_value_apply(value, code);
}

template <class T>
void NumberEdit<T>::ApplyValue(value_t v)
{
	auto validated_v = ValidateValue(v);
	if (value_valid && validated_v == value)
		return;
	value = validated_v;
	value_valid = true;
	if (on_value_changed)
		on_value_changed(validated_v);
}

template <class T>
void NumberEdit<T>::ApplyMinMax(value_t min_value_, value_t max_value_)
{
	if (min_value == min_value_ && max_value == max_value_)
		return;
	min_value = min_value_;
	max_value = max_value_;
	if (allow_out_of_range == out_of_range_allowed)
		return;
	ApplyValue(value);
}

template <class T>
auto NumberEdit<T>::ValidateValue(value_t v) -> value_t
{
	switch (allow_out_of_range)
	{
		case out_of_range_allowed:
			return v;
		default:
		case out_of_range_prohibited:
			return range(v, min_value, max_value);
	}
}

template <class T>
auto NumberEdit<T>::GetValueFromDefault(const GUIValue<value_t> &default_value,
		value_t min_value, value_t max_value) -> value_t
{
	if (default_value.value_valid)
		return default_value.value;
	return range(default_value.value, min_value, max_value);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct FileNameEditBase::PrivateStorage
{
	shared_ptr<XRAD_GUI::FileNameEditApi> gui_edit;
};

//--------------------------------------------------------------

FileNameEditBase::FileNameEditBase(
		const wstring &caption,
		const GUIValue<value_t> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	caption(caption),
	default_value(ConvertGUIValue(default_value, GetPathGenericFromAutodetect(default_value.value))),
	control_layout(control_layout),
	on_value_changed(on_value_changed),
	on_value_apply(on_value_apply),
	value(GetPathGenericFromAutodetect(default_value.value)),
	filename_validity(true),
	value_valid(default_value.value_valid && default_value.is_stored != saved_default_value),
	private_storage(make_shared<PrivateStorage>())
{
	if (default_value.is_stored != saved_default_value && !default_value.value_valid)
		throw invalid_argument("FileNameEditBase: Default value is neither specified nor stored.");
}

void FileNameEditBase::SetOnValueChanged(const on_value_changed_t &on_value_changed)
{
	this->on_value_changed = on_value_changed;
}

void FileNameEditBase::SetValue(const value_t &v)
{
	ApplyValue(make_pair(v, true));
	if (private_storage->gui_edit)
	{
		// В ApplyValue() может быть вызван обработчик, который может изменить состояние value.
		// Поэтому в gui_edit передаем не v, а value.
		private_storage->gui_edit->SetValue(value);
	}
}

void FileNameEditBase::PerformInternal(const function<bool (const Control*)> &action) const
{
}

void FileNameEditBase::PerformInternal(const function<bool (Control*)> &action)
{
}

void FileNameEditBase::LoadValues()
{
}

void FileNameEditBase::LoadHistory(const wstring &settings_path, const wstring &generic_id)
{
	history_settings_path = settings_path;
	history_caption = SelectHistoryCaption(caption, L"FileNameEdit", generic_id);
	bool history_value_loaded = false;
	history_value = GetSavedParameter(history_settings_path, history_caption, history_value,
			&history_value_loaded);
	if (default_value.is_stored == saved_default_value && history_value_loaded)
	{
		SetValue(history_value);
	}
}

void FileNameEditBase::SaveHistory()
{
	SaveParameter(history_settings_path, history_caption, value);
}

void FileNameEditBase::BindGuiControls(const GuiControlBindings &bindings)
{
}

namespace
{

pair<wstring, bool> ConvertUIFileNameValue(
		const pair<wstring, XRAD_GUI::FileNameEditApi::ValueValidity> &v)
{
	return make_pair(v.first, v.second != XRAD_GUI::FileNameEditApi::ValueValidity::Invalid);
}

} // namespace

void FileNameEditBase::UpdateFromGuiControl()
{
	if (!private_storage->gui_edit)
		throw runtime_error("FileNameEditBase::UpdateFromGuiControl: control is not created.");
	ApplyValue(ConvertUIFileNameValue(private_storage->gui_edit->UIValue()));
}

void FileNameEditBase::ReleaseGuiControl()
{
	private_storage->gui_edit.reset();
}

void FileNameEditBase::ApplyDialogResult(DialogResultCode code)
{
	if (on_value_apply)
		on_value_apply(value, code);
}

void FileNameEditBase::ApplyValue(const pair<value_t, bool> &v)
{
	auto v_norm = GetPathGenericFromAutodetect(v.first);
	if (value_valid && v_norm == value && v.second == filename_validity)
		return;
	value = v_norm;
	filename_validity = v.second;
	value_valid = true;
	if (on_value_changed)
		on_value_changed(v.first);
}

//--------------------------------------------------------------

FileLoadEdit::FileLoadEdit(
		const wstring &caption,
		const GUIValue<value_t> &default_value,
		const wstring &file_type,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	parent(caption, default_value, control_layout, on_value_changed, on_value_apply),
	file_type(file_type)
{
}

GuiControl FileLoadEdit::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("FileLoadEdit::CreateGuiControl: control already created.");
	auto gui_edit = XRAD_GUI::FileNameEditApi::Create(
			XRAD_GUI::FileNameEditApi::CreateFileLoadValidator(file_type),
			caption,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("FileLoadEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(ConvertUIFileNameValue(private_storage->gui_edit->UIValue()));
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}

//--------------------------------------------------------------

FileSaveEdit::FileSaveEdit(
		const wstring &caption,
		const GUIValue<value_t> &default_value,
		const wstring &file_type,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	parent(caption, default_value, control_layout, on_value_changed, on_value_apply),
	file_type(file_type)
{
}

GuiControl FileSaveEdit::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("FileSaveEdit::CreateGuiControl: control already created.");
	auto gui_edit = XRAD_GUI::FileNameEditApi::Create(
			XRAD_GUI::FileNameEditApi::CreateFileSaveValidator(file_type),
			caption,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("FileSaveEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(ConvertUIFileNameValue(private_storage->gui_edit->UIValue()));
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}

//--------------------------------------------------------------

DirectoryReadEdit::DirectoryReadEdit(
		const wstring &caption,
		const GUIValue<value_t> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	parent(caption, default_value, control_layout, on_value_changed, on_value_apply)
{
}

GuiControl DirectoryReadEdit::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("DirectoryReadEdit::CreateGuiControl: control already created.");
	auto gui_edit = XRAD_GUI::FileNameEditApi::Create(
			XRAD_GUI::FileNameEditApi::CreateDirectoryReadValidator(),
			caption,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("DirectoryReadEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(ConvertUIFileNameValue(private_storage->gui_edit->UIValue()));
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}

//--------------------------------------------------------------

DirectoryWriteEdit::DirectoryWriteEdit(
		const wstring &caption,
		const GUIValue<value_t> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	parent(caption, default_value, control_layout, on_value_changed, on_value_apply)
{
}

GuiControl DirectoryWriteEdit::CreateGuiControlImpl(GuiControlBindings *bindings)
{
	if (private_storage->gui_edit)
		throw runtime_error("DirectoryWriteEdit::CreateGuiControl: control already created.");
	auto gui_edit = XRAD_GUI::FileNameEditApi::Create(
			XRAD_GUI::FileNameEditApi::CreateDirectoryWriteValidator(),
			caption,
			value,
			default_value,
			history_value,
			LayoutToUI(control_layout),
			[this]()
			{
				if (!private_storage->gui_edit)
					throw runtime_error("DirectoryWriteEdit::DoUICheckedChanged: control is not created.");
				// Запрашиваем текущее состояние UI (асинхронное): защита от зацикливания.
				ApplyValue(ConvertUIFileNameValue(private_storage->gui_edit->UIValue()));
			});
	private_storage->gui_edit = gui_edit;
	return gui_edit;
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



ValueFileLoadEdit::ValueFileLoadEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				const wstring &file_type,
				Layout control_layout,
				const on_value_changed_t &on_value_changed):
	parent(caption,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			file_type,
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

void ValueFileLoadEdit::Update()
{
	SetValue(*p_value);
}

void ValueFileLoadEdit::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

void ValueFileLoadEdit::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



ValueFileSaveEdit::ValueFileSaveEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				const wstring &file_type,
				Layout control_layout,
				const on_value_changed_t &on_value_changed):
	parent(caption,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			file_type,
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

void ValueFileSaveEdit::Update()
{
	SetValue(*p_value);
}

void ValueFileSaveEdit::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

void ValueFileSaveEdit::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



ValueDirectoryReadEdit::ValueDirectoryReadEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				Layout control_layout,
				const on_value_changed_t &on_value_changed):
	parent(caption,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

void ValueDirectoryReadEdit::Update()
{
	SetValue(*p_value);
}

void ValueDirectoryReadEdit::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

void ValueDirectoryReadEdit::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



ValueDirectoryWriteEdit::ValueDirectoryWriteEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				Layout control_layout,
				const on_value_changed_t &on_value_changed):
	parent(caption,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

void ValueDirectoryWriteEdit::Update()
{
	SetValue(*p_value);
}

void ValueDirectoryWriteEdit::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

void ValueDirectoryWriteEdit::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



struct Dialog::GuiDialog
{
	shared_ptr<XRAD_GUI::DialogApi> dialog;

	GuiDialog(shared_ptr<XRAD_GUI::DialogApi> &&dialog): dialog(dialog) {}
};

//--------------------------------------------------------------

struct Dialog::PrivateStorage
{
	shared_ptr<XRAD_GUI::DialogApi> gui_dialog;
	GuiControlBindings bindings;
};

//--------------------------------------------------------------

Dialog::Dialog(const wstring &caption,
		const default_value_t &default_value_policy):
	caption(caption),
	default_value_policy(default_value_policy),
	private_storage(make_shared<PrivateStorage>())
{
}

void Dialog::AddControl(shared_ptr<Control> control)
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::AddControl: not allowed in UI loop.");
	controls.push_back(control);
}

void Dialog::ThrowIfCanceled() const
{
	DialogResultCode code = DialogResultCode::Rejected;
	GetResult(&code);
	if (code != DialogResultCode::Rejected)
		return;
	throw canceled_operation(convert_to_string(L"Операция отменена пользователем."));
}

void Dialog::Show()
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::Show: The dialog is already shown.");
	internal_controls = controls;
	AppendInternalControls(&internal_controls);
	LoadValuesRoot();
	if (first_show)
	{
		LoadHistoryRoot();
		first_show = false;
	}
	{
		auto gui_dialog = CreateGuiControlsRoot().dialog;
		BindGuiControlsRoot();
		gui_dialog->Show();
	}
	UpdateFromGuiControlRoot();
	ReleaseGuiControlsRoot();
	if (result_code != DialogResultCode::Rejected)
		SaveHistoryRoot();
	ApplyDialogResultRoot();
}

void Dialog::ApplyHistory()
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::ApplyHistory: The dialog is already shown.");
	internal_controls = controls;
	AppendInternalControls(&internal_controls);
	LoadValuesRoot();
	if (first_show)
	{
		LoadHistoryRoot();
		first_show = false;
	}
	result_code = DialogResultCode::Accepted;
	ApplyDialogResultRoot();
}

void Dialog::Perform(const function<bool (const Control*)> &action) const
{
	for (auto &control: internal_controls)
	{
		const auto *const_control = control.get();
		const_control->Perform(action);
	}
}

void Dialog::Perform(const function<bool (Control*)> &action)
{
	for (auto &control: internal_controls)
	{
		control->Perform(action);
	}
}

void Dialog::LoadValuesRoot()
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::LoadValues: control already created.");
	for (auto &control: internal_controls)
	{
		control->LoadValues();
	}
}

void Dialog::LoadHistoryRoot()
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::LoadHistory: control already created.");
	auto root_path = L"DynamicDialog";
	auto path = CombineSettingsPath(root_path, caption);
	LoadDialogHistory(path);
	size_t i = 0;
	for (auto &control: internal_controls)
	{
		control->LoadHistory(path, ssprintf(L"#%zu", i));
		++i;
	}
}

void Dialog::LoadDialogHistory(const wstring &settings_path)
{
	history_settings_path = settings_path;
	history_caption = L"_dialog_result";
	bool history_value_loaded = false;
	history_value = DecodeEnumValue(GetSavedParameter(
			history_settings_path, history_caption, wstring(), &history_value_loaded),
			history_value);
	if (default_value_policy == saved_default_value && history_value_loaded)
	{
		SetDefaultButton(history_value);
	}
}

void Dialog::SaveHistoryRoot()
{
	SaveDialogHistory();
	for (auto &control: internal_controls)
	{
		control->SaveHistory();
	}
}

void Dialog::SaveDialogHistory()
{
	auto result = GetResult();
	SaveParameter(history_settings_path, history_caption, EncodeEnumValue(result));
	if (result && default_value_policy == saved_default_value)
	{
		SetDefaultButton(result);
	}
}

Dialog::GuiDialog Dialog::CreateGuiControlsRoot()
{
	if (private_storage->gui_dialog)
		throw runtime_error("Dialog::CreateGuiControls: control already created.");
	SetResult(nullptr, DialogResultCode::Rejected);
	auto gui_dialog = XRAD_GUI::DialogApi::Create(caption);
	auto *bindings = &private_storage->bindings;
	for (auto &control: internal_controls)
	{
		auto gui_control = control->CreateGuiControl(bindings);
		gui_dialog->AddControl(gui_control.control);
	}
	private_storage->gui_dialog = gui_dialog;
	return gui_dialog;
}

void Dialog::BindGuiControlsRoot()
{
	if (!private_storage->gui_dialog)
		throw runtime_error("Dialog::UpdateFromGuiControl: control is not created.");
	for (auto &control: internal_controls)
	{
		control->BindGuiControls(private_storage->bindings);
	}
}

void Dialog::UpdateFromGuiControlRoot()
{
	if (!private_storage->gui_dialog)
		throw runtime_error("Dialog::UpdateFromGuiControl: control is not created.");
	auto gui_result = private_storage->gui_dialog->GetResult();
	DynamicDialog::Button *dd_result_button = nullptr;
	DialogResultCode dd_result_code = DialogResultCode::Rejected;
	for (auto &button_binding: private_storage->bindings.buttons)
	{
		if (button_binding.second == gui_result)
		{
			dd_result_button = button_binding.first;
			dd_result_code = dd_result_button->ResultCode();
			break;
		}
	}
	SetResult(dd_result_button, dd_result_code);

	for (auto &control: internal_controls)
	{
		control->UpdateFromGuiControl();
	}
}

void Dialog::ReleaseGuiControlsRoot()
{
	private_storage->gui_dialog.reset();
	private_storage->bindings.clear();
	for (auto &control: internal_controls)
	{
		control->ReleaseGuiControl();
	}
}

void Dialog::ApplyDialogResultRoot()
{
	for (auto &control: internal_controls)
	{
		control->ApplyDialogResult(result_code);
	}
}

void Dialog::AppendInternalControls(vector<shared_ptr<Control>> *append_controls)
{
	// Do nothing.
}

void Dialog::SetDefaultButton(const Button *default_button)
{
	Perform([default_button](Control *control)
			{
				if (auto *button = dynamic_cast<Button*>(control))
				{
					button->SetDefault(button == default_button);
				}
				return true;
			});
}

wstring Dialog::EncodeEnumValue(const Button *value) const
{
	if (!value)
		return wstring();
	return value->Caption();
}

const Button *Dialog::DecodeEnumValue(const wstring &str, const Button *default_value) const
{
	const Button *result = default_value;
	Perform([&str, &result](const Control* control)
			{
				if (auto *button = dynamic_cast<const Button*>(control))
				{
					if (button->Caption() == str)
					{
						result = button;
						return false;
					}
				}
				return true;
			});
	return result;
}



//--------------------------------------------------------------

} // namespace DynamicDialog

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

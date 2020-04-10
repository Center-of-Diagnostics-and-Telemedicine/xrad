// file DynamicDialog.hh
//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

namespace DynamicDialog
{

//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
EnumRadioButtonChoiceImpl<T>::EnumRadioButtonChoiceImpl(
		const vector<choice_t> &choices,
		const GUIValue<value_type> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	EnumRadioButtonChoiceImpl(false, {}, choices, default_value, control_layout,
			on_value_changed, on_value_apply)
{
}

template <class T>
EnumRadioButtonChoiceImpl<T>::EnumRadioButtonChoiceImpl(const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_type> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	EnumRadioButtonChoiceImpl(true, caption, choices, default_value, control_layout,
			on_value_changed, on_value_apply)
{
}

template <class T>
EnumRadioButtonChoiceImpl<T>::EnumRadioButtonChoiceImpl(bool use_caption, const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<T> &default_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	default_value(default_value.value),
	on_value_changed(on_value_changed),
	on_value_apply(on_value_apply)
{
	bool default_choice_found = false;
	vector<shared_ptr<RadioButton>> items;
	for (auto &c: choices)
	{
		bool checked = false;
		if (!default_choice_found)
		{
			if (c.is_default)
			{
				checked = true;
				default_choice_found = true;
			}
			else if (try_equals<false>(c.id, default_value.value))
			{
				checked = true;
				// default_choice_found не устанавливаем, чтобы у флага c.is_default был приоритет
			}
		}
		auto rb = make_shared<RadioButton>(c.caption, checked);
		items.push_back(rb);
		item_data.emplace_back(rb, c.id);
	}
	container = make_shared<RadioButtonContainer>(use_caption, caption, std::move(items),
			ConvertGUIValue(default_value, nullptr),
			control_layout,
			[this](const RadioButton *rb)
			{
				DoChoiceChanged(rb);
			},
			[this](const RadioButton *rb, DialogResultCode code)
			{
				DoChoiceApply(rb, code);
			});
}

template <class T>
auto EnumRadioButtonChoiceImpl<T>::Choice() const -> value_type
{
	auto container_choice = container->Choice();
	for (auto &item: item_data)
	{
		if (item.first.get() == container_choice)
			return item.second;
	}
	return default_value;
}

template <class T>
void EnumRadioButtonChoiceImpl<T>::SetChoice(value_type v)
{
	const RadioButton *choice = nullptr;
	for (auto &item: item_data)
	{
		if (try_equals<false>(item.second, v))
		{
			choice = item.first.get();
			break;
		}
	}
	container->SetChoice(choice);
}

template <class T>
const RadioButtonContainer *EnumRadioButtonChoiceImpl<T>::ProxyControl() const
{
	return container.get();
}

template <class T>
RadioButtonContainer *EnumRadioButtonChoiceImpl<T>::ProxyControl()
{
	return container.get();
}

template <class T>
void EnumRadioButtonChoiceImpl<T>::DoChoiceChanged(const RadioButton *rb)
{
	if (!on_value_changed)
		return;
	for (auto &item: item_data)
	{
		if (item.first.get() == rb)
		{
			on_value_changed(item.second);
			return;
		}
	}
	on_value_changed(default_value);
}

template <class T>
void EnumRadioButtonChoiceImpl<T>::DoChoiceApply(const RadioButton *rb, DialogResultCode code)
{
	if (!on_value_apply)
		return;
	for (auto &item: item_data)
	{
		if (item.first.get() == rb)
		{
			on_value_apply(item.second, code);
			return;
		}
	}
	on_value_apply(default_value, code);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
ValueEnumRadioButtonChoiceImpl<T>::ValueEnumRadioButtonChoiceImpl(const vector<choice_t> &choices,
		const GUIValue<value_type*> &p_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed):
	parent(choices,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			control_layout,
			on_value_changed),
	p_value(p_value.value)
{
}

template <class T>
ValueEnumRadioButtonChoiceImpl<T>::ValueEnumRadioButtonChoiceImpl(const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_type*> &p_value,
		Layout control_layout,
		const on_value_changed_t &on_value_changed):
	parent(caption, choices,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			control_layout,
			on_value_changed),
	p_value(p_value.value)
{
}

template <class T>
void ValueEnumRadioButtonChoiceImpl<T>::Update()
{
	SetChoice(*p_value);
}

template <class T>
void ValueEnumRadioButtonChoiceImpl<T>::LoadValues()
{
	parent::LoadValues();
	SetChoice(*p_value);
}

template <class T>
void ValueEnumRadioButtonChoiceImpl<T>::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Choice();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
EnumComboBoxImpl<T>::EnumComboBoxImpl(const vector<choice_t> &choices,
		const GUIValue<value_type> &default_value,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	EnumComboBoxImpl(false, {}, choices, default_value, on_value_changed, on_value_apply)
{
}

template <class T>
EnumComboBoxImpl<T>::EnumComboBoxImpl(const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_type> &default_value,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	EnumComboBoxImpl(true, caption, choices, default_value, on_value_changed, on_value_apply)
{
}

template <class T>
EnumComboBoxImpl<T>::EnumComboBoxImpl(bool use_caption, const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_type> &default_value,
		const on_value_changed_t &on_value_changed,
		const on_value_apply_t &on_value_apply):
	default_value(default_value.value),
	on_value_changed(on_value_changed),
	on_value_apply(on_value_apply)
{
	bool default_choice_found = false;
	size_t default_choice_index = (size_t)-1;
	vector<wstring> items;
	for (size_t i = 0; i < choices.size(); ++i)
	{
		auto &c = choices[i];
		if (!default_choice_found)
		{
			if (c.is_default)
			{
				default_choice_found = true;
				default_choice_index = i;
			}
			else if (try_equals<false>(c.id, default_value.value))
			{
				default_choice_index = i;
				// default_choice_found не устанавливаем, чтобы у флага c.is_default был приоритет
			}
		}
		items.push_back(c.caption);
		item_data.push_back(c.id);
	}
	combobox = make_shared<ComboBox>(use_caption, caption, items,
			ConvertGUIValue(default_value, default_choice_index),
			[this](size_t item_index)
			{
				DoChoiceChanged(item_index);
			},
			[this](size_t item_index, DialogResultCode code)
			{
				DoChoiceApply(item_index, code);
			});
}

template <class T>
auto EnumComboBoxImpl<T>::Choice() const -> value_type
{
	auto item_index = combobox->Choice();
	static_assert(std::is_same<decltype(item_index), size_t>::value,
			"EnumComboBoxImpl<T>: Invalid item_index type.");
	if (item_index >= item_data.size())
		return default_value;
	return item_data[item_index];
}

template <class T>
void EnumComboBoxImpl<T>::SetChoice(value_type v)
{
	size_t choice_index = (size_t)-1;
	for (size_t i = 0; i < item_data.size(); ++i)
	{
		if (try_equals<false>(item_data[i], v))
		{
			choice_index = i;
			break;
		}
	}
	combobox->SetChoice(choice_index);
}

template <class T>
const ComboBox *EnumComboBoxImpl<T>::ProxyControl() const
{
	return combobox.get();
}

template <class T>
ComboBox *EnumComboBoxImpl<T>::ProxyControl()
{
	return combobox.get();
}

template <class T>
void EnumComboBoxImpl<T>::DoChoiceChanged(size_t item_index)
{
	if (!on_value_changed)
		return;
	if (item_index >= item_data.size())
		on_value_changed(default_value);
	else
		on_value_changed(item_data[item_index]);
}

template <class T>
void EnumComboBoxImpl<T>::DoChoiceApply(size_t item_index, DialogResultCode code)
{
	if (!on_value_apply)
		return;
	if (item_index >= item_data.size())
		on_value_apply(default_value, code);
	else
		on_value_apply(item_data[item_index], code);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
ValueEnumComboBoxImpl<T>::ValueEnumComboBoxImpl(const vector<choice_t> &choices,
		const GUIValue<value_type*> &p_value,
		const on_value_changed_t &on_value_changed):
	parent(choices,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			on_value_changed),
	p_value(p_value.value)
{
}

template <class T>
ValueEnumComboBoxImpl<T>::ValueEnumComboBoxImpl(const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_type*> &p_value,
		const on_value_changed_t &on_value_changed):
	parent(caption, choices,
			ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			on_value_changed),
	p_value(p_value.value)
{
}

template <class T>
void ValueEnumComboBoxImpl<T>::Update()
{
	SetChoice(*p_value);
}

template <class T>
void ValueEnumComboBoxImpl<T>::LoadValues()
{
	parent::LoadValues();
	SetChoice(*p_value);
}

template <class T>
void ValueEnumComboBoxImpl<T>::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Choice();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
ValueNumberEdit<T>::ValueNumberEdit(const wstring &caption, const GUIValue<value_t*> &p_value,
		value_t min_value,
		value_t max_value,
		out_of_range_control allow_out_of_range,
		Layout control_layout,
		const on_value_changed_t &on_value_changed):
	parent(caption, ConvertGUIValue(p_value, *CheckNotNull(p_value.value)),
			min_value, max_value, allow_out_of_range,
			control_layout, on_value_changed),
	p_value(p_value.value)
{
}

template <class T>
void ValueNumberEdit<T>::Update()
{
	SetValue(*p_value);
}

template <class T>
void ValueNumberEdit<T>::LoadValues()
{
	parent::LoadValues();
	SetValue(*p_value);
}

template <class T>
void ValueNumberEdit<T>::ApplyDialogResult(DialogResultCode code)
{
	if (code == DialogResultCode::Accepted)
	{
		*p_value = Value();
	}
	parent::ApplyDialogResult(code);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------



template <class T>
EnumDialogImpl<T>::EnumDialogImpl(const wstring &caption,
		const vector<choice_t> &choices,
		const GUIValue<value_t> &default_choice,
		const value_t &cancel_choice):
	Dialog(caption),
	default_choice(default_choice),
	cancel_choice(cancel_choice)
{
	size_t default_choice_index = (size_t)-1;
	for (size_t i = 0; i < choices.size(); ++i)
	{
		auto &c = choices[i];
		if (!default_choice_found)
		{
			if (c.is_default)
			{
				default_choice_found = true;
				default_choice_index = i;
			}
			else if (default_choice.value_valid && try_equals<false>(c.id, default_choice.value))
			{
				default_choice_index = i;
				// default_choice_found не устанавливаем, чтобы у флага c.is_default был приоритет
			}
		}
	}
	for (size_t i = 0; i < choices.size(); ++i)
	{
		auto &c = choices[i];
		auto dialog_result_code =
				c.is_cancel || (try_equals<false>(c.id, cancel_choice) && i != default_choice_index)?
				DialogResultCode::Rejected:
				DialogResultCode::Accepted;
		auto button = make_shared<Button>(c.caption, dialog_result_code);
		if (i == default_choice_index)
			button->SetDefault();
		buttons.push_back(make_pair(button, c.id));
	}
}

template <class T>
void EnumDialogImpl<T>::AddEnumButton(shared_ptr<Button> button, const value_t &value,
		ControlContainer *container)
{
	control_buttons.push_back(make_pair(button, value));
	if (!default_choice_found)
	{
		if (button->IsDefault())
		{
			default_choice_found = true;
		}
		else if (default_choice.value_valid && try_equals<false>(value, default_choice.value))
		{
			// default_choice_found не устанавливаем, чтобы у флага c.is_default был приоритет
		}
	}
	else
	{
		button->SetDefault(false);
	}
	// Внимание: переменная button становится недействительной.
	if (!container)
		AddControl(std::move(button));
	else
		container->AddControl(std::move(button));
}

template <class T>
void EnumDialogImpl<T>::AddEnumButton(shared_ptr<Button> button, const value_t &value,
		const shared_ptr<ControlContainer> &container)
{
	AddEnumButton(std::move(button), value, container.get());
}

template <class T>
auto EnumDialogImpl<T>::Choice() const -> value_t
{
	DialogResultCode code = DialogResultCode::Rejected;
	auto result = GetResult(&code);
	for (auto &b: buttons)
	{
		if (result == b.first.get())
			return b.second;
	}
	for (auto &b: control_buttons)
	{
		if (result == b.first.get())
			return b.second;
	}
	// Сюда попадаем только в том случае, если в диалог были добавлены дополнительные кнопки.
	if (code == DialogResultCode::Rejected)
		return cancel_choice;
	// Для DialogResultCode::Accepted с неизвестной кнопкой возвращаем default.
	return default_choice.value_valid? default_choice.value: value_t();
}

template <class T>
void EnumDialogImpl<T>::AppendInternalControls(vector<shared_ptr<Control>> *append_controls)
{
	auto button_layout = make_shared<ControlContainer>(Layout::Horizontal);
	button_layout->CreateControl<Stretch>();
	for (auto &b: buttons)
	{
		button_layout->AddControl(b.first);
	}
	button_layout->CreateControl<Stretch>();
	append_controls->push_back(button_layout);
}



//--------------------------------------------------------------
//
//--------------------------------------------------------------

} // namespace DynamicDialog

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

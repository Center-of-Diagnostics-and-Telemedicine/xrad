// file DynamicDialog.h
//--------------------------------------------------------------
#ifndef __DynamicDialog_h
#define __DynamicDialog_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADGUI/Sources/Core/GUICore.h>
#include "XRADGUI.h"

XRAD_BEGIN

//--------------------------------------------------------------

namespace DynamicDialog
{

//--------------------------------------------------------------

template <class T>
using Choice = xrad::Button<T>;

//--------------------------------------------------------------

namespace
{
template <class T>
T *CheckNotNull(T *ptr)
{
	XRAD_ASSERT_THROW(ptr != nullptr);
	return ptr;
}
} // namespace

//--------------------------------------------------------------

//! \brief Вспомогательная структура для скрытия деталей реализации и
//! устранения зависимости заголовочных файлов от заголовочных файлов GUI
struct GuiControl;

//! \brief Вспомогательная структура для скрытия деталей реализации и
//! устранения зависимости заголовочных файлов от заголовочных файлов GUI
struct GuiControlBindings;

//--------------------------------------------------------------

enum class Layout { Vertical, Horizontal };

//! \brief Тип результата диалога, принципиально только два варианта
enum class DialogResultCode { Accepted, Rejected };

//--------------------------------------------------------------

class Control
{
		//! \name Детали реализации: функции взаимодействия с UI
		//! @{
	public:
		//! \brief Выполнить действие для себя и вложенных контролов (если они есть)
		void Perform(const function<bool (const Control*)> &action) const;

		//! \brief Выполнить действие для себя и вложенных контролов (если они есть)
		void Perform(const function<bool (Control*)> &action);

	private:
		//! \brief Выполнить действие для вложенных контролов (если они есть)
		virtual void PerformInternal(const function<bool (const Control*)> &action) const = 0;

		//! \brief Выполнить действие для вложенных контролов (если они есть)
		virtual void PerformInternal(const function<bool (Control*)> &action) = 0;

	public:
		//! \brief Загрузить значение из внешней переменной (для контролов, привязанных к переменным)
		virtual void LoadValues() = 0;

		//! \brief Загрузить и применить значение из системного хранилища
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) = 0;

		//! \brief Сохранить значение в системном хранилище
		virtual void SaveHistory() = 0;

		//! \brief Вызывается в начале ShowDialog для создания элементов UI
		GuiControl CreateGuiControl(GuiControlBindings *bindings);

	private:
		//! \brief Вызывается в начале ShowDialog для создания элементов UI;
		//! добавлять созданный контрол в bindings->controls не требуется,
		//! это производится в CreateGuiControl()
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) = 0;

	public:
		//! \brief Вызывается в начале ShowDialog, после вызова всех CreateGuiControl,
		//! для связывания элементов UI друг с другом (использует GuiControlBindings::controls)
		virtual void BindGuiControls(const GuiControlBindings &bindings) = 0;

		//! \brief Вызывается после закрытия диалога до ReleaseGuiControl()
		//!   для чтения состояния из элементов UI
		virtual void UpdateFromGuiControl() = 0;

		//! \brief Вызывается в конце ShowDialog для удаления элементов UI
		virtual void ReleaseGuiControl() = 0;

		//! \brief Вызывается в конце ShowDialog после ReleaseGuiControl() для применения
		//!   или отмены результатов работы в UI
		virtual void ApplyDialogResult(DialogResultCode code) = 0;
		//! @}
};

//--------------------------------------------------------------

//! \brief Служебный класс для пробрасывания методов интерфейса Control одного объекта в другой
//!
//! Класс не шаблонный из-за метода CreateGuiControlImpl(): возвращает структуру со скрытой
//! реализацией.
class ControlProxy: public Control
{
		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

		//! \name Интерфейс ControlProxy
		//! @{
	protected:
		//! \brief Возвращает проксируемый контрол
		virtual const Control *ProxyControl() const = 0;

		//! \brief Возвращает проксируемый контрол
		virtual Control *ProxyControl() = 0;
		//! @}
};

//--------------------------------------------------------------

class ControlContainer: public Control
{
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать группу контролов без общей рамки
		ControlContainer(Layout control_layout = Layout::Vertical):
				control_layout(control_layout)
				{}
		//! \brief Создать группу контролов с общей рамкой
		ControlContainer(const wstring &group_caption, Layout control_layout = Layout::Vertical):
				use_frame(true),
				caption(group_caption),
				control_layout(control_layout)
				{}
		ControlContainer(const ControlContainer &) = delete;
		ControlContainer &operator=(const ControlContainer &) = delete;
		void AddControl(shared_ptr<Control> control) { controls.push_back(control); }

		//! \brief Создать контрол и добавить его в контейнер.
		//! Аргументы передаются в конструктор контрола
		template <class T, class... Args>
		shared_ptr<T> CreateControl(Args&&... args)
		{
			auto control = make_shared<T>(std::forward<Args>(args)...);
			AddControl(control);
			return control;
		}
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		const bool use_frame = false;
		const wstring caption;
		const Layout control_layout;
		vector<shared_ptr<Control>> controls;
};

//--------------------------------------------------------------

//! \brief Основа простых неинтерактивных контролов, не содержащих вложенных контролов,
//! у которых часть функций интерфейса Control ничего не делает
class StaticControlBase: public Control
{
		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}
};

//--------------------------------------------------------------

class Separator: public StaticControlBase
{
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать разделительную линию
		Separator();
		Separator(const Separator &) = delete;
		Separator &operator=(const Separator &) = delete;
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void ReleaseGuiControl() override;
		//! @}

	private:
		bool gui_in_use = false;
};

//--------------------------------------------------------------

class Stretch: public StaticControlBase
{
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать растягивающийся пробел
		Stretch();
		Stretch(const Stretch &) = delete;
		Stretch&operator=(const Stretch &) = delete;
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void ReleaseGuiControl() override;
		//! @}

	private:
		bool gui_in_use = false;
};

//--------------------------------------------------------------

class TextLabel: public StaticControlBase
{
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать текстовое поле (нередактируемое)
		TextLabel(const wstring &text);
		TextLabel(const TextLabel &) = delete;
		TextLabel &operator=(const TextLabel &) = delete;
		//! @}

		//! \name Работа с данными
		//! @{
	public:
		wstring Text() const { return text; }
		void SetText(const wstring &t);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void ReleaseGuiControl() override;
		//! @}

	private:
		wstring text;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

class Button: public Control
{
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать кнопку, закрывающую диалог
		Button(const wstring &caption, DialogResultCode code = DialogResultCode::Accepted);

		//! \brief Способ блокировки диалогового окна при выполнении команды без завершения диалога
		enum class ActionPolicy
		{
			//! \brief Диалоговое окно блокируется
			Block,
			//! \brief Диалоговое окно скрывается
			Hide,
			//! \brief Никаких блокировок не применяется
			None
		};

		//! \brief Создать кнопку, выполняющую действие, но не закрывающую диалог
		Button(const wstring &caption, const function<void ()> &action,
				ActionPolicy action_policy = ActionPolicy::Block);

		Button(const Button &) = delete;
		Button &operator=(const Button &) = delete;

		DialogResultCode ResultCode() const { return result_code; }
		void SetResultCode(DialogResultCode code);

		bool IsDefault() const { return default_button; }
		void SetDefault(bool is_default_button = true);

		/*!
			\brief Задать ограниченный список зависимостей по данным для кнопки

			Если кнопка закрывает диалог с кодом DialogResultCode::Accepted, то по умолчанию проверяется
			корректность заполнения всех не заблокированных полей диалога. Если вызвана эта функция,
			проверяется корректность заполнения только указанных полей. В списке можно задавать
			и контейнеры, тогда проверяются все содержащиеся в них элементы. Если список пустой,
			никакие поля диалога не проверяются.

			Если кнопка закрывает диалог с кодом DialogResultCode::Rejected, поля диалога не проверяются
			независимо от вызова этой функции.

			Если кнопка выполняет действие (лямбду) без закрытия диалога, то по умолчанию никакие поля
			диалога не проверяются.
			Если вызывана эта функция, проверяется корректность заполнения указанных полей.
			Методы ApplyDialogResult() не вызываются, поэтому связанные поля у Value-контролов
			не обновляются. Доступ к значениям возможен через методы Checked(), Choice(), Value() и т.п.
		*/
		void SetValueDependencies(vector<weak_ptr<Control>> dependencies);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		const wstring caption;
		DialogResultCode result_code = DialogResultCode::Accepted;
		bool default_button = false;
		const function<void ()> action;
		const ActionPolicy action_policy = ActionPolicy::Block;
		bool use_value_dependencies = false;
		vector<weak_ptr<Control>> value_dependencies;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

class CheckBox: public Control
{
		//! \name Инициализация
		//! @{
	public:
		using on_value_changed_t = function<void (bool checked)>;

		/*!
			\param default_value Особенности текущей реализации: если is_stored == saved_default_value,
				то обработчик on_checked_changed будет вызван при показе диалога, даже если загруженное
				значение не отличается от заданного в default_value.
				TODO: Перенести это замечание в общее место для всех контролов с GUIValue.
		*/
		CheckBox(const wstring &caption, const GUIValue<bool> &default_value,
				const on_value_changed_t &on_checked_changed = on_value_changed_t());
		CheckBox(const CheckBox &) = delete;
		CheckBox &operator=(const CheckBox &) = delete;

		void SetSubControl(shared_ptr<Control> control);

		//! \brief Создать контрол и установить его в качестве дочернего контрола.
		//! Аргументы передаются в конструктор контрола
		template <class T, class... Args>
		shared_ptr<T> CreateSubControl(Args&&... args)
		{
			auto control = make_shared<T>(std::forward<Args>(args)...);
			SetSubControl(control);
			return control;
		}

		void SetOnCheckedChanged(const on_value_changed_t &on_checked_changed);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		bool Checked() const { return value; }
		void SetChecked(bool c);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		void ApplyValue(bool v);
	private:
		const wstring caption;
		const GUIValue<bool> default_value;
		on_value_changed_t on_value_changed;
		shared_ptr<Control> sub_control;
	private:
		bool value;
		bool value_valid;
	private:
		wstring history_settings_path;
		wstring history_caption;
		bool history_value = false;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief CheckBox, управляющий значением bool по указателю. Обновление значения производится
//!   только по завершении диалога с результатом DialogResultCode::Accepted.
class ValueCheckBox: public CheckBox
{
		PARENT(CheckBox);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueCheckBox(const wstring &caption, const GUIValue<bool*> &p_checked,
				const on_value_changed_t &on_checked_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		bool *p_value;
};

//--------------------------------------------------------------

// Совместимость со старой версией DynamicDialog:
[[deprecated("Use CheckBox (Check_B_ox) instead.")]] typedef CheckBox Checkbox;
[[deprecated("Use ValueCheckBox (ValueCheck_B_ox) instead.")]] typedef ValueCheckBox ValueCheckbox;

//--------------------------------------------------------------

//! \brief Вспомогательная структура для скрытия деталей реализации в заголовочных файлах
struct GuiControlRadioButton;

/*!
	\brief Класс радиокнопки. Предназначен для добавления только в RadioButtonContainer

	Программное переключение кнопок осуществляется методами RadioButtonContainer.

	Класс не наследуется от Control, не попадает в bindings.controls.
	При необходимости наследование можно добавить, но так, чтобы обеспечить запрет на использование
	радиокнопки вне RadioButtonContainer.
*/
class RadioButton
{
		//! \name Инициализация
		//! @{
	public:
		using on_checked_changed_t = function<void (bool checked)>;

		RadioButton(const wstring &caption,
				const on_checked_changed_t &on_checked_changed = on_checked_changed_t());
		RadioButton(const wstring &caption, bool default_checked,
				const on_checked_changed_t &on_checked_changed = on_checked_changed_t());
		RadioButton(const RadioButton &) = delete;
		RadioButton &operator=(const RadioButton &) = delete;

		void SetSubControl(shared_ptr<Control> control);

		//! \brief Создать контрол и установить его в качестве дочернего контрола.
		//! Аргументы передаются в конструктор контрола
		template <class T, class... Args>
		shared_ptr<T> CreateSubControl(Args&&... args)
		{
			auto control = make_shared<T>(std::forward<Args>(args)...);
			SetSubControl(control);
			return control;
		}

		void SetOnCheckedChanged(const function<void (bool checked)> &on_checked_changed);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		bool Checked() const { return checked; }

		// Можно при необходимости сделать функцию void Check().
		//! @}

		//! \name Детали реализации: функции взаимодействия с группой
		//! @{
	private:
		friend class RadioButtonContainer;
		void CheckedChanged(bool c);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс, аналогичный Control)
		//! @{
	public:
		void Perform(const function<bool (const Control*)> &action) const;
		void Perform(const function<bool (Control*)> &action);
	private:
		void PerformInternal(const function<bool (const Control*)> &action) const;
		void PerformInternal(const function<bool (Control*)> &action);
	public:
		void LoadValues();
		void LoadHistory(const wstring &settings_path, const wstring &generic_id);
		void SaveHistory();
		GuiControlRadioButton CreateGuiControlRadioButton(GuiControlBindings *bindings);
		void BindGuiControls(const GuiControlBindings &bindings);
		void UpdateFromGuiControl();
		void ReleaseGuiControl();
		void ApplyDialogResult(DialogResultCode code);
		//! @}

	private:
		const wstring caption;
		bool checked;
		function<void (bool checked)> on_checked_changed;
		shared_ptr<Control> sub_control;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

/*!
	\brief Класс, отвечающий за группу радиокнопок. Содержит в себе только радиокнопки

	Добавление других контролов, кроме радиокнопок, запрещено из соображений создания более
	простых GUI. При этом другие контролы могут быть субконтролами радиокнопок.
*/
class RadioButtonContainer: public Control
{
	public:
		using on_value_changed_t = function<void (const RadioButton *choice)>;
		using on_value_apply_t = function<void (const RadioButton *choice, DialogResultCode code)>;
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать группу радиокнопок без общей рамки
		RadioButtonContainer(
				default_value_t default_value_type = default_value_t::fixed,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_choice_changed = on_value_changed_t(),
				const on_value_apply_t &on_choice_apply = on_value_apply_t());

		//! \brief Создать группу радиокнопок без общей рамки
		RadioButtonContainer(
				vector<shared_ptr<RadioButton>> controls,
				const GUIValue<const RadioButton*> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_choice_changed = on_value_changed_t(),
				const on_value_apply_t &on_choice_apply = on_value_apply_t());

		//! \brief Создать группу радиокнопок с общей рамкой
		RadioButtonContainer(const wstring &group_caption,
				default_value_t default_value_type = default_value_t::fixed,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_choice_changed = on_value_changed_t(),
				const on_value_apply_t &on_choice_apply = on_value_apply_t());

		//! \brief Создать группу радиокнопок с общей рамкой
		RadioButtonContainer(const wstring &group_caption,
				vector<shared_ptr<RadioButton>> controls,
				const GUIValue<const RadioButton*> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_choice_changed = on_value_changed_t(),
				const on_value_apply_t &on_choice_apply = on_value_apply_t());

		//! \brief Создать группу радиокнопок
		RadioButtonContainer(bool use_frame, const wstring &group_caption,
				vector<shared_ptr<RadioButton>> controls,
				const GUIValue<const RadioButton*> &default_value,
				Layout control_layout,
				const on_value_changed_t &on_choice_changed,
				const on_value_apply_t &on_choice_apply);

	public:
		RadioButtonContainer(const RadioButtonContainer &) = delete;
		RadioButtonContainer &operator=(const RadioButtonContainer &) = delete;

		void AddControl(shared_ptr<RadioButton> control);

		//! \brief Создать контрол = RadioButton и добавить его в контейнер.
		//! Аргументы передаются в конструктор контрола
		template <class... Args>
		shared_ptr<RadioButton> CreateRadioButton(Args&&... args)
		{
			auto control = make_shared<RadioButton>(std::forward<Args>(args)...);
			AddControl(control);
			return control;
		}
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		//! \brief Выбранная радиокнопка. NULL, если ни одна кнопка не выбрана
		const RadioButton *Choice() const { return IndexToButton(value); }

		/*!
			\brief Установить выбор на один из элементов группы. Вне цикла UI можно также сбрасывать
				выбор со всех элементов, передавая value = NULL

			UI может не поддерживать снятие выбора со всех элементов группы, поэтому такая возможность
			блокируется при запущенном UI.
		*/
		void SetChoice(const RadioButton *value);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		void ApplyValue(size_t v);

		size_t ButtonToIndex(const RadioButton *b) const;
		const RadioButton *IndexToButton(size_t i) const;

		wstring EncodeEnumValue(size_t value) const;
		size_t DecodeEnumValue(const wstring &str, size_t default_value) const;
		wstring ComputeStoredCaption() const;

	private:
		const bool use_frame = false;
		const wstring caption;

		//! Это поле должно идти раньше default_value, поскольку default_value инициализируется
		//! с использованием этого поля.
		vector<shared_ptr<RadioButton>> controls;

		//! \brief Значение по умолчанию. Это поле в классе должно идти раньше value, value_valid,
		//! поскольку указанные поля вычисляются по этому полю
		GUIValue<const RadioButton*> default_value;

		const Layout control_layout;
		const on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
	private:
		//! \brief Индекс выбранной кнопки (от 0 до controls.size()-1) или (size_t)-1
		size_t value;
		bool value_valid;
	private:
		wstring history_settings_path;
		wstring history_caption;
		size_t history_value = (size_t)-1;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief Класс-реализация для создания группы радиокнопок, связанных со значениями произвольного
//! типа (не обязательно enum)
template <class T>
class EnumRadioButtonChoiceImpl: public ControlProxy
{
	public:
		using value_type = T;
		using choice_t = Choice<T>;
		using on_value_changed_t = function<void (const value_type &choice)>;
		using on_value_apply_t = function<void (const value_type &value, DialogResultCode code)>;

		//! \name Инициализация
		//! @{
	public:
		EnumRadioButtonChoiceImpl(const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		EnumRadioButtonChoiceImpl(const wstring &caption,
				const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		EnumRadioButtonChoiceImpl(bool use_caption, const wstring &caption,
				const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		//! \brief Доступ к реализации: ComboBox. Для отладочных целей
		shared_ptr<RadioButtonContainer> InternalContainer() { return container; }
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return container->Caption(); }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		//! \brief Возвращает выбранное значение. Если ничего не выбрано, возвращает default_value
		value_type Choice() const;

		//! \brief Установить выбор на заданный элемент
		void SetChoice(value_type v);
		//! @}

		//! \name Интерфейс ControlProxy
		//! @{
	protected:
		virtual const RadioButtonContainer *ProxyControl() const override;
		virtual RadioButtonContainer *ProxyControl() override;
		//! @}

	private:
		void DoChoiceChanged(const RadioButton *rb);
		void DoChoiceApply(const RadioButton *rb, DialogResultCode code);

	private:
		const value_type default_value;
		const on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
		vector<pair<shared_ptr<RadioButton>, value_type>> item_data;
		shared_ptr<RadioButtonContainer> container;
};

//--------------------------------------------------------------

/*!
	\brief RadioButtonContainer, управляющий значением по указателю

	Обновление значения производится
	только по завершении диалога с результатом DialogResultCode::Accepted.

	Значение *p_value на момент вызова конструктора используется как значение по умолчанию.
	Значение *p_value на момент показа диалога (dialog->Show()) используется как начальное значение
	(если оно не переопределяется сохраненным значением SavedGUIValue). Начальное значение может
	отличаться от значения по умолчанию.

	См. также ValueEnumComboBoxImpl.
*/
template <class T>
class ValueEnumRadioButtonChoiceImpl: public EnumRadioButtonChoiceImpl<T>
{
		PARENT(EnumRadioButtonChoiceImpl<T>);

		//! \name Инициализация
		//! @{
	public:
		using parent::value_type;
		using parent::on_value_changed_t;

		ValueEnumRadioButtonChoiceImpl(
				const vector<choice_t> &choices,
				const GUIValue<value_type*> &p_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		ValueEnumRadioButtonChoiceImpl(const wstring &caption,
				const vector<choice_t> &choices,
				const GUIValue<value_type*> &p_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_type *p_value;
};

//--------------------------------------------------------------

/*!
	\brief Класс для создания группы радиокнопок, связанных со значениями произвольного
	типа (не обязательно enum). Для создания объектов использовать статические функции Create

	Функции Create можно было бы сделать членами шаблонного класса EnumRadioButtonChoiceImpl,
	но тогда для вызова этих функций потребовалось бы указывать параметр шаблона T, что неудобно.
	Можно сделать набор функций не членов класса.

	Созданы пары функций с аргументами const GUIValue<T*> p_value и T *p_value, поскольку
	из-за шаблонной природы функций автоматическое преобразование T* в GUIValue<T*> не производится.
*/
class EnumRadioButtonChoice
{
	public:
		//! \brief Создать группу радиокнопок, привязанную к значениям произвольного типа,
		//! управляющую значением по указателю
		template <class T>
		static shared_ptr<ValueEnumRadioButtonChoiceImpl<T>> Create(
				const vector<Choice<T>> &choices,
				const GUIValue<T*> p_value,
				Layout control_layout = Layout::Vertical,
				const typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumRadioButtonChoiceImpl<T>>(
					choices, p_value,
					control_layout,
					on_value_changed);
		}

		//! \brief Создать группу радиокнопок, привязанную к значениям произвольного типа,
		//! управляющую значением по указателю
		template <class T>
		static shared_ptr<ValueEnumRadioButtonChoiceImpl<T>> Create(
				const vector<Choice<T>> &choices,
				T *p_value,
				Layout control_layout = Layout::Vertical,
				const typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumRadioButtonChoiceImpl<T>>(
					choices, p_value,
					control_layout,
					on_value_changed);
		}

		/*!
			\brief Создать группу радиокнопок, привязанную к значениям произвольного типа

			Необходимость данной функции сомнительна.

			\note
			Имя функции отличается от имени функции, принимающей параметр value типа указателя.
			Если имя сделать одинаковым, при использовании той функции возникает предупреждение
			C4505 [MSVC2015] о неиспользуемой функции ProxyControl() (ошибок в работе программы
			при этом не выявлено).
			Поскольку необходимость данной функции сомнительна, ей дано другое имя.
		*/
		template <class T>
		static shared_ptr<EnumRadioButtonChoiceImpl<T>> CreateX(const wstring &caption,
				const vector<Choice<T>> &choices,
				const T &value,
				Layout control_layout = Layout::Vertical,
				const typename EnumRadioButtonChoiceImpl<T>::on_value_changed_t &on_value_changed =
						typename EnumRadioButtonChoiceImpl<T>::on_value_changed_t())
		{
			return make_shared<EnumRadioButtonChoiceImpl<T>>(
					caption, choices, value,
					control_layout, on_value_changed);
		}

		//! \brief Создать группу радиокнопок, привязанную к значениям произвольного типа,
		//! управляющую значением по указателю
		template <class T>
		static shared_ptr<ValueEnumRadioButtonChoiceImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				const GUIValue<T*> p_value,
				Layout control_layout = Layout::Vertical,
				const typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumRadioButtonChoiceImpl<T>>(
					caption, choices, p_value,
					control_layout, on_value_changed);
		}

		//! \brief Создать группу радиокнопок, привязанную к значениям произвольного типа,
		//! управляющую значением по указателю
		template <class T>
		static shared_ptr<ValueEnumRadioButtonChoiceImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				T *p_value,
				Layout control_layout = Layout::Vertical,
				const typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumRadioButtonChoiceImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumRadioButtonChoiceImpl<T>>(
					caption, choices, p_value,
					control_layout, on_value_changed);
		}
};

//--------------------------------------------------------------

class ComboBox: public Control
{
	public:
		using on_value_changed_t = function<void (size_t value)>;
		using on_value_apply_t = function<void (size_t value, DialogResultCode code)>;
		//! \name Инициализация
		//! @{
	public:
		//! \brief Создать ComboBox без заголовка
		ComboBox(const vector<wstring> &items, const GUIValue<size_t> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		//! \brief Создать ComboBox с заголовком
		ComboBox(const wstring &caption, const vector<wstring> &items,
				const GUIValue<size_t> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		//! \brief Создать ComboBox
		ComboBox(bool use_caption, const wstring &caption,
				const vector<wstring> &items,
				const GUIValue<size_t> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		ComboBox(const ComboBox &) = delete;
		ComboBox &operator=(const ComboBox &) = delete;
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		const vector<wstring> Items() const { return items; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		//! \brief Индекс выбранного элемента. (size_t)-1, если ни один элемент не выбран
		size_t Choice() const { return value; }

		//! \brief Установить выбор на элемент с заданным индексом
		void SetChoice(size_t item_index);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		void ApplyValue(size_t v);
		wstring EncodeEnumValue(size_t value) const;
		size_t DecodeEnumValue(const wstring &str, size_t default_value) const;
		wstring ComputeStoredCaption() const;

	private:
		const bool use_caption = false;
		const wstring caption;
		const vector<wstring> items;
		const GUIValue<size_t> default_value;
		const on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
	private:
		size_t value;
		bool value_valid;
	private:
		wstring history_settings_path;
		wstring history_caption;
		size_t history_value = (size_t)-1;
private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief Класс-реализация для создания комбобокса, связанного со значениями произвольного
//! типа (не обязательно enum)
template <class T>
class EnumComboBoxImpl: public ControlProxy
{
	public:
		using value_type = T;
		using choice_t = Choice<value_type>;
		using on_value_changed_t = function<void (const value_type &choice)>;
		using on_value_apply_t = function<void (const value_type &value, DialogResultCode code)>;

		//! \name Инициализация
		//! @{
	public:
		EnumComboBoxImpl(const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		EnumComboBoxImpl(const wstring &caption, const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		EnumComboBoxImpl(bool use_caption, const wstring &caption,
				const vector<choice_t> &choices,
				const GUIValue<value_type> &default_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());

		//! \brief Доступ к реализации: ComboBox. Для отладочных целей
		shared_ptr<ComboBox> InternalComboBox() { return combobox; }
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return combobox->Caption(); }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		//! \brief Возвращает выбранное значение. Если ничего не выбрано, возвращает default_value
		value_type Choice() const;

		//! \brief Установить выбор на заданный элемент
		void SetChoice(value_type v);
		//! @}

		//! \name Интерфейс ControlProxy
		//! @{
	protected:
		virtual const ComboBox *ProxyControl() const override;
		virtual ComboBox *ProxyControl() override;
		//! @}

	private:
		void DoChoiceChanged(size_t item_index);
		void DoChoiceApply(size_t item_index, DialogResultCode code);

	private:
		vector<value_type> item_data;
		const value_type default_value;
		const on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
		shared_ptr<ComboBox> combobox;
};

//--------------------------------------------------------------

//! \brief Combobox, управляющий значением по указателю. Обновление значения производится
//!   только по завершении диалога с результатом DialogResultCode::Accepted.
template <class T>
class ValueEnumComboBoxImpl: public EnumComboBoxImpl<T>
{
		PARENT(EnumComboBoxImpl<T>);

		//! \name Инициализация
		//! @{
	public:
		using parent::value_type;
		using parent::on_value_changed_t;

		ValueEnumComboBoxImpl(const vector<choice_t> &choices,
				const GUIValue<value_type*> &p_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		ValueEnumComboBoxImpl(const wstring &caption, const vector<choice_t> &choices,
				const GUIValue<value_type*> &p_value,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_type *p_value;
};

//--------------------------------------------------------------

/*!
	\brief Класс для создания комбобокса, связанного со значениями произвольного
	типа (не обязательно enum). Для создания объектов использовать статические функции Create

	Функции Create можно было бы сделать членами шаблонного класса EnumComboBoxImpl,
	но тогда для вызова этих функций потребовалось бы указывать параметр шаблона T, что неудобно.
	Можно сделать набор функций не членов класса.
*/
class EnumComboBox
{
	public:
		//! \brief Создать комбобокс, привязанный к значениям произвольного типа,
		//! управляющий значением по указателю
		template <class T>
		static shared_ptr<ValueEnumComboBoxImpl<T>> Create(
				const vector<Choice<T>> &choices,
				const GUIValue<T*> &p_value,
				const typename ValueEnumComboBoxImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumComboBoxImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumComboBoxImpl<T>>(choices, p_value, on_value_changed);
		}

		//! \brief Создать комбобокс, привязанный к значениям произвольного типа,
		//! управляющий значением по указателю
		template <class T>
		static shared_ptr<ValueEnumComboBoxImpl<T>> Create(
				const vector<Choice<T>> &choices,
				T *p_value,
				const typename ValueEnumComboBoxImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumComboBoxImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumComboBoxImpl<T>>(choices, p_value, on_value_changed);
		}

		//! \brief Создать комбобокс, привязанный к значениям произвольного типа,
		//! управляющий значением по указателю
		template <class T>
		static shared_ptr<ValueEnumComboBoxImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				const GUIValue<T*> &p_value,
				const typename ValueEnumComboBoxImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumComboBoxImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumComboBoxImpl<T>>(caption, choices, p_value, on_value_changed);
		}

		//! \brief Создать комбобокс, привязанный к значениям произвольного типа,
		//! управляющий значением по указателю
		template <class T>
		static shared_ptr<ValueEnumComboBoxImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				T *p_value,
				const typename ValueEnumComboBoxImpl<T>::on_value_changed_t &on_value_changed =
						typename ValueEnumComboBoxImpl<T>::on_value_changed_t())
		{
			return make_shared<ValueEnumComboBoxImpl<T>>(caption, choices, p_value, on_value_changed);
		}
};

//--------------------------------------------------------------

//! \brief Реализация поля ввода значения типа wstring. Использовать следует StringEdit, TextEdit
template <class EditTag>
class StringEditBase: public Control
{
		//! \name Инициализация
		//! @{
	public:
		using on_value_changed_t = function<void (const wstring &value)>;
		using on_value_apply_t = function<void (const wstring &value, DialogResultCode code)>;

		StringEditBase(const wstring &caption, const GUIValue<wstring> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		StringEditBase(const StringEditBase &) = delete;
		StringEditBase &operator=(const StringEditBase &) = delete;
		void SetOnValueChanged(const on_value_changed_t &on_value_changed);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		wstring Value() const { return value; }
		void SetValue(const wstring &v);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		void ApplyValue(const wstring &v);
	private:
		const wstring caption;
		const GUIValue<wstring> default_value;
		const Layout control_layout;
		on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
	private:
		wstring value;
		bool value_valid;
	private:
		wstring history_settings_path;
		wstring history_caption;
		wstring history_value;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

class StringEditTag;

//! \brief Поле ввода строкового значения (однострочного текста)
using StringEdit = StringEditBase<StringEditTag>;

//--------------------------------------------------------------

class TextEditTag;

//! \brief Поле редактирования многострочного текста. Переводы строк нормирует на "\n"
using TextEdit = StringEditBase<TextEditTag>;

//--------------------------------------------------------------

//! \brief Реализация поля ввода, управляющего значением wstring по указателю.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted.
//! Использовать следует ValueStringEdit, ValueTextEdit
template <class EditTag>
class ValueStringEditBase: public StringEditBase<EditTag>
{
		PARENT(StringEditBase<EditTag>);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueStringEditBase(const wstring &caption, const GUIValue<wstring*> &p_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		wstring *p_value;
};

//--------------------------------------------------------------

//! \brief Поле ввода, управляющее значением wstring по указателю.
//! Однострочный текст.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted
using ValueStringEdit = ValueStringEditBase<StringEditTag>;

//! \brief Поле ввода, управляющее значением wstring по указателю.
//! Однострочный текст.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted
using ValueTextEdit = ValueStringEditBase<TextEditTag>;

//--------------------------------------------------------------

/*!
	\brief Поле ввода числа. Доступны реализации только для ограниченного количества типов T

	Допустимые значения типа T:
	- int
	- unsigned int
	- long long
	- unsigned long long
	- double
	- float
*/
template <class T>
class NumberEdit: public Control
{
		static_assert(
				std::is_same<T, int>::value ||
				std::is_same<T, unsigned int>::value ||
				std::is_same<T, long long>::value ||
				std::is_same<T, unsigned long long>::value ||
				std::is_same<T, double>::value ||
				std::is_same<T, float>::value,
				"Unsupported template T parameter type for NumberEdit<T>.");

		//! \name Инициализация
		//! @{
	public:
		using value_t = T;
		using on_value_changed_t = function<void (value_t value)>;
		using on_value_apply_t = function<void (value_t value, DialogResultCode code)>;

		NumberEdit(const wstring &caption,
				const GUIValue<value_t> &default_value,
				value_t min_value,
				value_t max_value,
				out_of_range_control allow_out_of_range = out_of_range_prohibited,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		NumberEdit(const NumberEdit &) = delete;
		NumberEdit &operator=(const NumberEdit &) = delete;
		void SetOnValueChanged(const on_value_changed_t &on_value_changed);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		value_t Value() const { return value; }
		void SetValue(value_t v);

		value_t Min() const { return min_value; }
		value_t Max() const { return max_value; }
		void SetMinMax(value_t min_value_, value_t max_value_);
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	private:
		void ApplyValue(value_t v);
		void ApplyMinMax(value_t min_value_, value_t max_value_);
		value_t ValidateValue(value_t v);
		static value_t GetValueFromDefault(const GUIValue<value_t> &v,
				value_t min_value, value_t max_value);
	private:
		const wstring caption;
		const GUIValue<value_t> default_value;
		const out_of_range_control allow_out_of_range;
		const Layout control_layout;
		on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
	private:
		value_t min_value;
		value_t max_value;
		value_t value;
		bool value_valid;
	private:
		wstring history_settings_path;
		wstring history_caption;
		value_t history_value;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief Поле ввода, управляющее числовым значением по указателю. Обновление значения производится
//!   только по завершении диалога с результатом DialogResultCode::Accepted
template <class T>
class ValueNumberEdit: public NumberEdit<T>
{
		PARENT(NumberEdit<T>);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueNumberEdit(const wstring &caption, const GUIValue<value_t*> &p_value,
				value_t min_value,
				value_t max_value,
				out_of_range_control allow_out_of_range = out_of_range_prohibited,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Создать контрол, allow_out_of_range = out_of_range_prohibited
		ValueNumberEdit(const wstring &caption, const GUIValue<value_t*> &p_value,
				value_t min_value,
				value_t max_value,
				Layout control_layout,
				const on_value_changed_t &on_value_changed = on_value_changed_t()):
			ValueNumberEdit(caption,
					p_value, min_value, max_value, out_of_range_prohibited,
					control_layout, on_value_changed)
		{
		}

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_t *p_value;
};

//--------------------------------------------------------------

//! \brief Базовый класс для поля ввода имени файла или директории
class FileNameEditBase: public Control
{
		//! \name Инициализация
		//! @{
	public:
		using value_t = wstring;
		using on_value_changed_t = function<void (value_t value)>;
		using on_value_apply_t = function<void (value_t value, DialogResultCode code)>;

		FileNameEditBase(
				const wstring &caption,
				const GUIValue<value_t> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		FileNameEditBase(const FileNameEditBase &) = delete;
		FileNameEditBase &operator=(const FileNameEditBase &) = delete;
		void SetOnValueChanged(const on_value_changed_t &on_value_changed);
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с состоянием
		//! @{
	public:
		value_t Value() const { return value; }
		void SetValue(const value_t &v);

		//! \brief Проверить валидность имени файла.
		//! Работает верно только при редактировании значения через UI
		bool FileNameValid() const { return filename_validity; }
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void PerformInternal(const function<bool (const Control*)> &action) const override;
		virtual void PerformInternal(const function<bool (Control*)> &action) override;
		virtual void LoadValues() override;
		virtual void LoadHistory(const wstring &settings_path, const wstring &generic_id) override;
		virtual void SaveHistory() override;
		virtual void BindGuiControls(const GuiControlBindings &bindings) override;
		virtual void UpdateFromGuiControl() override;
		virtual void ReleaseGuiControl() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	protected:
		void ApplyValue(const pair<value_t, bool> &v);
	protected:
		const wstring caption;
		const GUIValue<value_t> default_value;
		const Layout control_layout;
		on_value_changed_t on_value_changed;
		const on_value_apply_t on_value_apply;
	protected:
		value_t value;
		bool filename_validity;
		bool value_valid;
	protected:
		wstring history_settings_path;
		wstring history_caption;
		value_t history_value;
	protected:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени файла для чтения
class FileLoadEdit: public FileNameEditBase
{
		PARENT(FileNameEditBase);
		//! \name Инициализация
		//! @{
	public:
		FileLoadEdit(
				const wstring &caption,
				const GUIValue<value_t> &default_value,
				const wstring &file_type,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		//! @}

	private:
		const wstring file_type;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени файла для записи
class FileSaveEdit: public FileNameEditBase
{
		PARENT(FileNameEditBase);
		//! \name Инициализация
		//! @{
	public:
		FileSaveEdit(
				const wstring &caption,
				const GUIValue<value_t> &default_value,
				const wstring &file_type,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		//! @}

	private:
		const wstring file_type;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени директории для чтения
class DirectoryReadEdit: public FileNameEditBase
{
		PARENT(FileNameEditBase);
		//! \name Инициализация
		//! @{
	public:
		DirectoryReadEdit(
				const wstring &caption,
				const GUIValue<value_t> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		//! @}
};

//--------------------------------------------------------------

//! \brief Поле ввода имени директории для записи
class DirectoryWriteEdit: public FileNameEditBase
{
		PARENT(FileNameEditBase);
		//! \name Инициализация
		//! @{
	public:
		DirectoryWriteEdit(
				const wstring &caption,
				const GUIValue<value_t> &default_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t(),
				const on_value_apply_t &on_value_apply = on_value_apply_t());
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual GuiControl CreateGuiControlImpl(GuiControlBindings *bindings) override;
		//! @}
};

//--------------------------------------------------------------

//! \brief Поле ввода имени файла для чтения, управляющее значением по указателю.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted.
class ValueFileLoadEdit: public FileLoadEdit
{
		PARENT(FileLoadEdit);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueFileLoadEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				const wstring &file_type,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_t *p_value;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени файла для записи, управляющее значением по указателю.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted.
class ValueFileSaveEdit: public FileSaveEdit
{
		PARENT(FileSaveEdit);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueFileSaveEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				const wstring &file_type,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_t *p_value;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени директории для чтения, управляющее значением по указателю.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted.
class ValueDirectoryReadEdit: public DirectoryReadEdit
{
		PARENT(DirectoryReadEdit);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueDirectoryReadEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_t *p_value;
};

//--------------------------------------------------------------

//! \brief Поле ввода имени директории для записи, управляющее значением по указателю.
//! Обновление значения производится только по завершении диалога с результатом
//! DialogResultCode::Accepted.
class ValueDirectoryWriteEdit: public DirectoryWriteEdit
{
		PARENT(DirectoryWriteEdit);
		//! \name Инициализация
		//! @{
	public:
		using parent::on_value_changed_t;

		ValueDirectoryWriteEdit(const wstring &caption,
				const GUIValue<value_t*> &p_value,
				Layout control_layout = Layout::Vertical,
				const on_value_changed_t &on_value_changed = on_value_changed_t());

		//! \brief Взять новое значение из контролируемой переменной
		void Update();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI (интерфейс Control)
		//! @{
	public:
		virtual void LoadValues() override;
		virtual void ApplyDialogResult(DialogResultCode code) override;
		//! @}

	public:
		value_t *p_value;
};

//--------------------------------------------------------------

class Dialog
{
		//! \name Инициализация
		//! @{
	public:
		Dialog(const wstring &caption,
				const default_value_t &default_value_policy = saved_default_value);
		Dialog(const Dialog &) = delete;
		Dialog &operator=(const Dialog &) = delete;
		virtual ~Dialog() = default;
		void AddControl(shared_ptr<Control> control);

		//! \brief Создать контрол и добавить его в диалог.
		//! Аргументы передаются в конструктор контрола
		template <class T, class... Args>
		shared_ptr<T> CreateControl(Args&&... args)
		{
			auto control = make_shared<T>(std::forward<Args>(args)...);
			AddControl(control);
			return control;
		}
		//! @}

		//! \name Чтение неизменяемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Работа с результатом
		//! @{
	public:
		const Button *GetResult(DialogResultCode *o_code = nullptr) const
		{
			if (o_code)
				*o_code = result_code;
			return result_button;
		}
		DialogResultCode GetResultCode() const
		{
			return result_code;
		}
		void SetResult(const Button *button, DialogResultCode code)
		{
			result_button = button;
			result_code = code;
		}

		//! \brief Вызвать исключение canceled_operation, если была нажата отмена
		//! (DialogResultCode::Rejected)
		void ThrowIfCanceled() const;
		//! @}

		//! \name Показ диалога
		//! @{
	public:
		//! \brief Показать диалог (в модальном режиме)
		void Show();

		/*!
			\brief Загрузить и применить значения из истории так, как это происходило бы
			при показе диалога и нажатии OK, но без показа диалога

			Предполагается, что применить допускается любые значения из истории,
			минимальную проверку валидности и исправление они проходят при загрузке.

			В некоторых случаях это не так. Например, для имени файла (файл для чтения) при загрузке
			истории не происходит проверка существования файла.
		*/
		void ApplyHistory();
		//! @}

		//! \name Детали реализации: функции взаимодействия с UI.
		//!   Функции аналогичны функциям Control
		//! @{
	private:
		void Perform(const function<bool (const Control*)> &action) const;
		void Perform(const function<bool (Control*)> &action);
		void LoadValuesRoot();
		void LoadHistoryRoot();
		void LoadDialogHistory(const wstring &settings_path);
		void SaveHistoryRoot();
		void SaveDialogHistory();
		struct GuiDialog;
		GuiDialog CreateGuiControlsRoot();
		void BindGuiControlsRoot();
		void UpdateFromGuiControlRoot();
		void ReleaseGuiControlsRoot();
		void ApplyDialogResultRoot();
		//! @}

		//! \name Методы расширения диалога
		//! @{
	private:
		/*!
			\brief Функция для перегрузки в наследниках. Позволяет менять массив контролов,
			из которых создается диалог

			\param append_controls [in, out] Указатель на копию controls.
		*/
		virtual void AppendInternalControls(vector<shared_ptr<Control>> *append_controls);
		//! @}

	private:
		//! \brief Установить кнопку по умолчанию (до создания GUI)
		void SetDefaultButton(const Button *default_button);

		wstring EncodeEnumValue(const Button *value) const;
		const Button *DecodeEnumValue(const wstring &str, const Button *default_value) const;

	private:
		const wstring caption;
		const default_value_t default_value_policy;
		vector<shared_ptr<Control>> controls;
		const Button *result_button = nullptr;
		DialogResultCode result_code = DialogResultCode::Rejected;
		vector<shared_ptr<Control>> internal_controls;
		bool first_show = true;
	private:
		wstring history_settings_path;
		wstring history_caption;
		const Button *history_value = nullptr;
	private:
		struct PrivateStorage;
		shared_ptr<PrivateStorage> private_storage;
};

//--------------------------------------------------------------

//! \brief Класс-реализация диалога с нижним рядом кнопок, связанных со значениями произвольного
//! типа (не обязательно enum). Для создания объекта следует использовать статические методы
//! EnumDialog
template <class T>
class EnumDialogImpl: public Dialog
{
		//! \name Инициализация
		//! @{
	public:
		using value_t = T;
		using choice_t = Choice<T>;

		/*!
			\brief Создать диалог

			\param default_choice [in] Значение по умолчанию. Определяет политику работы с историей,
				а также кнопку по умолчанию,
				если флаг is_default у choices и у кнопок, добавляемых методом AddEnumButton(), не задан.
				См. также \ref Choice().
		*/
		EnumDialogImpl(const wstring &caption,
				const vector<choice_t> &choices,
				const GUIValue<value_t> &default_choice,
				const value_t &cancel_choice = value_t());

		/*!
			\brief Добавить кнопку, связанную со значением value, в диалог

			Если container == NULL, кнопка добавляется в глобальный контейнер диалога.
			Если container != NULL, кнопка добавляется в container, диалог связывает эту кнопку
			со значением value. Добавление container на диалог является ответственностью пользователя.
		*/
		void AddEnumButton(shared_ptr<Button> button, const value_t &value,
				ControlContainer *container = nullptr);

		//! \brief См. AddEnumButton от ControlContainer*
		void AddEnumButton(shared_ptr<Button> button, const value_t &value,
				const shared_ptr<ControlContainer> &container);

		// С созданием функции CreateEnumButton есть определенные сложности.
		// Список Args&&... args должен быть последним в списке аргументов функции.
		// В то же время функция должна иметь 3 перегруженных варианта с дополнительными
		// аргументами: {const T &value}, {const T &value, ControlContainer *container},
		// {const T &value, const shared_ptr<ControlContainer> &container}.
		// Эти аргументы должны идти перед Args&&... args.
		// Такие перегруженные варианты дают неверный подбор варианта в месте вызова:
		// вариант с одним дополнительным аргументом обычно выбирается вместо других двух,
		// даже если передается container.

		//! @}

		//! \name Чтение результата
		//! @{
	public:
		/*!
			\brief Возвращает выбранный вариант.
			Если нажата незарегистрированная кнопка с DialogResultCode::Accepted,
			возвращает default_choice.value, если default_choice.value_valid == true,
			иначе возвращает value_t().
		*/
		value_t Choice() const;
		//! @}

		//! \name Методы расширения диалога
		//! @{
	private:
		virtual void AppendInternalControls(vector<shared_ptr<Control>> *append_controls) override;
		//! @}

	private:
		const GUIValue<value_t> default_choice;
		const value_t cancel_choice;
		vector<pair<shared_ptr<Button>, const value_t>> buttons;
		vector<pair<shared_ptr<Button>, const value_t>> control_buttons;
	private:
		bool default_choice_found = false;
};

//--------------------------------------------------------------

/*!
	\brief Класс для создания диалога с нижним рядом кнопок, связанных со значениями произвольного
	типа (не обязательно enum).
	Для создания объектов использовать статические функции Create

	- При нажатии кнопки из списка заданных в параметре choice или добаленной в диалог методом
		AddEnumButton() возвращается связанное с кнопкой значение.
		dialog_result_code устанавливается:
		- DialogResultCode::Rejected, если кнопка is_cancel или
			(id кнопки равен cancel_choice и кнопка не является кнопкой по умолчанию);
		- DialogResultCode::Accepted в противном случае.

	- При закрытии диалога "крестиком" возвращается cancel_choice.

	- Если в диалог добавлены дополнительные кнопки (не методом AddEnumButton),
		при нажатии такой кнопки возвращается:
		- если кнопка устанавливает код DialogResultCode::Accepted: default_choice.value, если
			default_choice.value_valid == true, или value_t() иначе;
		- если кнопка устанавливает код DialogResultCode::Rejected: cancel_choice.

	\note
	Функции Create можно было бы сделать членами шаблонного класса EnumDialogImpl,
	но тогда для вызова этих функций потребовалось бы указывать параметр шаблона T, что неудобно.
*/
class EnumDialog
{
	public:
		//! \brief См. EnumDialog
		template <class T>
		static shared_ptr<EnumDialogImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				const GUIValue<T> &default_choice = saved_default_value,
				const T &cancel_choice = T())
		{
			return make_shared<EnumDialogImpl<T>>(caption,
					choices,
					default_choice,
					cancel_choice);
		}

		//! \brief См. EnumDialog
		template <class T>
		static shared_ptr<EnumDialogImpl<T>> Create(const wstring &caption,
				const vector<Choice<T>> &choices,
				const T &default_choice,
				const T &cancel_choice = T())
		{
			return make_shared<EnumDialogImpl<T>>(caption,
					choices,
					default_choice,
					cancel_choice);
		}

		//! \brief См. EnumDialog
		template <class T>
		static shared_ptr<EnumDialogImpl<T>> Create(const wstring &caption,
				const std::initializer_list<Choice<T>> &choices,
				const GUIValue<T> &default_choice = saved_default_value,
				const T &cancel_choice = T())
		{
			return make_shared<EnumDialogImpl<T>>(caption,
					choices,
					default_choice,
					cancel_choice);
		}

		//! \brief См. EnumDialog
		template <class T>
		static shared_ptr<EnumDialogImpl<T>> Create(const wstring &caption,
				const std::initializer_list<Choice<T>> &choices,
				const T &default_choice,
				const T &cancel_choice = T())
		{
			return make_shared<EnumDialogImpl<T>>(caption,
					choices,
					default_choice,
					cancel_choice);
		}
};

/*!
	\brief Класс для создания диалога с кнопками OK и Cancel.
	Для создания объектов использовать статическую функцию Create

	- При нажатии OK возвращается Result::OK,
		dialog_result_code устанавливается в DialogResultCode::Accepted.

	- При нажатии Cancel возвращается Result::Cancel,
		dialog_result_code устанавливается в DialogResultCode::Rejected.

	- При закрытии диалога "крестиком" возвращается Result::Cancel,
		dialog_result_code устанавливается в DialogResultCode::Rejected.

	- Если в диалог добавлены дополнительные кнопки, при нажатии такой кнопки возвращается:
		- Result::UserDefined, если кнопка устанавливает код DialogResultCode::Accepted,
		- Result::Cancel, если кнопка устанавливает код DialogResultCode::Rejected.

	По умолчанию создается диалог с enum = OKCancelDialog::Result. Можно использовать другой
	"совместимый" enum, для чего его нужно задать явно в качестве шаблонного параметра
	Create&lt;T&gt;().
*/
class OKCancelDialog
{
	public:
		enum class Result { UserDefined = 0 /* default value */, Cancel, OK };

		//! \brief Тип возвращаемого диалога
		template <class R = Result>
		using dialog_t = EnumDialogImpl<R>;

		/*!
			\brief Создать диалог с кнопками OK, Cancel.
			Параметр шаблона R задает enum диалога, по умолчанию R = Result.
			Он должен обязательно содержать значения с именами из enum Result
		*/
		template <class R = Result>
		static shared_ptr<EnumDialogImpl<R>> Create(const wstring &caption,
				const GUIValue<R> &default_choice = saved_default_value)
		{
			return EnumDialog::Create(caption,
					{
						MakeButton(L"OK", R::OK).SetDefault(),
						MakeButton(L"Cancel", R::Cancel).SetCancel()
					},
					default_choice.value_valid? default_choice:
							MakeGUIValue(R::UserDefined, default_choice.is_stored),
					R::Cancel);
		}
};

//--------------------------------------------------------------

} // namespace DynamicDialog

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

#include "DynamicDialog.hh"

//--------------------------------------------------------------
#endif // __DynamicDialog_h

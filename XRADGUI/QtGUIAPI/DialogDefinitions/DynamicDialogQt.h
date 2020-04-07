// file DynamicDialogQt.h
//--------------------------------------------------------------
#ifndef __DynamicDialogQt_h
#define __DynamicDialogQt_h
//--------------------------------------------------------------

#include "StayOnTopDialog.h"
#include "ThreadSync.h"
#include <atomic>
#include <XRAD/GUICore.h>
#include <GUIValue.h>

// Устранение конфликта имен с WinAPI
#ifdef CreateDialog
#undef CreateDialog
#endif

//--------------------------------------------------------------

namespace XRAD_GUI
{

XRAD_USING;

//--------------------------------------------------------------

//! \brief Перенаправляет обработку KeyPress
class KeyPressEventFilter: public QObject
{
		Q_OBJECT
	public:
		using handler_t = function<bool (QObject *obj, QKeyEvent *event)>;
		KeyPressEventFilter(const handler_t &handler, QObject *parent):
				QObject(parent), handler(handler)
		{
		}
	protected:
		bool eventFilter(QObject *obj, QEvent *event) override
		{
			if (event->type() == QEvent::KeyPress)
				return handler(obj, static_cast<QKeyEvent*>(event));
			return false;
		}
	private:
		handler_t handler;
};

//--------------------------------------------------------------

class DynamicDialogResult
{
	public:
		DynamicDialogResult() = default;
		DynamicDialogResult(function<void ()> &&action):
				action(std::move(action)) {}
		DynamicDialogResult(const function<void ()> &action):
				action(action) {}
		DynamicDialogResult(const function<void ()> &action, shared_ptr<void> lock):
				action(action), lock(std::move(lock)) {}
	public:
		function<void ()> action;
		shared_ptr<void> lock;
};

//--------------------------------------------------------------

enum class DynamicDialogLayout { Vertical, Horizontal };

using DialogResultCode = QDialog::DialogCode;

//--------------------------------------------------------------

class ControlApi;
class ButtonApi;

//--------------------------------------------------------------

//! \brief Интерфейс диалога для UI потока: методы вызываются в UI потоке
class DynamicDialogInterface
{
	public:
		//! \brief Способ блокировки диалогового окна при выполнении асинхронных команд
		enum class ActionPolicy
		{
			//! \brief Диалоговое окно блокируется
			Block,
			//! \brief Диалоговое окно скрывается
			Hide,
			//! \brief Никаких блокировок не применяется
			None
		};

		//! \brief Проверить корректность заполнения полей диалога. В случае ошибки
		//! выдать сообщение
		virtual bool TryAccept(const vector<ControlApi*> &value_dependencies) = 0;

		//! \brief Указатель на диалог для использования его в качестве родителя стандартных диалогов
		virtual QWidget *GetDialog() = 0;

		//! \brief Выдать интерактивный MessageBox с вопросом, с кнопками "Да" и "Нет"
		virtual bool AskYesNo(const wstring &question) = 0;

		//! \brief Закрыть диалог, установить control в качестве результата
		//!
		//! Если value_dependencies == NULL, проверяются все поля диалога.
		virtual void EndDialog(ButtonApi *control, DialogResultCode code,
				const vector<ControlApi*> *value_dependencies = nullptr) = 0;

		//! \brief Передать в рабочий поток callback для выполнения (постановка в очередь)
		virtual void ProcessCallback(const function<void ()> &callback,
				ActionPolicy action_policy = ActionPolicy::None) = 0;
};

//--------------------------------------------------------------

//! \brief Интерфейс диалога для рабочего потока: методы вызываются в рабочем потоке
class DialogApiInterface
{
	public:
		//! \brief Выполнить callback в UI потоке (текущий поток блокируется)
		virtual void ProcessUICallback(function<void ()> callback) = 0;
};

//--------------------------------------------------------------

class ControlApi
{
		//! \name Функции, вызываемые в Worker потоке
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) = 0;
		//! @}

		//! \name Функции, вызываемые в UI потоке
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) = 0;
		virtual void ReleaseDialog() = 0;

		/*!
			\brief Проверить корректность данных для возможности закрытия диалога
			с DialogResultCode::Accepted

			- Если данные корректные, возвращает true, err_message не модифицирует.
			- Если данные некорректные, возвращает false.
				Если нужно выдать сообщение об ошибке, в err_message записывается сообщение об ошибке.
				Если сообщение об ошибке уже выдано
				(через интерфейс DynamicDialogInterface), то err_message не модифицируется (пустое).
				Также может установить фокус на контрол с некорректными данными.
		*/
		virtual bool TryAccept(QString *err_message) = 0;

		//! \brief Сохранить данные при закрытии диалога с DialogResultCode::Accepted
		virtual void Accept() = 0;

		virtual void Enable(bool enable) = 0;
		//! @}
};

//--------------------------------------------------------------

class ContainerApi: public ControlApi, public std::enable_shared_from_this<ContainerApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		using Layout = DynamicDialogLayout;

		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		ContainerApi(PrivateTag, bool use_frame, const wstring &caption, Layout control_layout);
		ContainerApi(const ContainerApi &) = delete;
		ContainerApi &operator=(const ContainerApi &) = delete;
		//! \brief Функция для создания объекта
		static shared_ptr<ContainerApi> Create(bool use_frame, const wstring &caption,
				Layout control_layout);
		void AddControl(shared_ptr<ControlApi> control);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

		//! \name Немодифицируемые данные
		//! @{
	private:
		const bool use_frame;
		const wstring caption;
		const Layout control_layout;
		vector<shared_ptr<ControlApi>> controls;
		//! @}
};

//--------------------------------------------------------------

class SeparatorApi: public ControlApi, public std::enable_shared_from_this<SeparatorApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		SeparatorApi(PrivateTag);
		SeparatorApi(const SeparatorApi &) = delete;
		SeparatorApi &operator=(const SeparatorApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<SeparatorApi> Create();
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QFrame> ui_frame;
		//! @}
};

//--------------------------------------------------------------

class StretchApi: public ControlApi, public std::enable_shared_from_this<StretchApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		StretchApi(PrivateTag);
		StretchApi(const StretchApi &) = delete;
		StretchApi &operator=(const StretchApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<StretchApi> Create();
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		bool ui_created = false;
		//! @}
};

//--------------------------------------------------------------

class TextLabelApi: public ControlApi, public std::enable_shared_from_this<TextLabelApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		TextLabelApi(PrivateTag, const wstring &text);
		TextLabelApi(const TextLabelApi &) = delete;
		TextLabelApi &operator=(const TextLabelApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<TextLabelApi> Create(const wstring &caption);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Изменить состояние UI из рабочего потока
		void SetText(const wstring &t);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		wstring text;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		//! @}
};

//--------------------------------------------------------------

class ButtonApi: public ControlApi, public std::enable_shared_from_this<ButtonApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		using ActionPolicy = DynamicDialogInterface::ActionPolicy;

		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		ButtonApi(PrivateTag, const wstring &caption, DialogResultCode result_code, bool default_button,
				const function<void ()> &action,
				ActionPolicy action_policy);
		ButtonApi(const ButtonApi &) = delete;
		ButtonApi &operator=(const ButtonApi &) = delete;
		//! \brief Функция для создания объекта
		static shared_ptr<ButtonApi> Create(const wstring &caption, DialogResultCode result_code,
				bool default_button,
				const function<void ()> &action,
				ActionPolicy action_policy);

		void SetValueDependencies(vector<ControlApi*> dependencies);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

	private:
		//! \brief Вызывается при нажатии кнопки, в UI потоке
		void DoClicked();

		//! \name Немодифицируемые данные
		//! @{
	private:
		const wstring caption;
		const DialogResultCode result_code;
		const bool default_button;
		const function<void ()> action;
		const ActionPolicy action_policy;
		bool use_value_dependencies = false;
		vector<ControlApi*> value_dependencies;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QPushButton> ui_button;
		DynamicDialogInterface *ui_ddi = nullptr;
		//! @}
};

//--------------------------------------------------------------

class CheckBoxApi: public ControlApi, public std::enable_shared_from_this<CheckBoxApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		CheckBoxApi(PrivateTag, const wstring &caption,
				bool value,
				GUIValue<bool> default_value,
				bool history_value,
				const shared_ptr<ControlApi> &sub_control,
				function<void ()> on_value_changed);
		CheckBoxApi(const CheckBoxApi &) = delete;
		CheckBoxApi &operator=(const CheckBoxApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<CheckBoxApi> Create(const wstring &caption,
				bool value,
				GUIValue<bool> default_value,
				bool history_value,
				const shared_ptr<ControlApi> &sub_control,
				function<void ()> on_value_changed);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		bool UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(bool v);
	private:
		//! \brief Изменить состояние из UI потока
		void UISetValue(bool v);

		//! \brief Установить новое значение в UI-контроле из UI потока
		void UIChangeEditValue(bool v);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

		//! \name Немодифицируемые данные
		//! @{
	private:
		const wstring caption;
		const GUIValue<bool> default_value;
		const bool history_value;
		const shared_ptr<ControlApi> sub_control;
		const function<void ()> on_value_changed;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		std::atomic<bool> ui_value;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QCheckBox> ui_checkbox;
		DynamicDialogInterface *ui_ddi = nullptr;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

class RadioButtonApi: public std::enable_shared_from_this<RadioButtonApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		RadioButtonApi(PrivateTag, const wstring &caption, bool checked,
				const shared_ptr<ControlApi> &sub_control);
		RadioButtonApi(const RadioButtonApi &) = delete;
		RadioButtonApi &operator=(const RadioButtonApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<RadioButtonApi> Create(const wstring &caption, bool checked,
				const shared_ptr<ControlApi> &sub_control);
		//! @}

		//! \name Чтение немодифицируемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Интерфейс для RadioButtonContainerApi
		//! @{
	public:
		QRadioButton *GetRadioButton();
		void Check();
		void Uncheck();
		//! @}

		//! \name Функции, вызываемые в Worker потоке (аналог интерфейса ControlApi)
		//! @{
	public:
		void SetDialogApiInterface(DialogApiInterface *dialog_api);
		//! @}

		//! \name Функции, вызываемые в UI потоке (аналог интерфейса ControlApi)
		//! @{
	public:
		void AddToGroup(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				QButtonGroup *ui_group, const wstring &settings_path);

		void ReleaseDialog();
		bool TryAccept(QString *err_message);
		void Accept();
		void Enable(bool enable);
		//! @}

	private:
		void UICheckedChanged(bool c);

		//! \name Немодифицируемые данные
		//! @{
	private:
		const wstring caption;
		const shared_ptr<ControlApi> sub_control;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		bool ui_checked;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QRadioButton> ui_radiobutton;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

class RadioButtonContainerApi: public ControlApi,
		public std::enable_shared_from_this<RadioButtonContainerApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		using Layout = DynamicDialogLayout;

		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		RadioButtonContainerApi(PrivateTag, bool use_frame, const wstring &caption,
				size_t value,
				const GUIValue<size_t> &default_value,
				size_t history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		RadioButtonContainerApi(const RadioButtonContainerApi &) = delete;
		RadioButtonContainerApi &operator=(const RadioButtonContainerApi &) = delete;
		//! \brief Функция для создания объекта
		static shared_ptr<RadioButtonContainerApi> Create(bool use_frame, const wstring &caption,
				size_t value,
				const GUIValue<size_t> &default_value,
				size_t history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		void AddControl(shared_ptr<RadioButtonApi> control);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		size_t UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(size_t v);
	private:
		//! \brief Изменить состояние из UI потока
		void UISetValue(size_t v);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(size_t v);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

	private:
		void UISetGroupIndex(size_t index);
		size_t UIButtonToIndex(QAbstractButton *button);
		wstring FormatValueStd(size_t value);

		//! \name Немодифицируемые данные
		//! @{
	private:
		const bool use_frame;
		const wstring caption;
		const GUIValue<size_t> default_value;
		const size_t history_value;
		const Layout control_layout;
		const function<void ()> on_value_changed;
		vector<shared_ptr<RadioButtonApi>> controls;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		std::atomic<size_t> ui_value;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QButtonGroup> ui_group;
		//! \brief GroupBox в случае наличия рамки. При отсутствии рамки NULL.
		QPointer<QGroupBox> ui_group_box;
		DynamicDialogInterface *ui_ddi = nullptr;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

class ComboBoxApi: public ControlApi, public std::enable_shared_from_this<ComboBoxApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		ComboBoxApi(PrivateTag, bool use_caption, const wstring &caption,
				const vector<wstring> &items,
				size_t value,
				const GUIValue<size_t> &default_value,
				size_t history_value,
				function<void ()> on_value_changed);
		ComboBoxApi(const ComboBoxApi &) = delete;
		ComboBoxApi &operator=(const ComboBoxApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<ComboBoxApi> Create(bool use_caption, const wstring &caption,
				const vector<wstring> &items,
				size_t value,
				const GUIValue<size_t> &default_value,
				size_t history_value,
				function<void ()> on_value_changed);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		size_t UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(size_t v);
	private:
		//! \brief Изменить состояние из UI потока
		void UISetValue(size_t v);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(size_t v);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

	private:
		void UISetComboIndex(size_t index);
		wstring FormatValueStd(size_t value);

		//! \name Немодифицируемые данные
		//! @{
	private:
		const bool use_caption;
		const wstring caption;
		const vector<wstring> items;
		const GUIValue<size_t> default_value;
		const size_t history_value;
		const function<void ()> on_value_changed;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		std::atomic<size_t> ui_value;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		QPointer<QComboBox> ui_combobox;
		DynamicDialogInterface *ui_ddi = nullptr;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

class StringEditBaseApi: public ControlApi
{
		//! \name Инициализация
		//! @{
	public:
		using Layout = DynamicDialogLayout;

	protected:
		//! \brief Закрытый конструктор
		StringEditBaseApi(const wstring &caption,
				const wstring &value,
				const GUIValue<wstring> &default_value,
				const wstring &history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		StringEditBaseApi(const StringEditBaseApi &) = delete;
		StringEditBaseApi &operator=(const StringEditBaseApi &) = delete;
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		wstring UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(const wstring &v);
	protected:
		//! \brief Изменить состояние из UI потока
		void UISetValue(const wstring &v);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(const wstring &v);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		//! @}

	protected:
		virtual shared_ptr<StringEditBaseApi> SharedFromThisBase() = 0;
		virtual void SetEditText(const wstring &v) = 0;

	protected:
		void InstallUIEditFilter(QObject *ui_edit);
		void RemoveUIEditFilter(QObject *ui_edit);
	private:
		//! \brief Обработать нажатие клавиши в ui_edit
		bool ProcessUIEditKeyPress(QKeyEvent *event);

		//! \name Немодифицируемые данные
		//! @{
	protected:
		const bool use_caption;
		const wstring caption;
		const GUIValue<wstring> default_value;
		const wstring history_value;
		const Layout control_layout;
		const function<void ()> on_value_changed;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	protected:
		//! \brief Мьютекс для организации совместного доступа к ui_value
		//! (используется в тех частях кода, при исполнении которых возможно обращение
		//! к ui_value из разных потоков)
		mutable std::mutex ui_value_mutex;

		//! \brief Текущее значение, изменяется только из UI, доступ защищается ui_value_mutex
		//! (в тех частях кода, при исполнении которых возможно обращение из разных потоков)
		wstring ui_value;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	protected:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	protected:
		QPointer<KeyPressEventFilter> ui_edit_filter;
		DynamicDialogInterface *ui_ddi = nullptr;
		//! @}
};

//--------------------------------------------------------------

class StringEditApi: public StringEditBaseApi, public std::enable_shared_from_this<StringEditApi>
{
		PARENT(StringEditBaseApi);
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		StringEditApi(PrivateTag, const wstring &caption,
				const wstring &value,
				const GUIValue<wstring> &default_value,
				const wstring &history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! \brief Функция для создания объекта
		static shared_ptr<StringEditApi> Create(const wstring &caption,
				const wstring &value,
				const GUIValue<wstring> &default_value,
				const wstring &history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual void Enable(bool enable) override;
		//! @}

	protected:
		virtual shared_ptr<StringEditBaseApi> SharedFromThisBase() override;
		virtual void SetEditText(const wstring &v) override;

	private:
		static wstring FormatValueStd(const wstring &v, bool compact = false);
		static QString FormatValue(const wstring &v, bool compact = false);

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		QPointer<QLineEdit> ui_edit;
		QPointer<QPushButton> ui_default_button;
		//! @}
};

//--------------------------------------------------------------

class TextEditApi: public StringEditBaseApi, public std::enable_shared_from_this<TextEditApi>
{
		PARENT(StringEditBaseApi);
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		TextEditApi(PrivateTag, const wstring &caption,
				const wstring &value,
				const GUIValue<wstring> &default_value,
				const wstring &history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! \brief Функция для создания объекта
		static shared_ptr<TextEditApi> Create(const wstring &caption,
				const wstring &value,
				const GUIValue<wstring> &default_value,
				const wstring &history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual void Enable(bool enable) override;
		//! @}

	protected:
		virtual shared_ptr<StringEditBaseApi> SharedFromThisBase() override;
		virtual void SetEditText(const wstring &v) override;

	private:
		void DoTextEditTextChanged();
		void SetWordWrap(bool v);
		void SetFixedWidthFont(bool v);
		static wstring FormatValueStd(const wstring &v, bool compact = false);
		static QString FormatValue(const wstring &v, bool compact = false);

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		QPointer<QTextEdit> ui_edit;
		bool changing_text = false;
		//! @}
};

//--------------------------------------------------------------

/*!
	\brief Поле ввода числа. Доступны реализации только для ограниченного количества типов T
*/
template <class T>
class NumberEditApi: public ControlApi, public std::enable_shared_from_this<NumberEditApi<T>>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		using Layout = DynamicDialogLayout;
		using value_t = T;

		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		NumberEditApi(PrivateTag, const wstring &caption,
				value_t value,
				const GUIValue<value_t> &default_value,
				value_t history_value,
				value_t min_value,
				value_t max_value,
				out_of_range_control allow_out_of_range,
				Layout control_layout,
				function<void ()> on_value_changed);
		NumberEditApi(const NumberEditApi &) = delete;
		NumberEditApi &operator=(const NumberEditApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<NumberEditApi> Create(const wstring &caption,
				value_t value,
				const GUIValue<value_t> &default_value,
				value_t history_value,
				value_t min_value,
				value_t max_value,
				out_of_range_control allow_out_of_range,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		value_t UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(value_t v, value_t min_value_, value_t max_value_);
	private:
		//! \brief Изменить состояние из UI потока
		//!
		//! \return false, если значение v вне диапазона.
		bool UISetValue(value_t v);

		void UISetValueValid(bool valid);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(value_t v);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(value_t v, value_t min_value_, value_t max_value_);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

	private:
		//! \brief Визуально показать признак валидности текста
		void VisualizeValueValid(bool valid);

		//! \brief Обработать нажатие клавиши в ui_edit
		bool ProcessUIEditKeyPress(QKeyEvent *event);

		//! \brief Вернуть валидное значение (в допустимом диапазоне)
		//!
		//! \param err [out] Устанавливает true в случае ошибки (значение вне диапазона),
		//! иначе не изменяет.
		value_t ValidateValue(value_t v, bool *err = nullptr);

		static wstring FormatValueStd(value_t v, bool compact = false);
		static QString FormatValue(value_t v, bool compact = false);
		static bool ParseValue(value_t *o_value, QString str);

		//! \name Немодифицируемые данные
		//! @{
	private:
		const bool use_caption;
		const wstring caption;
		const GUIValue<value_t> default_value;
		const value_t history_value;
		const out_of_range_control allow_out_of_range;
		const Layout control_layout;
		const function<void ()> on_value_changed;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		value_t ui_min_value;
		value_t ui_max_value;

		//! \brief Мьютекс для организации совместного доступа к ui_value
		//! (используется в тех частях кода, при исполнении которых возможно обращение
		//! к ui_value из разных потоков)
		mutable std::mutex ui_value_mutex;
		//! \brief Текущее значение, изменяется только из UI, доступ защищается ui_value_mutex
		//! (в тех частях кода, при исполнении которых возможно обращение из разных потоков)
		value_t ui_value;
		//! \brief Текст в редакторе является допустимым числом (и соответствует ui_value),
		//! доступ только из UI
		bool ui_value_valid;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		QPointer<QLineEdit> ui_edit;
		QPointer<KeyPressEventFilter> ui_edit_filter;
		QPointer<QPushButton> ui_default_button;
		QPointer<QPushButton> ui_min_button;
		QPointer<QPushButton> ui_max_button;
		QPalette ui_edit_default_palette;
		DynamicDialogInterface *ui_ddi = nullptr;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

/*!
	\brief Поле ввода имени файла или директории
*/
class FileNameEditApi: public ControlApi, public std::enable_shared_from_this<FileNameEditApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		using Layout = DynamicDialogLayout;
		using value_t = wstring;
		enum class ValueValidity { Unchecked, Invalid, Valid };

		class FileNameValidator
		{
			public:
				using ValueValidity = ValueValidity;

				/*!
					\param current_filename Имя файла в generic формате.
					\return Имя файла в произвольном формате.
				*/
				virtual wstring Browse(const wstring &caption, const wstring &current_filename,
						QWidget *parent) = 0;
				//! \param filename Имя файла в generic формате.
				virtual ValueValidity Validate(const wstring &filename) = 0;
				//! \param filename Имя файла в generic формате.
				virtual ValueValidity ValidateUI(const wstring &filename, wstring *err_message,
						DynamicDialogInterface *ddi) = 0;
				virtual ~FileNameValidator() = default;
		};

		static unique_ptr<FileNameValidator> CreateFileLoadValidator(const wstring &file_type);
		static unique_ptr<FileNameValidator> CreateFileSaveValidator(const wstring &file_type);
		static unique_ptr<FileNameValidator> CreateDirectoryReadValidator();
		static unique_ptr<FileNameValidator> CreateDirectoryWriteValidator();

		//! \brief Закрытый конструктор. Для создания объекта использовать функцию Create()
		FileNameEditApi(PrivateTag,
				unique_ptr<FileNameValidator> &&validator,
				const wstring &caption,
				const value_t &value,
				const GUIValue<value_t> &default_value,
				value_t history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		FileNameEditApi(const FileNameEditApi &) = delete;
		FileNameEditApi &operator=(const FileNameEditApi &) = delete;
	public:
		//! \brief Функция для создания объекта
		static shared_ptr<FileNameEditApi> Create(
				unique_ptr<FileNameValidator> &&validator,
				const wstring &caption,
				const value_t &value,
				const GUIValue<value_t> &default_value,
				value_t history_value,
				Layout control_layout,
				function<void ()> on_value_changed);
		//! @}

		//! \name Чтение/запись модифицируемых данных
		//! @{
	public:
		//! \brief Получить текущее состояние UI
		pair<value_t, ValueValidity> UIValue() const;
		//! \brief Изменить состояние UI из рабочего потока
		void SetValue(value_t v);
	private:
		//! \brief Изменить состояние из обработчика изменений в ui_edit, из UI потока
		void DoEditTextEdited();

		void UISetValueValidity(ValueValidity valididy);

		//! \brief Установить новое значение в редакторе из UI потока
		void UIChangeEditValue(value_t v, ValueValidity valididy = ValueValidity::Unchecked);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void SetDialogApiInterface(DialogApiInterface *dialog_api) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс ControlApi)
		//! @{
	public:
		virtual void AddToDialog(QDialog *dialog, QBoxLayout *layout,
				DynamicDialogInterface *ddi,
				const wstring &settings_path) override;
		virtual void ReleaseDialog() override;
		virtual bool TryAccept(QString *err_message) override;
		virtual void Accept() override;
		virtual void Enable(bool enable) override;
		//! @}

	private:
		//! \brief Визуально показать признак валидности текста
		void VisualizeValueValidity(ValueValidity validity);

		//! \brief Обработать нажатие клавиши в ui_edit
		bool ProcessUIEditKeyPress(QKeyEvent *event);

		void Browse();

		static wstring FormatValueStd(value_t v, bool compact = false);
		static QString FormatValue(value_t v, bool compact = false);
		static value_t ParseValue(const wstring &str);
		static value_t ParseValue(const QString &str);

		//! \name Немодифицируемые данные
		//! @{
	private:
		class FileLoadValidator: public FileNameValidator
		{
			public:
				FileLoadValidator(const wstring &file_type): file_type(file_type) {}

				virtual wstring Browse(const wstring &caption, const wstring &current_filename,
						QWidget *parent) override;
				virtual ValueValidity Validate(const wstring &filename) override;
				virtual ValueValidity ValidateUI(const wstring &filename, wstring *err_message,
						DynamicDialogInterface *ddi) override;
			private:
				ValueValidity DoValidate(const wstring &filename, wstring *err_message);
			private:
				const wstring file_type;
		};

		class FileSaveValidator: public FileNameValidator
		{
			public:
				FileSaveValidator(const wstring &file_type): file_type(file_type) {}

				virtual wstring Browse(const wstring &caption, const wstring &current_filename,
						QWidget *parent) override;
				virtual ValueValidity Validate(const wstring &filename) override;
				virtual ValueValidity ValidateUI(const wstring &filename, wstring *err_message,
						DynamicDialogInterface *ddi) override;
			private:
				enum class ExtendedReason { None, EmptyFileName, DirectoryNotFound, FileExists,
						ObjectIsDirectory };
				ValueValidity DoValidate(const wstring &filename,
						ExtendedReason *ex_reason,
						wstring *path = nullptr,
						wstring *filename_without_path = nullptr);
			private:
				const wstring file_type;
		};

		class DirectoryReadValidator: public FileNameValidator
		{
			public:
				virtual wstring Browse(const wstring &caption, const wstring &current_filename,
						QWidget *parent) override;
				virtual ValueValidity Validate(const wstring &filename) override;
				virtual ValueValidity ValidateUI(const wstring &filename, wstring *err_message,
						DynamicDialogInterface *ddi) override;
			private:
				ValueValidity DoValidate(const wstring &filename, wstring *err_message);
		};

		class DirectoryWriteValidator: public FileNameValidator
		{
			public:
				virtual wstring Browse(const wstring &caption, const wstring &current_filename,
						QWidget *parent) override;
				virtual ValueValidity Validate(const wstring &filename) override;
				virtual ValueValidity ValidateUI(const wstring &filename, wstring *err_message,
						DynamicDialogInterface *ddi) override;
			private:
				enum class ExtendedReason { None, EmptyFileName, DirectoryNotFound, ObjectIsFile };
				ValueValidity DoValidate(const wstring &filename, ExtendedReason *ex_reason);
		};

		const unique_ptr<FileNameValidator> validator;

		const bool use_caption;
		const wstring caption;
		const GUIValue<value_t> default_value;
		const value_t history_value;
		const Layout control_layout;
		const function<void ()> on_value_changed;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		//! \brief Мьютекс для организации совместного доступа к ui_value
		//! (используется в тех частях кода, при исполнении которых возможно обращение
		//! к ui_value из разных потоков)
		mutable std::mutex ui_value_mutex;
		//! \brief Текущее значение, изменяется только из UI, доступ защищается ui_value_mutex
		//! (в тех частях кода, при исполнении которых возможно обращение из разных потоков)
		value_t ui_value;

		//! \brief Состояние проверки текста,
		//! доступ защищается ui_value_mutex
		ValueValidity ui_value_validity;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		DialogApiInterface *dialog_api_interface = nullptr;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<QLabel> ui_label;
		QPointer<QLineEdit> ui_edit;
		QPointer<KeyPressEventFilter> ui_edit_filter;
		QPointer<QPushButton> ui_browse_button;
		QPalette ui_edit_default_palette;
		DynamicDialogInterface *ui_ddi = nullptr;
		bool ui_enabled = true;
		//! @}
};

//--------------------------------------------------------------

class DynamicDialog: public StayOnTopDialog
{
		Q_OBJECT
	public:
		DynamicDialog(const wstring &caption,
				const weak_ptr<DynamicDialogInterface> &dialog_api,
				ThreadQueueWriter<DynamicDialogResult> &&thread_channel);

		void CancelDialog();
		ThreadQueueWriter<DynamicDialogResult> &GetThreadChannel();

	private:
		void do_finished();

	private:
		//! \brief Указатель на базовый DialogApi, может быть удален из рабочего потока
		weak_ptr<DynamicDialogInterface> w_dialog_api;

		/*!
			\brief Объект для разблокировки рабочего потока

			Он хранится в DynamicDialog, а не в DialogApi, чтобы при удалении диалога автоматически
			разблокировался рабочий поток.
		*/
		ThreadQueueWriter<DynamicDialogResult> thread_channel;
};

//--------------------------------------------------------------

class DialogApi: public DialogApiInterface, protected DynamicDialogInterface,
		public std::enable_shared_from_this<DialogApi>
{
	private:
		//! \brief Фиктивная структура для ограничения доступа к закрытому конструктору
		struct PrivateTag {};

		//! \name Инициализация
		//! @{
	public:
		//! \brief Закрытый конструктор, для создания объекта использовать функцию Create()
		DialogApi(PrivateTag, const wstring &caption);
		DialogApi(const DialogApi &) = delete;
		DialogApi &operator=(const DialogApi &) = delete;
	public:
		static shared_ptr<DialogApi> Create(const wstring &caption);
	public:
		//! \brief Добавить дочерний контрол. Не должна вызываться после создания диалога в UI
		void AddControl(shared_ptr<ControlApi> control);
		//! @}

		//! \name Чтение немодифицируемых данных
		//! @{
	public:
		wstring Caption() const { return caption; }
		//! @}

		//! \name Чтение модифицируемых данных
		//! @{
	public:
		//! \brief Чтение результата: указатель на нажатую кнопку или NULL при отмене диалога
		ButtonApi *GetResult() { return result_button; }
		//! @}

		//! \name Функции, вызываемые в Worker потоке
		//! @{
	public:
		void Show();
		void SetDialogApiInterface(bool apply);
		//! @}

		//! \name Функции, вызываемые в UI потоке
		//! @{
	private:
		DynamicDialog *CreateDialog(ThreadQueueWriter<DynamicDialogResult> &&thread_channel);
		void AddToDialog();
		void ReleaseDialog();
	public:
		void DeleteDialog();
		bool TryAccept(QString *err_message, const vector<ControlApi*> *value_dependencies);
		void Accept();
		void SetResult(ButtonApi *control);
		//! @}

		//! \name Функции, вызываемые в Worker потоке (интерфейс DialogApiInterface)
		//! @{
	public:
		virtual void ProcessUICallback(function<void ()> callback) override;
		//! @}

		//! \name Функции, вызываемые в UI потоке (интерфейс DynamicDialogInterface)
		//! @{
	public:
		virtual bool TryAccept(const vector<ControlApi*> &value_dependencies) override;
		virtual QWidget *GetDialog() override;
		virtual bool AskYesNo(const wstring &question) override;
		virtual void EndDialog(ButtonApi *control, DialogResultCode code,
				const vector<ControlApi*> *value_dependencies = nullptr) override;
		virtual void ProcessCallback(const function<void ()> &callback,
				ActionPolicy action_policy) override;
		//! @}

		//! \name Блокировка диалога
		//! @{
	private:
		shared_ptr<void> LockDialog();
	public:
		class DialogLocker;
		friend class DialogLocker;
	private:
		void LockDialogSync();
		void UnlockDialogAsync();
		//! @}

		//! \name Временное скрытие диалога
		//! @{
	private:
		shared_ptr<void> HideDialog();
	public:
		class DialogHider;
		friend class DialogHider;
	private:
		void HideDialogSync();
		void UnhideDialogAsync();
		//! @}

		//! \name Немодифицируемые данные
		//! @{
	private:
		const wstring caption;
		vector<shared_ptr<ControlApi>> controls;
		//! @}

		//! \name Модифицируемые данные
		//! @{
	private:
		/*!
			\details
			Доступ к переменной из разных потоков разграничен объектами синхронизации.
			См. вызовы GetResult(), SetResult(), DeleteDialog(): SetResult() устанавливает переменную
			только до вызова DeleteDialog(), GetResult() вызывается только после DeleteDialog().
		*/
		ButtonApi *result_button = nullptr;
		//! @}

		//! \name Данные для Worker потока
		//! @{
	private:
		bool thread_api = false;
		//! @}

		//! \name Данные для UI потока
		//! @{
	private:
		QPointer<DynamicDialog> dialog;
		size_t dialog_lock_count = 0;
		size_t dialog_hide_count = 0;
		//! @}
};

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // __DynamicDialogQt_h

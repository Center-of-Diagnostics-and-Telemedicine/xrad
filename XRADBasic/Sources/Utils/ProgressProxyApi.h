// file ProgressProxyApi.h
//--------------------------------------------------------------
#ifndef XRAD__File_ProgressProxyApi_h
#define XRAD__File_ProgressProxyApi_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/FlowControl.h>
#include <atomic>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Базовый класс-обработчик для \ref ProgressProxyApi
class ProgressApi
{
	public:
		virtual void Start(const wstring &message, double count) = 0;
		virtual void End() = 0;
		virtual bool Started() const = 0;
		virtual void SetPosition(double position) = 0;
		virtual void Update() = 0;
		virtual void ReportOverflow() = 0;
};

//--------------------------------------------------------------

//! \brief Класс для реализации индикаторов прогресса с помощью ProgressApi
class ProgressProxyApi: public ProgressProxyCore
{
	public:
		//! \brief Создать прогресс верхнего уровня
		ProgressProxyApi(shared_ptr<ProgressApi> api_lock);
		//! \brief Создать подпрогресс, используется внутри класса
		ProgressProxyApi(double start_pos, double end_pos, double api_pos_norm,
				shared_ptr<ProgressApi> api_lock);
		virtual ~ProgressProxyApi();
	public:
		//! \brief Экспериментальная возможность: печать времени выполнения каждого прогресса,
		//! по умолчанию выключена
		static void EnableLog(bool enable);
	protected:
		virtual void start(const wstring &prompt, double count) override;
		virtual void set_position(double pos) override;
		virtual performance_time_t end() override;
		virtual void update() override;
		virtual shared_ptr<ProgressProxyCore> subprogress(double start_pos, double end_pos) override;
	private:
		enum class Mode { TopLevel, Subprogress };
		const Mode mode;
		//! \brief Позиция начала в прогрессе верхнего уровня, от 0 до 1
		const double start_position;
		//! \brief Интервал изменения позиции в прогрессе верхнего уровня, от 0 до 1 - start_position
		const double interval;
		//! \brief Флаг блокировки отображения прогресса при попытке создать второе окно прогресса
		bool void_progress = false;
		bool started = false;
		//! \brief Нормировочный коэффициент позиции для внешних вызовов
		double ext_position_normalizer = 1;
		//! \brief Нормировочный коэффициент для api_SetProgressPosition
		double api_position_normalizer = 1;
		performance_time_t start_time;
	private:
		shared_ptr<ProgressApi> api_lock;
		ProgressApi *api;
		wstring m_prompt;
	private:
		static std::atomic<bool> log_enabled;
};

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_ProgressProxyApi_h

/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FlowControl.h
//--------------------------------------------------------------
#ifndef XRAD__File_FlowControl_h
#define XRAD__File_FlowControl_h
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"
#include <string>
#include <memory>
#include <vector>
#include "PerformanceCounter.h"

XRAD_BEGIN

/*!
	\brief Интерфейс контроля и управления выполнением вычислительных задач

	Позволяет прерывать выполнение задачи и получать состояние задачи.
	Для прерывания следует использовать исключения:
	- \ref canceled_operation — прерывает выполнение задачи, но не завершает
		интерактивное приложение (для этого данное исключение должно ловиться
		во внешнем "интерактивном" цикле);
	- \ref quit_application — прерывает выполнение задачи и всего приложения.
*/
class ProgressProxyCore
{
	protected:
		ProgressProxyCore() = default;
		virtual ~ProgressProxyCore() = default;
	public:
		virtual void start(const wstring &prompt, double count) = 0;
		virtual void set_position(double pos) = 0;
		//! \brief Exceptions: noexcept
		virtual performance_time_t end() = 0;
		virtual void update() = 0;

		virtual shared_ptr<ProgressProxyCore> subprogress(double start_pos, double end_pos) = 0;
};

typedef shared_ptr<ProgressProxyCore>	ProgressProxy;

//! \brief Вспомогательный класс для создания пустой реализации ProgressProxyCore
//!
//! См. \ref VoidProgressProxy().
class VoidProgressProxyImplementation: public ProgressProxyCore
{
	public:
		virtual void start(const wstring &/*prompt*/, double /*count*/) override
		{
			start_time = GetPerformanceCounterStd();
		}
		virtual void set_position(double /*pos*/) override {}
		virtual performance_time_t end() override
		{
			return GetPerformanceCounterStd() - start_time;
		}
		virtual void update() override {}
		virtual shared_ptr<ProgressProxyCore> subprogress(double /*start_pos*/, double /*end_pos*/)
				override
		{
			return make_shared<VoidProgressProxyImplementation>();
		}
	private:
		performance_time_t start_time = GetPerformanceCounterStd();
};

//! \brief Получить пустую (ничего не делающую) реализацию ProgressProxyCore
inline ProgressProxy VoidProgressProxy()
{
	return make_shared<VoidProgressProxyImplementation>();
}



/*!
	\brief Класс для управления индикатором прогресса

	\par Создание простого индикатора прогресса

	\code
	RandomProgressBar pb(GUIProgressProxy()); // Можно использовать GUIRandomProgressBar.
	pb.start(L"Обработка", 1);
	for (int i = 0; i < 20; ++i)
	{
		ProcessData(i);
		pb.set_position(double(i+1)/20);
	}
	pb.end();
	\endcode

	Для запуска прогресса вызывается метод start(). Максимальная позиция, т.е. значение позиции,
	при которой индикатор доходит до конца, задается параметром count.

	Замечание. На данный момент максимальная позиция должна быть целым числом.

	Для закрытия прогресса служит метод end().
	Вызов метода end() происходит также автоматически в деструкторе объекта RandomProgressBar.

	Метод set_position() служит для перемещения индикатора прогресса.
	Значение позиции, равное 0, соответствует начальному положению индикатора, count — конечному.



	\par Создание вложенных прогрессов

	\code
	void MainProc()
	{
		// Внешний прогресс с произвольными шагами
		GUIRandomProgressBar pb;
		pb.start(L"Обработка", 1);
		for (int i = 0; i < 20; ++i)
		{
			ProcessData1(i, pb.subprogress(double(i)/20, double(i+0.3)/20));
			ProcessData2(i, pb.subprogress(double(i+0.3)/20, double(i+1)/20));
			pb.set_position(double(i+1)/20); // здесь вызов set_position() необязателен,
				// если точно известно, что в ProcessData индикатор прогресса доводится до конца
		}
		pb.end();
	}
	void ProcessData1(int i, ProgressProxy pp)
	{
		// Вложенный прогресс может быть как с равномерными, так и с произвольными шагами.
		// В этом примере он с равномерными шагами.
		ProgressBar pb(pp);
		pb.start(L"Обработка элемента", 100);
		for (int j = 0; j < 100; ++j)
		{
			printf("%i: %i\n", i, j);
			++pb;
		}
		pb.end();
	}
	void ProcessData2(int i, ProgressProxy pp)
	{
		// ...
	}
	\endcode

	В примере предполагается, что времена работы функций ProcessData1 и ProcessData2
	относятся как 3:7.



	--- 🐻🐻🐻 ---

	\note
	Для одного RandomProgressBar (точнее: для одного ProgressProxy) вызов метода start должен
	производиться не более одного раза. Единственное исключение текущей реализации — прогресс
	верхнего уровня. Для него повторный вызов start() после end() допусти́м (возможно, это будет
	запрещено в будущем).

	\note
	Предполагается, что в будущем каждый объект, возвращаемый GUIProgressProxy(),
	будет создавать новое окно прогресса.

	\sa
	- ProgressBar.
*/
class RandomProgressBar
{
	public:
		//! \brief Инициализировать индикатор прогресса.
		//! Визуальных действий не происходит до вызова start()
		RandomProgressBar(ProgressProxy in_proxy);

		//! \brief Деструктор завершает прогресс, если он не завершен
		~RandomProgressBar();

		//! \brief Создать индикатор прогресса, см. start() от wstring
		void	start(const string &prompt, double max_position = 1);

		/*!
			\brief Создать индикатор прогресса

			Если это прогресс верхнего уровня, создается окно индикатора прогресса.
			Если это вложненный прогресс, визуальный эффект зависит от реализации.
			Например, в окне индикатора может создаваться дополнительная полоска индикатора
			с подписью prompt.

			Повторный вызов start() не допускается.
		*/
		void	start(const wstring &prompt, double max_position = 1);

		//! \brief Установить положение индикатора прогресса в заданную позицию (от 0 до count)
		void set_position(double pos);

		/*!
			\brief Завершить прогресс. Возвращает время с момента вызова start()

			Если это прогресс верхнего уровня, окно индикатора прогресса закрывается.
			Если это вложенный прогресс, завершается только он.

			\todo Сделать учет времени на уровне базового proxy.
		*/
		performance_time_t end();

		/*!
			\brief Инициализировать подпрогресс. Никаких визуальных действий
			не происходит до вызова у него start()

			Подпрогресс при своем проходе от начала до конца двигает позицию текущего
			индикатора прогресса от start_pos до end_pos (они должны быть в пределах от 0 до count).

			В данный момент эта операция дает осмысленный результат только между start() и end().
			Вне этого интервала возвращается VoidProgressProxy(). В будущем у вызова до start() может
			быть определена особая семантика.
		*/
		ProgressProxy subprogress(double start_pos, double end_pos);

		ProgressProxy subprogress(const pair<double,double> &positions)
		{
			return subprogress(positions.first, positions.second);
		}

	private:
		ProgressProxy proxy;
		bool	started;
};

/*!
	\brief Класс для прогресса с равномерными шагами

	\par Создание простого индикатора прогресса

	\code
	ProgressBar pb(GUIProgressProxy()); // Можно использовать GUIProgressBar.
	pb.start(L"Обработка", 100);
	for (int i = 0; i < 100; ++i)
	{
		ProcessData(i);
		++pb;
	}
	pb.end();
	\endcode

	Для запуска прогресса служит метод start(). Количество шагов индикатора задается параметром count.

	Для закрытия прогресса служит метод end().
	Вызов метода end() происходит также автоматически в деструкторе объекта ProgressBar.

	Метод next() увеличивают положение индикатора прогресса на 1 шаг.
	Чтобы индикатор прогресса дошел до конца,
	этот метод должен быть вызван count раз (количество шагов, заданное при вызове start()).

	Для удобства в классе реализованы операторы ++. Они эквивалентны методу next().



	\par Создание вложенных прогрессов

	\code
	void MainProc()
	{
		// Внешний прогресс с равномерными шагами
		GUIProgressBar pb;
		pb.start(L"Обработка", 100);
		for (int i = 0; i < 100; ++i)
		{
			ProcessData1(i, pb.subprogress(i, i+0.3));
			ProcessData2(i, pb.subprogress(i+0.3, i+1));
			++pb; // вызов next() или ++ обязателен, даже если в ProcessData индикатор прогресса
					// доводится до конца
		}
		pb.end();
	}
	void ProcessData1(int i, ProgressProxy pp)
	{
		// Вложенный прогресс может быть как с равномерными, так и с произвольными шагами.
		// В этом примере он с равномерными шагами.
		ProgressBar pb(pp);
		pb.start(L"Обработка элемента", 100);
		for (int j = 0; j < 100; ++j)
		{
			printf("%i: %i\n", i, j);
			++pb;
		}
		pb.end();
	}
	void ProcessData2(int i, ProgressProxy pp)
	{
		// ...
	}
	\endcode

	В примере предполагается, что времена работы функций ProcessData1 и ProcessData2
	относятся как 3:7.



	\par Создание вложенных прогрессов: устаревший способ

	Этот способ использовался ранее. Сейчас он не поддерживается.

	\code
	void MainProc()
	{
		// Внешний прогресс (с равномерными шагами)
		GUIProgressBar pb; // Ранее использовался ProgressBar. Создает экземпляр GUIProgressProxy().
		pb.start(L"Обработка", 100, 2); // параметр 2: будет 2 вложенных прогресса
		for (int i = 0; i < 100; ++i)
		{
			ProcessData1(i); // здесь будет первый вложенный прогресс
			ProcessData2(i); // здесь будет второй вложенный прогресс
			++pb; // вызов ++ обязателен (раньше был необязателен в некоторых случаях)
		}
		pb.end();
	}
	void ProcessData1(int i)
	{
		// Вложенный прогресс
		GUIProgressBar pb; // Ранее использовался ProgressBar.
				// Создает новый экземпляр GUIProgressProxy().
		pb.start(L"Обработка элемента", 100);
		for (int j = 0; j < 100; ++j)
		{
			printf("%i: %i\n", i, j);
			++pb;
		}
		pb.end();
	}
	void ProcessData2(int i)
	{
		// Аналогично ProcessData1
	}
	\endcode

	--- 🐻🐻🐻 ---

	\note
	Для одного ProgressBar (точнее: для одного ProgressProxy) вызов метода start должен
	производиться не более одного раза. Единственное исключение текущей реализации — прогресс
	верхнего уровня. Для него повторный вызов start() после end() допусти́м (возможно, это будет
	запрещено в будущем).

	\note
	Предполагается, что в будущем каждый объект, возвращаемый GUIProgressProxy(),
	будет создавать новое окно прогресса.

	\note
	Раньше был доступен конструктор по умолчанию для ProgressBar, он вызывал GUIProgressProxy()
	аналогично современному GUIProgressBar.

	\sa
	- RandomProgressBar.
*/
class ProgressBar
{
	public:
		//! \brief Инициализировать индикатор прогресса.
		//! Визуальных действий не происходит до вызова start()
		ProgressBar(ProgressProxy in_proxy);

		//! \brief Деструктор завершает прогресс, если он не завершен
		~ProgressBar();

		//! \brief Создать индикатор прогресса, см. start() от wstring
		void	start(const string &prompt, size_t count);

		/*!
			\brief Создать индикатор прогресса

			Если это прогресс верхнего уровня, создается окно индикатора прогресса.
			Если это вложненный прогресс, визуальный эффект зависит от реализации.
			Например, в окне индикатора может создаваться дополнительная полоска индикатора
			с подписью prompt.

			Повторный вызов start() не допускается.
		*/
		void	start(const wstring &prompt, size_t count);

		/*!
			\brief Завершить прогресс. Возвращает время с момента вызова start()

			Если это прогресс верхнего уровня, окно индикатора прогресса закрывается.
			Если это вложенный прогресс, завершается только он.

			\todo Сделать учет времени на уровне базового proxy.
		*/
		performance_time_t end();

		//! \brief Сделать следующий шаг индикатора прогресса
		void next();

		//! \brief Сделать следующий шаг индикатора прогресса
		void	operator++() { next(); }
		//! \brief Сделать следующий шаг индикатора прогресса
		void	operator++(int) { next(); }

		/*!
			\brief Инициализировать подпрогресс. Никаких визуальных действий
			не происходит до вызова у него start()

			Подпрогресс при своем проходе от начала до конца двигает позицию текущего
			индикатора прогресса от start_pos до end_pos (они должны быть в пределах от 0 до count).

			В данный момент эта операция дает осмысленный результат только между start() и end().
			Вне этого интервала возвращается VoidProgressProxy(). В будущем у вызова до start() может
			быть определена особая семантика.
		*/
		ProgressProxy substep(double step_min = 0, double step_max = 1);
		ProgressProxy substep(pair<double,double> step_positions)
		{
			return substep(step_positions.first, step_positions.second);
		}

	private:
		ProgressProxy proxy;
		bool started = false;
		size_t counter = 0;
		size_t max_count = 0;
};

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FlowControl_h

//	file XRADGUIFunction.cpp
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#include "pre.h"
#include "GUIController.h"

#include "XRADGUIUtils.h"
#include "SavedSettings.h"
#include "XRADGUIAPI.h"
#include <XRADGUI/Sources/GUI/I18nSupport.h>
#include <XRADGUI/Sources/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>


void	ForceResourceLoad()
{
	Q_INIT_RESOURCE(XRADResources);
}

//--------------------------------------------------------------

namespace XRAD_GUI
{

shared_ptr<GUIController> global_gui_controller;

bool IsPointerAValidGUIWidget(const void *ptr)
{
	return global_gui_controller->WidgetExists(static_cast<const QWidget*>(ptr));
}

XRAD_USING






// QThread*	XRADGUIClass::interface_thread = NULL; //!< указатель на интерфейсный поток, пока не проинициализирован и не используется
//ConsoleWindow*	GUIController::main_window = NULL; //!< указатель на главное окно интерфейса
//QApplication*		GUIController::xrad_application = NULL; //!< указатель на объект QApplication
//ThreadGUI*	GUIController::dialogs_creator = NULL;//!< указатель на объект ThreadGUI


ProgressBarCounter *GUIController::progress_bar_counter = NULL;





bool GUIController::StayOnTopAllowed = true;
set<IGUISettingsChanged*> GUIController::GUISettingsChangedTargets;



//--------------------------------------------------------------
//
//	initialization
//



GUIController::GUIController(int *in_result_ptr, int in_argc, char *in_argv[]) :
	work_thread(NULL),
	main_window(NULL),
	xrad_application(NULL),
	dialogs_creator(NULL),

	result_ptr(NULL), argc(0), argv(NULL),
	GUI_update_interval(sec(0.1)),
	progress_update_interval(msec(1)),

	workthread_pause_scheduled(false),
	cancel_scheduled(false),
	quit_scheduled(false)


{
	argc = in_argc;
	argv = in_argv;
	result_ptr = in_result_ptr;

	// Устанавливаем язык по умолчанию сразу.
	SetDefaultLanguageId(GetStdLanguageIdRu());

	//TODO следующая строка в Qt5 не срабатывает
	//		QInternal::callFunction(QInternal::SetCurrentThreadToMainThread, NULL);

	xrad_application = new QApplication(argc, argv);
	ForceResourceLoad();
	//QResource(":/cursors/brightness_contrast.png");
	//QThread *curThread = QThread::currentThread();
	//xrad_application = new QApplication(curThread,args);

	// Применяем сохраненные настройки языка. Эта операция должна выполняться до обращения
	// к языковым функциям, но требует созданного QApplication.
	SetLanguageId(LoadLanguageId());

	StayOnTopAllowed = GUILoadParameter("GUI", "StayOnTopAllowed", (size_t)(StayOnTopAllowed? 1: 0)) != 0;

	InitLogFile();



	//		GUIController &gui_controller;

	work_thread = new ThreadUser(*this);
	dialogs_creator = new ThreadGUI(*this, NULL);// должен создаваться прежде main_window, тот от него зависит
	main_window = new ConsoleWindow(*this);
	dialogs_creator->SetMainWindow(main_window);
	progress_bar_counter = new ProgressBarCounter;

	InitDialogs();
}

GUIController::~GUIController(void)
{
	dialogs_creator->SetMainWindow(nullptr);
	DestroyObject(main_window);
	DestroyObject(dialogs_creator);
	DestroyObject(work_thread);
	DestroyObject(xrad_application);
	DestroyObject(progress_bar_counter);
}

void GUIController::xrad_main_call()
{
	// инициализируем результат main() на случай аварийного ее завершения
	if(result_ptr)
		*result_ptr = EXIT_FAILURE;

	ThreadSetup ts; (void)ts;
	try
	{
		int result = EXIT_FAILURE;
		try
		{
			result = xrad_main(argc, argv);//Главный вызов всей программы, комментировать
		}
		catch(...)
		{
			api_ShowMessage(convert_to_wstring(XRAD_GUI::GetGUIExceptionString()), api_msgError, L"Uncaught exception in xrad_main");
		}
		if(result_ptr)
			*result_ptr = result;
		printf("\nApplication finished. Press Ctrl-Q to exit\n");
		work_thread->ForceUpdateGUI(sec(0));
	}
	catch(...)
	{
	}
};



bool GUIController::IsProgressActive()
{
	return (GUIController::progress_bar_counter->_level() == 0) ? false : true;
}


bool GUIController::ActiveProgressBar()
{
	return dialogs_creator->ActiveProgressBar();
}

bool GUIController::GetStayOnTopAllowed()
{
	return StayOnTopAllowed;
}

void GUIController::SetStayOnTopAllowed(bool allow)
{
	if(StayOnTopAllowed == allow)
		return;
	StayOnTopAllowed = allow;
	GUISaveParameter("GUI", "StayOnTopAllowed", (size_t)(StayOnTopAllowed? 1: 0));
	NotifyGUISettingsChanged();
}



//--------------------------------------------------------------
//
//	runtime control
//


void GUIController::AddGUISettingsChangedTarget(IGUISettingsChanged *target)
{
	GUISettingsChangedTargets.insert(target);
}

void GUIController::RemoveGUISettingsChangedTarget(IGUISettingsChanged *target)
{
	GUISettingsChangedTargets.erase(target);
}

void GUIController::NotifyGUISettingsChanged()
{
	for (auto *target: GUISettingsChangedTargets)
	{
		target->GUISettingsChanged();
	}
}

//--------------------------------------------------------------







void GUIController::InitLogFile(void)
{
	// Об отладочных логах Qt см. http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
	// Устарело:
	//теперь Debug пишем в log-файл (используется модуль LogQDebug.h)
	//		qInstallMsgHandler(myMessageOutput);
	// перед использованием почистим log-файл
	//QFile fMessFile(qApp->applicationDirPath() + "/myProjectLog.log");
	//fMessFile.open(QIODevice::Append | QIODevice::Text | QIODevice::Truncate);
	//fMessFile.close();
}


int GUIController::Run()
{
	work_thread->start();
	if(GUIController::main_window)
	{
		GUIController::main_window->show();
	}
	if(GUIController::xrad_application)
	{
		return GUIController::xrad_application->exec();
	}
	else return 0;
}

void GUIController::FinishDialogs(void)
{
	for(size_t i = widgets.size(); i-- > 0;)
	{
		if(auto *widget = widgets[i])
		{
			widget->close();
		}
	}
	widgets.clear();
}




//--------------------------------------------------------------



// эти функции к удалению.
// из них, возможно, следует взять список управляемых параметров для классов графика и растра
// (вдруг что-то из нижеперечисленного не управляется как следует)



//вывод растрового изображения
#if 0

void GUIController::ShowRasterAnimation(
	const QString &qtitle, const void **data,
	int n_frames, int vs, int hs,
	double z0, double dz, const QString &qz_label,
	double y0, double dy, const QString &qy_label,
	double x0, double dx, const QString &qx_label,
	double v0, double vmax, double gamma, const QString &qvalue_label,
	display_sample_type pt)
{
	try
	{
		ImageWindow *image_ptr = emit GUIGlobals::work_thread->request_CreateRasterImageSet(qtitle, vs, hs);
		emit GUIGlobals::work_thread->request_SetImageAxesScales(image_ptr, z0, dz, y0, dy, x0, dx);
		emit GUIGlobals::work_thread->request_SetupImageLabels(image_ptr, qtitle, qz_label, qy_label, qx_label, qvalue_label);
		emit GUIGlobals::work_thread->request_SetupImageDefaultRanges(image_ptr, v0, vmax, gamma);

		for(int i = 0; i < n_frames; ++i)
		{
			emit GUIGlobals::work_thread->request_SetupImageFrame(image_ptr, -1, static_cast<const unsigned char*>(data[i]), pt);
		}
		emit GUIGlobals::work_thread->request_ShowDataWindow(static_cast<ImageWindow*>(image_ptr), true);

		GUIGlobals::work_thread->Suspend(ThreadUser::suspend_for_data_analyze);
	}
	catch(...)
	{
	}

}
#endif //0


//--------------------------------------------------------------
//void initDialogs(ThreadUser* work_thread)



// добавление объекта в массив диалогов
void GUIController::AddWidget(QWidget *widget)
{
	widgets.push_back(widget);
}

// удаление объекта из массива диалогов
void GUIController::RemoveWidget(QWidget *widget)
{
	auto found = std::find(widgets.begin(), widgets.end(), widget);
	if(found != widgets.end())
	{
		widgets.erase(found);
	}
}

// существует ли объект в массиве диалогов (его могли уже удалить, а мы и не знаем того)
bool GUIController::WidgetExists(const QWidget *widget)
{
	auto found = std::find(widgets.begin(), widgets.end(), widget);
	return found != widgets.end();
}


// Qt 5 style: Проверка совместимости типов аргументов функций происходит на этапе компиляции.
#define make_connection_bc(f1, f2) \
	QObject::connect( \
		work_thread, &ThreadUser::f1, \
		dialogs_creator, &ThreadGUI::f2, Qt::BlockingQueuedConnection)
#define make_connection_qc(f1, f2) \
	QObject::connect( \
		work_thread, &ThreadUser::f1, \
		dialogs_creator, &ThreadGUI::f2, Qt::QueuedConnection)
#define make_connection_dc(f1, f2) \
	QObject::connect( \
		work_thread, &ThreadUser::f1, \
		dialogs_creator, &ThreadGUI::f2, Qt::DirectConnection)

// Qt::DirectConnection	1	Когда посылается сигнал он немедленно доставляется слоту.
// Qt::QueuedConnection	2	Когда посылается сигнал он ставится в очередь до тех пор пока цикл обработки событий не доставит его слоту.
// Qt::BlockingQueuedConnection	4	Также как и QueuedConnection, за исключением того, что текущий поток блокируется до тех пор пока слот не будет доставлен.
//									Этот тип соединения должен быть использован только для приемников в различных потоках. Заметьте, что неправильное применение
//									этого типа может привести взаимным блокировкам в вашем приложении.
// Qt::AutoConnection	0	Если сигнал посылается из потока, в котором живет принимающий объект, слот вызывается непосредственно, как в случае
// 							с Qt::DirectConnection; иначе сигнал ставится в очередь, как в случае с Qt::QueuedConnection.

void GUIController::InitDialogs()
{
	//UTF-8?
	QTextCodec *codec = QTextCodec::codecForName("CP1251");
	//	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
#if QT_VERSION < 0x050000
	QTextCodec::setCodecForTr(codec);
#else
	QTextCodec::setCodecForLocale(codec);
#endif

	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);
	QString	currentPath = settings.value("GetFileName/currentPath", QCoreApplication::applicationDirPath()).toString();
	QDir::setCurrent(currentPath);

	qRegisterMetaType<file_dialog_mode>("file_dialog_mode");
	qRegisterMetaType<std::string>("string"); // для соединений Qt::QueuedConnection
	qRegisterMetaType<XRAD_GUI::GraphWindow *>("XRAD_GUI::GraphWindow *"); // для соединений Qt::QueuedConnection
	qRegisterMetaType<XRAD_GUI::ImageWindow *>("XRAD_GUI::ImageWindow *"); // для соединений Qt::QueuedConnection
	qRegisterMetaType<std::size_t>("size_t"); // для соединений Qt::QueuedConnection
	qRegisterMetaType<QDialog *>("QDialog *"); // для соединений Qt::QueuedConnection

	make_connection_bc(request_Pause, do_Pause);

	make_connection_bc(request_ShowMessage, do_ShowMessage);

	make_connection_bc(request_ShowString, do_ShowString);
	make_connection_bc(request_ShowIntegral, do_ShowIntegral);
	make_connection_bc(request_ShowFloating, do_ShowFloating);

	make_connection_bc(request_GetString, do_GetString);
	make_connection_bc(request_GetIntegral, do_GetIntegral);
	make_connection_bc(request_GetFloating, do_GetFloating);

	make_connection_bc(request_GetButtonDecision, do_GetButtonDecision);
	make_connection_bc(request_GetRadioButtonDecision, do_GetRadioButtonDecision);
	make_connection_bc(request_GetCheckboxDecision, do_GetCheckboxDecision);

	make_connection_bc(request_GetFileName, do_GetFileName);

	make_connection_bc(request_GetFolderName, do_GetFolderName);

	// прогресс: две первые функции QueuedConnection, чтобы не задерживали счет.
	// третья Blocking, чтобы дождаться результата (время счета).
	// нет! все надо Blocking, не иначе. проверяем 2015_02_12
	make_connection_bc(request_StartProgress, do_StartProgress);
	make_connection_bc(request_SetProgressPosition, do_SetProgressPosition);
	make_connection_bc(request_EndProgress, do_EndProgress);

	make_connection_bc(request_UpdateConsole, do_UpdateConsole);

	make_connection_bc(request_CloseDataWindow, do_CloseDataWindow);
	make_connection_bc(request_HideDataWindow, do_HideDataWindow);
	make_connection_bc(request_SetDataWindowTitle, do_SetDataWindowTitle);

	make_connection_bc(request_CreateGraph, do_CreateGraph);
	make_connection_bc(request_SetupGraphCurve, do_SetupGraphCurve);
	make_connection_bc(request_SetupGraphLabels, do_SetupGraphLabels);
	make_connection_bc(request_SetupGraphTitle, do_SetupGraphTitle);

	make_connection_bc(request_GetGraphScale, do_GetGraphScale);
	make_connection_bc(request_SetGraphScale, do_SetGraphScale);
	make_connection_bc(request_SetGraphStyle, do_SetGraphStyle);
	make_connection_bc(request_SaveGraphPicture, do_SaveGraphPicture);

	make_connection_bc(request_CreateRasterImageSet, do_CreateRasterImageSet);
	make_connection_bc(request_SetupImageFrame, do_SetupImageFrame);
	make_connection_bc(request_AddImageFrames, do_AddImageFrames);
	make_connection_bc(request_SetupImageLabels, do_SetupImageLabels);
	make_connection_bc(request_SetupImageDefaultRanges, do_SetupImageDefaultRanges);
	make_connection_bc(request_SetImageAxesScales, do_SetImageAxesScales);

	make_connection_bc(request_CreateTextDisplayer, do_CreateTextDisplayer);
	make_connection_bc(request_SetFixedWidth, do_SetFixedWidth);
	make_connection_bc(request_SetEditable, do_SetEditable);
	make_connection_bc(request_SetFontSize, do_SetFontSize);
	make_connection_bc(request_SetPersistent, do_SetPersistent);
	make_connection_bc(request_SetStayOnTop, do_SetStayOnTop);
	make_connection_bc(request_SetText, do_SetText);
	make_connection_bc(request_SetTitle, do_SetTitle);
	make_connection_bc(request_GetText, do_GetText);

	// должно быть BlockingQueuedConnection - из-за неправильного параметра QueuedConnection некорректно отображались графики
	// (иногда начинала рисовать еще до того, как нужный массив скопировался в буфер)

	make_connection_bc(request_ShowDataWindow, do_ShowDataWindow);
	//	make_connection_dc(request_ShowDataWindow, do_ShowDataWindow);
}

#undef make_connection_bc
#undef make_connection_qc
#undef make_connection_dc





}//namespace XRAD_GUI

#ifndef __pre_h
#define __pre_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

#include <vector>
#include <list>
#include <string>

//--------------------------------------------------------------

#ifdef XRAD_COMPILER_MSC
// Type name first seen using 'class' now seen using 'struct'
#pragma warning (disable: 4099)

#pragma warning (disable: 4251) // Class XXX needs to have dll-interface...

#pragma warning(push, 3)
// уровень 4 плохо сказывается на исходниках Qt,
#pragma warning(disable : 4481)

#endif // XRAD_COMPILER_MSC

//--------------------------------------------------------------

#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtCore/QPointer>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QMutex>
#include <QtCore/QDebug>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaType>
#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtCore/QProcess>
#include <QtCore/QWaitCondition>
#include <QtCore/QList>
#include <QtCore/QTimer>

#include <QtGui/QtGui>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

#if QT_VERSION < 0x050000
	#error QT_VERSION is too low (Qt5 required)
#endif

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLayoutItem>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushbutton>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QDesktopWidget>

#include <QtSvg/QSvgGenerator>

//--------------------------------------------------------------

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_color_map.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_shapeitem.h>

#include <qwt_matrix_raster_data.h>
#include <qwt_symbol.h>
#include <qwt_picker.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_legend.h>

// проблемный файл
//#include <qwt_plot_curve.h>

//--------------------------------------------------------------

#ifdef XRAD_COMPILER_MSC
#pragma warning(pop)
// Qt закончено, восстанавливаем высокую чувствительность
#endif // XRAD_COMPILER_MSC

//--------------------------------------------------------------
#endif //__pre_h

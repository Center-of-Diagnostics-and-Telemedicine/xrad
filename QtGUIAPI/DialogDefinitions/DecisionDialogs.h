//	file ButtonDecisionDialog.h
//	Created by IRD on 06.2013
//  Version 3.0.2
#ifndef DecisionDialogs
#define DecisionDialogs

//--------------------------------------------------------------

#include "StayOnTopDialog.h"

using namespace std;

//--------------------------------------------------------------

namespace XRAD_GUI
{
XRAD_USING



// динамическая форма с командными кнопками
class ButtonDecisionDialog : public StayOnTopDialog
{
		Q_OBJECT
	public:
		ButtonDecisionDialog(size_t &in_result, QString prompt, const vector<QString> &buttons,
							size_t default_button, size_t in_cancel_button);
	private:
		size_t &result;

		const size_t n_buttons;
		const size_t cancel_button;
		size_t n_rows;
		size_t n_columns;
		bool	cancel_button_assigned;
		bool	default_button_assigned;

		void	CalculateButtonsGrid();

		QList<QObject*> button_objects;
		QGridLayout *layout;
		bool eventFilter(QObject *target, QEvent *event);

	private slots:
		void ButtonClicked(void);
};



class DecisionDialog : public StayOnTopDialog
{
		Q_OBJECT
	protected:
		DecisionDialog();
		QVBoxLayout *dialog_layout;
		bool DecisionDialog::eventFilter(QObject *target, QEvent *event);

	private:

		QPushButton *button_ok;//temp
		QPushButton *button_cancel;

		QHBoxLayout *buttons_layout;

	private slots:
		virtual void OKClicked();
		virtual void CancelClicked();
		virtual void	UpdateResult() = 0;
};



// динамическая форма с флажками
class CheckboxDecisionDialog : public DecisionDialog
{
		Q_OBJECT

	public:
		CheckboxDecisionDialog(const QString &prompt, const vector<pair<QString, bool*> > &boxes);

	private:
		vector<bool *> values;
		vector<QCheckBox*> checkboxes;

	private slots:
		void UpdateResult() override;
};




// динамическая форма с радио-кнопками
class RadioButtonDecisionDialog : public DecisionDialog
//	class RadioButtonDecisionDialog : public QDialog
{
		Q_OBJECT

	public:
		RadioButtonDecisionDialog(size_t &in_result, const QString &prompt, const vector<QString> &buttons,
				size_t default_button);

		size_t previous_result;
		performance_time_t previous_click_time;
		const performance_time_t double_click_delay;

	private:
		const size_t	n_buttons;

		bool	cancel_button_assigned;
		bool	default_button_assigned;

		QGroupBox *group_box;
	//	QPushButton *button_ok;
		QVBoxLayout *group_box_layout;
	//	QVBoxLayout *dialog_layout;

		vector<QRadioButton*> radio_buttons;
		size_t &result;
		void showEvent(QShowEvent * event);
		bool eventFilter(QObject *target, QEvent *event);

	public slots:
		void	OKClicked() override;
		void	CheckForDoubleClick();
		void	UpdateResult() override;
};



}//namespace XRAD_GUI

#endif //DecisionDialogs

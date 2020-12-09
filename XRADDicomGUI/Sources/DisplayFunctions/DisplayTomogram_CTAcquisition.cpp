/*!
	\file
	\date 4/23/2018 5:46:49 PM
	\author kovbas
*/
#include "pre.h"
#include "DisplayTomogram_CTAcquisition.h"

XRAD_BEGIN

enum	display_ct_options_t
{
	display_ct_slices,
	display_ct_metadata,
	display_ct_DICOM_file_content,
	display_ct_current,
	display_ct_voltage,
	display_CTDI_vol,
	display_ct_thickness,
	display_ct_positions,
	exit_ct_display
};

const vector<wstring> &options_ct_names()
{
	static	vector<wstring>	options(exit_ct_display + 1);
	do_once
	{
		options[display_ct_slices] = L"Display CT Slices";
		options[display_ct_metadata] = L"Display CT Metadata";
		options[display_ct_DICOM_file_content] = L"Display DICOM-file content";
		options[display_ct_current] = L"Display CT current";
		options[display_ct_voltage] = L"Display CT voltage";
		options[display_CTDI_vol] = L"Display CTDI vol";
		options[display_ct_thickness] = L"Display CT slice thickness";
		options[display_ct_positions] = L"Display CT slice position";
		options[exit_ct_display] = L"Exit display";
	}
	return	options;
}

void	DisplayTomogram_CTAcquisition(const CTAcquisition &ct, const wstring &title)
{
	point3_F64 t;
	RealFunctionMD_F32 x;
	display_ct_options_t options{ display_ct_slices };
	auto title_full{ title + L" [" + ct.series_description() + L"]" };

	while (options != exit_ct_display)
	{
		try
		{
			options = static_cast<display_ct_options_t>(GetButtonDecision(title_full, options_ct_names()));

			switch (options)
			{
			case display_ct_slices:
			//	 x = ct.load_ordered_slices();
				DisplayMathFunction3D(ct.load_ordered_slices(), title_full);
			//	DisplayMathFunction3D(ct.slices(), title_full);
			//	t = ct.scales();
				break;

			case display_ct_metadata:
				ShowText(title_full, ct.summary_info());
				break;

			case display_ct_DICOM_file_content:
				ShowText(title_full, ct.get_dicom_file_content(GetUnsigned("Enter number of frame.", 0, 0, ct.n_elements() - 1)));
				break;

			case display_ct_current:
				DisplayMathFunction(ct.currents(), 0, 1, title_full + L" Current", L"mA", L"Slice No");
				break;

			case display_ct_voltage:
				DisplayMathFunction(ct.voltages(), 0, 1, title_full + L" Voltage", L"kV", L"Slice No");
				break;

			case display_CTDI_vol:
				DisplayMathFunction(ct.CTDIvols(), 0, 1, title_full + L" CTDIVol", L"mGy?", L"Slice No");
				break;

			case display_ct_thickness:
				DisplayMathFunction(ct.thickness(), 0, 1, title_full + L" Slice Thickness", L"mm", L"Slice No");
				break;
			case display_ct_positions:
			{
				auto image_positions_patient{ct.image_positions_patient()};
				RealFunctionF64	locations_z(image_positions_patient.size(), 0);
				static_assert(std::is_same<decltype(image_positions_patient)::value_type::value_type,
						decltype(locations_z)::value_type>::value, "Type mismatch.");
				locations_z.CopyData(image_positions_patient, [](double &x, const auto &p) {return x = p[0];});
				DisplayMathFunction(locations_z, 0, 1, title_full + L" Slice Position", L"mm", L"Slice No");
			}
			break;
			}
		}
		catch (canceled_operation &)
		{
			// нажатие кнопки cancel в любом диалоговом окне, включая прогресс.
			// (например, отмена ошибочного выбранной команды без необходимости
			// дожидаться ее завершения)
		}
		catch (quit_application &)
		{
			// команда принудительного выхода из программы
			throw;
		}
		catch (exception &ex)
		{
			// все остальное, включая нехватку памяти
			ShowString("An error occured", ex.what());
		}
	}
}

XRAD_END

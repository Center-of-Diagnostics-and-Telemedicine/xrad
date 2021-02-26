/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:46:58 PM
	\author kovbas
*/
#include "pre.h"
#include "DisplayTomogram_MRAcquisitionSiemens.h"

XRAD_BEGIN

enum	display_mr_siemens_options
{
	display_mr_slices,
	display_mr_metadata,
	display_mr_DICOM_file_content,
	display_mr_thickness,
	display_mr_siemens_diffusion_b_value_siemens,
	display_mr_positions,
	exit_mr_display
};

const vector<wstring> &options_mr_siemens_names()
{
	static	vector<wstring>	options(exit_mr_display + 1);
	do_once
	{
		options[display_mr_slices] = L"Display MR Slices";
		options[display_mr_metadata] = L"Display MR Metadata";
		options[display_mr_DICOM_file_content] = L"Display DICOM-file content";
		options[display_mr_thickness] = L"Display MR slice thickness";
		options[display_mr_siemens_diffusion_b_value_siemens] = L"Display MR diffusion_b_value_siemens";
		options[display_mr_positions] = L"Display MR slice position";
		options[exit_mr_display] = L"Exit display";
	}
	return	options;
}

void	DisplayTomogram_MRAcquisitionSiemens(const MRAcquisitionSiemens &mr, const wstring &title)
{
	size_t display_mr_options(0);
	wstring title_full = title + L" [" + mr.series_description() + L"]";

	while (display_mr_options != exit_mr_display)
	{
		try
		{
			display_mr_options = GetButtonDecision(title_full, options_mr_siemens_names());

			switch (display_mr_options)
			{
			case display_mr_slices:
				DisplayMathFunction3D(mr.load_ordered_slices(), title_full);
				break;

			case display_mr_metadata:
				ShowText(title_full, mr.summary_info());
				break;

			case display_mr_DICOM_file_content:
				ShowText(title_full, mr.get_dicom_file_content(GetUnsigned("Enter number of frame.", 0, 0, mr.n_elements() - 1)));
				break;


			case display_mr_thickness:
				DisplayMathFunction(mr.thickness(), 0, 1, title_full + L" Slice Thickness", L"mm", L"Slice No");
				break;

			case display_mr_siemens_diffusion_b_value_siemens:
				DisplayMathFunction(mr.diffusion_b_values_siemens(), 0, 1, title_full + L" diffusion_b_value_siemens", L"__", L"Slice No");
				break;

			case display_mr_positions:
			{
				auto image_positions_patient{ mr.image_positions_patient() };
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

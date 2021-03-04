/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:46:06 PM
	\author kovbas
*/
#include "pre.h"
#include "DisplayTomogram_GenericAcquisition.h"

XRAD_BEGIN

enum	display_tomogram_options
{
	display_tomogram_slices,
	display_tomogram_metadata,
	display_DICOM_file_content,
	display_tomogram_thickness,
	display_tomogram_positions,
	exit_tomogram_display
};

const vector<wstring> &options_generic_tomogram_names()
{
	static	vector<wstring>	options(exit_tomogram_display + 1);
	do_once
	{
		options[display_tomogram_slices] = L"Display Tomogram Slices";
	options[display_tomogram_metadata] = L"Display Tomogram Metadata";
	options[display_DICOM_file_content] = L"Display DICOM-file content";
	options[display_tomogram_thickness] = L"Display Tomogram slice thickness";
	options[display_tomogram_positions] = L"Display Tomogram slice position";
	options[exit_tomogram_display] = L"Exit display";
	}
	return	options;
}

void	DisplayTomogram_GenericAcquisition(const TomogramAcquisition &tm, const wstring &title)
{
	size_t display_tomogram_options(0);
	wstring title_full = title + L" [" + tm.series_description() + L"]";
	while (display_tomogram_options != exit_tomogram_display)
	{
		try
		{
			display_tomogram_options = GetButtonDecision(title_full, options_generic_tomogram_names());

			switch (display_tomogram_options)
			{
			case display_tomogram_slices:
				DisplayMathFunction3D(tm.load_ordered_slices(), title_full);
				break;

			case display_tomogram_metadata:
				ShowText(title_full, tm.summary_info());
				break;

			case display_DICOM_file_content:
				ShowText(title_full, tm.get_dicom_file_content(GetUnsigned("Enter number of frame.", 0, 0, tm.n_elements() - 1)));
				break;

			case display_tomogram_thickness:
				DisplayMathFunction(tm.thickness(), 0, 1, title_full + L" Slice Thikness", L"Slice No", L"mm");
				break;
			case display_tomogram_positions:
			{
				auto image_positions_patient{ tm.image_positions_patient() };
				RealFunctionF64	locations_z(image_positions_patient.size(), 0);
				static_assert(std::is_same<decltype(image_positions_patient)::value_type::value_type,
						decltype(locations_z)::value_type>::value, "Type mismatch.");
				locations_z.CopyData(image_positions_patient, [](double &x, const auto &p) {return x = p[0];});
				DisplayMathFunction(locations_z, 0, 1, title_full + L" Slice Position", L"Slice No", L"mm");
			}

			//					DisplayMathFunction(tm.locations, 0, 1, title_full + L" Slice Position", L"Slice No", L"mm");
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

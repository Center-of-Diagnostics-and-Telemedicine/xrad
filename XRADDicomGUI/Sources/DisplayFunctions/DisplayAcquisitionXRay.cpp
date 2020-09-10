/*!
	\file
	\date 4/23/2018 5:47:27 PM
	\author kovbas
*/
#include "pre.h"
#include "DisplayAcquisitionXRay.h"

XRAD_BEGIN

enum	display_rg_options
{
	display_rg,
	display_rg_metadata,
	display_rg_DICOM_file_content,
	//display_rg_current,
	//display_rg_voltage,
	//display_CTDI_vol,
	display_rg_thickness,
	display_rg_positions,
	exit_rg_display
};

const vector<wstring> &options_generic_rg_names()
{
	static	vector<wstring>	options(exit_rg_display + 1);
	do_once
	{
		options[display_rg] = L"Display Image";
	options[display_rg_metadata] = L"Display Image Metadata";
	options[display_rg_DICOM_file_content] = L"Display DICOM-file content";
	//options[display_rg_current] = L"Display Image current";
	//options[display_rg_voltage] = L"Display Image voltage";
	//options[display_CTDI_vol] = L"Display CTDI vol";
	options[display_rg_thickness] = L"Display Image slice thickness";
	options[display_rg_positions] = L"Display Image slice position";
	options[exit_rg_display] = L"Exit display";
	}
	return	options;
}

void	DisplayXRayGeneric(const XRayAcquisition &rg, const wstring &title)
{
	size_t display_rg_options(0);
	wstring title_full = title + L" [" + rg.series_description() + L"]";
	while (display_rg_options != exit_rg_display)
	{
		try
		{
			display_rg_options = GetButtonDecision(title_full, options_generic_rg_names());
			switch (display_rg_options)
			{
			case display_rg:
			{
				// создать вектор изображений
				auto rg_slices = rg.slices();
				if (rg_slices.size() == 0)
					ShowText(title_full, L"There is no images to show");
				else
				{
					size_t n = 0;
					if (rg_slices.size() > 1)
						n = GetUnsigned("Enter image number to viewing", 0, 0, rg_slices.size() - 1);
					DisplayMathFunction2D(rg_slices[n], title_full);
				}
			}
			break;

			case display_rg_metadata:
				ShowText(title_full, rg.summary_info());
				break;

			case display_rg_DICOM_file_content:
			{
				size_t n = 0;
				if(!rg.n_elements())
				{
					Error("XRay acquisition is empty");
				}
				else
				{
					if(rg.n_elements() > 1)
						n = GetUnsigned("Enter image number to viewing content DICOM file", 0, 0, rg.n_elements() - 1);
					ShowText(title_full, rg.get_dicom_file_content(n));
				}
			}
			break;

			/*?case display_rg_thickness:
				//todo Kovbas Это актуально для рентгенов?
				DisplayMathFunction(image.thickness(), 0, 1, title_full + L" Slice Thikness", L"Slice No", L"mm");
				break;*/
			case display_rg_positions:
			{
				//RealFunctionF32	locations_z(image.image_positions_patient.size(), 0);
				//locations_z.CopyData(image.image_positions_patient, [](float &x, const auto &p) {return x = p[0];});
				//DisplayMathFunction(locations_z, 0, 1, title_full + L" Slice Position", L"Slice No", L"mm");
				//					DisplayMathFunction(ct.positions, 0, 1, title_full + L" Slice Position", L"Slice No", L"mm");
			}

			//					DisplayMathFunction(image.locations, 0, 1, title_full + L" Slice Position", L"Slice No", L"mm");
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
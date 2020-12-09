/*!
	\file
	\date 4/23/2018 5:47:27 PM
	\author kovbas
*/
#include "pre.h"
#include "DisplayAcquisitionXRay.h"

XRAD_BEGIN
enum	display_img_options
{
	display_img,
	display_img_metadata,
	display_img_DICOM_file_content,
	exit_img_display
};

const vector<wstring> &options_generic_img_names()
{
	static	vector<wstring>	options(exit_img_display + 1);
	do_once
	{
		options[display_img] = L"Display Image";
	options[display_img_metadata] = L"Display Image Metadata";
	options[display_img_DICOM_file_content] = L"Display DICOM-file content";

	
	options[exit_img_display] = L"Exit display";
	}
	return	options;
}

void	DisplayGenericImageAcquisition(const GenericImageAcquisition &img, const wstring &title)
{
	size_t display_img_options(0);
	wstring title_full = title + L" [" + img.series_description() + L"]";
	while (display_img_options != exit_img_display)
	{
		try
		{
			display_img_options = GetButtonDecision(title_full, options_generic_img_names());
			switch (display_img_options)
			{
			case display_img:
			{
				// создать вектор изображений
				auto img_slices = img.slices();
				if (img_slices.size() == 0)
					ShowText(title_full, L"There is no images to show");
				else
				{
					size_t n = 0;
					if (img_slices.size() > 1)
						n = GetUnsigned("Enter image number to viewing", 0, 0, img_slices.size() - 1);
					DisplayMathFunction2D(img_slices[n], title_full);
				}
			}
			break;

			case display_img_metadata:
				ShowText(title_full, img.summary_info());
				break;

			case display_img_DICOM_file_content:
			{
				size_t n = 0;
				if (!img.n_elements())
				{
					Error("acquisition is empty");
				}
				else
				{
					if (img.n_elements() > 1)
						n = GetUnsigned("Enter image number to viewing content DICOM file", 0, 0, img.n_elements() - 1);
					ShowText(title_full, img.get_dicom_file_content(n));
				}
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
/*!
	\file
	\date 2/21/2018 3:39:35 PM
	\author kovbas
*/
#ifndef image_h__
#define image_h__

#include "LoadGenericClasses.h"
#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

namespace Dicom
{
	/*!

		\note:
		// minimal data for parsing pixeldata. You have to save these to new dicom-file
		//wstring transfer_syntax_uid;
		//wstring photometric_interpretation;
		//size_t vertical_size;//, rows;
		//size_t horizontal_size;//, columns;
		//size_t bits_allocated;//, bitsAllocated;
		//size_t precision;//, bitsStored;
		//size_t high_bit; //usually = bitStored - 1
		//size_t ncomponents;//, samplesPerPixel;
		//bool signedness;//, pixelRepresentation;
		// end of minimal data for parsing pixeldata
	*/
	class image : public instance
	{
		PARENT(instance);
		//-virtual void	init_cache(){ m_cache = make_unique<gray_image_cache>(); }
		//?gray_image_cache &icache(){return dynamic_cast<gray_image_cache &>(*m_cache);}
		//?const gray_image_cache &icache() const {return dynamic_cast<gray_image_cache &>(*m_cache);}
	public:

		//! \brief Доступ к изображению, как оно хранится в файле (без учета slope и intercept)
		//-RealFunction2D_F32 &internal_image(){ return dicom_container()->image(); }
		//-const RealFunction2D_F32 &internal_image() const { return dicom_container()->image(); }

		//! \brief Получение изображения с учетом поправок slope и intercept в заранее подготовленный буфер
		virtual void get_image(RealFunction2D_F32 &image_p) const
		{
			if(image_p.vsize() != vsize() || image_p.hsize() != hsize())
			{
				ForceDebugBreak();
				throw invalid_argument("Dicom::image::load_image, invalid buffer dimensions");
			}
			//	Подробнее о rescale_slope и rescale_intercept.
			//	https://blog.kitware.com/dicom-rescale-intercept-rescale-slope-and-itk/
			//	NB. Теоретически они могут быть разным для разных срезов одной томограммы
			//double	intercept = dicom_container()->get_double(e_rescale_intercept, m_frame_no, 0);
			//double	slope = dicom_container()->get_double(e_rescale_slope, m_frame_no, 1);

			//получаем изображение из файла
			size_t bpp = dicom_container()->get_uint(e_bits_allocated);
			bool signedness = dicom_container()->get_uint(e_pixel_representation) != 0;
			//size_t ncomponents = 0;
			size_t ncomponents = dicom_container()->get_uint(e_samples_per_pixel);
			dicom_container()->get_pixeldata(image_p, bpp, signedness, ncomponents, m_frame_no);

			// Важно: копирование в существующий массив без переаллокирования
			image_p.CopyData(image_p/*, [&intercept, &slope](auto &y, const auto &x){y = x*slope + intercept;}*/);
		}

		virtual void get_image(RealFunction2D_F32 &image_p, size_t m_frame_no) const
		{
			if (image_p.vsize() != vsize() || image_p.hsize() != hsize())
			{
				ForceDebugBreak();
				throw invalid_argument("Dicom::image::load_image, invalid buffer dimensions");
			}
;

			//получаем изображение из файла
			size_t bpp = dicom_container()->get_uint(e_bits_allocated);
			bool signedness = dicom_container()->get_uint(e_pixel_representation) != 0;
			//size_t ncomponents = 0;
			size_t ncomponents = dicom_container()->get_uint(e_samples_per_pixel);
			dicom_container()->get_pixeldata(image_p, bpp, signedness, ncomponents, m_frame_no);

			// Важно: копирование в существующий массив без переаллокирования
			image_p.CopyData(image_p/*, [&intercept, &slope](auto &y, const auto &x){y = x*slope + intercept;}*/);
		}

		//! \brief Получение изображения с учетом поправок slope и intercept во вновь создаваемый буфер
		virtual  RealFunction2D_F32 get_image() const
		{
			RealFunction2D_F32 result(vsize(), hsize());
			get_image(result);
			return result;

		}

		//! \brief Выгрузка в кэш изображения с обратной коррекцией slope и intercept.
		//Размер изображения может отличаться от исходного -  вот это плохая идея.... todo
		virtual void set_image(const RealFunction2D_F32 &image_p)
		{
#if 0
			double	intercept = dicom_container()->get_double(e_rescale_intercept, 0);
			double	slope = dicom_container()->get_double(e_rescale_slope, 1);

			set_vsize(image_in.vsize());
			set_hsize(image_in.hsize());

			internal_image().MakeCopy(image_in, [&intercept, &slope](auto &y, const auto &x){y = (x-intercept)/slope;});
#else
			double	intercept = dicom_container()->get_double(e_rescale_intercept, 0);
			double	slope = dicom_container()->get_double(e_rescale_slope, 1);

			set_vsize(image_p.vsize());
			set_hsize(image_p.hsize());

			//internal_image().MakeCopy(image_p, [&intercept, &slope](auto &y, const auto &x){y = (x-intercept)/slope;});
			RealFunction2D_F32 img_tmp;
			img_tmp.MakeCopy(image_p, [&intercept, &slope](auto &y, const auto &x) {y = (x - intercept) / slope;});
			dicom_container()->set_pixeldata(img_tmp, bits_allocated(), signedness(), ncomponents());

#endif
		}
		//! \brief Выгрузка в кэш изображений мультифрейма с обратной коррекцией slope и intercept.
		//Размер изображения может отличаться от исходного -  вот это плохая идея.... todo
		virtual void set_mf_images(const RealFunctionMD_F32 &image_p)
		{
			double	intercept = dicom_container()->get_double(e_rescale_intercept, 0);
			double	slope = dicom_container()->get_double(e_rescale_slope, 1);

			
			set_vsize(image_p.sizes()[1]);
			set_hsize(image_p.sizes()[2]);

			//internal_image().MakeCopy(image_p, [&intercept, &slope](auto &y, const auto &x){y = (x-intercept)/slope;});
			RealFunctionMD_F32 img_tmp;
			img_tmp.MakeCopy(image_p, [&intercept, &slope](auto &y, const auto &x) {y = (x - intercept) / slope; });
			dicom_container()->set_pixeldata_mf(img_tmp, bits_allocated(), signedness(), ncomponents());
		}

		//constructors
		//?image();

		//gets
		wstring photometric_interpretation() const { return dicom_container()->get_wstring(e_photometric_interpretation); }
		void	set_photometric_interpretation(const wstring &in_pm) { dicom_container()->set_wstring(e_photometric_interpretation, in_pm, m_frame_no); }


		size_t vsize() const { return dicom_container()->get_uint(e_rows); }
		size_t hsize() const { return dicom_container()->get_uint(e_columns); }
		double x_scale() const { return scales_xy()[0]; }
		double y_scale() const { return scales_xy()[1]; }
		std::vector<double> scales_xy() const { return dicom_container()->get_double_values(e_pixel_spacing); }
		
		size_t bits_allocated() const { return dicom_container()->get_uint(e_bits_allocated); }
		size_t bytes_per_pixel() const { return  (bits_allocated() / CHAR_BIT); }
		bool signedness() const { return dicom_container()->get_bool(e_signedness); }
		size_t ncomponents() const { return dicom_container()->get_uint(e_samples_per_pixel); }
		//bool get_image(RealFunction2D_F32 &dst_img) {}; // задел геттера. Пока не используется

		//sets
		void set_vsize(const size_t val) { dicom_container()->set_uint(e_rows, val); }
		void set_hsize(const size_t val) { dicom_container()->set_uint(e_columns, val); }
		void set_precision(const size_t val) { dicom_container()->set_uint(e_bits_stored, val); }
		void set_signedness(const bool val) { if (val) dicom_container()->set_uint(e_signedness, 1); else dicom_container()->set_uint(e_signedness, 0); }
		void set_ncomponents(const size_t val) { dicom_container()->set_uint(e_samples_per_pixel, val); }
		//void set_image(const RealFunction2D_F32 &image_in) { get_image().MakeCopy(image_in); } //потенциально не будет сохранять изображение, если изначально не было выделено под это место в текущей переменной

		//virtual void set_new_values_to_file() override;
		//virtual void set_new_values_to_instance(Container::error_process_mode epm) override;
		//void set_to_file_image_data();
		//void set_to_file_pixeldata();
		//void set_to_instance_image_data();
		//void set_to_instance_pixeldata(Container::error_process_mode epm);

		//methods
		//-virtual bool collect_image() override;
//		void delete_image();

// 		virtual void clear() override
// 		{
// 			delete_image();
// 			parent::clear();
// 		}

		virtual	instance *clone() const override
		{
			return new image(*this);
		}

	protected:
		//virtual	bool collect_everything_internal() override;

		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

			tagslist_to_check_data_p.insert(e_transfer_syntax_uid);
			tagslist_to_check_data_p.insert(e_image_type);
			tagslist_to_check_data_p.insert(e_photometric_interpretation);

			tagslist_to_check_data_p.insert(e_columns); //(0x0028, 0x0011)
			tagslist_to_check_data_p.insert(e_rows); //(0x0028, 0x0010)
			tagslist_to_check_data_p.insert(e_signedness); // (0028,0103) # Pixel Representation // мы всегда получаем из файла значение 1 или 0, поэтому для своих целей нужно переводить в bool
			tagslist_to_check_data_p.insert(e_samples_per_pixel);  // (0028,0002) # Samples per Pixel

			tagslist_to_check_data_p.insert(e_bits_stored); // (0028,0101) # Bits Stored
			tagslist_to_check_data_p.insert(e_bits_allocated); // (0028,0100) # Bits Allocated

			tagslist_to_check_data_p.insert(e_high_bit);


// 			tagslist_to_check_data_p.insert(e_rescale_intercept);
// 			tagslist_to_check_data_p.insert(e_rescale_slope);

// 			tagslist_to_check_data_p.insert(e_window_center);
// 			tagslist_to_check_data_p.insert(e_window_width);
		}


	private:
		//-bool collect_image_internal();
		//void anonymization_image() (set_wstring(e_image_comments, L"NPCMR processed"););


	};

} //namespace Dicom

XRAD_END

#endif // image_h__
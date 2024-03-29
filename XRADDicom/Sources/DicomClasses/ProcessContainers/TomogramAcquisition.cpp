﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
/*!
	\file
	\date 2016-09-29 11:43
	\author kns
*/
#include "pre.h"
#include "TomogramAcquisition.h"
#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>

#include <XRADBasic/Sources/Utils/ParallelProcessor.h>
//#include <XRADDicom/DicomClasses/DicomStorageAnalyze.h>

XRAD_BEGIN

//--------------------------------------------------------------

TomogramAcquisition::TomogramAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p)
	: ProcessAcquisition(acquisition_loader_p)
{
	//TODO здесь анализировать косинусы и определять направление осей
}

TomogramAcquisition &TomogramAcquisition::operator=(const TomogramAcquisition &original)
{
	return *this;
}

void TomogramAcquisition::put_elements_to_instance(Dicom::instance &instance, size_t n) const
{
	auto	&ts = dynamic_cast<Dicom::tomogram_slice &>(instance);

	// кладём общие данные, чтобы можно было в последствии распарсить файлы
	if (ts.stack_id() == L"")
		ts.set_instance_number(n + 1);
	else
		ts.set_in_stack_position_number(n + 1);

	//кладём координаты среза
	//?ts.set_image_position_patient({ image_positions_patient[n].x(), image_positions_patient[n].y(), image_positions_patient[n].z() });

	//кладём срез как есть
	//todo (Kovbas) надо это как-то реализовать
	//?ts.set_image(m_slices.slice({n, slice_mask(0), slice_mask(1)}));
}

point3_F64 TomogramAcquisition::scales() const
{
	point3_F64	result;

	//fabs -- важно, т.к. image_positions_patient[*].z() могут меняться как по возрастанию, так и по убыванию
	double	z0, z1;
	auto image_positions_patient{ self::image_positions_patient() };
	size_t sort_axis;
	self::sort_axis(sort_axis);
	switch(sort_axis)
	{
		case 0:
			z0 = image_positions_patient[0].x();
			z1 = image_positions_patient[image_positions_patient.size() - 1].x();
			break;

		case 1:
			z0 = image_positions_patient[0].y();
			z1 = image_positions_patient[image_positions_patient.size() - 1].y();
			break;

		case 2:
			z0 = image_positions_patient[0].z();
			z1 = image_positions_patient[image_positions_patient.size() - 1].z();
			break;

		default:
			throw invalid_argument(ssprintf("TomogramAcquisition::scales(), invalid sort axis = %zu", sort_axis));
	}
	if(z0==z1) result.z() = thickness()[0];
	else result.z() = fabs(z0 - z1)/ image_positions_patient.size();

	auto scales_xy{ self::scales_xy() };
	result.y() = fabs(scales_xy[0].y());
	result.x() = fabs(scales_xy[0].x());

	if (result.z() <= 0)
		throw invalid_argument(classname() + "::scales(), invalid z scale");
	if (result.y() <= 0)
		throw invalid_argument(classname() + "::scales(), invalid y scale");
	if (result.x() <= 0)
		throw invalid_argument(classname() + "::scales(), invalid x scale");

	return result;
}

RealFunctionF32 TomogramAcquisition::prepare_RealFunctionF32(Dicom::tag_e elem_tag) const
{
	RealFunctionF32 arrTmp(m_acquisition_loader->size());

	size_t i{ 0 };
	for (auto el : *m_acquisition_loader)
	{
		arrTmp[i] = el->get_double(elem_tag);
		++i;
	}
	return arrTmp;
}

RealFunctionF64 TomogramAcquisition::prepare_RealFunctionF64(Dicom::tag_e elem_tag) const
{
	RealFunctionF64 arrTmp(m_acquisition_loader->size());

	size_t i{ 0 };
	for (auto el : *m_acquisition_loader)
	{
		arrTmp[i] = el->get_double(elem_tag);
		++i;
	}
	return arrTmp;
}



RealFunctionF64	TomogramAcquisition::thickness() const
{
	Dicom::tomogram_slice &first= dynamic_cast<Dicom::tomogram_slice&>(*(m_acquisition_loader->front()));
	
	if (m_acquisition_loader->front()->get_m_frame_no())
	{
		RealFunctionF64 result( 1 );
		result[0] = first.thickness_mf();
		return result;
	}
	else
	{
		return prepare_RealFunctionF64(Dicom::e_slice_thickness);
	}
}



VectorFunction2_F64	TomogramAcquisition::scales_xy() const
{
	VectorFunction2_F64 vecTmp(m_acquisition_loader->size());

	size_t i{ 0 };
	size_t to_be_resized{ 0 };

	for (auto el : *m_acquisition_loader)
	{
		Dicom::tomogram_slice &current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			vector<double> scales = current_slice.scales_xy();
			vecTmp[i].y() = scales[1];
			vecTmp[i].x() = scales[0];
			++i;
		}
		else
		{
			to_be_resized = i + current_slice.get_m_frame_no();

			vecTmp.resize(to_be_resized);

			for (size_t ii = 0; ii < current_slice.get_m_frame_no(); ++ii)
			{
				vector<double> tmp = current_slice.scales_xy_mf();//внутри мультифрейма шаг точек измениться не может
				vecTmp[i + ii].y() = tmp[1];
				vecTmp[i + ii].x() = tmp[0];
			}
			i += to_be_resized;
		}
	}
	return vecTmp;
}

VectorFunction3_F64	TomogramAcquisition::image_positions_patient() const
{
	VectorFunction3_F64 vecTmp(m_acquisition_loader->size());

	size_t i{ 0 };
	size_t to_be_resized{ 0 };
	for (auto el : *m_acquisition_loader)
	{
		Dicom::tomogram_slice &current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			vector<double> positions = current_slice.image_position_patient();
			vecTmp[i].z() = positions[2];
			vecTmp[i].y() = positions[1];
			vecTmp[i].x() = positions[0];
			++i;
		}
		else
		{
			to_be_resized = i + current_slice.get_m_frame_no();

			vecTmp.resize(to_be_resized);

			for (size_t ii = 0; ii < current_slice.get_m_frame_no(); ++ii)
			{
				vector<double> tmp = current_slice.image_position_patient(ii);
				vecTmp[i+ii].z() = tmp[2];
				vecTmp[i+ii].y() = tmp[1];
				vecTmp[i+ii].x() = tmp[0];
			}
			i += to_be_resized;
		}
	}
	return vecTmp;
}

index_vector	TomogramAcquisition::sizes() const
{
	Dicom::tomogram_slice &first_slice = dynamic_cast<Dicom::tomogram_slice&>(*(m_acquisition_loader->front()));
	return {(*m_acquisition_loader).size(), first_slice.vsize(), first_slice.hsize()};
}

size_t	TomogramAcquisition::sizes(size_t dim) const
{
	return sizes()[dim];
}

RealFunctionF32	TomogramAcquisition::currents() const
{
	return load_ordered_currents(non_sorted_slice_order());
}

RealFunctionF32	TomogramAcquisition::load_ordered_currents() const
{
	return load_ordered_currents(determine_slice_order());
}

RealFunctionF32	TomogramAcquisition::load_ordered_currents(
	const vector<pair<size_t, size_t>>& slice_order) const
{
	//    XRAD_ASSERT_THROW(slice_order.size() == sizes(0));
	RealFunctionF32 currents(slice_order.size());


	for (size_t i = 0; i < slice_order.size(); i++)
	{
		auto instance_index = slice_order[i].first;

		XRAD_ASSERT_THROW_M(instance_index < slice_order.size(), runtime_error,
			"Invalid slice order data: index is too big.");

		auto el = (*m_acquisition_loader)[instance_index];
		Dicom::tomogram_slice& current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
//			current_slice.get_image(slices.GetSlice({ i, slice_mask(0), slice_mask(1) }).ref());
			currents.at(i) = current_slice.get_double(Dicom::e_tube_current);
		}

		else
		{
//			current_slice.get_image(slices.GetSlice({ i, slice_mask(0), slice_mask(1) }).ref(), slice_order[i].second);
		currents.at(i) = current_slice.get_current_mf(slice_order[i].second);
		}
	}

	return currents;
}

vector<pair<size_t, size_t>>	TomogramAcquisition::non_sorted_slice_order() const
{

	vector<pair<size_t, size_t>> frame_order;
	size_t index = 0;

	for (auto& el : *m_acquisition_loader)
	{
		Dicom::tomogram_slice& current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			frame_order.emplace_back(make_pair(EnsureType<size_t>(index), 0));
			++index;
		}

		else
		{
			for (size_t i = 0; i < current_slice.get_m_frame_no(); ++i)
			{
				frame_order.emplace_back(make_pair(EnsureType<size_t>(index), i));
			}
			index += current_slice.get_m_frame_no();
		}
	}

	return frame_order;
}


RealFunctionMD_F32	TomogramAcquisition::slices() const
{
	return load_ordered_slices(non_sorted_slice_order());
}

RealFunctionMD_F32	TomogramAcquisition::load_ordered_slices() const
{
	return load_ordered_slices(determine_slice_order());
}

RealFunctionMD_F32    TomogramAcquisition::load_ordered_slices(
	const vector<pair<size_t, size_t>> &slice_order) const
{
	//    XRAD_ASSERT_THROW(slice_order.size() == sizes(0));
	RealFunctionMD_F32 slices;

	Dicom::tomogram_slice &first_slice = dynamic_cast<Dicom::tomogram_slice&>(*(m_acquisition_loader->front()));

	slices.realloc({ slice_order.size(),first_slice.vsize(), first_slice.hsize() });

	for (size_t i = 0; i < slice_order.size(); i++)
	{
		auto instance_index = slice_order[i].first;

		XRAD_ASSERT_THROW_M(instance_index < slice_order.size(), runtime_error,
			"Invalid slice order data: index is too big.");

		auto el = (*m_acquisition_loader)[instance_index];
		Dicom::tomogram_slice &current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			current_slice.get_image(slices.GetSlice({ i, slice_mask(0), slice_mask(1) }).ref());

		}

		else
		{
			current_slice.get_image(slices.GetSlice({ i, slice_mask(0), slice_mask(1) }).ref(), slice_order[i].second);
		}
	}

	return slices;
}


vector<pair<size_t, size_t>> TomogramAcquisition::determine_slice_order() const
{
	size_t sort_axis = 0;
	if (!this->sort_axis(sort_axis))
		throw runtime_error("Cannot determine tomogram sort axis.");
	vector<pair<double, pair<size_t, size_t>>> frame_positions;
	size_t index = 0;

	for (auto &el : *m_acquisition_loader)
	{

		Dicom::tomogram_slice &current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			auto c = el->dicom_container()->get_double_values(Dicom::e_image_position_patient)[sort_axis];
			frame_positions.emplace_back(EnsureType<double>(c), make_pair(EnsureType<size_t>(index), 0));
			++index;
		}

		else
		{
			for (size_t i = 0; i < current_slice.get_m_frame_no(); ++i)
			{
				auto c = current_slice.image_position_patient(i)[sort_axis];
				frame_positions.emplace_back(EnsureType<double>(c), make_pair(EnsureType<size_t>(index), i));
			}
			index += current_slice.get_m_frame_no();
		}
	}

	std::sort(frame_positions.begin(), frame_positions.end(), std::less<pair<double, pair<size_t, size_t>>>());

	vector<pair<size_t, size_t>> frame_order;
	frame_order.reserve(frame_positions.size());

	for (auto &fp : frame_positions)
		frame_order.push_back(fp.second);

	return frame_order;
}



bool TomogramAcquisition::sort_axis(size_t &sort_axis_p) const
{
	sort_axis_p = size_t(-1);

	RealFunction2D_F32 pp(3, m_acquisition_loader->size());

	size_t i = 0;
	for (auto &el : *m_acquisition_loader)
	{

		Dicom::tomogram_slice &current_slice = dynamic_cast<Dicom::tomogram_slice&>(*el);

		if (!current_slice.get_m_frame_no())
		{
			vector<double> position = current_slice.image_position_patient();
			if (position.size() != 3)
				return false;
			std::copy(position.begin(), position.end(), pp.col(i).begin());
			++i;
		}

		else
		{
			size_t frame_count = current_slice.get_m_frame_no();

			pp.resize(3, pp.sizes(1) + frame_count - 1);

			for (size_t local_multiframe_index = 0; local_multiframe_index < frame_count;
					++local_multiframe_index)
			{
				vector<double> position = current_slice.image_position_patient(local_multiframe_index);
				if (position.size() != 3)
					return false;
				std::copy(position.begin(), position.end(), pp.col(i + local_multiframe_index).begin());
			}
			i += frame_count;
		}
	}


	RealFunctionF32	mp({MaxValue(pp.row(0)) - MinValue(pp.row(0)),
			MaxValue(pp.row(1)) - MinValue(pp.row(1)),
			MaxValue(pp.row(2)) - MinValue(pp.row(2))});
	MaxValue(mp, &sort_axis_p);
	return true;
}

//--------------------------------------------------------------

#if 0
generic_dicom_params_ptr TomogramAcquisition::GenerateSliceParams(size_t slice_no) const
Dicom::instance_ptr TomogramAcquisition::GenerateSliceParams(size_t slice_no) const
{
	//todo (Kovbas) разобраться со всей этой функцией! Она ещё используется? Нужна будет?
	KNS Вряд ли будет нужна.Отключаю, но не удаляю, с целью найти места, где ее, может быть, еще вызывают
		/*
		double location_to_sort;
		size_t axis_sort;
		point3_F64	img_position;
		point3_F64	img_cos_x;
		point3_F64	img_cos_y;
		double	slice_thickness;
		double	slice_location;
		*/
		Dicom::tomogram_slice *result = new Dicom::tomogram_slice;

	//+-result->slice_thickness = thickness[slice_no];
	//result->patient_name = wstring_to_string(name, e_encode_literals);
	result->set_patient_name(name);
	//result->precision = 16;
	result->set_precision(16);
	//result->signedness = 0;
	result->set_signedness(0);
	//result->ncomponents = 1;
	result->set_ncomponents(1);
	//result->nframes = 1;
	//result->set_nframes(1); //todo kovbas нужно ли это?
	//if (modality_string(modality()).empty())
	//{
	//	//result->modality_str = "CT";
	//	result->set_modality_str("CT");
	//}
	//else
	//{
	//	//result->modality_str = modality_string(modality());
	//	result->set_modality_str(modality_string(modality()));
	//}


	//return generic_dicom_params_ptr(result);
	return Dicom::instance_ptr(result);
}
#endif//0

//+-
#if 0
void TomogramAcquisition::PutFrameDataCreate(size_t frame, DicomDataContainer &dicom_file) const
{
	//generic_dicom_params_ptr	tomogram_params = GenerateSliceParams(frame);
	Dicom::instance_ptr tomogram_params = GenerateSliceParams(frame);

	RealFunction2D_F32::invariable data_slice;
	m_slices.GetSlice(data_slice, { frame, slice_mask(0), slice_mask(1) });
	size_t rowstep = (data_slice.hsize()) * sizeof(uint16_t);
	size_t framestep = (data_slice.hsize())*(data_slice.vsize()) * sizeof(uint16_t);

	RealFunction2D_F32 rescale_buffer(data_slice);


	RealFunction2D_I16 write_buffer(rescale_buffer);

	char* ptr = reinterpret_cast<char*>(write_buffer.data());
	dicom_file.set_pixeldata(ptr, true,
		int(data_slice.vsize()), int(data_slice.hsize()),
		precision, signedness,
		ncomponents, ncomponents, int(rowstep), int(framestep));

	//+-tomogram_params->put_metadata(dicom_file);

	//	std::vector<double> locations_vector(3, 0);
	// 	locations_vector[2] = positions[frame].z();
	// 	locations_vector[1] = positions[frame].y();
	// 	locations_vector[0] = positions[frame].z();
	// 	std::vector<double> scales(2, 1);
	// 	scales[0] = scales_xy[frame].x();
	// 	scales[1] = scales_xy[frame].y();

	// буферные массивы для записи координат
	// порядок координат в файле xyz, во внутреннем векторе zyx. из-за этого индексы перевернуты
	std::vector<double> locations_vector(positions[frame].rbegin(), positions[frame].rend());
	// порядок координат в файле xy, во внутреннем векторе yx. из-за этого индексы перевернуты
	std::vector<double> scales(scales_xy[frame].rbegin(), scales_xy[frame].rend());

	dicom_file.from_double_values(e_image_position, locations_vector);
	dicom_file.from_double_values(e_scales, scales);

	dicom_file.from_string(e_patient_name, wstring_to_string(name, e_encode_literals).c_str());
}
#endif

//+-
#if 0
//void TomogramAcquisition::PutFrameDataMod(const size_t &frame, tomogram_slice &tomogram_params, DicomDataContainer &dicom_file) const
void TomogramAcquisition::PutFrameDataMod(const size_t &frame, DicomDataContainer &dicom_file) const
{
	// изображение приходится класть в файл не из объекта класса, в котором оно содержится, а из наследуемого класса,
	// т.к. перед тем, как его положить в файл, к нему нужно применять специфичные для типа томограммы изменения,
	// данные для которых в этом классе отсутствуют, но присутствуют в дочернем


	//todo разобраться с тем, что кладётся в файл/файлы в этом методе. Также это есть в методе upload_data в классах LoadGenericClasses. Возможно, что лучше реализовать через те методы вывод.
	//+-tomogram_params.put_metadata(dicom_file);
#if 1
	//dicom_file.from_string(e_patient_name, patient_name);
	//dicom_file.from_string(e_study_date, study_date);
	//dicom_file.from_string(e_patient_id, patient_id);
	//dicom_file.from_string(e_series_description, series_description);
	//dicom_file.from_string(e_study_id, study_id);
	//dicom_file.from_string(e_accession_number, convert_to_string(accession_number));
	//dicom_file.from_string(e_study_id, convert_to_string(study_id));

	//dicom_file.from_string(e_modality_string, modality_str);
	//dicom_file.from_string(e_series_id, series_id);
	//dicom_file.from_string(e_modality_string, modality_str);
	//dicom_file.from_int(e_acquisition_number, acquisition_number); //добавлено, чтобы оставить возможность отделять снимки по этому параметру
#endif
	dicom_file.from_int(e_signedness, 1);



	std::vector<double> locations_vector(3, 0);
	std::vector<double> scales(2, 1);

	// порядок координат в файле xyz, во внутреннем векторе zyx. из-за этого индексы перевернуты
	locations_vector[2] = positions[frame].z();
	locations_vector[1] = positions[frame].y();
	locations_vector[0] = positions[frame].x();

	// порядок координат в файле xy, во внутреннем векторе yx. из-за этого индексы перевернуты
	scales[0] = scales_xy[frame].x();
	scales[1] = scales_xy[frame].y();
	//if (!(dicom_file.exist_element(e_image_position)))		 // не требуется проверка, она проводится в интерфейсе взаимодействия с файлами
	//{
	//	dicom_file.add_dataelement(e_image_position)->set_value(locations_vector);
	//}
	dicom_file.from_double_values(e_image_position, locations_vector);

	//if (!(dicom_file.exist_element(e_scales)))		 // не требуется проверка, она проводится в интерфейсе взаимодействия с файлами
	//{
	//	dicom_file.add_dataelement(e_scales)->set_value(scales);
	//}
	dicom_file.from_double_values(e_scales, scales);
}
#endif

XRAD_END

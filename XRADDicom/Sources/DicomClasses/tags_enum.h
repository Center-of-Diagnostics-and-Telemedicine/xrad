/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 12/26/2017 1:35:14 PM
	\author kovbas
*/
#ifndef tags_h__
#define tags_h__

#include <XRADBasic/Core.h>
#include <cstdint>

XRAD_BEGIN

namespace Dicom
{

	enum tag_e : uint32_t //tag_t
	{
		e_specific_character_set =	0x00080005,//	00080005	0008,0005	SpecificCharacterSet	ISO_IR 192

		e_date_time = 0x040A120,
		e_date = 0x040A121,
		e_time = 0x040A122,
		e_person_name = 0x040A123,

		//personal data
		e_patient_name =			0x00100010,
		e_patient_birthdate =		0x00100030,
		e_patient_birthtime = 		0x00100032,
		e_patient_sex =				0x00100040,
		e_patient_age =				0x00101010,
		e_patient_id =				0x00100020,

		//additional personal data
		e_other_patient_ids =		0x00101000,
		e_other_patient_ids_sequence = 0x00101002,
		e_type_of_patient_id =		0x00101022,
		e_other_patient_names =		0x00101001,
		e_patient_birthname =		0x00101005,
		e_patient_mothers_birthname = 0x00101060,
		e_medical_record_locator =	0x00101090,
		e_referenced_patient_photo = 0x00101100,
		e_patient_telephone_numbers = 0x00102154,
		e_patient_address =			0x00101040,

		e_current_patient_location = 0x00380300,
		e_patient_institution_residence = 0x00380400,
		e_country_of_residence = 0x00102150,
		e_region_of_residence = 0x00102152,


		//personal data history
		e_assigning_faculty_sequence = 0x00401036,
		e_scheduled_procedure_id = 0x00400009,
		e_requested_procedure_id = 0x00401001,
		e_filler_order_number = 0x00402017,

		//patient info
		e_patient_size =			0x00101020,//	00101020	0010,1020	 Patient Size	1.78
		e_patient_weight =			0x00101030,//	00101030	0010,1030	 Patient Weight	58
		e_patient_comments =		0x00104000,//	00104000	0010,4000	 Patient Comments	Brain
		e_formatted_patient_name =	0x00111022,//	00111022	0011,1022	 Formatted Patient Name

		//clinic data
		e_institution_name =		0x00080080,
		e_institution_address =		0x00080081,

		e_station_name =			0x00081010,//	00081010	0008,1010	 StationName	GP134
		e_institutional_department_name =	0x00081040,//	00081040	0008,1040	 Institutional Department Name	134 ГП ОЛД

		e_physician_name =			0x00080090, //	00080090	0008,0090	 ReferringPhysicianName
		e_institution_code_sequence = 0x00080082,
		e_referring_physicians_name = 0x00080090,
		e_referring_physicians_address = 0x00080092,
		e_referring_physicians_id_sequence = 0x00080096,
		e_referring_physicians_telephone_number = 0x00080094,

		e_physician_of_record = 0x00081048,
		e_physician_of_record_ids = 0x00081049,

		e_responsible_person = 0x00102297,
		e_evaluator_name = 0x00142006,

		e_operators_name = 0x00081070,
		e_name_of_physician_reading_study = 0x00081060,
		e_physician_reading_study_id_sequence = 0x00081062,
		e_performing_physicians_name = 0x00081050,
		e_performing_physicians_id_sequence = 0x00081052,


		e_coding_scheme_designator = 0x00080102,

		e_issuer_of_patient_id =	0x00100021,//	00100021	0010,0021	 Issuer Of Patient ID	MOWAGHUB

		//General Equipment (technical data general)
		e_manufacturer =			0x00080070,//	00080070	0008,0070	 Manufacturer	TOSHIBA_MEC
		e_manufacturers_model_name = 0x00081090,//	00081090	0008,1090	Manufacturers Model Name
		e_device_serial_number =	0x00181000,//	00181000	0018,1000	 DeviceSerialNumber	7124
		e_software_versions =		0x00181020,//	00181020	0018,1020	 SoftwareVersions	V9.60*R244
		e_scan_options =			0x00180022,//	00180022	0018,0022	 Scan  Options

		//instance data (от исследования до номера в сборке, стеке)
		e_accession_number =		0x00080050,// в обиходе применительно к ЕРИС содержимое этого поля называют AGFA Number
		e_retrieve_ae_title =		0x00080054,
		e_station_ae_title =		0x00080055,

		e_study_instance_uid =		0x0020000D, //0020,000D
		e_study_id =				0x00200010,
		e_study_date =				0x00080020,
		e_study_time =				0x00080030,
		e_study_description =		0x00081030,	//	00081030	0008,1030	 StudyDescription	Brain
		e_study_comment =			0x00324000, // Коварный параметр, может содержать персональные данные

		e_series_instance_uid =		0x0020000E, //SeriesInstanceUID
		e_series_number =			0x00200011,
		e_series_date =				0x00080021,
		e_series_time =				0x00080031,
		e_series_description =		0x0008103E, //	0008103E	0008,103E	 SeriesDescription	AX T2
		e_frame_of_reference_uid =	0x00200052,

		e_overlay_date = 0x00080024,
		e_overlay_time = 0x00080034,
		e_curve_date = 0x00080025,
		e_curve_time = 0x00080035,

		e_instance_number =			0x00200013,
		e_sop_instance_uid =		0x00080018,//(0008,0018)
		e_media_storage_sop_instance_uid =		0x00020003,

		e_referenced_sop_instance_uid = 0x00081155,

		e_instance_creation_date = 0x00080012,
		e_instance_creation_time = 0x00080013,
		e_stack_id =				0x00209056, //номер стека
		e_in_stack_position_number =	0x00209057, //номер в стеке  In Stack Position Number (0020,9057)
		e_temporal_position_index =	0x00209128, // это понадобится для МРТ
		e_acquisition_number =		0x00200012, //номер сборки. Их может быть несколько внутри одной серии, стека
		//?	00201002	0020,1002	 Images In Acquisition	28
		e_acquisition_date =		0x00080022,
		e_content_date =			0x00080023, //	00080023	0008,0023	Image Date
		e_acquisition_date_time =	0x0008002A,
		e_acquisition_time =		0x00080032,
		e_content_time =			0x00080033,//	00080033	0008,0033	Image Time

		e_body_part_examined =		0x00180015,//	00180015	0018,0015	 Body Part Examined	HEAD
		e_protocol_name =			0x00181030, //ProtocolName // описание того, какая часть исследуется

		e_dimension_index_values =	0x00209157,//(0020, 9157)	UL	3	12	1\1\1	DimensionIndexValues

		//модальность файла
		e_modality =				0x00080060,
		e_modality_in_study =		0x00080061,//?	00080061	0008,0061	 ModalitiesInStudy	MR

		//мультифрейм
		e_frame_content_sequence =	0x00209111, //Identifies general characteristics of this frame
		e_number_of_frames =		0x00280008, //количество фреймов в файле. Не обязательно существует. Если существует, то файл нужно обрабатывать как мультифрейм

		//image data
		e_transfer_syntax_uid	=	0x00020010,
		e_image_type =				0x00080008,	// (image type)
		e_photometric_interpretation =	0x00280004,
		e_image_position_patient =	0x00200032,
		e_image_orientation_patient = 0x00200037,//cosines
		e_slice_location =			0x00201041,
		e_pixel_spacing =			0x00280030, //e_scales = 0x00280030,  // размеры между точками в срезе
		e_imager_pixel_spacing =	0x00181164, // размеры между точками обычно в рентгеновских снимках
		e_columns =					0x00280011,
		e_rows =					0x00280010,
		e_bits_allocated =			0x00280100,
		e_bits_stored =				0x00280101, //e_precision = 0x00280101,
		e_high_bit =				0x00280102,
		e_pixel_representation =	0x00280103, //signedness
		e_signedness =				e_pixel_representation,
		e_samples_per_pixel =		0x00280002, //ncomponents
		//e_ncomponents =				e_samples_per_pixel,

		e_patient_orientation =		0x00200020,//	00200020	0020,0020	 Patient Orientation	L\P
		e_patient_position =		0x00185100,//	00185100	0018,5100	 PatientPosition	HFS

		e_largest_pixel_value =		0x00280109,//	00280109	0028,0109	Largest pixel value
		e_window_center =			0x00281050,//	00281050	0028,1050	Window Center
		e_window_width =			0x00281051,//	00281051	0028,1051	Window width

		e_image_comments =			0x00204000,

		//теги, указывающие на изображения
		//Pixel Data (7FE0,0010), Float Pixel Data (7FE0,0008) or Double Float Pixel Data (7FE0,0009)
		e_pixel_data =				0x7fe00010, // точки - целые значения (в основном используется)

		//tomogramm data
		e_slice_thickness =			0x00180050,

		//CT and XRay data
		e_ct_exposure_sequence =	0x00189321,
		e_rescale_intercept =		0x00281052,
		e_rescale_slope =			0x00281053,
		e_CTDIvol =					0x00189345, //Computed Tomography Dose Index
		e_tube_voltage_KVP =			0x00180060,
		e_tube_current =			0x00181151,
		e_exposure_time =			0x00181150,
		e_exposure =				0x00181152,
		e_detector_type =			0x00187004,
		e_distance_source_to_detector = 0x00181110,
		e_convolution_kernel =		0x00181210,

		e_acquisition_device_processing_description = 0x00181400,

		e_shared_functional_groups_sequence = 0x52009229, //Shared Functional Groups Sequence (5200,9229)
		e_per_frame_functional_groups_sequence = 0x52009230, // Per-frame Functional Groups Sequence (5200,9230)

		//MR data
		e_echo_pulse_sequence =		0x00189008,//	00189008	0018,9008	Echo Pulse Sequence	1C
		e_repetition_time =			0x00180080, //0018,0080	DS	Repetition time(TR)
		e_echo_time =				0x00180081, //0018,0081	DS	Echo Time(TE)
		e_flip_angle =				0x00181314,//0018,1314	DS	Flip angle
		e_sar =						0x00181316,//0018,1316	DS	SAR
		e_number_of_averages =		0x00180083,//0018,0083	DS	Number of Averages
		e_spacing_between_slices =	0x00180088,//0018,0088	DS	Spacing between slices
		e_echo_train_length =		0x00180091,//0018,0091	DS	Echo train Length
		e_rf_echo_train_length =	0x00189240,//	00189240	0018,9240	RF Echo train Length	1C
		e_parallel_acquisition_technique =	0x00189078,//0018,9078	1C	Parallel Acquisition Technique
		e_pixel_bandwidth =			0x00180095,//	00180095	0018,0095	Pixel Bandwidth	1C
		e_receive_coil_name =		0x00181250,//0018,1250	1C	Receive Coil Name

		e_diffusion_b_value =		0x00189087,//0018,9087	1C	Diffusion b-value 0019,100С
		e_diffusion_b_value_siemens = 0x0019100c,//0019,100c //0x00189087,//0018,9087	1C	Diffusion b-value 0019,100С
		e_diffusion_directionality =	0x00189075,//0018,9075	1C	Diffusion Directionality

		e_mr_velocity_encoding_sequence =	0x00189197,//	00189197	0018,9197	MR Velocity Encoding Sequence	1
		e_velocity_encoding_direction =	0x00189090,//0018,9090	1C	Velocity Encoding Direction
		e_velocity_encoding_maximum_value =	0x00189217,//	00189217	0018,9217	Maximum velocity in cm/s
		e_velocity_encoding_minimum_value =	0x00189091,//	00189091	0018,9091	Velocity Encoding Minimum​ Value​	1C
		e_slab_thickness =			0x00189104,//	00189104	0018,9104	Slab Thickness	1
		e_slab_orientation =		0x00189105,//	00189105	0018,9105	Slab Orientation	1

		e_asl_crusher_flow_limit =	0x0018925A,//	0018925A	0018,925A	ASL Crusher Flow Limit	1C
		e_asl_bolus_cut_off_flag =	0x0018925C,//0018,925C	1	ASL Bolus Cut-off Flag
		e_asl_bolus_cut_off_delay_time =	0x0018925F,//0018,925F	1	ASL Bolus Cut-off Delay Time

		e_scanning_sequence =		0x00180020,//	00180020	0018,0020	 Scanning Sequence	SE
		e_sequence_variant =		0x00180021,//	00180021	0018,0021	 Sequence Variant	NONE
		e_mr_acquisition_type =		0x00180023,//	00180023	0018,0023	 MRAcquisition Type	2D
		e_sequence_name =			0x00180024,//	00180024	0018,0024	 Sequence Name	FSE+15_nBW_slt
		e_imaging_frequency =		0x00180084,//	00180084	0018,0084	 Imaging Frequency	63.80679184
		e_imaged_nucleus =			0x00180085,//	00180085	0018,0085	 Imaged Nucleus	H
		e_echo_numbers =			0x00180086,//	00180086	0018,0086	 Echo Numbers	1
		e_number_of_phase_encoding_steps =		0x00180089,//	00180089	0018,0089	 Number Of Phase Encoding Steps	130
		e_percent_phase_field_of_view =		0x00180094,//	00180094	0018,0094	 Percent Phase Field Of View	125
		e_acquisition_matrix =		0x00181310,//	00181310	0018,1310	 AcquisitionMatrix	0\384\224\0
		e_in_plane_phase_encoding_direction =		0x00181312,//	00181312	0018,1312	 InPlanePhaseEncodingDirection	ROW

		e_scheduled_procedure_step_start_date = 0x00400002,// следующие поля неизвестно, зачем нужны, но для полноценной анонимизации исследований с учетом дат их нужно тоже обрабатывать
		e_scheduled_procedure_step_end_date =	0x00400004,
		e_performed_procedure_step_start_date = 0x00400244,
		e_performed_procedure_step_id =		0x00400253,
		e_commments_on_the_performed_procedure_step = 0x00400280,

		e_magnetic_field_strength =			0x00180087,



		//	00321060	0032,1060	> RequestedProcedureDescription	Магнитно-резонансная томография головного мозга с контрастированием
		//	00321064	0032,1064	 Requested Procedure Code Sequence	[This is a sequence]
		//	00400007	0040,0007	> ScheduledProcedureStepDescription	MAGNITNO-REZONANSNAYA TOMOGRAFIYA GOLOVNOGO MOZGA S KONTRAST
		//	00400008	0040,0008	> ScheduledProtocolCodeSequence	[This is a sequence]
		//	00400009	0040,0009	> ScheduledProcedureStepID	AGFA000001323907
		//	00400275	0040,0275	 Request Attributes Sequence	[This is a sequence]
		//	00401001	0040,1001	> RequestedProcedureID	AGFA000001323778
		//	00401003	0040,1003	 RequestedProcedurePriority	MEDIUM
		//	700D0010	700D,0010	 Private Creator Unknown	TOSHIBA_MEC_MR3

		//?	00080018	0008,0018	SOPInstanceUID	1.2.392.200036.9116.4.1.7124.8115.10.2001.1.3389344076

		//	00080100	0008,0100	> CodeValue	A05.23.009.001
		//	00080102	0008,0102	> Coding Scheme Designator	AGFA
		//	00080103	0008,0103	> Coding Scheme Version	1
		//	00080104	0008,0104	>> CodeMeaning	MAGNITNO-REZONANSNAYA TOMOGRAFIYA GOLOVNOGO MOZGA S KONTRAST
		//?	00081032	0008,1032	 ProcedureCodeSequence	[This is a sequence]

		//DICOMDIR
		e_file_set_id =				0x00041130,//(0004,1130) FileSetID

		// groups
		e_item_delimitation_tag_group =			0xfffe,
		e_pixel_data_group =		0x7fe0,
		e_meta_info_group =			0x0002,
		e_dicomdir_group =			0x0004,

		//e_results_id = 0x40080040 //этот таг предполагается использовать как условную границу, докуда читать дайком при предпросмотре


		//private tags
			e_agfa_private_creator = 0x7FD40010
	};

} // Dicom

XRAD_END

#endif // tags_h__

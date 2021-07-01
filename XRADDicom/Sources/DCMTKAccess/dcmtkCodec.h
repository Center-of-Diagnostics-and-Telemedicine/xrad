/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef dcmtkCodec_h__
#define dcmtkCodec_h__
/*!
	\file
	\author Kovbas
	\brief Содержит инструменты для кодирования/декодирования изображений, включения необходимых для DCMTK кодеров/декодеров при создании объекта.
*/

#include <XRADBasic/Core.h>

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>

XRAD_BEGIN

Dicom::e_compression_type_t	recognizeCodecType(const string& codec_code_string);
Dicom::e_compression_type_t	recognizeCodecType(const E_TransferSyntax transf_syntax);

namespace Dicom
{


	class dcmtkCodec
	{
	public:
		enum dcmtk_codec_regime_e
		{
			e_encode,
			e_decode
		};

		//constructors
		/*
			Третий параметр не нужен при декодировании, поэтому задаётся значение по умолчанию. В случае кодирования то, чем изначально было закодировано, указывать нужно. Если не указать, то будет использоваться значение "не кодировано"
		*/
		dcmtkCodec(dcmtk_codec_regime_e regime_in, const string &codec_code_string, const string &codec_code_string_current = "1.2.840.10008.1.2.1" /*изображение не кодировано*/);
		dcmtkCodec(dcmtk_codec_regime_e regime_in, E_TransferSyntax transfer_syntax_uid_in, E_TransferSyntax transfer_syntax_uid_in_current = EXS_LittleEndianExplicit);

		//destructors
		~dcmtkCodec();

		//gets
		e_compression_type_t getDcmCodecType() { return codec_type; };

		//sets

		//methods
		//извлечение изображений
		bool getPixelData(DcmDataset &dataset, unique_ptr<char[]> &pixeldata, size_t &vertical_size, size_t &horizontal_size, size_t &bpp, bool &signedness, size_t &ncomponents, size_t numOfFrame = 0);
		bool getPixelData_compressed(DcmDataset &dataset, unique_ptr<char[]> &pixData, size_t &length);

	private:
		std::string msgHdr = "[dcmtkCodec] - ";

		// переменные для контроля количества включений кодеров/декодеров
		static size_t numOpenedDCMTKDecoderJPEG;
		static size_t numOpenedDCMTKDecoderJPEGLS;
		static size_t numOpenedDCMTKEncoderJPEG;

		const e_compression_type_t codec_type;
		const e_compression_type_t codec_type_coded;
		// режим работы кодека: кодирование/декодирование
		const dcmtk_codec_regime_e codding_regime;

		e_compression_type_t	recognizeCodecType(const string &codec_code_string);
		e_compression_type_t	recognizeCodecType(const E_TransferSyntax transf_syntax);

		//additional functions
		void initialize();
		void decoder_initializer(e_compression_type_t codec_type_in);
		void encoder_initializer(e_compression_type_t codec_type_in);
		void decoder_uninitializer(e_compression_type_t codec_type_in);
		void encoder_uninitializer(e_compression_type_t codec_type_in);

		//компрессия/декомпрессия изображений
		bool compressPixelDataJPEG2000(unique_ptr<char[]> &dst, size_t &dst_len, const unique_ptr<char[]> &src, size_t src_len, size_t hs, size_t vs, size_t prec, size_t bpp, bool sign);
	};

} //namespace Dicom

XRAD_END

#endif // dcmtkCodec_h__
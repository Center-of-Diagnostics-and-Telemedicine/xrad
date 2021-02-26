/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\author kovbas
*/
#include "pre.h"
#include "dcmtkCodec.h"
#include "dcmtkElementsTools.h"
#include <mutex>

XRAD_BEGIN

namespace Dicom
{
	size_t dcmtkCodec::numOpenedDCMTKDecoderJPEG = 0;
	size_t dcmtkCodec::numOpenedDCMTKDecoderJPEGLS = 0;
	std::mutex DCMTKDecoderJPEGInitialize;
	std::mutex DCMTKDecoderJPEGLSInitialize;

	size_t dcmtkCodec::numOpenedDCMTKEncoderJPEG = 0;
	std::mutex DCMTKEncoderJPEGInitialize;

	dcmtkCodec::dcmtkCodec(dcmtk_codec_regime_e regime_in, const string &codec_code_string, const string &codec_code_string_current)
		: codding_regime(regime_in), codec_type(recognizeCodecType(codec_code_string)), codec_type_coded(recognizeCodecType(codec_code_string_current))
	{
		dcmtkCodec::initialize();
	}

	dcmtkCodec::dcmtkCodec(dcmtk_codec_regime_e regime_in, E_TransferSyntax transfer_syntax_uid_in, E_TransferSyntax transfer_syntax_uid_in_current)
		: codding_regime(regime_in), codec_type(recognizeCodecType(transfer_syntax_uid_in)), codec_type_coded(recognizeCodecType(transfer_syntax_uid_in_current))
	{
		dcmtkCodec::initialize();
	}

	void dcmtkCodec::decoder_initializer(e_compression_type_t codec_type_in)
	{
		switch (codec_type_in)
		{
		case e_jpeg: case e_jpeg_lossless:
		{
			std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGInitialize);
			if (numOpenedDCMTKDecoderJPEG++ == 0)
				DJDecoderRegistration::registerCodecs(); // register JPEG decoder
		}
			break;

		case e_jpeg_ls:
		{
			std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGLSInitialize);
			if (numOpenedDCMTKDecoderJPEGLS++ == 0)
				DJLSDecoderRegistration::registerCodecs(); // register JPEG-LS decoder
		}
			break;

		case e_jpeg2k:
			break;

		case e_unknown:
			//throw invalid_argument(msgHdr + "It is an unknown codec. I cannot decode it.");
			break;

		}
	}

	void dcmtkCodec::encoder_initializer(e_compression_type_t codec_type_in)
	{
		switch (codec_type_in)
		{
		case e_jpeg_lossless: //case e_jpeg_ls_lossless:
		{
			std::lock_guard<std::mutex> guard(DCMTKEncoderJPEGInitialize);
			if (numOpenedDCMTKEncoderJPEG++ == 0)
				DJEncoderRegistration::registerCodecs(); // register JPEG and JPEG-LS encoder
			break;
		}

		case e_jpeg2k:
			break;

		case e_unknown: //todo (Kovbas) этот вариант, скорее всего, вообще не нужен. Т.к. если не известен, то не кодируем.
		//	throw invalid_argument(msgHdr + "It is an unknown codec. I cannot encode it.");
			break;
		}
	}

	void dcmtkCodec::initialize()
	{
		switch (codding_regime)
		{
		case e_decode:
			decoder_initializer(codec_type);
			break;

		case e_encode:
			decoder_initializer(codec_type_coded);
			encoder_initializer(codec_type);
			break;
		}
	}

	e_compression_type_t	dcmtkCodec::recognizeCodecType(const string &codec_code_string)
	{
		string tmpStr = codec_code_string;
		if (tmpStr.length() != 0)
			if (tmpStr[tmpStr.length() - 1] != '\0')
				tmpStr = tmpStr.substr(0, codec_code_string.length() - 1);
			else
				tmpStr = tmpStr.substr(0, tmpStr.length() - 2);
		else
			throw invalid_argument(msgHdr + "Error: Cannot find TransferSyntaxID (it is empty) " + codec_code_string + ". Cant open an image.");

		if ((tmpStr == "1.2.840.10008.1.2.4.5") || (tmpStr == "1.2.840.10008.1.2.4.7"))
		{
			return e_jpeg;
		}

		else if (tmpStr == "1.2.840.10008.1.2.4.8")
		{
			//1.2.840.10008.1.2.4.8x - JPEG-LS // hp license? See the bottom here http://support.dcmtk.org/docs/mod_dcmjpls.html //answer  here http://www.labs.hp.com/research/info_theory/loco/
			return e_jpeg_ls;
		}

		else if (tmpStr == "1.2.840.10008.1.2.4.9")
		{
			return e_jpeg2k;
		}

		else if ((tmpStr == "1.2.840.10008.1.") || tmpStr == "1.2.840.10008.1.2.")
		{
			return e_uncompressed;
		}
		else
		{
			return e_unknown;
			//throw invalid_argument(msgHdr + "Error: unsupported codec = " + codec_code_string);
		}
	}

	e_compression_type_t	dcmtkCodec::recognizeCodecType(E_TransferSyntax transfer_syntax_uid_in)
	{
		switch (transfer_syntax_uid_in)
		{
		/// Implicit VR Little Endian
		case EXS_LittleEndianImplicit:
		/// Implicit VR Big Endian (pseudo transfer syntax that does not really exist)
		case EXS_BigEndianImplicit:
		/// Explicit VR Little Endian
		case EXS_LittleEndianExplicit:
		/// Explicit VR Big Endian
		case EXS_BigEndianExplicit:
		/// JPEG Baseline (lossy)
			return e_uncompressed;
			break;

		/// JPEG Baseline (lossy)
		case EXS_JPEGProcess1:
		/// JPEG Extended Sequential (lossy, 8/12 bit)
		case EXS_JPEGProcess2_4:
		/// JPEG Extended Sequential (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess3_5:
		/// JPEG Spectral Selection, Non-Hierarchical (lossy, 8/12 bit)
		case EXS_JPEGProcess6_8:
		/// JPEG Spectral Selection, Non-Hierarchical (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess7_9:
		/// JPEG Full Progression, Non-Hierarchical (lossy, 8/12 bit)
		case EXS_JPEGProcess10_12:
		/// JPEG Full Progression, Non-Hierarchical (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess11_13:
		/// JPEG Lossless with any selection value
		case EXS_JPEGProcess14:
		/// JPEG Lossless with any selection value, arithmetic coding
		case EXS_JPEGProcess15:
		/// JPEG Extended Sequential, Hierarchical (lossy, 8/12 bit)
		case EXS_JPEGProcess16_18:
		/// JPEG Extended Sequential, Hierarchical (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess17_19:
		/// JPEG Spectral Selection, Hierarchical (lossy, 8/12 bit)
		case EXS_JPEGProcess20_22:
		/// JPEG Spectral Selection, Hierarchical (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess21_23:
		/// JPEG Full Progression, Hierarchical (lossy, 8/12 bit)
		case EXS_JPEGProcess24_26:
		/// JPEG Full Progression, Hierarchical (lossy, 8/12 bit), arithmetic coding
		case EXS_JPEGProcess25_27:
		/// JPEG Lossless, Hierarchical
		case EXS_JPEGProcess28:
		/// JPEG Lossless, Hierarchical, arithmetic coding
		case EXS_JPEGProcess29:
		/// JPEG Lossless, Selection Value 1
		//case EXS_JPEGProcess14SV1 = 21,
		/// Run Length Encoding (lossless)
		case EXS_RLELossless:
		/// Deflated Explicit VR Little Endian
		case EXS_DeflatedLittleEndianExplicit:
			return e_jpeg;
			break;

		case EXS_JPEGProcess14SV1:
			return e_jpeg_lossless;
			break;

		/// JPEG-LS (lossless)
		case EXS_JPEGLSLossless:
		/// JPEG-LS (lossless or near-lossless mode)
		case EXS_JPEGLSLossy:
			return e_jpeg_ls;
			break;

		/// JPEG 2000 (lossless)
		case EXS_JPEG2000LosslessOnly:
		/// JPEG 2000 (lossless or lossy)
		case EXS_JPEG2000:
		/// JPEG 2000 part 2 multi-component extensions (lossless)
		case EXS_JPEG2000MulticomponentLosslessOnly:
		/// JPEG 2000 part 2 multi-component extensions (lossless or lossy)
		case EXS_JPEG2000Multicomponent:
			return e_jpeg2k;
			break;

		default:
			return e_unknown;
			break;
		}
	}

	void dcmtkCodec::decoder_uninitializer(e_compression_type_t codec_type_in)
	{
		switch (codec_type_in)
		{
		case e_jpeg: case e_jpeg_lossless:
		{
			std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGInitialize);
			if (--numOpenedDCMTKDecoderJPEG == 0)
				DJDecoderRegistration::cleanup(); // unregister JPEG decoder
		}
		break;

		case e_jpeg_ls:
		{
			std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGLSInitialize);
			if (--numOpenedDCMTKDecoderJPEGLS == 0)
				DJLSDecoderRegistration::cleanup(); // unregister JPEG-LS decoder
		}
		break;

		case e_jpeg2k:
			break;
		}
	}
	void dcmtkCodec::encoder_uninitializer(e_compression_type_t codec_type_in)
	{
		switch (codec_type_in)
		{
		case e_jpeg_lossless: //case e_jpeg_ls_lossless:
		{
			std::lock_guard<std::mutex> guard(DCMTKEncoderJPEGInitialize);
			if (--numOpenedDCMTKEncoderJPEG == 0)
				DJEncoderRegistration::cleanup(); // unregister JPEG and JPEG-LS encoder
		}
		break;

		case e_jpeg2k:
			break;
		}
	}

	dcmtkCodec::~dcmtkCodec()
	{
		// отключение кодека
		// !!! обязательное отключение кодека после получения изображения !!!
		switch (codding_regime)
		{
		case e_decode:
			decoder_uninitializer(codec_type);
			break;

		case e_encode:
			decoder_uninitializer(codec_type_coded);
			encoder_uninitializer(codec_type);
			break;
		}
	}

	// получение изображения и декодирование его чисто средствами dcmtk (для не JPEG2000)
	bool getPixelsConventional(DcmDataset &dcmDataset, unique_ptr<char[]> &pixeldata, size_t &vs_in, size_t &hs_in, size_t &bpp, bool &signedness, size_t &ncomp, size_t numOfFrame = 0)
	{
		//	try
		{
			E_TransferSyntax xfer;
			xfer = dcmDataset.getOriginalXfer();

			//раскодирование
			dcmDataset.chooseRepresentation(EXS_LittleEndianExplicit, nullptr);

			// при создании объекта image копируются данные о всех изображениях из загруженного в память файла. После из этого выцепляются нужные для изображения данные.
			// при мультифрейме это создаёт проблему, поэтому пока мультифреймы не обрабатываем.
			// информация отсюда http://forum.dcmtk.org/viewtopic.php?f=1&t=240
			//мы используем ключ CIF_IgnoreModalityTransformation, т.к. нам требуется извлечение данных без применения преобразований, заложенных в модальности. Это медленнее, но получаем то, что нужно
			//при использовании, например, ключа CIF_UsePartialAccessToPixelData извлечение изображений куда быстрее, но к ним применены преобразования, которые нам не требуются
			//note (Kovbas) последняя договорённость, что мы забираем изображения с уже применёнными преобразованиями
			//unique_ptr<DicomImage> image(new DicomImage(&dcmDataset, xfer, CIF_IgnoreModalityTransformation, unsigned long(numOfFrame), 1 /* fcount */));
			unique_ptr<DicomImage> image(new DicomImage(&dcmDataset, xfer, CIF_UsePartialAccessToPixelData, (unsigned long)(numOfFrame), 1 /* fcount */));
			vs_in = image->getHeight();
			hs_in = image->getWidth();
			ncomp = image->isMonochrome() ? 1 : 3;

			if (image->getStatus() != EIS_Normal) throw runtime_error("There is no pixel data in dataset!");

			const DiPixel &diPixel = *(image->getInterData());
			EP_Representation rep = diPixel.getRepresentation();

			bpp = 0;
			switch (rep)
			{
			/// unsigned 8 bit integer
			case EPR_Uint8:
				signedness = false;
				bpp = 8;
				break;
			/// signed 8 bit integer
			case EPR_Sint8:
				signedness = true;
				bpp = 8;
				break;
			/// unsigned 16 bit integer
			case EPR_Uint16:
				signedness = false;
				bpp = 16;
				break;
			/// signed 16 bit integer
			case EPR_Sint16:
				signedness = true;
				bpp = 16;
				break;
			/// unsigned 32 bit integer
			case EPR_Uint32:
				signedness = false;
				bpp = 32;
				break;
			/// signed 32 bit integer
			case EPR_Sint32:
				signedness = true;
				bpp = 32;
				break;
			default:
				break;
			}

			if (bpp == 0) throw runtime_error("bpp of pixelData in Dataset == 0. It is an incorrect condition or there is no pixel data.");

			size_t pixeldata_length = ((bpp + (CHAR_BIT - 1))/CHAR_BIT)*vs_in*hs_in;

			pixeldata = make_unique<char[]>(pixeldata_length);
			memcpy(pixeldata.get(), reinterpret_cast<const char*>(diPixel.getData()), pixeldata_length);

			//shared_cfile	file("c:/temp/bitmap_dump.raw", "wb");
			//file.write(reinterpret_cast<const char*>(diPixel.getData()), pixeldata_length, 1);

			//закрыть кодек - // это происходит автоматически при уничтожении объекта кодека
			return true;
		}
		//	catch (exception &ex)
		{

		}
	}

	bool getPixelDataAsRAW(DcmDataset &dcmDataSet, Uint8* &pixData, size_t &length, size_t numOfFrame = 0)
	{
#if 0
		OFCondition dcmRes;
		// находим элемент с изображением
		DcmElement* element = NULL;
		dcmRes = dcmDataSet.findAndGetElement(DCM_PixelData, element);

		checkOFResult(dcmRes, string("Get the picture"));
		//if ((result.bad() || element == NULL)) throw runtime_error("We didnt get picture");

		// забираем элемент с изображением
		DcmPixelData *dpix = NULL;
		dpix = OFstatic_cast(DcmPixelData*, element);
		/* Since we have compressed data, we must utilize DcmPixelSequence
		in order to access it in raw format, e. g. for decompressing it
		with an external library.
		*/
		// забираем последовательность байт жатого изображения
		DcmPixelSequence *dseq = NULL;
		E_TransferSyntax xferSyntax = EXS_LittleEndianExplicit;
		const DcmRepresentationParameter *rep = NULL;
		// Find the key that is needed to access the right representation of the data within DCMTK
		dpix->getOriginalRepresentationKey(xferSyntax, rep);
		// Access original data representation and get result within pixel sequence
		dcmRes = dpix->getEncapsulatedRepresentation(xferSyntax, rep, dseq);
		//if (dcmRes != EC_Normal) throw runtime_error(string("Couldnt get encapsulated data"));
		DcmPixelItem* pixitem = NULL;

		// Access first frame (skipping offset table)
		dseq->getItem(pixitem, 1);
		if (pixitem == NULL) throw runtime_error(string("pixitem == null"));

		// Get the length of this pixel item (i.e. fragment, i.e. most of the time, the lenght of the frame)
		length = pixitem->getLength();

		if (length == 0) throw runtime_error(string("Length of pixitem == 0"));

		// Finally, get the compressed data for this pixel item
		dcmRes = pixitem->getUint8Array(pixData);

		if (dcmRes.bad()) throw runtime_error("Cant get RAW array");

		return true;
#else
		// source https://groups.google.com/forum/#!topic/comp.protocols.dicom/UPECrdJYyAU
		/*in order to access the original JPEG compressed pixel data you cannot use the
			DicomImage class.Instead, you should use the lower level dcmdata routines as
			follows(assuming that the "dataset" variable already contains the DICOM data) :
		*/
		DcmStack stack;
		/* search for PixelData element on top-level */
		if (dcmDataSet.search(DCM_PixelData, stack, ESM_fromHere, OFFalse /*searchIntoSub*/).good())
		{
			DcmPixelData *pixelData = (DcmPixelData *)stack.top();
			if (pixelData != nullptr)
			{
				/* get pixel data sequence (if available) */
				E_TransferSyntax xfer = EXS_Unknown;
				const DcmRepresentationParameter *repParam = nullptr;
				pixelData->getOriginalRepresentationKey(xfer, repParam);
				if ((xfer != EXS_Unknown) && DcmXfer(xfer).isEncapsulated())
				{
					DcmPixelSequence *pixelSeq = nullptr;
					if (pixelData->getEncapsulatedRepresentation(xfer, repParam, pixelSeq).good() && (pixelSeq != nullptr))
					{
						DcmPixelItem* pixitem = nullptr;
						/* now "pixelSeq" points to the sequence of pixel items */
						/* pixelSeq->card() returns the number of pixel items and */
						/* pixelSeq->getItem() allows to access a particular pixel item */
						long nFrame = 1;
						//if (dcmDataSet.tagExists(DCM_NumberOfFrames) && dcmDataSet.findAndGetLongInt(DCM_NumberOfFrames, nFrame).good())
						if (dcmDataSet.tagExists(DCM_NumberOfFrames))
							if (dcmDataSet.findAndGetLongInt(DCM_NumberOfFrames, nFrame).good())
								if (nFrame > 0)
									nFrame = long(numOfFrame); //преобразуем к типу, используемому в DCMTK


						if (pixelSeq->getItem(pixitem, nFrame).good())
						{
							length = pixitem->getLength();
							if (pixitem->getUint8Array(pixData).bad()) throw runtime_error("Cant get RAW array");
						}
						else
						{
							throw runtime_error("Can't get pixelItem for JPEG2000");
						}

					}
				}
			}
		}

		return true;
#endif
	}

	// получение изображения JPEG2000
	//функция чтения для openjpeg
	static OPJ_SIZE_T opj_memory_stream_read(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data)
	{
		memcpy(p_buffer, p_user_data, p_nb_bytes);
		return p_nb_bytes;
	}
	opj_image_t* decodeJPEG2000(Uint8 *pixData, size_t length)
	{
		string msgHdr = "[DecodeJPEG2000PixData] - ";
		OPJ_BOOL opRes;
		//создать кодек
		opj_dparameters_t parameters;	/* decompression parameters */
		opj_set_default_decoder_parameters(&parameters);
		opj_codec_t* p_decompressor = opj_create_decompress(OPJ_CODEC_J2K);
		opRes = opj_setup_decoder(p_decompressor, &parameters);
		if (!opRes) throw runtime_error("Cant create decoder for JPEG2000.");

		//создать поток с входными данными
		opj_stream_t* p_stream;

		{ //(Kovbas) я не могу понять почему мы не можем воспользоваться потоком по умолчанию изменив некоторые его параметры. Ерунда какая-то, по-моему!!!
			// Это относится к выделенному куску.
			p_stream = opj_stream_create(length, OPJ_TRUE);
			opj_stream_set_read_function(p_stream, opj_memory_stream_read);
			//p_stream = opj_stream_default_create(OPJ_TRUE);
		}

		opj_stream_set_user_data(p_stream, pixData, nullptr);
		opj_stream_set_user_data_length(p_stream, length);

		// декодировать данные
		opj_image_t* image;
		opRes = opj_read_header(p_stream, p_decompressor, &image);
		if (!opRes) throw runtime_error(msgHdr + "Cant read JPEG2000 header and data.");
		opRes = opj_decode(p_decompressor, p_stream, image);
		if (!opRes) throw runtime_error(msgHdr + "Cant decode JPEG2000 data.");

		//удалить поток с входными данными
		opj_stream_destroy(p_stream);

		//удалить кодек
		opj_destroy_codec(p_decompressor);

		return image;
	}

	bool getPixelsJpeg2000(DcmDataset &dcmDataset, unique_ptr<char[]> &pixeldata, size_t &vs_in, size_t &hs_in, size_t &bpp, bool &signedness, size_t &ncomp, size_t numOfFrame = 0)
	{
		try
		{
			// забираем данные
			size_t codedPixLen;
			// забираем закодированные данные
			Uint8* pixDataArr;
			getPixelDataAsRAW(dcmDataset, pixDataArr, codedPixLen, numOfFrame);

			// раскодируем
			opj_image_t* image = decodeJPEG2000(pixDataArr, codedPixLen);

			if ((image == nullptr) || (image->comps->data == nullptr)) runtime_error("Couldn't decompress Jpeg2000 image.");

			//забираем данные о размерах изображения. Сравниваем взятые из файла с потоковыми. В случае разницы, берём размеры из потока
			vs_in = image->comps->h;
			hs_in = image->comps->w;
			size_t prec = image->comps->prec;
			//(Kovbas) image->comps->bpp почему-то не отдаёт корректное значение, поэтому сделано так:
			//bpp = image->comps->bpp; // это не даёт корректный результат
			if ( (image->comps->bpp >= prec) && (prec > 0) )
				bpp = image->comps->bpp;
			else
			{
				if ((prec < 1) || (prec > 32))
					throw invalid_argument("invalid bpp");
				else if (prec <= 8)
				{
					bpp = 8;
				}
				else if (prec <= 16)
				{
					bpp = 16;
				}
				else if (prec <= 32)
				{
					bpp = 32;
				}
				else
					throw invalid_argument("prec is too much (more than 32)");
			}
			auto	inc = (bpp + (CHAR_BIT - 1)) / CHAR_BIT; // используется при переносе изображения из массива OpenJPEG в наш массив

			ncomp = image->numcomps;

			//(Kovbas) странная, но не необычная ситуация, когда знаковость не совпадает, но из файла данные более корректны, чем из потока
			// в данном случае есть единственное решение всегда верить файлу, а не потоку
			//bool signedness_loc = image->comps->sgnd != 0; //Kovbas мы везде знаковость используем как bool, поэтому здесь происходит эта ручная конвертация
			//if (signedness_loc != signedness)
				//;

			size_t pixeldata_length = (bpp + (CHAR_BIT - 1)) / CHAR_BIT*image->comps->h*image->comps->w;
			pixeldata = make_unique<char[]>(pixeldata_length);
			char*  newPtr = pixeldata.get();
			// эта проверка осталась на случай, если в OpenJPEG что-то изменится
			static_assert(sizeof(decltype(*image->comps->data)) == 4, "must be 32 bit");

			for (size_t i = 0; i < image->comps->h; i++)
				for (size_t j = 0; j < image->comps->w; j++)
				{
					size_t	bit_offset = (i*hs_in + j)*bpp;
					ptrdiff_t	byte_offset = bit_offset / CHAR_BIT;
					char *ptr = reinterpret_cast<char*>(image->comps->data);
					ptr += byte_offset;

					if (signedness && bpp > 1)
					{
						int32_t	buffer = image->comps->data[(i*hs_in + j)];
						buffer <<= (32 - prec);
						buffer >>= (32 - prec);
						if (bpp == 16) *(int16_t*)newPtr = buffer, newPtr += inc;
						if (bpp == 32) *(int32_t*)newPtr = buffer, newPtr += inc;
					}
					else
					{
						uint32_t	buffer = image->comps->data[(i*hs_in + j)];
						buffer <<= (32 - prec);
						buffer >>= (32 - prec);
						if (bpp == 16) *(uint16_t*)newPtr = buffer, newPtr += inc;
						if (bpp == 32) *(uint32_t*)newPtr = buffer, newPtr += inc;
					}
				}

			//delete image;
			////free(image->comps->data); //это выдаёт ошибку, в openjpeg рекомендована функция, которая использоана в следующей строке
			opj_image_data_free(image->comps->data);
			free(image->comps);
			free(image);

			return true;
		}

		catch (exception &)
		{
			return false; //todo (Kovbas) Обработать ошибку
		}
	}

	bool dcmtkCodec::getPixelData(DcmDataset &dataset, unique_ptr<char[]> &pixeldata, size_t &vs, size_t &hs, size_t &bpp, bool &signedness, size_t &ncomp, size_t numOfFrame)
	{
		switch (codec_type)
		{
		case e_unknown:
			throw runtime_error(msgHdr + "I dont know this codec. Cant decompress it.");

		case e_jpeg2k:
			return getPixelsJpeg2000(dataset, pixeldata, vs, hs, bpp, signedness, ncomp, numOfFrame);

		default:
			return getPixelsConventional(dataset, pixeldata, vs, hs, bpp, signedness, ncomp, numOfFrame);
		}
	}

	bool dcmtkCodec::getPixelData_compressed(DcmDataset &dataset, unique_ptr<char[]> &pixelData, size_t &length)
	{
		if (codec_type != e_uncompressed)
		{
			Uint8* pixData;
			getPixelDataAsRAW(dataset, pixData, length);
			pixelData = make_unique<char[]>(length);
			memcpy(pixelData.get(), pixData, length);

			return (pixData != nullptr);
		}
		else
			return false;
	}

	// it is from https://groups.google.com/forum/#!msg/openjpeg/8cebr0u7JgY/hc5k6r_LDAAJ
#ifndef UNUSED_ARGUMENT
#define UNUSED_ARGUMENT(x) (void)x
#endif

/* Error and message callback does not use the FILE* client object. */
	void error_callback(const char *msg, void *client_data)
	{
		UNUSED_ARGUMENT(client_data);
		UNUSED_ARGUMENT(msg);
		//OperatorConsole.printf("***[OpenJP2(J2k)]: %s\n", msg);
	}

	/* Warning callback expecting a FILE* client object. */
	void warning_callback(const char *msg, void *client_data)
	{
		UNUSED_ARGUMENT(client_data);
		UNUSED_ARGUMENT(msg);
		//OperatorConsole.printf("Warn[OpenJP2(J2k)]: %s\n", msg);
	}

	/* Debug callback expecting no client object. */
	void info_callback(const char *msg, void *client_data)
	{
		UNUSED_ARGUMENT(client_data);
		UNUSED_ARGUMENT(msg);
		//if (DebugLevel > 1)SystemDebug.printf("Info[OpenJP2(J2k)]: %s\n", msg);
	}
	// These routines are added to use memory instead of a file for input and output.
	//Structure need to treat memory as a stream.
	typedef struct
	{
		OPJ_UINT8* pData; //Our data.
		OPJ_SIZE_T dataSize; //How big is our data.
		OPJ_SIZE_T offset; //Where are we currently in our data.
	}opj_memory_stream;
	//This will read from our memory to the buffer.
	static OPJ_SIZE_T opj_memory_stream_read1(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
		OPJ_SIZE_T dataEndOffset = l_memory_stream->dataSize - 1;
		OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.
												//Check if the current offset is outside our data buffer.
		if (l_memory_stream->offset >= dataEndOffset) return (OPJ_SIZE_T)-1;
		//Check if we are reading more than we have.
		if (p_nb_bytes > (dataEndOffset - l_memory_stream->offset))
			l_nb_bytes_read = dataEndOffset - l_memory_stream->offset;//Read all we have.
																	  //Copy the data to the internal buffer.
		memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);
		l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.
		return l_nb_bytes_read;
	}
	//This will write from the buffer to our memory.
	static OPJ_SIZE_T opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
		OPJ_SIZE_T dataEndOffset = l_memory_stream->dataSize - 1;
		OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.
												 //Check if the current offset is outside our data buffer.
		if (l_memory_stream->offset >= dataEndOffset) return (OPJ_SIZE_T)-1;
		//Check if we are write more than we have space for.
		if (p_nb_bytes > (dataEndOffset - l_memory_stream->offset))
			l_nb_bytes_write = dataEndOffset - l_memory_stream->offset;//Write the remaining space.
																	   //Copy the data from the internal buffer.
		memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);
		l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.
		return l_nb_bytes_write;
	}
	//Moves the pointer forward, but never more than we have.
	static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;
		OPJ_SIZE_T dataEndOffset = l_memory_stream->dataSize - 1;
		OPJ_SIZE_T l_nb_bytes;

		if (p_nb_bytes < 0) return -1;//No skipping backwards.
		l_nb_bytes = (OPJ_SIZE_T)p_nb_bytes;//Allowed because it is positive.
											// Do not allow jumping past the end.
		if (l_nb_bytes > dataEndOffset - l_memory_stream->offset)
			l_nb_bytes = dataEndOffset - l_memory_stream->offset;//Jump the max.
																 //Make the jump.
		l_memory_stream->offset += l_nb_bytes;
		//Returm how far we jumped.
		return l_nb_bytes;
	}
	//Sets the pointer to anywhere in the memory.
	static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;

		if (p_nb_bytes < 0) return OPJ_FALSE;//No before the buffer.
		if (p_nb_bytes > (OPJ_OFF_T)(l_memory_stream->dataSize - 1)) return OPJ_FALSE;//No after the buffer.
		l_memory_stream->offset = (OPJ_SIZE_T)p_nb_bytes;//Move to new position.
		return OPJ_TRUE;
	}
	//The system need a routine to do when finished, the name tells you what I want it to do.
	static void opj_memory_stream_do_nothing(void * p_user_data)
	{
		OPJ_ARG_NOT_USED(p_user_data);
	}
	//Create a stream to use memory as the input or output.
	opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream)
	{
		// 	opj_stream_t* l_stream;
		// 	if(!(l_stream = opj_stream_default_create(p_is_read_stream))) return (NULL);//warning 4706: assignment within conditional expression. То же нужно записать более корректно.
		opj_stream_t* l_stream = opj_stream_default_create(p_is_read_stream);
		if (l_stream == nullptr) return nullptr;

		//Set how to work with the frame buffer.
		if (p_is_read_stream)
			opj_stream_set_read_function(l_stream, opj_memory_stream_read1);
		else
			opj_stream_set_write_function(l_stream, opj_memory_stream_write);
		opj_stream_set_seek_function(l_stream, opj_memory_stream_seek);
		opj_stream_set_skip_function(l_stream, opj_memory_stream_skip);
		opj_stream_set_user_data(l_stream, p_memoryStream, opj_memory_stream_do_nothing);
		opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);
		return l_stream;
	}
	/* This routine will take all of the listed color spaces in 08_03 of the standard
	*  in little endian, uncompressed format and  compress it to jpeg2000 in whatever
	*  format it came in.  I don't know if dicom viewers can support this. It uses the
	*  openjpeg library version 2.1  You can get it here:
	*  https://github.com/uclouvain/openjpeg
	*  OpenJPEG can compress anything, with any size for each plane, but the standard only
	*  allows YBR_RCT lossless and YBR_ICT lossy.
	*  If I have made some mistakes (most likely), you can contact me bruce.barton
	*  (the mail symbol goes here) me.com.  Let me know where I can find a sample of
	*  the image that didn't work. */
	//bool CompressJPEG2Ko(DICOMDataObject* pDDO, int j2kQuality)
	//bool encodeJPEG2000(unique_ptr<char[]> &dst, size_t &dstLen, const unique_ptr<char[]> &src, const size_t &srcLen, opj_image_cmptparm_t *cmptparms)
#define MAX_NUM_COMPS 3
	bool encodeJPEG2000(unique_ptr<char[]> &dst, size_t &dstLen, const unique_ptr<char[]> &src, size_t srcLen, opj_image_cmptparm_t *cmptparms_in)
	{
		string msgHdr = "[OpenJPEG] - ";
		int j2kQuality = 100;
		//ImageData           imageData;
		//ImageData           imageData;
		//physical_time             starttime;
		//UINT32		byteCnt; //возможно нужен будет при переводе на работу с цветными изображениями
		//char		*bufferg_ptr, *bufferb_ptr, *out_ptr;
		//unsigned int	mask;
		//int			*colorBuffer_ptr[MAX_NUM_COMPS];//, *colorBuffer[3];
		size_t		cnt;
		//Kind of Openjpeg Stuff (shound be)
		opj_memory_stream frameBufferStream;
		//OpenJPEG Stuff
		//OPJ_BOOL bSuccess; //пока не используется
		opj_cparameters_t	parameters;	// compression parameters.
		opj_stream_t *l_stream = 00;
		opj_codec_t* l_codec = 00;
		opj_image_cmptparm_t    cmptparms[MAX_NUM_COMPS]; // maximum of 3 components
		opj_image_t		*image = nullptr;
		OPJ_COLOR_SPACE     colorSpace;
		//const char		comment[] = "Created by OpenJPEG version ";
		//size_t		clen, vlen;
		//const char		*version = opj_version();
		OPJ_UINT32 numcomps = 1;

		//Init the clock for warnings.
		//starttime = 0;
		//If debug > 0, get start time and print who.
		/*if (DebugLevel > 0)
		{
			starttime = GetPerformanceCounter();
			SystemDebug.printf("OpenJP2 %s compress started.\n", version);
		}*/
		//Get the data.
		//if (!imageData.GetImageData(pDDO, "OpenJP2 compress", TRUE)) return FALSE;
		////See if its real.
		//if (!imageData.CalculateImageLengths())
		//{//Should of had it, look for arrays
		//	if (!imageData.GetEncapsulatedDDO(NULL)) return FALSE;//no marker
		//}
		////Create the encapsulation array.
		//if (!imageData.CreateOutputArray()) return FALSE;
		////Check and set the quality for lossy.
		//if (j2kQuality < MIN_QUALITY)// Set to 0 to use dicom.ini value.
		//{
		//	j2kQuality = IniValue::GetInstance()->sscscpPtr->LossyQuality;//Use the default or dicom.ini value.
		//}
		//if (j2kQuality > 100) j2kQuality = 100;
		////Make the input standard.
		//if (imageData.components == 1)
		//{
		//	if (!imageData.MakeStdGray()) return FALSE;
		//}
		//else if (!imageData.MakeYBR_FULL_OrRGB(TRUE))return FALSE;
		////Allocate all of the memory needed.
		////Make an frame buffer as big as an uncompressed image to hold the compressed stream.
		//if (!imageData.CreateFrameBuffer())
		//{
		//	OperatorConsole.printf("***[OpenJP2 compress]: Could not allocate an image frame buffer.\n");
		//	return (FALSE);
		//}
		//Set the "OpenJpeg like" stream data.
		//frameBufferStream.pData = (OPJ_UINT8 *)imageData.frameBuffer;
		//frameBufferStream.dataSize = imageData.frameSize;
		//frameBufferStream.offset = 0;
		frameBufferStream.pData = (OPJ_UINT8*)malloc(srcLen * sizeof(OPJ_UINT8));
		frameBufferStream.dataSize = srcLen;
		frameBufferStream.offset = 0;
		//Create comment for codestream buffer, use spareBuffer so imageData will free.
		//clen = strlen(comment);//Made comment above, strnlen not needed.
		//vlen = strnlen(version, 10);
		/*if ((imageData.spareBuffer = (char*)malloc(clen + vlen + 1)) == NULL)
		{
			OperatorConsole.printf("***[OpenJP2 compress]: Could not allocate the comment buffer.\n");
			return (FALSE);
		}*/
		//parameters.cp_comment = imageData.spareBuffer;
		//memcpy(parameters.cp_comment, comment, clen);
		//memcpy(&parameters.cp_comment[clen], version, vlen);
		//parameters.cp_comment[clen + vlen] = 0;
		//Set the parameters. Created from image_to_j2k.c and convert.c.
		//parameters.tcp_mct = imageData.components == 3 ? 1 : 0; //Decide if MCT should be used.
		parameters.tcp_mct = 1; //Decide if MCT should be used.
																//Set encoding parameters values.
		opj_set_default_encoder_parameters(&parameters);
		if (j2kQuality < 100)
		{
			parameters.tcp_rates[0] = 100.0f / j2kQuality;
			// Lossy (ICT) or lossless (RCT)?
			parameters.irreversible = 1;//ICT
		}
		else parameters.tcp_rates[0] = 0;// MOD antonin : losslessbug
		parameters.tcp_numlayers = 1;
		parameters.cp_disto_alloc = 1;
		//Set the image components parameters.
		//for (cnt = 0; cnt < imageData.components; cnt++)
		for (cnt = 0; cnt < numcomps; cnt++)
		{
			cmptparms[cnt].dx = parameters.subsampling_dx;
			cmptparms[cnt].dy = parameters.subsampling_dy;
			cmptparms[cnt].h = cmptparms_in->h;
			cmptparms[cnt].w = cmptparms_in->w;
			cmptparms[cnt].prec = cmptparms_in->prec;
			cmptparms[cnt].bpp = cmptparms_in->bpp;
			cmptparms[cnt].sgnd = cmptparms_in->sgnd;
			cmptparms[cnt].x0 = 0;
			cmptparms[cnt].y0 = 0;
		}
		// Also set the colorspace
		colorSpace = OPJ_CLRSPC_GRAY;// Set the default.
		//if (imageData.components == 3) colorSpace = OPJ_CLRSPC_SRGB;
		// можно оставить
		//Print out some more info for debug.
		//if (DebugLevel > 1) imageData.PrintInfo();
		//mask = ((1 << cmptparms_in->prec) - 1);
		//Frames loop.
		//while (TRUE)
		{
			//Create an image struct.
			image = opj_image_create(numcomps, &cmptparms[0], colorSpace);//4706
			if (image == nullptr) throw runtime_error(msgHdr + "Could not create image object for comress image.");

			// Set image offset and reference grid
			image->x0 = 0;
			image->y0 = 0;
			//image->x1 = (OPJ_UINT32)(imageData.width - 1) *	(OPJ_UINT32)parameters.subsampling_dx + 1;
			//image->y1 = (OPJ_UINT32)(imageData.height - 1) * (OPJ_UINT32)parameters.subsampling_dy + 1;
			image->x1 = (OPJ_UINT32)(cmptparms_in->w - 1) *	(OPJ_UINT32)parameters.subsampling_dx + 1;
			image->y1 = (OPJ_UINT32)(cmptparms_in->h - 1) * (OPJ_UINT32)parameters.subsampling_dy + 1;

			//Almost never used (planes)
			//bufferg_ptr = imageData.inPtr + imageData.singleColorFrameSize;
			//bufferb_ptr = bufferg_ptr + imageData.singleColorFrameSize;
			//bufferg_ptr = imageData.inPtr + imageData.singleColorFrameSize;
			//bufferb_ptr = bufferg_ptr + imageData.singleColorFrameSize;
			//char* mycolorBuffer_ptr[MAX_NUM_COMPS]; //возможно нужен будет при переводе на работу с цветными изображениями
			//(Re)set the buffer pointerss.
			//for (cnt = 0; cnt < image->numcomps; cnt++)
			//	//colorBuffer_ptr[cnt] = (int *)(image->comps[cnt].data);//colorBuffer[cnt];
			//	mycolorBuffer_ptr[cnt] = (char*)(image->comps[cnt].data);//colorBuffer[cnt];
			// Get a J2K compressor handle.
			l_codec = opj_create_compress(OPJ_CODEC_J2K);
			//Catch events using our callbacks and give a local context.
			opj_set_info_handler(l_codec, info_callback, nullptr);
			opj_set_warning_handler(l_codec, warning_callback, nullptr);
			opj_set_error_handler(l_codec, error_callback, nullptr);
			//opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, stderr);
			// Setup the encoder parameters using the current image and user parameters.
			opj_setup_encoder(l_codec, &parameters, image);
			// Open a byte stream for writing.
			l_stream = opj_stream_create_default_memory_stream(&frameBufferStream, OPJ_FALSE);
			if (!(l_stream))
			{
				opj_destroy_codec(l_codec);
				opj_image_destroy(image);
				throw runtime_error(msgHdr + "Failed to allocate output stream memory.\n");
			}
			////Set the color stuff.
			//switch (image->color_space)
			//{
			//case OPJ_CLRSPC_GRAY:
			//	//if (imageData.precision_allocated == 8) // 8 Bits, char to int.
			//	//if (cmptparms_in->bpp == 8) // 8 Bits, char to int.
			//	//{
			//	//	for (byteCnt = 0; byteCnt < imageData.frameSize; byteCnt++)
			//	//	{
			//	//		*colorBuffer_ptr[0]++ = ((int)*imageData.inPtr & mask);
			//	//		imageData.inPtr++;
			//	//	}
			//	//	break;
			//	//}
			//	////16 Bits
			//	////if (imageData.precision > 8)//Byte swap
			//	//if (cmptparms_in->prec > 8)//Byte swap
			//	//{
			//	//	for (byteCnt = 0; byteCnt < srcLen; byteCnt += 2)
			//	//	{
			//	//		*colorBuffer_ptr[0] = ((int)*imageData.inPtr & 0xFF);
			//	//		imageData.inPtr++;
			//	//		*colorBuffer_ptr[0]++ |= (((int)*imageData.inPtr & 0xFF) << 8) & mask;
			//	//		imageData.inPtr++;
			//	//	}
			//	//	break;
			//	//}
			//	mycolorBuffer_ptr[0] = (char*)malloc(srcLen*sizeof(char));
			//	ptr = src.get();
			//	ptrBuf = mycolorBuffer_ptr[0];
			//	//for (byteCnt = 0; byteCnt < srcLen; byteCnt += 2)// 8 in 16.
			//	for (byteCnt = 0; byteCnt < srcLen; byteCnt++)// 8 in 16.
			//	{

			//		//*colorBuffer_ptr[0]++ = ((int)*imageData.inPtr  & mask);
			//		mycolorBuffer_ptr[0][byteCnt] = *ptr++;
			//		//*ptrBuf++ = *ptr++;
			//		//imageData.inPtr++;
			//		//imageData.inPtr++;
			//	}
			//	break;
			//default: //OPJ_CLRSPC_SRGB:
			//	//if (imageData.planes)
			//	//{
			//	//	for (byteCnt = 0; byteCnt < imageData.singleColorFrameSize; byteCnt++)
			//	//	{
			//	//		*(colorBuffer_ptr[0])++ = (int)*imageData.inPtr & mask;
			//	//		imageData.inPtr++;
			//	//		*(colorBuffer_ptr[1])++ = (int)*bufferg_ptr & mask;
			//	//		bufferg_ptr++;
			//	//		*(colorBuffer_ptr[2])++ = (int)*bufferb_ptr & mask;
			//	//		bufferb_ptr++;
			//	//	}
			//	//	break;
			//	//}//Normal RGB
			//	//for (byteCnt = 0; byteCnt < imageData.singleColorFrameSize; byteCnt++)
			//	//{
			//	//	*(colorBuffer_ptr[0])++ = (int)*imageData.inPtr & mask;
			//	//	imageData.inPtr++;
			//	//	*(colorBuffer_ptr[1])++ = (int)*imageData.inPtr & mask;
			//	//	imageData.inPtr++;
			//	//	*(colorBuffer_ptr[2])++ = (int)*imageData.inPtr & mask;
			//	//	imageData.inPtr++;
			//	//}
			//	break;
			//}//Filled the buffers and ready to go.

			// переносим данные исходного изображения в объект изображения, который пойдёт на обработку
			for (size_t cntcomps = 0; cntcomps < image->numcomps; cntcomps++)
			{
				char* ptrSrc = src.get();
				OPJ_INT32* ptrImg = image->comps[cntcomps].data;

				for (size_t cntEl = 0; cntEl < image->y1*image->x1; cntEl++)
				{
					//colorBuffer_ptr[cnt] = (int *)(image->comps[cnt].data);//colorBuffer[cnt];
					//image->comps[cnt].data[cnt] = reinterpret_cast<OPJ_INT32>(mycolorBuffer_ptr[cnt]);//colorBuffer[cnt];
					unsigned char ch[sizeof(*ptrImg)]; //[4]

					size_t byteN = cmptparms_in->bpp / CHAR_BIT;
					for (size_t byteNum = 0; byteNum < byteN; byteNum++)
						ch[byteNum] = (unsigned char)(*ptrSrc++);

					if (cmptparms_in->sgnd)
						*ptrImg = static_cast<char>(ch[byteN - 1]);
					else
						*ptrImg = (ch[byteN - 1]);

					for (size_t byteNum = (byteN - 1); byteNum > 0; byteNum--)
					{
						*ptrImg <<= CHAR_BIT;
						*ptrImg |= ch[byteNum - 1];
					}
					ptrImg++;
				}
			}

			//encode the image.
			if (!opj_start_compress(l_codec, image, l_stream)) throw runtime_error(msgHdr + "Could not start encoding the image.");
			if (!opj_encode(l_codec, l_stream)) throw runtime_error(msgHdr + "Could not encode the image.");
			if (!opj_end_compress(l_codec, l_stream)) throw runtime_error(msgHdr + "Could not end encoding the image.");

			//Done with not reusables.
			opj_image_destroy(image);
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);

			//Jpeg2k is encapsulated, make a new vr to encapsulate.
			//if (!imageData.CreateOutputBuffer(frameBufferStream.offset & (UINT32)-1))//Sets outputImageLengthEven.
			//{
			//	return (FALSE);
			//}
			//Copy the Image data.
			char* buf_ptr = (char *)frameBufferStream.pData;
			dstLen = frameBufferStream.offset;
			dst = make_unique<char[]>(dstLen);
			char* out_ptr = dst.get();
			for (cnt = 0; cnt < dstLen; ++cnt)
				*out_ptr++ = *buf_ptr++;
			//Done with the frameBuffer for now, reset it if more frames
			free(frameBufferStream.pData);
			frameBufferStream.offset = 0;

			////Save the image object.
			//if (!imageData.AddToOutputArray(imageData.outputImageLengthEven))
			//{
			//	opj_image_destroy(image);
			//	return (FALSE);
			//}
			////Done with all of the frames.
			//imageData.currFrame++;
			//if (imageData.currFrame >= imageData.frames) break;
			////If not enough data, look in silly arrays.
			//if (!imageData.CheckEnoughData(FALSE))
			//{//Deal with silly input arrays.
			//	if (!imageData.GetEncapsulatedVR(NULL)) break;//No marker
			//												  //New data, time to fix, make the input standard.
			//	if (imageData.components == 1)
			//	{
			//		if (!imageData.MakeStdGray())
			//		{
			//			return (FALSE);
			//		}
			//	}
			//	else if (!imageData.MakeRGB(TRUE))
			//	{
			//		return (FALSE);
			//	}
			//}
			//if (imageData.planes) imageData.inPtr = bufferb_ptr;
		}//Back for the next frame, end of while(TRUE)
		 //Should we kill it and keep the uncompressed data?
		//imageData.CheckEnoughFrames();
		// проверить
		//Finish encapsulating.
		//imageData.AddToOutputArray((UINT32)-1);
		// реализовано
		//Save the new data.
		//imageData.SwitchToOutputArray();
		// реализовано
		//Change the dicom parameters.
		//Needed for anything that stores <= 8 in 16 (now 8 in 8).
		//imageData.FixPrecision(imageData.precision);
		//на изменение
		//Set color if needed.
		//if (imageData.components > 1)
		//{
		//	//Reset the plane's VR, if there.
		//	imageData.SetPlanesToNone();
		//	//Set the color profile
		//	if ((j2kQuality < 100))
		//		pDDO->ChangeVR(0x0028, 0x0004, "YBR_ICT\0", 'CS');
		//	else pDDO->ChangeVR(0x0028, 0x0004, "YBR_RCT\0", 'CS');
		//}
		// тоже разобраться с куском выше
		//Change the transfer syntax to JPEG2K!
		//if (j2kQuality < 100)
		//	pDDO->ChangeVR(0x0002, 0x0010, "1.2.840.10008.1.2.4.91\0", 'IU');
		//else pDDO->ChangeVR(0x0002, 0x0010, "1.2.840.10008.1.2.4.90\0", 'IU');
		// а вот это интересно (то, что выше)
		// If debug > 0, print when finished
		//if (DebugLevel > 0) SystemDebug.printf("OpenJP2 compress time %lf milliseconds.\n", double((GetPerformanceCounter() - starttime).sec()) * 1000);
		return true;
	}

	bool dcmtkCodec::compressPixelDataJPEG2000(unique_ptr<char[]> &dst, size_t &dst_len, const unique_ptr<char[]> &src, size_t src_len, size_t hs, size_t vs, size_t prec, size_t bpp, bool sign)
	{
		opj_image_cmptparm_t imgParams;
		imgParams.dx = 1; //0;
		imgParams.dy = 1; //0;
		imgParams.w = OPJ_UINT32(hs);
		imgParams.h = OPJ_UINT32(vs);
		imgParams.x0 = 0;
		imgParams.y0 = 0;
		imgParams.prec = OPJ_UINT32(prec);
		imgParams.bpp = OPJ_UINT32(bpp);
		imgParams.sgnd = OPJ_UINT32(sign);

		if (!encodeJPEG2000(dst, dst_len, src, src_len, &imgParams))	throw runtime_error(msgHdr + "Cannot compress image to JPEG2000.");

		return true;
	}

}//namespace Dicom

XRAD_END

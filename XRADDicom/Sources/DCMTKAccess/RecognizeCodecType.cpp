#include "pre.h"

//
#include <XRADDicom/XRADDicom.h>

#include <string>
#include <exception>

using namespace xrad;
using namespace Dicom;


XRAD_BEGIN

//namespace Dicom
//{
//	using namespace xrad;
	using namespace Dicom;

	e_compression_type_t	recognizeCodecType(const string& codec_code_string)
	{
		string tmpStr = codec_code_string;
		if (tmpStr.length() != 0)
			if (tmpStr[tmpStr.length() - 1] != '\0')
				tmpStr = tmpStr.substr(0, codec_code_string.length() - 1);
			else
				tmpStr = tmpStr.substr(0, tmpStr.length() - 2);
		else
			throw invalid_argument("function recognizeCodecType Error: Cannot find TransferSyntaxID (it is empty) " + codec_code_string + ". Cant open an image.");

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

	e_compression_type_t	recognizeCodecType(E_TransferSyntax transfer_syntax_uid_in)
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

//}

XRAD_END
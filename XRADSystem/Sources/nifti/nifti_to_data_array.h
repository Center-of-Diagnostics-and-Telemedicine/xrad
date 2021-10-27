#ifndef __nifti_to_data_array_h
#define __nifti_to_data_array_h

//------------------------------------------------------------------
//
//	created:	2021/10/25	17:11
//	filename: 	nifti_to_data_array.h
//	file path:	q:\Projects\NiftiTest\NiftiTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include <XRADSystem/sources/nifti/nifti_datatypes.h>

#include <XRADSystem/CFile.h>
#include <XRADBasic/ContainersAlgebra.h>

namespace xrad
{

namespace nifti_aux
{

	template<class ROW_T>
	void	realloc_array(DataArrayMD<ROW_T>&arr, const index_vector& sizes)
	{
		XRAD_ASSERT_THROW(sizes.size() >= 3);
		arr.realloc(sizes);
	}

	template<class ROW_T>
	void	realloc_array(DataArray2D<ROW_T>&arr, const index_vector& sizes)
	{
		XRAD_ASSERT_THROW(sizes.size() == 2);
		arr.realloc(sizes[0], sizes[1]);
	}

	template<class VT>
	void	realloc_array(DataArray<VT>&arr, const index_vector& sizes)
	{
		XRAD_ASSERT_THROW(sizes.size() == 1);
		arr.realloc(sizes[0]);
	}


	template<class ARR>
	void load_nifti_util(ARR &result, RealFunctionF64 &scales, wstring filename)
	{
		shared_cfile	header_file(filename, L"rb");
		nifti_1_header hdr;
		index_vector	sizes;
		header_file.read(&hdr, sizeof(nifti_1_header), 1);
		// FILE *file = fopen(name, "rb"); fread(file, buffer, n, 1);

		XRAD_ASSERT_THROW(hdr.sizeof_hdr==sizeof(nifti_1_header));
		XRAD_ASSERT_THROW(hdr.dim[0] < 7);

		scales.realloc(hdr.dim[0]);
		sizes.realloc(hdr.dim[0]);
		std::copy(hdr.dim + 1, hdr.dim + sizes.size()+1, sizes.rbegin());
		std::copy(hdr.pixdim + 1, hdr.pixdim + scales.size()+1, scales.rbegin());

		realloc_array(result, sizes);

		shared_cfile	data_file;

		if(!strcmp(hdr.magic, "n+1"))
		{
			data_file = header_file;
	//		data_file.seek(sizeof(nifti1_extender) + sizeof(nifti_1_header), SEEK_SET);//fseek
			data_file.seek(hdr.vox_offset, SEEK_SET);//fseek
		}
		else if(!strcmp(hdr.magic, "ni1"))
		{
			wstring	img_filename = file_path(filename) + wpath_separator() + filename_without_extension(filename) + L".img";
			data_file.open(img_filename, L"rb");
			data_file.seek(0, SEEK_SET);//fseek
		}



		
		data_file.read_numbers(result, nifti_format_to_io_enum(hdr.datatype, hdr.bitpix));

		//TODO hdr.scl_slope, hdr.scl_inter сейчас никак не учтены. С точки зрения CT тестовый датасет некорректен (intercept должен был бы быть равен -1000 или 1000 или наподобие)
	}
}

/*

enum number_complexity_e
{
	boolean_logical,
	boolean_bitwise,
	scalar,
	complex,
	rgb,
	array,
	array2D,
	arrayMD
};


*/


void	get_nifti_info(const wstring &filename, index_vector &sizes, number_complexity_e &format, size_t bits_per_number)
{
	shared_cfile	header_file(filename, L"rb");
	nifti_1_header hdr;
	header_file.read(&hdr, sizeof(nifti_1_header), 1);
	sizes.realloc(hdr.dim[0]);
	std::copy(hdr.dim + 1, hdr.dim + sizes.size()+1, sizes.rbegin());

	switch(hdr.datatype)
	{
		case DT_UINT8:
		//case DT_UNSIGNED_CHAR:
		case DT_INT8:
		case DT_UINT16:
		case DT_INT16:
		//case DT_SIGNED_SHORT:
		case DT_INT32:
		case DT_UINT32:
		case DT_INT64:
		case DT_UINT64:
		//case DT_FLOAT:
		case DT_FLOAT32:
		//case DT_DOUBLE:
		case DT_FLOAT64:
		case DT_FLOAT128:
			format = scalar;
			bits_per_number = hdr.bitpix;
			break;

		case DT_COMPLEX64:
		case DT_COMPLEX128:
		case DT_COMPLEX256:
			format = complex;
			bits_per_number = hdr.bitpix/2;
			break;

		case DT_RGB24:
		//case DT_RGB:
			format = rgb;
			bits_per_number = hdr.bitpix/3;
			break;

		case DT_RGBA32:
			format = rgba;
			bits_per_number = hdr.bitpix/4;
			break;

		default:
			throw invalid_argument("unknown nifti data format that is not supported yet");
			break;
	}
}

template<class VT>
void load_nifti(DataArray<VT> &result, RealFunctionF64& scales, wstring filename)
{
	nifti_aux::load_nifti_util(result, scales, filename);
}

template<class ROW_T>
void load_nifti(DataArray2D<ROW_T>& result, RealFunctionF64& scales, wstring filename)
{
	nifti_aux::load_nifti_util(result, scales, filename);
}

template<class SLICE_T>
void load_nifti(DataArrayMD<SLICE_T>& result, RealFunctionF64& scales, wstring filename)
{
	nifti_aux::load_nifti_util(result, scales, filename);
}


}//namespace xrad

#endif //__nifti_to_data_array_h

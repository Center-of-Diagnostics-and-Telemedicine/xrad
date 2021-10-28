#ifndef data_array_to_nifti_h__
#define data_array_to_nifti_h__

/*!
	\file
	\date 2019/10/21 17:53
	\author kulberg

	\brief  
*/

#include <XRADSystem/sources/nifti/nifti_datatypes.h>

#include <XRADSystem/ThirdParty/nifti/niftilib/nifti1.h>
//#include <XRADSystem/ThirdParty/nifti/niftilib/nifti1_io.h>

#include <XRADSystem/CFile.h>
#include <XRADBasic/ContainersAlgebra.h>

XRAD_BEGIN

enum
{
	MIN_HEADER_SIZE = 348,
	NII_HEADER_SIZE = 352
};

/*

about hdr.magic
https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/magic.html

"ni1" means that the image data is stored in the ".img" file corresponding
to the header file (starting at file offset 0).

"n+1" means that the image data is stored in the same file as the header
information.  We recommend that the combined header+data filename suffix
be ".nii".  When the dataset is stored in one file, the first byte of image
data is stored at byte location (int)vox_offset in this combined file.
The minimum allowed value of vox_offset is 352; for compatibility with
some software, vox_offset should be an integral multiple of 16.



*/

template<class SLICE>
nifti_1_header CreateNiftiHeader(const DataArrayMD<SLICE> &array, const vector<double> &scales, nifti_file_type type)
{
	size_t	n_dimensions = array.n_dimensions();

	XRAD_ASSERT_THROW_M(n_dimensions <= 7, invalid_argument, "Nifti export, too many image dimensions");
	XRAD_ASSERT_THROW_M(n_dimensions == scales.size(), invalid_argument, "Nifti export, scales.size() != n_dimensions");

	nifti_1_header hdr;

	memset((void *)&hdr, 0, sizeof(hdr));
	hdr.sizeof_hdr = MIN_HEADER_SIZE;

	std::fill(begin(hdr.dim), end(hdr.dim), 1);
	std::fill(begin(hdr.pixdim), end(hdr.pixdim), 1);

	hdr.dim[0] = short(n_dimensions);
	for(size_t i = 0; i < n_dimensions; ++i)
	{
		XRAD_ASSERT_THROW_M(array.sizes(i) <= size_t(max_short()), invalid_argument, "Nifti export, too big image dimension");
		hdr.dim[n_dimensions-i] = short(array.sizes(i));
		hdr.pixdim[n_dimensions-i] = scales[i];
	}

	hdr.datatype = nifti_datatype<typename SLICE::value_type>();
	hdr.bitpix = nifti_sample_size(SLICE::value_type());
	
	hdr.qform_code = 0;
	hdr.sform_code = 2;
	// как-то определяют преобразование координат в пространстве. Подробнее изучать:
	// https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/qsform.html

	hdr.scl_slope = 1.;
	hdr.scl_inter = 0;
	// единичная матрица преобразования координат. прочие поля и четвертый столбец нулевые (memset выше)
	hdr.srow_x[0] = 1;
	hdr.srow_y[1] = 1;
	hdr.srow_z[2] = 1;

	if(n_dimensions >= 4)
	{
		hdr.xyzt_units = NIFTI_UNITS_MM | NIFTI_UNITS_SEC;
	}
	else
	{
		hdr.xyzt_units = NIFTI_UNITS_MM;
	}

	if(type == nifti_file_type::nii)
	{
		strncpy(hdr.magic, "n+1\0", 4);
		hdr.vox_offset = float(NII_HEADER_SIZE);
	}
	else
	{
		strncpy(hdr.magic, "ni1\0", 4);
		hdr.vox_offset = float(0);//https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/vox_offset.html
	}

	hdr.regular    = 'r';

	return hdr;
}



template<class SLICE>
void write_nifti_file(const DataArrayMD<SLICE> &array, wstring filename, nifti_file_type type)
{
	auto hdr = CreateNiftiHeader(array, {2,2,2}, type);


	/********** if nii, write extender pad and image data   */
	if(type == nifti_file_type::nii)
	{
		shared_cfile	nii_writer;
		nii_writer.open(filename + L".nii", L"wb");
		nii_writer.write(&hdr, MIN_HEADER_SIZE, 1);

		nifti1_extender pad={0,0,0,0};
		nii_writer.write(&pad, 4, 1);
		nii_writer.write_numbers(array, nifti_format_to_io_enum(hdr.datatype, hdr.bitpix));
//		nii_writer.write(nifti_data.data(), (size_t)(hdr.bitpix/8), array.element_count());
	}

	/********** if hdr/img, close .hdr and write image data to .img */
	else
	{
		shared_cfile	hdr_writer, img_writer;
		hdr_writer.open(filename + L".hdr", L"wb");
		hdr_writer.write(&hdr, MIN_HEADER_SIZE, 1);

		img_writer.open(filename + L".img", L"wb");
		img_writer.write_numbers(array, nifti_format_to_io_enum(hdr.datatype, hdr.bitpix));
	}
}


XRAD_END

#endif // data_array_to_nifti_h__

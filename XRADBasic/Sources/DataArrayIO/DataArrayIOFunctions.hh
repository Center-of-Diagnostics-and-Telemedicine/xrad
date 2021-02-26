/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------

#ifndef XRAD__File_DataArrayIOFunctions_h
#error This file must be included from "DataArrayIOFunctions.h" only
#endif

XRAD_BEGIN

namespace DataArrayIOAuxiliaries
{

class io_type_does_not_match_data : public std::runtime_error
{
public:
	io_type_does_not_match_data(): std::runtime_error("Invalid I/O data type.") {}
	io_type_does_not_match_data(const char *what) : std::runtime_error(what){}
};

//	чтение текстовых данных прямо из файла с преобразованием порядка байт, если это нужно
template<class read_t, class store_iter>
inline	size_t read_data_text(store_iter data, size_t count, FILE *file)
{
	size_t	read_count(0);

	for(size_t i = 0; i < count; ++i, ++data)
	{
//		*data = store_iter::value_type(read_t::get(file, read_count));
		*data = typename iterator_traits<store_iter>::value_type(read_t::get(file, read_count));
	}
	// read_count прибавляется в get, если данные удалось прочитать
	// get сама присваивает результату 0 при невозможности читать файл (EOF),
	// поэтому второй цикл не требуется

	return read_count;
}

//	чтение двоичных данных из файла через буфер
template<class read_t, class store_iter>
size_t read_data(store_iter data, size_t count, FILE *file)
{
	DataArray<uint8_t> buffer(count*read_t::fsize());

	size_t	read_count = fread(&buffer[0], read_t::fsize(), count, file);

	for(size_t i = 0; i < read_count; ++i, ++data)
	{
//		*data = store_iter::value_type(read_t::get(&buffer[i*read_t::fsize()]));
		*data = typename iterator_traits<store_iter>::value_type(read_t::get(&buffer[i*read_t::fsize()]));
	}
	for(size_t i = read_count; i < count; ++i, ++data)
	{
//		*data = store_iter::value_type(0);
		*data = typename iterator_traits<store_iter>::value_type(0);
	}

	return read_count;
}


//--------------------------------------------------------------

//	запись через буфер
template<class write_t, class const_store_iter>
size_t write_data(const_store_iter data, size_t count, FILE *file)
{
	DataArray<uint8_t> buffer(count*write_t::fsize());

	for(size_t i = 0; i < count; ++i, ++data)
	{
		write_t::put(&buffer[i*write_t::fsize()], typename write_t::value_type(*data));
	}

	size_t	write_count = fwrite(&buffer[0], write_t::fsize(), count, file);
	return write_count;
}

//	запись текстовых данных прямо в файл
template<class write_t, class const_store_iter>
inline	size_t write_data_text(const_store_iter data, size_t count, FILE *file)
{
	size_t	write_count = 0;

	for(size_t i = 0; i < count; ++i, ++data)
	{
		write_count += write_t::put(file, typename write_t::value_type(*data));
	}
	return write_count;
}


// первым аргументом следующих объявлений обозначается формат хранения данных в файле
// вторым - формат внутреннего массива.
#define manage_iotype_case_read(io_enum, io_type)  case io_enum: result = read_data<io_type, store_iter>(data, count, file); break
#define manage_iotype_case_read_text(io_enum, io_type)  case io_enum: result = read_data_text<io_type, store_iter>(data, count, file); break
#define manage_iotype_case_write(io_enum, io_type)  case io_enum: result = write_data<io_type, store_const_iter>(data, count, file); break
#define manage_iotype_case_write_text(io_enum, io_type)  case io_enum: result = write_data_text<io_type, store_const_iter>(data, count, file); break
//undef в конце файла


//--------------------------------------------------------------
//
//	вспомогательные функции для функций ввода/вывода с enum
//
//--------------------------------------------------------------



template< class store_iter>
inline	size_t read_scalar_samples(store_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result = 0;

	switch(number_options)
	{
		manage_iotype_case_read(ioI8, int8_iotype);
		manage_iotype_case_read(ioUI8, uint8_iotype);

		manage_iotype_case_read(ioI16_LE, int16_le_iotype);
		manage_iotype_case_read(ioUI16_LE, uint16_le_iotype);

		manage_iotype_case_read(ioI32_LE, int32_le_iotype);
		manage_iotype_case_read(ioUI32_LE, uint32_le_iotype);

		manage_iotype_case_read(ioF32_LE, float32_le_iotype);
		manage_iotype_case_read(ioF64_LE, float64_le_iotype);

		manage_iotype_case_read(ioI16_BE, int16_be_iotype);
		manage_iotype_case_read(ioUI16_BE, uint16_be_iotype);

		manage_iotype_case_read(ioI32_BE, int32_be_iotype);
		manage_iotype_case_read(ioUI32_BE, uint32_be_iotype);

		manage_iotype_case_read(ioF32_BE, float32_be_iotype);
		manage_iotype_case_read(ioF64_BE, float64_be_iotype);

		manage_iotype_case_read_text(ioScalarText, scalar_text_iotype);

		//
		// error
		//
		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}

//--------------------------------------------------------------

template< class store_const_iter>
inline	size_t write_scalar_samples(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result = 0;
	switch(number_options)
	{
		//
		// scalar and pixel types
		//
		manage_iotype_case_write(ioI8, int8_iotype);
		manage_iotype_case_write(ioUI8, uint8_iotype);

		manage_iotype_case_write(ioI16_LE, int16_le_iotype);
		manage_iotype_case_write(ioI16_BE, int16_be_iotype);
		manage_iotype_case_write(ioI32_LE, int32_le_iotype);
		manage_iotype_case_write(ioI32_BE, int32_be_iotype);

		manage_iotype_case_write(ioUI16_LE, uint16_le_iotype);
		manage_iotype_case_write(ioUI16_BE, uint16_be_iotype);
		manage_iotype_case_write(ioUI32_LE, uint32_le_iotype);
		manage_iotype_case_write(ioUI32_BE, uint32_be_iotype);

		manage_iotype_case_write(ioF32_LE, float32_le_iotype);
		manage_iotype_case_write(ioF32_BE, float32_be_iotype);

		// text
		manage_iotype_case_write_text(ioScalarText, scalar_text_iotype);

		// error
		//
		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}

//--------------------------------------------------------------

template< class store_iter>
inline	size_t read_complex_samples(store_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result = 0;

	switch(number_options)
	{
		manage_iotype_case_read(ioComplexI8, complexI8_iotype);

		manage_iotype_case_read(ioComplexI16_LE, complexI16_le_iotype);
		manage_iotype_case_read(ioComplexI32_LE, complexI32_le_iotype);
		manage_iotype_case_read(ioComplexF32_LE, complexF32_le_iotype);

		manage_iotype_case_read(ioComplexI16_BE, complexI16_be_iotype);
		manage_iotype_case_read(ioComplexI32_BE, complexI32_be_iotype);
		manage_iotype_case_read(ioComplexF32_BE, complexF32_be_iotype);

		manage_iotype_case_read_text(ioComplexText, complex_text_iotype);

		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}

//--------------------------------------------------------------

template< class store_const_iter>
inline	size_t write_complex_samples(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result;

	switch(number_options)
	{
		manage_iotype_case_write(ioComplexI8, complexI8_iotype);

		manage_iotype_case_write(ioComplexI16_LE, complexI16_le_iotype);
		manage_iotype_case_write(ioComplexI32_LE, complexI32_le_iotype);
		manage_iotype_case_write(ioComplexF32_LE, complexF32_le_iotype);

		manage_iotype_case_write(ioComplexI16_BE, complexI16_be_iotype);
		manage_iotype_case_write(ioComplexI32_BE, complexI32_be_iotype);
		manage_iotype_case_write(ioComplexF32_BE, complexF32_be_iotype);

		manage_iotype_case_write_text(ioComplexText, complex_text_iotype);

		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}

//--------------------------------------------------------------

template< class store_iter>
inline	size_t read_rgb_samples(store_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result;
	switch(number_options)
	{
		manage_iotype_case_read(ioRGB_UI8, rgbUI8_iotype);

		manage_iotype_case_read(ioRGB_UI16_LE, rgbUI16_le_iotype);
		manage_iotype_case_read(ioRGB_UI32_LE, rgbUI32_le_iotype);
		manage_iotype_case_read(ioRGB_F32_LE, rgbF32_le_iotype);

		manage_iotype_case_read(ioRGB_UI16_BE, rgbUI16_be_iotype);
		manage_iotype_case_read(ioRGB_UI32_BE, rgbUI32_be_iotype);
		manage_iotype_case_read(ioRGB_F32_BE, rgbF32_be_iotype);

		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}

template< class store_const_iter>
inline	size_t write_rgb_samples(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	size_t result;

	switch(number_options)
	{
		manage_iotype_case_write(ioRGB_UI8, rgbUI8_iotype);

		manage_iotype_case_write(ioRGB_UI16_LE, rgbUI16_le_iotype);
		manage_iotype_case_write(ioRGB_UI32_LE, rgbUI32_le_iotype);
		manage_iotype_case_write(ioRGB_F32_LE, rgbF32_le_iotype);

		manage_iotype_case_write(ioRGB_UI16_BE, rgbUI16_be_iotype);
		manage_iotype_case_write(ioRGB_UI32_BE, rgbUI32_be_iotype);
		manage_iotype_case_write(ioRGB_F32_BE, rgbF32_be_iotype);

		default:
			ForceDebugBreak();
			throw io_type_does_not_match_data();
	}
	return result;
}


#undef manage_iotype_case_read
#undef manage_iotype_case_read_text
#undef manage_iotype_case_write
#undef manage_iotype_case_write_text




//--------------------------------------------------------------
//
//	выбор способа чтения для разной сложности отсчета в памяти.
//	допускается чтение "простых" данных в более сложные (например,
//	действительных в комплексные. это здесь регулируется
//
template< class store_iter>
inline size_t read_numbers_selector(store_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::scalar *)
{
	return read_scalar_samples(data, count, file, number_options);
}

template< class store_iter>
inline	size_t read_numbers_selector(store_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::complex *)
{
	try
	{
		// загрузка комплексного набора данных в комплексный массив
		return read_complex_samples(data, count, file, number_options);
	}
	catch(io_type_does_not_match_data &)
	{
		// попытка загрузить скалярный набор данных в комплексный массив
		return read_scalar_samples(data, count, file, number_options);
	}
}


template< class store_iter>
inline size_t read_numbers_selector(store_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::rgb *)
{
	try
	{
		// загрузка rgb набора данных в rgb массив
		return read_rgb_samples(data, count, file, number_options);
	}
	catch(io_type_does_not_match_data &)
	{
		// попытка загрузить скалярный набор данных в rgb массив
		return read_scalar_samples(data, count, file, number_options);
	}
}

//--------------------------------------------------------------
//
//	выбор способа чтения для разной сложности отсчета в памяти.
//	теоретически возможна запись "простых" данных в виде более сложные (например,
//	действительных в комплексные). но это не прорабатывалось

template< class store_const_iter>
inline size_t write_numbers_selector(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::complex*)
{
	return write_complex_samples(data, count, file, number_options);
}

template< class store_const_iter>
inline size_t write_numbers_selector(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::scalar *)
{
	return write_scalar_samples(data, count, file, number_options);
}

template< class store_const_iter>
inline size_t write_numbers_selector(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options, const number_complexity::rgb *)
{
	return write_rgb_samples(data, count, file, number_options);
}

//--------------------------------------------------------------



}//namespace DataArrayIOAuxiliaries



//--------------------------------------------------------------
//
//	функции ввода/вывода с enum
//
//--------------------------------------------------------------

template< class store_iter>
inline size_t fread_numbers(store_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	try
	{
		return DataArrayIOAuxiliaries::read_numbers_selector<store_iter>(data, count, file, number_options, complexity_t(*data));
	}
	catch(DataArrayIOAuxiliaries::io_type_does_not_match_data &)
	{
		ForceDebugBreak();
		return 0;
	}
}

//--------------------------------------------------------------

template< class store_const_iter>
inline size_t fwrite_numbers(store_const_iter data, size_t count, FILE *file, ioNumberOptions number_options)
{
	try
	{
		return DataArrayIOAuxiliaries::write_numbers_selector<store_const_iter>(data, count, file, number_options, complexity_t(*data));
	}
	catch(DataArrayIOAuxiliaries::io_type_does_not_match_data &)
	{
		ForceDebugBreak();
		return 0;
	}
}


//--------------------------------------------------------------

//	раньше первым аргументом двух нижеследующих функций был DataOwner<>. это
//	неправильно, так как внутренний DataOwner многомерных массивов может иметь
//	разрывные данные, с которыми простой итератор будет работать неправильно.
//	DataArray<> по определению одномерный массив, в котором непрерывность
//	данных гарантирована
//
//	18.05.2010 добавлены аналогичные функции для двумерных массивов. кнс

template<class VT>
inline size_t fread_numbers(DataArray<VT> &data, FILE *file, ioNumberOptions number_options)
{
	return fread_numbers(data.begin(), data.size(), file, number_options);
}

template<class VT>
inline size_t fwrite_numbers(const DataArray<VT> &data, FILE *file, ioNumberOptions number_options)
{
	return fwrite_numbers(data.begin(), data.size(), file, number_options);
}


template<class VT>
inline size_t fread_numbers(DataArray2D<VT> &data, FILE *file, ioNumberOptions number_options)
{
	size_t	result = 0;
	size_t	vs = data.vsize();
	for(size_t i = 0; i < vs; ++i)
	{
		result += fread_numbers(data.row(i), file, number_options);
	}
	return result;
}

template<class VT>
inline size_t fwrite_numbers(const DataArray2D<VT> &data, FILE *file, ioNumberOptions number_options)
{
	size_t	result = 0;
	size_t	vs = data.vsize();

	for(size_t i = 0; i < vs; ++i)
	{
		result += fwrite_numbers(data.row(i), file, number_options);
	}
	return result;
}



XRAD_END

// file DecompositionFFT.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "DecompositionFFT.h"
#include "WinogradShortFFT.h"
#include <XRADBasic/Sources/Utils/ProcessorPoolDispatcher.h>
#include <cstring>
#include <omp.h>

XRAD_BEGIN

namespace DecompositionFFT
{

//--------------------------------------------------------------

namespace
{

size_t revert_bits(size_t i, size_t data_size)
{
	size_t res = 0;
	for (size_t j = data_size; j > 1; j >>= 1, i >>= 1)
		res = (res << 1) + (i & 0x01);
	return res;
}

} // namespace

//--------------------------------------------------------------

DataArray<size_t> MakeRevIndexTable(size_t data_size)
{
	DataArray<size_t> result(data_size);
	for (size_t i = 0; i < data_size; ++i)
	{
		result[i] = revert_bits(i, data_size);
	}
	return result;
}

//--------------------------------------------------------------

template <class complex_t>
void DecompositionFFT(complex_t *data, size_t data_size, ftDirection direction,
		const complex_t *phasors,
		complex_t *buffer,
		const size_t *rev_index_table)
{
	// reordering
	using scalar_type = typename complex_t::scalar_type;
	scalar_type mul = (scalar_type)1./sqrt((scalar_type)data_size);
	if (direction == ftForward)
	{
	 	// forward
	 	for(size_t j=0; j < data_size; ++j)
		{
			//size_t k = rev_0(j, data_size);
			size_t k = rev_index_table[j];
			if (k >= j) // ==: умножение на множитель нужно всегда
			{
				auto tmp = data[k];
				data[k] = mul*data[j];
				data[j] = mul*tmp;
			}
		}
	}
	else
	{
		// reverse
		buffer[0] = mul*data[0];
		for (size_t j=1; j < data_size; ++j)
		{
			//buffer[rev_0(data_size-j, data_size)] = mul*data[j];
			buffer[rev_index_table[data_size-j]] = mul*data[j];
		}
		memcpy(data, buffer, data_size*sizeof(*data));
	}

	// transform
	for (size_t l=1; l < data_size; l <<= 1)
	{
		auto dsl = data_size/(l << 1);
		for (size_t k=0; k < data_size; k += (l << 1))
			for (size_t j=0, j_dsl = 0; j < l; ++j, j_dsl += dsl)
			{
				auto t = data[k+j];
				auto t1 = data[k+j+l]*phasors[j_dsl];
				data[k+j] = t + t1;
				data[k+j+l] = t - t1;
			}
	}
}

//--------------------------------------------------------------

// Инстанциируем реализации шаблонной функции для заданных типов данных

template
void DecompositionFFT(complexF32 *data, size_t data_size, ftDirection direction,
		const complexF32 *phasors,
		complexF32 *buffer,
		const size_t *rev_index_table);

template
void DecompositionFFT(complexF64 *data, size_t data_size, ftDirection direction,
		const complexF64 *phasors,
		complexF64 *buffer,
		const size_t *rev_index_table);

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

class FFTProcessor
{
	public:
		using phasors_t = std::tuple<
				const vector<DataArray<complexF64>>*,
				const vector<DataArray<complexF32>>*>;
		static constexpr size_t max_sample_size = std::max(sizeof(complexF32), sizeof(complexF64));
		using Log = function<void (const string &)>;

		FFTProcessor(size_t max_data_size,
				phasors_t phasors,
				const vector<DataArray<size_t>> *rev_index_tables,
				size_t processor_id,
				shared_ptr<Log> log = nullptr);
		template <class complex_t>
		void operator()(complex_t *data, size_t data_size, size_t power, ftDirection direction);
	private:
		phasors_t phasors;
		const vector<DataArray<size_t>> *rev_index_tables;
		DataArray<char> buffer;
		size_t processor_id;
		shared_ptr<Log> log;
};

FFTProcessor::FFTProcessor(size_t max_data_size,
		phasors_t phasors,
		const vector<DataArray<size_t>> *rev_index_tables,
		size_t processor_id,
		shared_ptr<Log> log):
	phasors(phasors),
	rev_index_tables(rev_index_tables),
	processor_id(processor_id),
	log(std::move(log))
{
	size_t buffer_size = max_data_size * max_sample_size;
	buffer.realloc(buffer_size);
}

template <class complex_t>
void FFTProcessor::operator()(complex_t *data, size_t data_size, size_t power,
		ftDirection direction)
{
	if (log)
	{
		(*log)(ssprintf("FFTProcessor[%zu]::FFT(): id = %s begin\n",
				EnsureType<size_t>(processor_id),
				EnsureType<const char*>(DebugThreadIdStr().c_str())));
	}

	DecompositionFFT(data, data_size, direction,
			(*get<const vector<DataArray<complex_t>>*>(phasors))[power].data(),
			reinterpret_cast<complex_t*>(buffer.data()),
			(*rev_index_tables)[power].data());

	if (log)
	{
		(*log)(ssprintf("FFTProcessor[%zu]::FFT(): id = %s end\n",
				EnsureType<size_t>(processor_id),
				EnsureType<const char*>(DebugThreadIdStr().c_str())));
	}
}

//--------------------------------------------------------------

struct FFTTables
{
	vector<DataArray<complexF64>> phasors_f64;
	vector<DataArray<complexF32>> phasors_f32;
	vector<DataArray<size_t>> rev_index_tables;

	FFTTables(size_t max_data_size, size_t max_power);

	template <class complex_t>
	const complex_t *get_phasors(size_t power) const = delete;
};

template <>
inline const complexF64 *FFTTables::get_phasors(size_t power) const
{
	return phasors_f64[power].data();
}

template <>
inline const complexF32 *FFTTables::get_phasors(size_t power) const
{
	return phasors_f32[power].data();
}

//--------------------------------------------------------------

FFTTables::FFTTables(size_t max_data_size, size_t max_power):
	phasors_f64(max_power+1),
	phasors_f32(max_power+1),
	rev_index_tables(max_power+1)
{
	phasors_f64.back() = ComputePhasors<DataArray<complexF64>>(max_data_size);
	for (auto it = phasors_f64.rbegin();;)
	{
		auto it_next = next(it);
		if (it_next == phasors_f64.rend())
			break;
		auto &phasors = *it;
		auto &phasors_next = *it_next;
		phasors_next.realloc(phasors.size()/2);
		for (size_t i = 0; i != phasors_next.size(); ++i)
		{
			phasors_next[i] = phasors[2*i];
		}
		it = it_next;
	}
	for (size_t i = 0; i < phasors_f64.size(); ++i)
	{
		phasors_f32[i].MakeCopy(phasors_f64[i]);
	}

	size_t data_size = 1;
	for (size_t power = 0; power <= max_power; ++power, data_size <<= 1)
	{
		rev_index_tables[power] = MakeRevIndexTable(data_size);
	}
}

//--------------------------------------------------------------

struct FFTDispatcherWithData
{
	FFTTables tables;
	size_t max_data_size;
	ProcessorPoolDispatcher<FFTProcessor> dispatcher;

	FFTDispatcherWithData(size_t max_data_size, size_t max_power);

	FFTProcessor CreateProcessor(size_t processor_id);
};

//--------------------------------------------------------------

FFTDispatcherWithData::FFTDispatcherWithData(size_t max_data_size, size_t max_power):
	tables(max_data_size, max_power),
	max_data_size(max_data_size),
	dispatcher(omp_get_num_procs(),
			[this](size_t processor_id) { return this->CreateProcessor(processor_id); })
{
}

//--------------------------------------------------------------

FFTProcessor FFTDispatcherWithData::CreateProcessor(size_t processor_id)
{
	return FFTProcessor(max_data_size,
			FFTProcessor::phasors_t(&tables.phasors_f64, &tables.phasors_f32),
			&tables.rev_index_tables,
			processor_id);
}

//--------------------------------------------------------------

template <size_t length>
struct IntegerLog2Lower
{
	enum: size_t { value = 1 + IntegerLog2Lower<(length >> 1)>::value };
};
template <>
struct IntegerLog2Lower<1>
{
	enum: size_t { value = 0 };
};

// \brief Максимально допустимый размер локального буфера БПФ (в стеке)
constexpr size_t MaxLocalBufferSize = 8192;
constexpr size_t FFTTablesMinSampleSize = sizeof(complexF32);

//! \brief Требуемая максимальная длина БПФ для статических таблиц.
//! Это соответствует MaxLocalBufferSize и может быть не степенью двойки
constexpr size_t FFTTablesMaxSize_nn = MaxLocalBufferSize / FFTTablesMinSampleSize;

constexpr size_t FFTTablesMaxPower = IntegerLog2Lower<FFTTablesMaxSize_nn>::value;
constexpr size_t FFTTablesMaxSize = 1 << FFTTablesMaxPower;
//! \brief Нормализованное значение локального буфера для БПФ.
//! Соответствует FFTTablesMaxSize, может быть меньше MaxLocalBufferSize.
constexpr size_t FFTTablesMaxLocalBufferSize = FFTTablesMaxSize * FFTTablesMinSampleSize;

//! \brief Таблицы БПФ для "коротких" длин, доступные всегда, не меняются после инициализации
const FFTTables small_fft_tables(FFTTablesMaxSize, FFTTablesMaxPower);

//! \brief Диспетчер, чтение и запись должны выполняться атомарными функциями
shared_ptr<FFTDispatcherWithData> dispatcher;

//! \brief Мьютекс на создание нового dispatcher (само изменение производится атомарной функцией)
mutex dispatcher_mutex;

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

size_t ceil_fft_length(size_t length, size_t *power_of_2)
{
	if (length <= 1)
	{
		if (power_of_2)
			*power_of_2 = 0;
		return 1;
	}
	length -= 1;
	if (length > (numeric_limits<size_t>::max() >> 1))
		throw length_error(ssprintf("FFT length is too large: %zu.", EnsureType<size_t>(length)));
	size_t res_length = 1;
	size_t res_power = 0;
	for (; length; length >>= 1)
	{
		res_length <<= 1;
		++res_power;
	}
	if (power_of_2)
		*power_of_2 = res_power;
	return res_length;
}

//--------------------------------------------------------------

void InitializeFFT(size_t length)
{
	size_t max_power = 0;
	size_t max_data_size = ceil_fft_length(length, &max_power);
	if (!max_power)
	{
		atomic_store(&dispatcher, shared_ptr<FFTDispatcherWithData>());
		return;
	}

	lock_guard<mutex> lock(dispatcher_mutex);
	if (dispatcher && dispatcher->max_data_size == length)
		return;
	// Замечание: Можно оптимизировать инициализацию для случая уменьшения длины.
	atomic_store(&dispatcher, make_shared<FFTDispatcherWithData>(max_data_size, max_power));
}

//--------------------------------------------------------------

namespace
{
shared_ptr<FFTDispatcherWithData> GetDispatcher()
{
	return atomic_load(&dispatcher);
}
} // namespace

//--------------------------------------------------------------

namespace
{
template <size_t buffer_size, class complex_t>
void DecompositionFFTStaticBuf(complex_t *data, size_t data_size, ftDirection direction,
		const complex_t *phasors,
		const size_t *rev_index_table)
{
	char buffer[buffer_size];
	DecompositionFFT(data, data_size, direction,
			phasors,
			reinterpret_cast<complex_t*>(buffer),
			rev_index_table);
}
} // namespace

//--------------------------------------------------------------

template <class complex_t>
inline void normalize_fft_result(complex_t *array_ptr, size_t size, double factor)
{
	for(auto a0 = array_ptr, a1 = array_ptr + size; a0 < a1; ++a0)
		*a0 *= factor;
}

//--------------------------------------------------------------

const double sqrt_1_2 = sqrt(0.5);
const double sqrt_1_8 = sqrt(0.125);

//--------------------------------------------------------------

template <class complex_t>
inline void FFT_template(complex_t *array, size_t size, ftDirection direction)
{
	// Частные случаи предопределенных коротких преобразований
	switch(size)
	{
		case 2:
			WinogradFFT::fft_2(array, direction);
			normalize_fft_result(array, size, sqrt_1_2);
			return;
		case 4:
			WinogradFFT::fft_4(array, direction);
			normalize_fft_result(array, size, 0.5);
			return;
		case 8:
			WinogradFFT::fft_8(array, direction);
			normalize_fft_result(array, size, sqrt_1_8);
			return;
	}

	size_t power = 0;
	size_t normalized_size = ceil_fft_length(size, &power);
	if (normalized_size != size)
	{
		throw runtime_error(ssprintf("Invalid FFT size: %zu.", EnsureType<size_t>(size)));
	}

	constexpr size_t MaxSmallSize = FFTTablesMaxLocalBufferSize / sizeof(complex_t);
	if (size <= MaxSmallSize)
	{
		// Для малых длин БПФ накладные расходы на синхронизацию потоков превышают время самого БПФ.
		// Поэтому используем локальный буфер.

		// Проверяем, что таблицы small_fft_tables содержат нужные данные:
		static_assert(FFTTablesMaxSize * sizeof(complex_t) >= FFTTablesMaxLocalBufferSize,
				"Invalid static FFT sizes.");

		DecompositionFFTStaticBuf<FFTTablesMaxLocalBufferSize>(array, size, direction,
				small_fft_tables.get_phasors<complex_t>(power),
				small_fft_tables.rev_index_tables[power].data());
	}
	else
	{
		auto local_dispatcher = GetDispatcher();
		if (!local_dispatcher)
			throw runtime_error("FFT is not initialized.");
		if (normalized_size > local_dispatcher->max_data_size)
		{
			throw runtime_error(ssprintf("FFT size exceeds initialized FFT size: %zu > %zu.",
					EnsureType<size_t>(size),
					EnsureType<size_t>(local_dispatcher->max_data_size)));
			// Замечание:
			// Есть возможность вместо исключения переинициализировать dispatcher с нужной длиной FFT.
			// Делать инициализацию нужно в GetDispatcher().
		}

		local_dispatcher->dispatcher.Perform(array, size, power, direction);
	}
}

//--------------------------------------------------------------

void FFT(complexF32 *array, size_t size, ftDirection direction)
{
	FFT_template(array, size, direction);
}

//--------------------------------------------------------------

void FFT(complexF64 *array, size_t size, ftDirection direction)
{
	FFT_template(array, size, direction);
}

//--------------------------------------------------------------

class GlobalInitializer
{
	public:
		GlobalInitializer()
		{
			InitializeFFT(65536);
		}
};

GlobalInitializer global_initializer;

//--------------------------------------------------------------

} // namespace DecompositionFFT

XRAD_END

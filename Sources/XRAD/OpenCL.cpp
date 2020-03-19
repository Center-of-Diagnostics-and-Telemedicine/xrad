// file XRADOpenCL.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "OpenCL.h"
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

#define __CL_ENABLE_EXCEPTIONS
#if defined (__APPLE__) || defined(MACOSX)
	#include <OpenCL/opencl.h>
	#include <OpenCL/cl.hpp>
#else
	#include <CL/opencl.h>
	#include <CL/cl.hpp>
#endif

//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

bool OpenCLInitialized = false;

// Объекты OpenCL нельзя оставлять инициализированными до момента автоматического вызова деструкторов глобальных переменных.
// Это приводит к ошибкам в случае нахождения этих объектов в dll при выгрузке dll (завершении программы).
// Все они должны быть явно деинициализированы в FinishOpenCL().
cl::Device device;
unique_ptr<cl::Context> context;
unique_ptr<cl::Program> program;
unique_ptr<multimap<size_t, unique_ptr<cl::Buffer>>> free_cl_buffers; // buffer depends on context
unique_ptr<mutex> free_cl_buffers_mutex;

unique_ptr<multimap<size_t, unique_ptr<vector<char>>>> free_mem_buffers;
unique_ptr<mutex> free_mem_buffers_mutex;

//--------------------------------------------------------------

vector<string> split(const string &str, const string &delimiter)
{
	vector<string> result;
	for(size_t pos = 0;;)
	{
		size_t end_pos = str.find_first_of(delimiter, pos);
		if( end_pos == str.npos)
			end_pos = str.length();
		if( end_pos != pos)
			result.push_back(str.substr(pos, end_pos - pos));
		if( end_pos == str.length())
			break;
		pos = end_pos + delimiter.length();
	}
	return result;
}

//--------------------------------------------------------------

bool get_device(size_t platform_no, size_t device_no, cl::Device *out_device)
{
	vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if(!platforms.size())
	{
		printf("No OpenCL platform is available.\n");
		return false;
	}
	if(platform_no >= platforms.size())
	{
		printf("Platform index %zu is out of range. Platform count is %zu.\n", platform_no, platforms.size());
		return false;
	}
	vector<cl::Device> devices;
	platforms[platform_no].getDevices(CL_DEVICE_TYPE_ALL, &devices);
	if(device_no >= devices.size())
	{
		printf("Device index %zu is out of range. Device count is %zu.\n", device_no, devices.size());
		return false;
	}
	*out_device = devices[device_no];
	return true;
}

//--------------------------------------------------------------

string opencl_program_source()
{
	return
			R"cl_code(
				__kernel void BiexpBlur2D(__global float* data, int count_x, int count_y, int step_x, int step_y, float a)
				{
					int j = get_global_id(0);
					__global float *row = data + j*step_y;
					for( int i=1; i<count_x; ++i)
					{
						row[i*step_x] += (row[i*step_x] - row[(i-1)*step_x])*a;
					}
					for( int i=count_x; i-- > 1;)
					{
						row[(i-1)*step_x] += (row[(i-1)*step_x] - row[i*step_x])*a;
					}
				}
			)cl_code";
}

//--------------------------------------------------------------

class cl_buffer
{
	public:
		cl_buffer(unique_ptr<cl::Buffer> &&buffer):
				buffer(std::move(buffer))
		{
		}
		cl_buffer(cl_buffer &&other):
				buffer(std::move(other.buffer))
		{
		}
		~cl_buffer();
		inline cl::Buffer &operator*() { return *buffer; }
	private:
		unique_ptr<cl::Buffer> buffer;
};

//! \brief Get an OpenCL buffer from the buffer pool, if possible, or allocate a new buffer.
//!   The buffer will be returned to the pool when the cl_buffer<T> is destroyed
cl_buffer get_cl_buffer(size_t buf_size)
{
	lock_guard<mutex> buffer_lock(*free_cl_buffers_mutex);
	if(free_cl_buffers)
	{
		auto it = free_cl_buffers->lower_bound(buf_size);
		if(it != free_cl_buffers->end())
		{
			unique_ptr<cl::Buffer> buffer = std::move(it->second);
			free_cl_buffers->erase(it);
			return buffer;
		}
		// it == end()
		if(!free_cl_buffers->empty())
		{
			// Reallocate a smaller buffer. Simply delete the smaller buffer and return a new one.
			--it;
			free_cl_buffers->erase(it);
		}
	}
	return make_unique<cl::Buffer>(*context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, buf_size);
}

void release_cl_buffer(unique_ptr<cl::Buffer> buffer)
{
	if(!buffer)
		return;
	size_t buffer_size = buffer->getInfo<CL_MEM_SIZE>(NULL);
	lock_guard<mutex> buffer_lock(*free_cl_buffers_mutex);
	if(!free_cl_buffers)
	{
		free_cl_buffers = make_unique<remove_reference_t<decltype(*free_cl_buffers)> >();
	}
	free_cl_buffers->insert(make_pair(buffer_size, std::move(buffer)));
}

inline cl_buffer::~cl_buffer()
{
	if(buffer) // the data hasn't been moved to another mem_buffer
		release_cl_buffer(std::move(buffer));
}

//--------------------------------------------------------------

template<class T>
class mem_buffer
{
	public:
		mem_buffer(unique_ptr<vector<char>> &&buffer):
				buffer(std::move(buffer))
		{
		}
		mem_buffer(mem_buffer &&other):
				buffer(std::move(other.buffer))
		{
		}
		~mem_buffer();
		inline T *data() { return (T*)buffer->data(); }
	private:
		unique_ptr<vector<char>> buffer;
};

//! \brief Get a memory buffer from the memory buffer pool, if possible, or allocate a new buffer.
//!   The buffer will be returned to the pool when the mem_buffer<T> is destroyed
template<class T>
mem_buffer<T> get_mem_buffer(size_t t_element_count)
{
	size_t buf_size = t_element_count * sizeof(T);
	lock_guard<mutex> buffer_lock(*free_mem_buffers_mutex);
	if(free_mem_buffers)
	{
		auto it = free_mem_buffers->lower_bound(buf_size);
		if(it != free_mem_buffers->end())
		{
			unique_ptr<vector<char>> buffer = std::move(it->second);
			free_mem_buffers->erase(it);
			return buffer;
		}
		// it == end()
		if(!free_mem_buffers->empty())
		{
			// Reallocate a smaller cached buffer. Simply delete the smaller buffer and return a new one.
			--it;
			free_mem_buffers->erase(it);
		}
	}
	return mem_buffer<T>(make_unique<vector<char>>(buf_size));
}

//! \brief Return a memory buffer to the free memory buffer pool. Don't call it directly, use mem_buffer<T>
void release_mem_buffer(unique_ptr<vector<char>> buffer)
{
	if(!buffer)
		return;
	size_t buffer_size = buffer->size();
	lock_guard<mutex> buffer_lock(*free_mem_buffers_mutex);
	if(!free_mem_buffers)
	{
		free_mem_buffers = make_unique<remove_reference_t<decltype(*free_mem_buffers)>>();
	}
	free_mem_buffers->insert(make_pair(buffer_size, std::move(buffer)));
}

template<class T>
inline mem_buffer<T>::~mem_buffer()
{
	if(buffer) // the data hasn't been moved to another mem_buffer
		release_mem_buffer(std::move(buffer));
}

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

void InitOpenCL(const std::string &params)
{
	if( OpenCLInitialized)
		return;
	size_t platform_no = 0;
	size_t device_no = 0;
	auto params_split = split(params, " ");
	for(auto param: params_split)
	{
		const char *param_c = param.c_str();
		if( !strncmp(param_c, "device=", sizeof("device=")-1))
		{
			int platform_i = -1;
			int device_i = -1;
			sscanf(param_c + (sizeof("device=") - 1), "%i,%i", &platform_i, &device_i);
			if( platform_i >= 0 && device_i >= 0)
			{
				platform_no = platform_i;
				device_no = device_i;
			}
		}
	}
	if (!get_device(platform_no, device_no, &device))
		return;
	context = make_unique<cl::Context>(device);
	program = make_unique<cl::Program>(*context, opencl_program_source(), true);
	free_cl_buffers_mutex = make_unique<mutex>();
	free_mem_buffers_mutex = make_unique<mutex>();
	OpenCLInitialized = true;
}

//--------------------------------------------------------------

void FinishOpenCL()
{
	if(!OpenCLInitialized)
		return;
	FreeOpenCLBuffers();
	free_mem_buffers_mutex.reset();
	free_cl_buffers_mutex.reset();
	program.reset();
	context.reset();
	device = cl::Device();
	OpenCLInitialized = false;
}

//--------------------------------------------------------------

bool OpenCLEnabled()
{
	return OpenCLInitialized;
}

//--------------------------------------------------------------

void FreeOpenCLBuffers()
{
	free_mem_buffers.reset();
	free_cl_buffers.reset();
}

//--------------------------------------------------------------

namespace
{

void BiexpBlur2D_OpenCL_float(float *data, size_t element_count_x, size_t element_count_y, double rx, double ry)
{
	size_t data_size = sizeof(cl_float) * element_count_x * element_count_y;
	size_t global_work_size_x = element_count_y;
	size_t global_work_size_y = element_count_x;
	cl_buffer buffer = get_cl_buffer(data_size);
	float normalizer_factor = multiply_normalizer(float(0));
	float ax = rx? ExponentialFlterCoefficient(rx)*normalizer_factor: 0;
	float ay = ry? ExponentialFlterCoefficient(ry)*normalizer_factor: 0;
	cl::CommandQueue queue(*context, device);
	cl::Kernel kernel_x(*program, "BiexpBlur2D");
	kernel_x.setArg(0, *buffer);
	kernel_x.setArg<int>(1, (int)element_count_x);
	kernel_x.setArg<int>(2, (int)element_count_y);
	kernel_x.setArg<int>(3, (int)1);
	kernel_x.setArg<int>(4, (int)element_count_x);
	kernel_x.setArg<float>(5, ax);
	cl::Kernel kernel_y(*program, "BiexpBlur2D");
	kernel_y.setArg(0, *buffer);
	kernel_y.setArg<int>(1, (int)element_count_y);
	kernel_y.setArg<int>(2, (int)element_count_x);
	kernel_y.setArg<int>(3, (int)element_count_x);
	kernel_y.setArg<int>(4, (int)1);
	kernel_y.setArg<float>(5, ay);
	queue.enqueueWriteBuffer(*buffer, CL_FALSE, 0, data_size, data);
	if( rx)
		queue.enqueueNDRangeKernel(kernel_x, cl::NullRange, cl::NDRange(global_work_size_x), cl::NullRange);
	if( ry)
		queue.enqueueNDRangeKernel(kernel_y, cl::NullRange, cl::NDRange(global_work_size_y), cl::NullRange);
	queue.enqueueReadBuffer(*buffer, CL_FALSE, 0, data_size, data);
	queue.finish();
}

} // namespace

//--------------------------------------------------------------

void Implementation::BiexpBlur2D_OpenCL_raw(float *data, size_t vsize, size_t hsize, ptrdiff_t vstep, ptrdiff_t hstep, double radius_v, double radius_h)
{
	if( vstep == 1 && hstep == ptrdiff_t(vsize))
	{
		BiexpBlur2D_OpenCL_float(data, vsize, hsize, radius_v, radius_h);
	}
	else if( hstep == 1 && vstep == ptrdiff_t(hsize))
	{
		BiexpBlur2D_OpenCL_float(data, hsize, vsize, radius_h, radius_v);
	}
	else
	{
		auto mem_buffer = get_mem_buffer<float>(hsize*vsize);
		if( hstep < vstep)
		{
			{
				const float *raw_data = data;
				float *buffer_data = mem_buffer.data();
				for (size_t v = 0; v < vsize; ++v, raw_data += vstep)
				{
					const float *raw_data_line = raw_data;
					for (size_t h = 0; h < hsize; ++h, raw_data_line += hstep)
					{
						*buffer_data++ = *raw_data_line;
					}
				}
			}
			BiexpBlur2D_OpenCL_float(mem_buffer.data(), hsize, vsize, radius_h, radius_v);
			{
				const float *buffer_data = mem_buffer.data();
				float *raw_data = data;
				for (size_t v = 0; v < vsize; ++v, raw_data += vstep)
				{
					float *raw_data_line = raw_data;
					for (size_t h = 0; h < hsize; ++h, raw_data_line += hstep)
					{
						*raw_data_line = *buffer_data++;
					}
				}
			}
		}
		else
		{
			{
				const float *raw_data = data;
				float *buffer_data = mem_buffer.data();
				for (size_t h = 0; h < hsize; ++h, raw_data += hstep)
				{
					const float *raw_data_line = raw_data;
					for (size_t v = 0; v < vsize; ++v, raw_data_line += vstep)
					{
						*buffer_data++ = *raw_data_line;
					}
				}
			}
			BiexpBlur2D_OpenCL_float(mem_buffer.data(), vsize, hsize, radius_v, radius_h);
			{
				const float *buffer_data = mem_buffer.data();
				float *raw_data = data;
				for (size_t h = 0; h < hsize; ++h, raw_data += hstep)
				{
					float *raw_data_line = raw_data;
					for (size_t v = 0; v < vsize; ++v, raw_data_line += vstep)
					{
						*raw_data_line = *buffer_data++;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------

XRAD_END

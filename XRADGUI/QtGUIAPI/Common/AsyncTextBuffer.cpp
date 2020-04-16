// file AsyncTextBuffer.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "AsyncTextBuffer.h"

namespace XRAD_GUI
{

//--------------------------------------------------------------

AsyncTextBuffer::AsyncTextBuffer(function<void ()> buffer_ready_mt, size_t buffer_size):
	buffer_size(buffer_size),
	buffer_ready_mt(buffer_ready_mt), buffer(buffer_size, 0),
	read_pos(0), write_pos(0)
{
	buffer_data = &buffer.front();
}

//--------------------------------------------------------------

string AsyncTextBuffer::ReadBufferMT()
{
	lock_guard<mutex> lock(buffer_mutex);
	auto local_read_pos = read_pos;
	auto local_write_pos = write_pos;
	if (local_read_pos == local_write_pos)
	{
		return string();
	}
	const char *nv_buffer_data = const_cast<const char*>(buffer_data);
	if (local_read_pos < local_write_pos)
	{
		string result(nv_buffer_data + local_read_pos, local_write_pos - local_read_pos);
		read_pos = local_write_pos;
		return result;
	}
	else
	{
		size_t size1 = buffer.size() - local_read_pos;
		string result(size1 + local_write_pos, 0);
		result.replace(0, size1, nv_buffer_data + local_read_pos, size1);
		result.replace(size1, local_write_pos, nv_buffer_data, local_write_pos);
		read_pos = local_write_pos;
		return result;
	}
}

//--------------------------------------------------------------

bool AsyncTextBuffer::AppendBufferMT(char c)
{
	size_t text_length = 0;
	{
		lock_guard<mutex> lock(buffer_mutex);
		auto local_write_pos = write_pos;
		auto local_read_pos = read_pos;
		size_t next_write_pos = local_write_pos + 1;
		if (next_write_pos == buffer.size())
			next_write_pos = 0;
		if (next_write_pos == local_read_pos)
			return false;
		buffer_data[local_write_pos] = c;
		write_pos = next_write_pos;
		if (local_read_pos < local_write_pos)
		{
			text_length = local_write_pos - local_read_pos;
		}
		else
		{
			text_length = buffer.size() - local_read_pos + local_write_pos;
		}
	}
	if (c == '\n' || c == '\r' || text_length * 2 >= buffer.size())
		buffer_ready_mt();
	return true;
}

//--------------------------------------------------------------

void AsyncTextBuffer::ReadyMT()
{
	buffer_ready_mt();
}

//--------------------------------------------------------------

} // namespace XRAD_GUI

#include "wstream.h"
#include "wbuffer.h"
#include "wint2str.h"
#include <string.h>

namespace wang {	

	static const int MAX_NUMERIC_SIZE = 32;

	wstream::wstream() : m_buf_ptr(0)
	{

	}
	wstream::wstream(wbuffer* buf_ptr) : m_buf_ptr(buf_ptr)
	{

	}

	wstream::~wstream()
	{
		
	}

	void wstream::init(wbuffer* buf_ptr)
	{
		m_buf_ptr = buf_ptr;
	}

	void wstream::append(const char * p, int len)
	{
		m_buf_ptr->write(p, len);
	}

	template<typename T>
	static void stream_serial_int(wbuffer& buf, T& value)
	{		
		//uint64 === 20 bit
		if (buf.avail() > MAX_NUMERIC_SIZE)
		{
			int size = int2str_dec(buf.get_buf() + buf.get_size(), value);
			buf.skip_no_check(size);
		}
		else
		{
			buf.set_error();
		}
	}

	wstream& wstream::operator<<(bool value)
	{
		if (value)
		{
			return *this << '1';
		}
		else
		{
			return *this << '0';
		}
	}

	wstream& wstream::operator<<(char value)
	{
		m_buf_ptr->write(&value, sizeof(value));
		return *this;
	}

	wstream& wstream::operator<<(signed char value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(unsigned char value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}

	wstream& wstream::operator<<(signed short value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(unsigned short value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}

	wstream& wstream::operator<<(signed int value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(unsigned int value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}

	wstream& wstream::operator<<(signed long value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(unsigned long value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}

	wstream& wstream::operator<<(signed long long value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(unsigned long long value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}

	wstream& wstream::operator<<(const char * value)
	{
		m_buf_ptr->write(value, strlen(value));
		return *this;
	}

	wstream& wstream::operator<<(const std::string& value)
	{
		m_buf_ptr->write(value.data(), value.length());
		return *this;
	}

	wstream& wstream::operator<<(float value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
	wstream& wstream::operator<<(double value)
	{
		stream_serial_int(*m_buf_ptr, value);
		return *this;
	}
}


#include "wcycle_buffer.h"
#include <string.h>

namespace wang
{

	wcycle_buffer::wcycle_buffer() : m_buf(NULL)
		, m_buf_size(0)
		, m_head(0)
		, m_tail(0)
		, m_cur_len(0)
	{

	}

	wcycle_buffer::wcycle_buffer(char * buf, unsigned int size) : m_buf(buf)
		, m_buf_size(size)
		, m_head(0)
		, m_tail(0)
		, m_cur_len(0)
	{

	}

	wcycle_buffer::~wcycle_buffer()
	{

	}

	bool wcycle_buffer::init(char * buf, unsigned int size)
	{
		m_buf = buf;

		m_buf_size = size;

		return true;
	}

	void wcycle_buffer::reset()
	{
		m_head = 0;
		m_tail = 0;
		m_cur_len = 0;
	}

	bool wcycle_buffer::peek(void* buf, unsigned int len)
	{
		if (len > length())
		{
			return false;
		}

		{
			unsigned int right_len = m_buf_size - m_head;
			if (len <= right_len)
			{
				memcpy(buf, &m_buf[m_head], len);
			}
			else
			{
				memcpy(buf, &m_buf[m_head], right_len);
				memcpy((char*)buf + right_len, &m_buf[0], len - right_len);
			}
		}

		return true;
	}

	bool wcycle_buffer::skip(unsigned int len)
	{
		if (len > length())
		{
			return false;
		}

		m_head = (m_head + len) % m_buf_size;
		m_cur_len -= len;

		return true;
	}

	bool wcycle_buffer::read(void* buf, unsigned int len)
	{
		if (peek(buf, len))
		{
			m_head = (m_head + len) % m_buf_size;
			m_cur_len -= len;
			return true;
		}
		return false;
	}

	bool wcycle_buffer::write(const void* buf, unsigned int len)
	{
		if (0 == len)
		{
			return true;
		}

		if (len > avail())
		{
			return false;
		}

		unsigned int right_len = m_buf_size - m_tail;
		if (len <= right_len)
		{
			memcpy(&m_buf[m_tail], buf, len);
		}
		else
		{
			memcpy(&m_buf[m_tail], buf, right_len);
			memcpy(m_buf, (const char*)buf + right_len, len - right_len);
		}

		m_tail = (m_tail + len) % m_buf_size;

		m_cur_len += len;

		return true;
	}
}
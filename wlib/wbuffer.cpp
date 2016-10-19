#include "wbuffer.h"
#include <string.h>

namespace wang {

	wbuffer::wbuffer() : m_buf(NULL)
		, m_buf_size(0)
		, m_pos(0)
		, m_error(false)
	{

	}

	wbuffer::wbuffer(char * buf, unsigned int size) : m_buf(buf)
		, m_buf_size(size)
		, m_pos(0)
		, m_error(false)
	{
		
	}

	wbuffer::~wbuffer()
	{

	}
	
	void wbuffer::init(char * buf, unsigned int size)
	{
		m_buf = buf;
		m_buf_size = size;
	}

	void wbuffer::reset()
	{
		m_pos = 0;
		m_error = false;
	}

	void wbuffer::write(const void* buf, unsigned int len)
	{
		if (!m_error && m_pos + len <= m_buf_size)
		{
			memcpy(m_buf + m_pos, buf, len);
			m_pos += len;
			return;
		}
		m_error = true;
	}

	void wbuffer::write(unsigned int pos, const void* buf, unsigned int len)
	{
		if (!m_error && pos + len <= m_buf_size)
		{
			memcpy(m_buf + pos, buf, len);
			return;
		}
		m_error = true;
	}

	void wbuffer::skip(unsigned int len)
	{
		if (!m_error && m_pos + len <= m_buf_size)
		{
			skip_no_check(len);
			return;
		}
		m_error = true;
	}
}


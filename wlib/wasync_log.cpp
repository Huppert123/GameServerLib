#include <functional>
#include <iostream>
#include <string.h>
#include "wasync_log.h"

#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif

namespace wang
{
	wasync_log::wasync_log() : m_stop(false)
		, m_max_buf_size_MB(0)
		, m_show_screen(false)
	{

	}

	wasync_log::~wasync_log()
	{
		destroy();
	}

	bool wasync_log::init(const std::string& filename, int max_buf_size_MB, bool show_screen)
	{
		if (!filename.empty())
		{
			m_file.open(filename.c_str());
			if (!m_file.is_open())
			{
				std::cerr << "open log file " << filename << " failed" << std::endl;
				return false;
			}
		}

		//m_cur_buf = std::make_unique<wlog_buf>();
		m_cur_buf.reset(new wlog_buf());

		//m_avail_bufs.emplace(std::make_unique<wlog_buf>());
		m_avail_bufs.emplace(new wlog_buf());

		m_filename = filename;

		m_max_buf_size_MB = max_buf_size_MB;

		m_show_screen = show_screen;

		std::thread td(std::bind(&wasync_log::_work_thread, this));
		m_thread.swap(td);

		return true;
	}

	void wasync_log::destroy()
	{
		{
			std::lock_guard<std::mutex> lock(m_lock);
			if (m_stop)
			{
				return;
			}
			m_stop = true;
		}
		m_condition.notify_all();

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		if (m_file.is_open())
		{
			m_file.close();
		}

		if (m_show_screen)
		{
			std::cout.flush();
		}

		m_cur_buf.reset();
		
		while (!m_avail_bufs.empty())
		{
			m_avail_bufs.pop();
		}
	}

	void wasync_log::append(const char* str, unsigned int len)
	{
		if (len >= wlog_buf::MAX_SIZE)
		{
			return;
		}
		{
			std::lock_guard<std::mutex> lock(m_lock);
			if (m_stop)
			{
				return;
			}
			if (wlog_buf::MAX_SIZE - m_cur_buf->size <= len)
			{
				if (m_max_buf_size_MB != 0 && m_data_bufs.size() > m_max_buf_size_MB)
				{
					return;
				}
				m_data_bufs.push(std::move(m_cur_buf));
				if (!m_avail_bufs.empty())
				{
					m_cur_buf = std::move(m_avail_bufs.front());
					m_avail_bufs.pop();
				}
				else
				{
					//m_cur_buf = std::make_unique<wlog_buf>();
					m_cur_buf.reset(new wlog_buf());
				}
			}
			memcpy(m_cur_buf->buf + m_cur_buf->size, str, len);
			m_cur_buf->size += len;
			m_cur_buf->buf[m_cur_buf->size] = 0;
		}
		{
			m_condition.notify_one();
		}
	}

	void wasync_log::_work_thread()
	{
		wlog_buf_ptr buf;
		while (true)
		{
			if (!buf)
			{
				std::unique_lock<std::mutex> lock{ m_lock };
				m_condition.wait(lock, [this]() { return !m_data_bufs.empty() || m_cur_buf->size > 0 || m_stop; });

				_get_next_data_buf(buf);
				if (!buf)
				{
					break;
				}
			}

			if (m_file.is_open())
			{
				m_file.write(buf->buf, buf->size);
			}
			if (m_show_screen)
			{
				std::cout.write(buf->buf, buf->size);
			}

			buf->size = 0;
			buf->buf[0] = 0;

			{
				std::lock_guard<std::mutex> lock{ m_lock };
				m_avail_bufs.push(std::move(buf));
				_get_next_data_buf(buf);
			}
			if (!buf)
			{
				if (m_file.is_open())
				{
					m_file.flush();
				}
				if (m_show_screen)
				{
					std::cout.flush();
				}
			}
		}
	}

	void wasync_log::_get_next_data_buf(wlog_buf_ptr& buf)
	{
		if (!m_data_bufs.empty())
		{
			buf = std::move(m_data_bufs.front());
			m_data_bufs.pop();
		}
		else if (m_cur_buf->size > 0)
		{
			buf = std::move(m_cur_buf);
			m_cur_buf = std::move(m_avail_bufs.front());
			m_avail_bufs.pop();
		}
	}

} // namespace wang
/********************************************************************
	created:	2016/08/15

	author:		wanghaibo
	
	purpose:	async log
*********************************************************************/

#ifndef _W_ASYNC_LOG_H__ 
#define _W_ASYNC_LOG_H__

#include <thread>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <string>

namespace wang
{
	class wasync_log
	{
	private:
		struct wlog_buf
		{
			static const unsigned int MAX_SIZE = 1024 * 1024;
			char		 buf[MAX_SIZE];
			unsigned int size;
		};
	
		typedef std::condition_variable wcond;
		typedef std::unique_ptr<wlog_buf> wlog_buf_ptr;
		typedef std::queue<wlog_buf_ptr> wbuf_queue;

	public:
		wasync_log();
		~wasync_log();

		bool init(const std::string& filename, int max_buf_size_MB, bool show_screen);
		void destroy();

		void append(const char* str, unsigned int len);

	private:
		void _work_thread();
		void _get_next_data_buf(wlog_buf_ptr& buf);

	private:
		wasync_log(const wasync_log&);
		wasync_log& operator=(const wasync_log&);

	private:
		std::ofstream	m_file;
		std::thread		m_thread;
		bool			m_stop;		

		std::mutex		m_lock;
		wcond			m_condition;
		wlog_buf_ptr	m_cur_buf;
		wbuf_queue		m_avail_bufs;
		wbuf_queue		m_data_bufs;

		std::string		m_filename;
		size_t			m_max_buf_size_MB;
		bool			m_show_screen;
	};
}

#endif
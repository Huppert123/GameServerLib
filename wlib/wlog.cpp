#include "wlog.h"
#include "wasync_log.h"
#include "wtime_api.h"

namespace wang
{
	static std::unique_ptr<wasync_log> s_log;

	namespace wlog_util
	{
		struct wlog_level_name
		{
			const char * p;
			int len;
			wlog_level_name():p(NULL), len(0){}
			wlog_level_name(const char * _p, int _len) : p(_p), len(_len){}
		};
		const wlog_level_name g_log_level_names[wlog::ENum] =
		{
			{ "FATAL", 5},
			{ "ERROR", 5},
			{ "WARN" , 4},
			{ "INFO" , 4},
			{ "DEBUG", 5},
		};

		void log_prefix(wstream& s, int level)
		{
			char time_str[ASCII_DATETIME_LEN];
			wtime_api::time64_format(time(NULL), '-', ' ', ':', time_str, false);
			s << '[';
			s.append(time_str, sizeof(time_str) - 1);
			s << "] [";
			s.append(g_log_level_names[level].p, g_log_level_names[level].len);
			s << "] ";
		}
	}

	wlog::wlog()
		: m_buf(m_data, sizeof(m_data))
		, m_stream(&m_buf)
	{
		
	}

	wlog::wlog(int level)
		: m_buf(m_data, sizeof(m_data))
		, m_stream(&m_buf)
	{
		wlog_util::log_prefix(m_stream, level);
	}

	wlog::wlog(const char* func, int line, int level)
		: m_buf(m_data, sizeof(m_data))
		, m_stream(&m_buf)
	{
		wlog_util::log_prefix(m_stream, level);
		m_stream << func << ':' << line << ' ';
	}

	wlog::~wlog()
	{
		m_stream << '\n';
		s_log->append(m_data, m_buf.get_size());
	}

	bool wlog::init(const std::string& filename, int max_buf_size_MB, bool show_screen)
	{
		std::string path;
		std::string name;
		
		std::string::size_type n = filename.find_last_of("\\/");
		if (n != std::string::npos)
		{
			path = filename.substr(0, n);
			name = filename.substr(n + 1);
		}
		else
		{
			name = filename;
		}

		char time_str[ASCII_DATETIME_LEN];
		wtime_api::time64_format(time(NULL), '-', '_', '-', time_str, false);

		std::string full_filename(path);
		full_filename.append(1, '/');
		full_filename.append(time_str, sizeof(time_str) - 1);
		full_filename.append(1, '_');
		full_filename += name;

		//s_log = std::make_unique<wasync_log>();
		s_log.reset(new wasync_log());
		return s_log->init(full_filename, max_buf_size_MB, show_screen);
	}

	void wlog::destroy()
	{
		if (s_log)
		{
			s_log->destroy();
			s_log.reset();
		}
	}
}
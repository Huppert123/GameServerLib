#ifndef _W_LOG_H__
#define _W_LOG_H__

#include "wbuffer.h"
#include "wstream.h"

namespace wang
{
	class wlog
	{
	public:
		enum ELogLevel
		{
			EFatal = 0,
			EError,
			EWarn,
			EInfo,
			EDebug,
			ENum,
		};

		wlog();
		explicit wlog(int level);
		explicit wlog(const char* func, int line, int level);
		~wlog();

		static bool init(const std::string& filename, int max_buf_size_MB, bool show_screen);
		static void destroy();

		wstream& stream() { return m_stream; }

	private:
		wlog(const wlog&);
		wlog& operator=(const wlog&);

	private:
		char m_data[2048];
		wbuffer m_buf;
		wstream m_stream;		
	};
}

#define SLOG wang::wlog().stream()

#define SLOG_DEBUG wang::wlog(wang::wlog::EDebug).stream()
#define SLOG_INFO  wang::wlog(wang::wlog::EInfo ).stream()
#define SLOG_WARN  wang::wlog(wang::wlog::EWarn ).stream()
#define SLOG_ERROR wang::wlog(wang::wlog::EError).stream()
#define SLOG_FATAL wang::wlog(wang::wlog::EFatal).stream()

#define LOG_DEBUG wang::wlog(__FUNCTION__, __LINE__, wang::wlog::EDebug).stream()
#define LOG_INFO  wang::wlog(__FUNCTION__, __LINE__, wang::wlog::EInfo ).stream()
#define LOG_WARN  wang::wlog(__FUNCTION__, __LINE__, wang::wlog::EWarn ).stream()
#define LOG_ERROR wang::wlog(__FUNCTION__, __LINE__, wang::wlog::EError).stream()
#define LOG_FATAL wang::wlog(__FUNCTION__, __LINE__, wang::wlog::EFatal).stream()

#endif  // _W_LOG_H__

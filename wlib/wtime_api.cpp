#include <stdio.h>
#include <chrono>
#include "wtime_api.h"
#include "wtime_const.h"

#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif

namespace wang {

	namespace wtime_base_api
	{
#if defined (_MSC_VER)
		static long long g_time_zone = _timezone;
#elif defined (__GNUC__)
		static long long g_time_zone = __timezone;
#else
#pragma error "unknow platform!!!"
#endif
		static const std::chrono::steady_clock::time_point g_start_time = std::chrono::steady_clock::now();

		void set_time_zone(int value)
		{
			g_time_zone = value * ETC_Hour;
		}

		long long get_time_ms()
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - g_start_time).count();
		}

		time_t get_gmt()
		{
			return ::time(NULL);
		}

		void time_t_to_tm(time_t time, tm& out)
		{
			if (g_time_zone != 0)
			{
				time += g_time_zone;
			}
#if defined(_MSC_VER)
			::gmtime_s(&out, &time);
#elif defined(__GNUC__)
			::gmtime_r(&time, &out);
#else
#			pragma error "Unknown Platform Not Supported. Abort! Abort!"
#endif // 
		}

		void time_t_to_tm_local(time_t time, tm& out)
		{
#if defined(_MSC_VER)
			::localtime_s(&out, &time);
#elif defined(__GNUC__)
			::localtime_r(&time, &out);
#else
#			pragma error "Unknown Platform Not Supported. Abort! Abort!"
#endif // 
		}

		tm time_t_to_tm(time_t time)
		{
			tm out;
			time_t_to_tm(time, out);
			return out;
		}

		void get_tm(tm& out)
		{	
			time_t_to_tm(get_gmt(), out);
		}

		tm get_tm()
		{
			tm out;
			get_tm(out);
			return out;
		}

		time_t tm_to_time_t(const tm& _tm)
		{
			tm now_tm = _tm;
			if(now_tm.tm_year >= 1900)
			{
				now_tm.tm_year -= 1900;
			}
			time_t ret = mktime(&now_tm);
			ret += g_time_zone;
			return ret;
		}

		// yyyyMMddHHmmss
		void time64_format(time_t time, char date_connector, char datetime_connector, char time_connector, char (&out)[ASCII_DATETIME_LEN], bool flag_zone)
		{
			tm now_tm;
			if (flag_zone)
			{
				time_t_to_tm(time, now_tm);
			}
			else
			{
				time_t_to_tm_local(time, now_tm);
			}

			sprintf(out, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d"
				, now_tm.tm_year + 1900, date_connector, now_tm.tm_mon + 1, date_connector, now_tm.tm_mday
				, datetime_connector
				, now_tm.tm_hour, time_connector, now_tm.tm_min, time_connector, now_tm.tm_sec
			);
		}

		time_t get_day_stamp_time64(time_t now, int hour, int minute)
		{
			tm now_tm;
			time_t_to_tm(now, now_tm);

			int day = ETC_Day;
			if (now_tm.tm_hour < hour
				|| (now_tm.tm_hour == hour && now_tm.tm_min < minute)
				)
			{
				day = 0;
			}

			return  (now 
				- (now_tm.tm_hour - hour) * ETC_Hour 
				- (now_tm.tm_min - minute) * ETC_Minute
				- now_tm.tm_sec
				+ day
				);
		}
		time_t get_day_stamp_time64(int hour, int minute)
		{
			return get_day_stamp_time64(get_gmt(), hour, minute);
		}

		time_t get_week_stamp_time64(time_t now, int wday, int hour, int minute)
		{
			tm now_tm;
			time_t_to_tm(now, now_tm);

			int week = ETC_Week;
			if (now_tm.tm_wday < wday
				|| (now_tm.tm_wday == wday && now_tm.tm_hour < hour)
				|| (now_tm.tm_wday == wday && now_tm.tm_hour == hour && now_tm.tm_min < minute)
				)
			{
				week = 0;
			}

			return (now
				- (now_tm.tm_wday - wday)  * ETC_Day 
				- (now_tm.tm_hour - hour)  * ETC_Hour 
				- (now_tm.tm_min - minute) * ETC_Minute 
				- now_tm.tm_sec
				+ week
				);
		}

		time_t get_week_stamp_time64(int wday, int hour, int minute)
		{
			return get_week_stamp_time64(get_gmt(), wday, hour, minute);
		}

		time_t get_this_week_stamp_time64(time_t now, int wday, int hour, int minute)
		{
			tm now_tm;
			time_t_to_tm(now, now_tm);

			return (now
				- (now_tm.tm_wday - wday)  * ETC_Day 
				- (now_tm.tm_hour - hour)  * ETC_Hour 
				- (now_tm.tm_min - minute) * ETC_Minute 
				- now_tm.tm_sec
				);
		}

		time_t get_this_week_stamp_time64(int wday, int hour, int minute)
		{
			return get_this_week_stamp_time64(get_gmt(), wday, hour, minute);
		}

		time_t get_today_stamp_time64(int hour)
		{
			tm now_tm;
			time_t now = get_gmt();
			time_t_to_tm(now, now_tm);

			return  (now 
				- (now_tm.tm_hour - hour) * ETC_Hour 
				- now_tm.tm_min * ETC_Minute
				- now_tm.tm_sec
				);
		}
	}

	namespace wtime_32_api
	{
		using namespace wtime_base_api;
		
		int time_t_to_time32(time_t time)
		{
			return static_cast<int>(time - BASE_TIME);
		}

		time_t time32_to_time_t(int time)
		{
			return BASE_TIME + time;
		}


		int get_time32()
		{
			return time_t_to_time32(get_gmt());
		}		


		int tm_to_time32(const tm& _tm)
		{
			return time_t_to_time32(tm_to_time_t(_tm));
		}

		void time32_to_tm(int time, tm& out)
		{
			time_t_to_tm(time32_to_time_t(time), out);
		}

		tm time32_to_tm(int time)
		{
			tm out;
			time32_to_tm(time, out);
			return out;
		}

		// yyyyMMddHHmmss
		void time_format(int time, char date_connector, char datetime_connector, char time_connector, char (&out)[ASCII_DATETIME_LEN], bool flag_zone)
		{
			time64_format(time32_to_time_t(time), date_connector, datetime_connector, time_connector, out, flag_zone);
		}

		int get_day_stamp(int now_t, int hour, int minute)
		{
			return  time_t_to_time32(get_day_stamp_time64(time32_to_time_t(now_t), hour, minute));
		}

		int get_day_stamp(int hour, int minute)
		{
			return  time_t_to_time32(get_day_stamp_time64(hour, minute));
		}

		int get_week_stamp(int now_t, int wday, int hour, int minute)
		{
			return  time_t_to_time32(get_week_stamp_time64(time32_to_time_t(now_t), wday, hour, minute));
		}

		int get_week_stamp(int wday, int hour, int minute)
		{
			return  time_t_to_time32(get_week_stamp_time64(wday, hour, minute));
		}

		int get_this_week_stamp(int now, int wday, int hour, int minute)
		{
			return  time_t_to_time32(get_this_week_stamp_time64(time32_to_time_t(now), wday, hour, minute));
		}

		int get_this_week_stamp(int wday, int hour, int minute)
		{
			return  time_t_to_time32(get_this_week_stamp_time64(wday, hour, minute));
		}

		int get_today_stamp_time32(int hour)
		{
			return  time_t_to_time32(get_today_stamp_time64(hour));
		}
		
	}
} // namespace wang

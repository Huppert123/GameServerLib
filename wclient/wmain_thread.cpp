#include "wmain_thread.h"
#include "wlog.h"
#include "wcfg.h"
#include "wwan_client.h"
#include "wmsg_dispatch.h"
#include "wtime_elapse.h"
#include "wutil.h"
#include <iostream>
#include "wclient_session.h"

namespace wang
{
	wmain_thread::wmain_thread() : m_stop(false)
		, m_update_thread_stop(false)
	{

	}

	wmain_thread::~wmain_thread()
	{

	}

	bool wmain_thread::init(const char* log_filename, const char* config_filename)
	{
		//1 log
		printf("init ...\n");
		wang::wlog::init(log_filename, 50, true);

		if (!g_cfg.init(config_filename))
		{
			return false;
		}

		if (!g_wan_client.init())
		{
			return false;
		}

		if (!g_msg_dispatch.init())
		{
			return false;
		}

		g_wan_client.start();

		std::thread tmp_thread(std::bind(&wmain_thread::_update_thread, this));
		m_update_thread.swap(tmp_thread);

		SLOG_INFO << "init ok";
		return true;
	}

	bool wmain_thread::work()
	{
		_process_input();
		m_update_thread_stop = true;
		if (m_update_thread.joinable())
		{
			m_update_thread.join();
		}
		SLOG_INFO << "Leave main loop";
		return true;
	}

	void wmain_thread::destroy()
	{
		g_wan_client.shutdown();
		g_wan_client.destroy();
		SLOG_INFO << "g_wan_server Destroy OK!";

		g_msg_dispatch.destroy();
		SLOG_INFO << "g_msg_dispatch Destroy OK!";

		g_cfg.destroy();
		SLOG_INFO << "destroy Destroy OK!";

		wang::wlog::destroy();
		printf("log destroy ok\n");

		printf("Destroy End\n");
	}

	void wmain_thread::_update_thread()
	{
		static const uint32 TICK_TIME = 100;
		//启动内网并等待初始化完成

		wtime_elapse time_elapse;
		uint32 uDelta;
		while (!m_update_thread_stop)
		{
			uDelta = time_elapse.get_elapse();

			g_wan_client.update(uDelta);

			uDelta = time_elapse.get_elapse();
			if (uDelta < TICK_TIME)
			{
				wsleep(TICK_TIME - uDelta);
			}
		}

		SLOG_INFO << "Leave main loop";
	}

	void wmain_thread::_process_input()
	{
		std::string str;
		while (!m_stop)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			str.clear();
			std::getline(std::cin, str);
			if (!str.empty() && str[0] != 0 && str[0] != '\r' && str[0] != '\n')
			{
				wclient_session* session_ptr = g_wan_client.get_wan_session_by_index(0);
				if (session_ptr && session_ptr->get_status() != 0)
				{
					session_ptr->send_msg(1, str.c_str(), str.length());
				}
				else
				{
					std::cerr << "session 0 no connected" << std::endl;
				}
			}
		}
	}
}
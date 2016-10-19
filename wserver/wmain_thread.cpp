#include "wmain_thread.h"
#include "wlog.h"
#include "wcfg.h"
#include "wwan_server.h"
#include "wmsg_dispatch.h"
#include "wtime_elapse.h"
#include "wutil.h"

namespace wang
{
	wmain_thread::wmain_thread() : m_stop(false)
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

		if (!g_wan_server.init())
		{
			return false;
		}

		if (!g_msg_dispatch.init())
		{
			return false;
		}

		g_wan_server.start();

		SLOG_INFO << "init ok";
		return true;
	}

	bool wmain_thread::work()
	{
		static const uint32 TICK_TIME = 100;
		//启动内网并等待初始化完成

		wtime_elapse time_elapse;
		uint32 uDelta;
		while (!m_stop)
		{
			uDelta = time_elapse.get_elapse();

			g_wan_server.update(uDelta);

			uDelta = time_elapse.get_elapse();
			if (uDelta < TICK_TIME)
			{
				wsleep(TICK_TIME - uDelta);
			}
		}

		SLOG_INFO << "Leave main loop";

		return true;
	}

	void wmain_thread::destroy()
	{
		g_wan_server.shutdown();
		g_wan_server.destroy();
		SLOG_INFO << "g_wan_server Destroy OK!";

		g_msg_dispatch.destroy();
		SLOG_INFO << "g_msg_dispatch Destroy OK!";

		g_cfg.destroy();
		SLOG_INFO << "destroy Destroy OK!";

		wang::wlog::destroy();
		printf("log destroy ok\n");

		printf("Destroy End\n");
	}
}
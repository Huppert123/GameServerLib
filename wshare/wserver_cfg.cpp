#include "wserver_cfg.h"
#include "wlog.h"

namespace wang {

	bool wserver_cfg::init(uint16 values_count, const char* config_filename)
	{
		if (false == wconfig::init(values_count))
		{
			LOG_ERROR << "init config, set values count error, values_count = " << values_count;
			return false;
		}

		if (!open_file(config_filename))
		{
			LOG_ERROR << "open config file error, " << config_filename;
			return false;
		}

		//ip port
		//game
		set_string(ECI_LanIp, "lan_ip", "127.0.0.1");
		set_uint32(ECI_LanPort, "lan_port", 7000);

		//gateway
		set_string(ECI_WanIp, "wan_ip", "127.0.0.1");
		set_uint32(ECI_WanPort, "wan_port", 8000);
		
		// [-11, 11]
		set_int32(ECI_TimeZone, "time_zone", 8);
		
		//单位秒
		set_uint32(ECI_HeartBeat, "heart_beat", 120, 1000);
		
		//Lan
		set_uint32(ECI_LanClientMaxNum, "lan_client_max_num", 1);
		//单位 B
		set_uint32(ECI_LanMaxRecvSize, "lan_max_recv_size", 1024);
		//单位 B
		set_uint32(ECI_LanRecvBufSize, "lan_recv_buf_size", 1024);
		//单位 B
		set_uint32(ECI_LanSendBufSize, "lan_send_buf_size", 1024);

		//Wan
		set_uint32(ECI_WanClientMaxNum, "wan_client_max_num", 1);
		//单位 B
		set_uint32(ECI_WanMaxRecvSize, "wan_max_recv_size", 1024);
		//单位 B
		set_uint32(ECI_WanRecvBufSize, "wan_recv_buf_size", 1024);
		//单位 B
		set_uint32(ECI_WanSendBufSize, "wan_send_buf_size", 1024);
		
		if (!_check())
		{
			return false;
		}

		_post_process();

		if (!init_more())
		{
			return false;
		}

		close_file();

		return true;
	}

	void wserver_cfg::destroy()
	{
		wconfig::destroy();
	}

	bool wserver_cfg::_check()
	{
		int time_zone = get_int32(ECI_TimeZone);
		if (time_zone < -11 || time_zone > 11)
		{
			LOG_ERROR << "invalid time zone [-11, 11], now is " << time_zone;
			return false;
		}
		return true;
	}

	void wserver_cfg::_post_process()
	{
		
	}
} // namespace wang

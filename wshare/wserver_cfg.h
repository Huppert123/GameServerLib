#ifndef _W_SERVER_CONFIG_H_
#define _W_SERVER_CONFIG_H_

#include "wconfig.h"

namespace wang {

	enum EServerConfigIndex
	{	
		//ip port
		ECI_LanIp,							// world ip
		ECI_LanPort,							// world 端口

		ECI_WanIp,								// 外网ip
		ECI_WanPort,							// 外网端口

		//common
		ECI_TimeZone,
		ECI_HeartBeat,
				
		//lan
		ECI_LanClientMaxNum,				// 最大内网客户端连接
		ECI_LanMaxRecvSize,					// 内网最大接收消息大小
		ECI_LanRecvBufSize,					// 内网接收缓冲区大小
		ECI_LanSendBufSize,					// 内网发送缓冲区大小

		//wan
		ECI_WanClientMaxNum,				// 最大外网客户端连接
		ECI_WanMaxRecvSize,					// 外网最大接收消息大小
		ECI_WanRecvBufSize,					// 外网接收缓冲区大小
		ECI_WanSendBufSize,					// 外网发送缓冲区大小
		
		ECI_ServerMax,
	};

	class wserver_cfg : public wconfig
	{
	public:
		void destroy();

	protected:
		bool init(uint16 values_count, const char* config_filename);
		virtual bool init_more() { return true; }

	private:
		bool _check();
		void _post_process();
	};

} // namespace wang

#endif // _NSHARED_NCONFIG_H_

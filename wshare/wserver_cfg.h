#ifndef _W_SERVER_CONFIG_H_
#define _W_SERVER_CONFIG_H_

#include "wconfig.h"

namespace wang {

	enum EServerConfigIndex
	{	
		//ip port
		ECI_LanIp,							// world ip
		ECI_LanPort,							// world �˿�

		ECI_WanIp,								// ����ip
		ECI_WanPort,							// �����˿�

		//common
		ECI_TimeZone,
		ECI_HeartBeat,
				
		//lan
		ECI_LanClientMaxNum,				// ��������ͻ�������
		ECI_LanMaxRecvSize,					// ������������Ϣ��С
		ECI_LanRecvBufSize,					// �������ջ�������С
		ECI_LanSendBufSize,					// �������ͻ�������С

		//wan
		ECI_WanClientMaxNum,				// ��������ͻ�������
		ECI_WanMaxRecvSize,					// ������������Ϣ��С
		ECI_WanRecvBufSize,					// �������ջ�������С
		ECI_WanSendBufSize,					// �������ͻ�������С
		
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

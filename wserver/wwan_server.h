#ifndef	_W_LAN_SERVER_H_
#define _W_LAN_SERVER_H_

#include "wnet_type.h"

namespace wang
{
	class wwan_session;
	class wnet_server;
	class wnet_msg_queue;
	class wnet_msg;

	class wwan_server
	{

	public:
		wwan_server();
		~wwan_server();

	public:
		bool init();
	
		void start();

		void shutdown();
		/** 
		 * 销毁由Init构造的数据, 只能调用一次
		 */
		void destroy();

		void update(uint32 uDeltaTime);	
	
		wwan_session* get_wan_session(uint32 session_id);

		uint32 get_active_session_num() const { return m_active_session_num; }
		
		wwan_session* get_sessions() { return m_session_ptr; }
		uint32 get_session_num() { return m_max_session_num; };

		void close(uint32 session_id);

		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size);

	private:
		//非外部调用函数
		void on_msg_receive(wnet_msg* msg_ptr);
		void on_connect(uint32 session_id, uint32 address, uint32 port);
		void on_disconnect(uint32 session_id);

	private:
		void	_process_msg();

	private:
		wnet_server*	m_server_ptr;
		wwan_session*	m_session_ptr;
		uint32			m_max_session_num;
		wnet_msg_queue*	m_msg_queue_ptr;
		uint32			m_update_timer;
		uint32			m_active_session_num;
	};

	extern wwan_server g_wan_server;
}
#endif


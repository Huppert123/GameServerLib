#ifndef	_W_SESSION_MGR_H_
#define _W_SESSION_MGR_H_

#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include "wnet_type.h"
#include "wnet_define.h"
#include "wsafe_cycle_queue.h"

namespace wang
{
	class wnet_mem_pool;
	class wnet_session;
	class wnet_msg;

	class wnet_session_mgr
	{
	public:
		//新消息回调
		typedef std::function<void(wnet_msg* msg_ptr)> wmsg_callback;

		//断开回调
		typedef std::function<void(uint32 session_id)> wdisconnect_callback;

	public:
		wnet_session_mgr();
		~wnet_session_mgr();

		bool init();

		void start();

		void shutdown();

		void destroy();

	public:
		//config
		void set_wan() { m_flag_wan = true; }
		bool is_wan() const { return m_flag_wan; }

		void set_max_session(uint32 value) { m_max_session = value; }
		uint32 get_max_session() const { return m_max_session; }

		void set_poll_size(uint32 value) { m_pool_size = value; }

		void set_thread_num(uint32 value) { m_thread_num = value; }

		void set_msg_id_key(uint16 value) { m_msg_id_key = value; }
		uint16 get_msg_id_key() const { return m_msg_id_key; }

		void set_msg_size_key(uint32 value) { m_msg_size_key = value; }
		uint32 get_msg_size_key() const { return m_msg_size_key; }
		
		void set_max_received_msg_size(uint32 value) { m_max_received_msg_size = value; }
		uint32 get_max_received_msg_size() const { return m_max_received_msg_size; }

		void set_send_buff_size(uint32 value) { m_send_buff_size = value; }
		uint32 get_send_buff_size() const { return m_send_buff_size; }

		void set_recv_buff_size(uint32 value) { m_recv_buff_size = value; }
		uint32 get_recv_buff_size() const { return m_recv_buff_size; }

	public:
		//call back
		void set_msg_callback(wmsg_callback callback) { m_msg_callback = callback; }
		wmsg_callback get_msg_callback() { return m_msg_callback; }

		void set_disconnect_callback(wdisconnect_callback callback) { m_disconnect_callback = callback; }

	public:
		wnet_mem_pool* get_pool_ptr() { return m_pool_ptr; }
		void* alloc(uint32 bytes);
		void free(void* const ptr);

	public:
		boost::asio::io_service& get_service() { return m_io_service; }

		// 取得一个可用的会话
		wnet_session* get_available_session();

		// 归还一个关闭的会话
		void return_session(wnet_session* session_ptr);

		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size);

		/**
		* 关闭指定连接
		* @param session_id	连接id
		*/
		void close(uint32 session_id);

		void handle_close(wnet_session* session_ptr);

	private:
		// 工作线程
		void _worker_thread();

	private:
		wnet_session_mgr(const wnet_session_mgr&);
		wnet_session_mgr& operator=(const wnet_session_mgr&);

	private:
		typedef std::unique_ptr<wnet_session> wsession_ptr;

	private:
		//config
		bool			m_flag_wan;
		uint32			m_max_session;
		uint16			m_msg_id_key;
		uint32			m_msg_size_key;
		uint32			m_max_received_msg_size;
		uint32			m_send_buff_size;
		uint32			m_recv_buff_size;
		uint32			m_pool_size;
		uint32			m_thread_num;

		//callback
		wmsg_callback			m_msg_callback;
		
		
		wdisconnect_callback	m_disconnect_callback;

		wnet_mem_pool*			m_pool_ptr;

		boost::asio::io_service				m_io_service;		// io服务
		boost::asio::io_service::work		m_work;				// 保证io服务work

		std::vector<wsession_ptr>			m_sessions;
		wsafe_cycle_queue<wnet_session*>	m_available_sessions;

		std::vector<std::thread>			m_threads;			// 线程池
	};

}
#endif


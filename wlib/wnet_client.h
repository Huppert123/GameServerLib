#ifndef _W_NET_CLIENT_H_
#define _W_NET_CLIENT_H_

#include "wnet_session_mgr.h"
#include "wsafe_cycle_queue.h"

namespace wang {

	class wnet_session;

	class wnet_client
	{
	private:
		class wconnect_para
		{
		public:
			wconnect_para(boost::asio::io_service& io_service);

		public:
			boost::asio::ip::tcp::socket	m_socket;
			boost::asio::ip::tcp::endpoint m_endpoint;
			boost::asio::deadline_timer		m_timer;
			uint32 m_para;
		};

		typedef std::unique_ptr<wconnect_para> wpara_ptr;

		//客户端主动连接回调
		typedef std::function<void(uint32 session_id, uint32 para, uint32 error_code)> wconnect_callback;

	public:
		wnet_client();
		~wnet_client();
		
		//config
		void set_reconnet_second(uint32 seconds) { m_reconnect_second = seconds; }

		void set_connect_callback(wconnect_callback callback) { m_connect_callback = callback; }

		bool init();

		/** 
		 * 启动客户端，初始化资源
		 */
		bool startup();

		/** 
		 * 关闭客户端，该函数所在执行线程不被强制关闭，建议在主线程中关闭
		 */
		void shutdown();

		void destroy();	


		/** 
		 * 异步的连接服务器
		 * @param session_id	分配的session_id
		 * @param ip_addresss	远程ip地址
		 * @param port			远程端口
		 * @param para			链接成功是回传的参数
		 * @return				true连接成功,false表示连接请求投递失败
		 */
		bool connect_to(const std::string& ip_address, uint16 port, uint32 para);

		/**
		* 向指定连接发送数据，内部会建立一份拷贝，此函数返回后msg指向的内存可以安全释放
		* note: 游戏逻辑服务器使用接口
		* @param session_id	连接id
		* @param msg_ptr		消息的地址
		* @param size			发送数据的实际长度
		*/
		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
		{
			return m_session_mgr.send_msg(session_id, msg_id, msg_ptr, size);
		}

		/**
		* 关闭指定连接
		* @param session_id	连接id
		*/
		void close(uint32 session_id)
		{
			m_session_mgr.close(session_id);
		}

	public:
		wnet_session_mgr& get_session_mgr() { return m_session_mgr; }


	private:
		void handle_connect(const boost::system::error_code& error, wconnect_para* para_ptr);
		void handle_reconnect(wconnect_para* para_ptr);

	private:
		wnet_client(const wnet_client&);
		wnet_client& operator=(const wnet_client&);
		
	private:
		//config
		uint32								m_reconnect_second;
		wconnect_callback					m_connect_callback;

		wnet_session_mgr					m_session_mgr;
		std::vector<wpara_ptr>				m_paras;
		std::mutex							m_mutex;			// 保护连接池线程安全
		wsafe_cycle_queue<wconnect_para*>	m_available_paras;
		
	};

} // namespace wang

#endif // _W_NET_CLIENT_H_

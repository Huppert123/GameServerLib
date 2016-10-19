#ifndef _W_NET_SERVER_H_
#define _W_NET_SERVER_H_

#include "wnet_session_mgr.h"

namespace wang {

	class wnet_session;

	class wnet_server
	{
	public:
		//服务端监听线程建立新连接回调
		typedef std::function<void(uint32 session_id, uint32 ip, uint16 port)> wlisten_callback;

	public:
		wnet_server();
		~wnet_server();

		//config
		void set_listen_callback(wlisten_callback callback) { m_listen_callback = callback; }

		bool init();

		/** 
		 * 启动服务器监听指定ip和端口
		 */
		bool startup(const std::string& ip, uint16 port);

		/** 
		 * 关闭服务器，该函数所在执行线程不被强制关闭，建议在主线程中关闭
		 */
		void shutdown();
		
		void destroy();

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
		
	public:
		void handle_accept(const boost::system::error_code& error);
		
	private:
		// 异步accpet请求
		void _async_accept();

	private:
		wnet_server(const wnet_server&);
		wnet_server& operator=(const wnet_server&);

	private:
		wlisten_callback					m_listen_callback;
		wnet_session_mgr					m_session_mgr;
		boost::asio::ip::tcp::acceptor		m_acceptor;			// 监听socket
		boost::asio::ip::tcp::socket		m_socket;
	};

} // namespace wang

#endif // _W_NET_SERVER_H_

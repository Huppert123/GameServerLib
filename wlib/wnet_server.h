#ifndef _W_NET_SERVER_H_
#define _W_NET_SERVER_H_

#include "wnet_session_mgr.h"

namespace wang {

	class wnet_session;

	class wnet_server
	{
	public:
		//����˼����߳̽��������ӻص�
		typedef std::function<void(uint32 session_id, uint32 ip, uint16 port)> wlisten_callback;

	public:
		wnet_server();
		~wnet_server();

		//config
		void set_listen_callback(wlisten_callback callback) { m_listen_callback = callback; }

		bool init();

		/** 
		 * ��������������ָ��ip�Ͷ˿�
		 */
		bool startup(const std::string& ip, uint16 port);

		/** 
		 * �رշ��������ú�������ִ���̲߳���ǿ�ƹرգ����������߳��йر�
		 */
		void shutdown();
		
		void destroy();

		/** 
		 * ��ָ�����ӷ������ݣ��ڲ��Ὠ��һ�ݿ������˺������غ�msgָ����ڴ���԰�ȫ�ͷ�
		 * note: ��Ϸ�߼�������ʹ�ýӿ�
		 * @param session_id	����id
		 * @param msg_ptr		��Ϣ�ĵ�ַ
		 * @param size			�������ݵ�ʵ�ʳ���
		 */
		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
		{
			return m_session_mgr.send_msg(session_id, msg_id, msg_ptr, size);
		}

		/** 
		 * �ر�ָ������
		 * @param session_id	����id
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
		// �첽accpet����
		void _async_accept();

	private:
		wnet_server(const wnet_server&);
		wnet_server& operator=(const wnet_server&);

	private:
		wlisten_callback					m_listen_callback;
		wnet_session_mgr					m_session_mgr;
		boost::asio::ip::tcp::acceptor		m_acceptor;			// ����socket
		boost::asio::ip::tcp::socket		m_socket;
	};

} // namespace wang

#endif // _W_NET_SERVER_H_

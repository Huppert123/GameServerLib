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

		//�ͻ����������ӻص�
		typedef std::function<void(uint32 session_id, uint32 para, uint32 error_code)> wconnect_callback;

	public:
		wnet_client();
		~wnet_client();
		
		//config
		void set_reconnet_second(uint32 seconds) { m_reconnect_second = seconds; }

		void set_connect_callback(wconnect_callback callback) { m_connect_callback = callback; }

		bool init();

		/** 
		 * �����ͻ��ˣ���ʼ����Դ
		 */
		bool startup();

		/** 
		 * �رտͻ��ˣ��ú�������ִ���̲߳���ǿ�ƹرգ����������߳��йر�
		 */
		void shutdown();

		void destroy();	


		/** 
		 * �첽�����ӷ�����
		 * @param session_id	�����session_id
		 * @param ip_addresss	Զ��ip��ַ
		 * @param port			Զ�̶˿�
		 * @param para			���ӳɹ��ǻش��Ĳ���
		 * @return				true���ӳɹ�,false��ʾ��������Ͷ��ʧ��
		 */
		bool connect_to(const std::string& ip_address, uint16 port, uint32 para);

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
		std::mutex							m_mutex;			// �������ӳ��̰߳�ȫ
		wsafe_cycle_queue<wconnect_para*>	m_available_paras;
		
	};

} // namespace wang

#endif // _W_NET_CLIENT_H_

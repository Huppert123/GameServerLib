/**
 *	network
 *
 *	Copyright (C) 2016 Wang
 *
 *	Author: H.B. Wang
 *	Date:	Aug, 2016
 */

#ifndef _W_NET_SESSION_H_
#define _W_NET_SESSION_H_

#include <mutex>
#include <atomic>
#include "wnet_type.h"
#include "wnet_define.h"

namespace wang {

	class wnet_session_mgr;
	class wnet_msg;

	class wnet_session
	{
	private:
		/**
		  *	����ײ�session״̬
		*/
		enum wENetSessionStatus
		{
			ENSS_NONE = 0,
			ENSS_INIT,
			ENSS_OPEN,
			ENSS_SHUT,
			ENSS_CLOSE,
		};

	public:
		wnet_session(boost::asio::io_service& io_service, wnet_session_mgr& session_mgr, uint16 index);
		~wnet_session();

		/**
		* ������Դ ���߳�
		* @param index	���� ������
		*/
		bool init();

		/**
		* �ͷ�����������ڴ���Դ�����߳�
		*/
		void destroy();		

		/**
		* ���ص�init��״̬, ׼������. ���߳�
		*/
		void reset();
				
		/**
		* ����˴�����, �ƶ����壬���߳�, must call begin_read after call open
		* @param socket	�����ӵ�socket 
		*/
		void open(boost::asio::ip::tcp::socket&& socket);
		
		// ���߳�
		void begin_read();

		/** 
		 * ��ָ�����ӷ�������, ���߳�. �ڲ��Ὠ��һ�ݿ������˺������غ�msg_ptrָ����ڴ���԰�ȫ�ͷ�
		 * @param session_id	����id
		 * @param msg_ptr		��Ϣ�ĵ�ַ
		 * @param size			��Ϣ�ĳ���
		 */
		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size);

		/** 
		 * �ر�ָ������, ���߳�
		 * @param session_id ����id
		 */
		void close(uint32 session_id);

		/**
		* ���߳�
		*/
		uint32 get_id() const { return m_session_id; }

	public:
		/**
		* ���߳�
		*/
		boost::asio::ip::tcp::socket& get_socket() { return m_socket; }

		// �����ݻص� ���߳�
		void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
		// д���ݻص� ���߳�
		void handle_write(const boost::system::error_code& error, std::size_t bytes_transferred);

	private:		

		// �黹����pool��Դ
		void _release();

		// �ر�ָ������
		void _close();

		bool push_to_queue(const void* msg_ptr, uint32 msg_size, uint16 code_msg_id, uint32 code_msg_size);
		void queue_to_buf();
		bool parse_msg();

	private:
		wnet_session(const wnet_session& rht);
		wnet_session& operator=(const wnet_session& rht);

	private:
#pragma pack(push,1)

		struct wreceive_buffer
		{
			uint32			max_size; // max size of buf
			uint32			recv_size;// recv size of buf
			uint32			expect_size;// expect recv size of buf
			char			buf[1];
		};

		struct wsend_buffer
		{
			uint32			size;	  // send size of buf
			uint32			max_size; // max size of buf
			uint32			send_size;// send size
			char			buf[1];
			bool empty() const { return size == 0; }
			uint32 avail() const { return max_size - size; }
			char*  get_cur_buf() { return buf + size; }
			void skip(uint32 len) { size += len; }
			uint32 append(const void* p, uint32 len);
		};

		struct wwait_buffer
		{
			wwait_buffer*	next_ptr;
			uint32			send_size; // send size of buf
			uint32			total_size; // size of buf
			char			buf[1]; // buf = msg_size + msg_buf
		};
#pragma pack(pop)

	private:
		uint32							m_session_id;		// session_idһ������| ��2�ֽ� ��ʾindex | ��2�ֽڱ�ʾguid |��0XFFFFFFFFΪ�Ƿ���client_id
		
		boost::asio::ip::tcp::socket	m_socket;			// session��socket��Դ

		wnet_session_mgr&				m_session_mgr;

		std::mutex						m_mutex;			// ����send������
		wENetSessionStatus					m_status;			// session״̬
		std::atomic<bool>				m_is_writing;		// ��Ͷ���첽write��û�з���
		std::atomic<bool>				m_is_reading;		//

		//receive msg
		wreceive_buffer*		m_recv_buf_ptr;			// ������Ϣ
		wnet_msg*				m_msg_ptr;

		//send msg
		wsend_buffer*			m_data_buf_ptr;		// �������ݻ�����
		wsend_buffer*			m_sending_buf_ptr;	// ���ڷ������ݻ�����

		wwait_buffer*			m_wait_head_ptr;	// �ȴ����ͻ�����ͷ
		wwait_buffer*			m_wait_tail_ptr;	// �ȴ����ͻ�����β
	};

} // namespace nexus

#endif // _W_NET_SESSION_H_

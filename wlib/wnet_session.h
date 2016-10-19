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
		  *	网络底层session状态
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
		* 申请资源 单线程
		* @param index	连接 索引号
		*/
		bool init();

		/**
		* 释放连接申请的内存资源，单线程
		*/
		void destroy();		

		/**
		* 返回到init后状态, 准备重用. 单线程
		*/
		void reset();
				
		/**
		* 服务端打开连接, 移动语义，单线程, must call begin_read after call open
		* @param socket	已连接的socket 
		*/
		void open(boost::asio::ip::tcp::socket&& socket);
		
		// 单线程
		void begin_read();

		/** 
		 * 向指定连接发送数据, 多线程. 内部会建立一份拷贝，此函数返回后msg_ptr指向的内存可以安全释放
		 * @param session_id	连接id
		 * @param msg_ptr		消息的地址
		 * @param size			消息的长度
		 */
		bool send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size);

		/** 
		 * 关闭指定连接, 多线程
		 * @param session_id 连接id
		 */
		void close(uint32 session_id);

		/**
		* 单线程
		*/
		uint32 get_id() const { return m_session_id; }

	public:
		/**
		* 单线程
		*/
		boost::asio::ip::tcp::socket& get_socket() { return m_socket; }

		// 读数据回调 单线程
		void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
		// 写数据回调 单线程
		void handle_write(const boost::system::error_code& error, std::size_t bytes_transferred);

	private:		

		// 归还各种pool资源
		void _release();

		// 关闭指定连接
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
		uint32							m_session_id;		// session_id一个连接| 低2字节 表示index | 高2字节表示guid |，0XFFFFFFFF为非法的client_id
		
		boost::asio::ip::tcp::socket	m_socket;			// session的socket资源

		wnet_session_mgr&				m_session_mgr;

		std::mutex						m_mutex;			// 保护send缓冲区
		wENetSessionStatus					m_status;			// session状态
		std::atomic<bool>				m_is_writing;		// 已投递异步write还没有返回
		std::atomic<bool>				m_is_reading;		//

		//receive msg
		wreceive_buffer*		m_recv_buf_ptr;			// 接收消息
		wnet_msg*				m_msg_ptr;

		//send msg
		wsend_buffer*			m_data_buf_ptr;		// 发送数据缓冲区
		wsend_buffer*			m_sending_buf_ptr;	// 正在发送数据缓冲区

		wwait_buffer*			m_wait_head_ptr;	// 等待发送缓冲区头
		wwait_buffer*			m_wait_tail_ptr;	// 等待发送缓冲区尾
	};

} // namespace nexus

#endif // _W_NET_SESSION_H_

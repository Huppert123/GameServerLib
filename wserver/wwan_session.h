/********************************************************************
	created:	2015/01/4

	author:		wanghaibo

	level:		网络层

	purpose:	网络数据的收发
*********************************************************************/

#ifndef _W_WAN_SESSION_H__ 
#define _W_WAN_SESSION_H__

#include "wnet_type.h"
#include "wcrypto.h"
#include <atomic>

namespace wang
{
	class wnet_msg;

	class wwan_session
	{
	private:
		enum ESessionStatus
		{
			ES_None = 0X0000,
			ES_Connected = 0X0001,
			ES_Gaming = 0X0002,
			ES_Max
		};

	public:
		wwan_session();
		~wwan_session();

	private:
		wwan_session(const wwan_session& rht);
		wwan_session& operator=(const wwan_session& rht);

	public:
		/**
		 * 返回true初始化成功
		 */
		void init();

		/**
		 * 销毁由Init构造的数据, 只能调用一次
		 */
		void destroy();

		void update(uint32 delta);

		bool send_msg(uint16 msg_id, const void* msg_ptr, uint32 msg_size);

		bool encrypt_send_msg(uint16 msg_id, const void* msg_ptr, int msg_size, void* encrypt_buf, int buf_size);

		void on_connected(uint32 session_id, uint32	address);
		void on_disconnected();

		uint32 get_id() const { return m_session_id; }
		uint32 get_status() const { return m_status; }

		bool is_used() const { return m_session_id != 0; }

		void inc_msg();

		bool is_gaming() const { return m_status == ES_Gaming; }
		void set_gaming() { m_status = ES_Gaming; }

	public:
		void set_encrypt_key(const void* encrypt_data, int encrypt_len, const void* decrypt_data, int decrypt_len);
		bool encrypt(const void* data_in, void* data_out, int len_in);
		bool decrypt(const void* data_in, void* data_out, int len_in);

	public:
		void handle_test(wnet_msg* msg_ptr);

	private:
		uint32				m_session_id;
		uint32				m_status;		//-- 状态
		uint32				m_server_type;
		std::atomic<uint32>	m_msg_count;
		int32				m_time_out;
		uint32				m_address;

		// 协议加密数据
		bool				m_crypto_flag;
		wcrypto				m_crypto;
	};
}
#endif

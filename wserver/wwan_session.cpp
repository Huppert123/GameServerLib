#include "wwan_session.h"
#include "wwan_server.h"
#include "wlog.h"
#include "wcfg.h"
#include "wnet_msg.h"

namespace wang
{
	wwan_session::wwan_session() : m_session_id(0)
		, m_status(ES_None)
		, m_server_type(0)
		, m_msg_count(0)
		, m_time_out(0)
		, m_address(0)
		, m_crypto_flag(false)
	{

	}

	wwan_session::~wwan_session()
	{
		destroy();
	}

	void	wwan_session::init()
	{
		m_status = ES_None;
	}

	void wwan_session::destroy()
	{
		m_session_id = 0;
		m_status = ES_None;
		m_server_type = 0;
		m_msg_count = 0;
		m_time_out = 0;
		m_address = 0;
		m_crypto_flag = false;
	}

	void wwan_session::update(uint32 delta)
	{
		m_time_out += delta;

		if (m_time_out < g_cfg.get_int32(ECI_HeartBeat))
		{
			return;
		}

		m_time_out = 0;

		if (0 == m_msg_count)
		{
			// 没有心跳断开连接
			g_wan_server.close(m_session_id);
		}
	}

	void wwan_session::on_disconnected()
	{
		m_session_id = 0;
		m_status = ES_None;
		m_server_type = 0;
		m_msg_count.exchange(0);
		m_time_out = 0;
		m_address = 0;
		m_crypto_flag = false;
	}

	bool wwan_session::send_msg(uint16 msg_id, const void* msg_ptr, uint32 msg_size)
	{
		return g_wan_server.send_msg(get_id(), msg_id, msg_ptr, msg_size);
	}

	bool wwan_session::encrypt_send_msg(uint16 msg_id, const void* msg_ptr, int msg_size, void* encrypt_buf, int buf_size)
	{
		if (msg_size < 0 || msg_size > buf_size)
		{
			LOG_ERROR << "msg_size > buf_size, " << msg_size << " > " << buf_size;
			return false;
		}
		if (!encrypt(msg_ptr, encrypt_buf, msg_size))
		{
			return send_msg(msg_id, msg_ptr, msg_size);
		}
		else
		{
			/*DEBUG_LOG("msg_id=%u", msg_id);
			std::string ret = wutil::get_hex_str(msg_ptr, msg_size).c_str();
			DEBUG_LOG("src_msg size=%u, %s", ret.length(), ret.c_str());

			ret = wutil::get_hex_str(encrypt_buf, msg_size).c_str();
			DEBUG_LOG("dst_msg size=%u, %s", ret.length(), ret.c_str());*/
			return send_msg(msg_id, encrypt_buf, msg_size);
		}
	}

	void  wwan_session::on_connected(uint32 session_id, uint32 address)
	{
		m_session_id = session_id;
		m_status = ES_Connected;
		m_time_out = 0;
		m_address = address;
	}

	void wwan_session::inc_msg()
	{
		++m_msg_count;
	}

	void wwan_session::set_encrypt_key(const void* encrypt_data, int encrypt_len, const void* decrypt_data, int decrypt_len)
	{
		if (!encrypt_data || 0 == encrypt_len || !decrypt_data || 0 == decrypt_len)
		{
			LOG_ERROR << "set_key crypt data error";
			return;
		}

		m_crypto_flag = true;
		m_crypto.set_encrypt_key(encrypt_data, encrypt_len);
		m_crypto.set_decrypt_key(decrypt_data, decrypt_len);
	}

	bool wwan_session::encrypt(const void* data_in, void* data_out, int len_in)
	{
		if (!m_crypto_flag || 0 == len_in)
		{
			return false;
		}

		m_crypto.encrypt(data_in, data_out, len_in);
		return true;
	}

	bool wwan_session::decrypt(const void* data_in, void* data_out, int len_in)
	{
		if (!m_crypto_flag || 0 == len_in)
		{
			return false;
		}

		m_crypto.decrypt(data_in, data_out, len_in);
		return true;
	}

	void wwan_session::handle_test(wnet_msg* msg_ptr)
	{
		LOG_DEBUG.append(msg_ptr->get_buf(), msg_ptr->get_size());
	}

}
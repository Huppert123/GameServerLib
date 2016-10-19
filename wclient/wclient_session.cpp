#include "wclient_session.h"
#include "wwan_client.h"
#include "wlog.h"
#include "wcfg.h"
#include "wnet_msg.h"

namespace wang
{
	wclient_session::wclient_session() : m_session_id(0)
		, m_status(ES_None)
		, m_server_type(0)
		, m_crypto_flag(false)
	{

	}

	wclient_session::~wclient_session()
	{
		destroy();
	}

	void	wclient_session::init()
	{
		m_status = ES_None;
	}

	void wclient_session::destroy()
	{
		m_session_id = 0;
		m_status = ES_None;
		m_server_type = 0;
		m_crypto_flag = false;
	}

	void wclient_session::update(uint32 delta)
	{
		
	}

	void wclient_session::on_disconnected()
	{
		m_session_id = 0;
		m_status = ES_None;
		m_server_type = 0;
		m_crypto_flag = false;
	}

	bool wclient_session::send_msg(uint16 msg_id, const void* msg_ptr, uint32 msg_size)
	{
		return g_wan_client.send_msg(get_id(), msg_id, msg_ptr, msg_size);
	}

	bool wclient_session::encrypt_send_msg(uint16 msg_id, const void* msg_ptr, int msg_size, void* encrypt_buf, int buf_size)
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

	void  wclient_session::on_connected(uint32 session_id)
	{
		m_session_id = session_id;
		m_status = ES_Connected;
	}

	void wclient_session::set_encrypt_key(const void* encrypt_data, int encrypt_len, const void* decrypt_data, int decrypt_len)
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

	bool wclient_session::encrypt(const void* data_in, void* data_out, int len_in)
	{
		if (!m_crypto_flag || 0 == len_in)
		{
			return false;
		}

		m_crypto.encrypt(data_in, data_out, len_in);
		return true;
	}

	bool wclient_session::decrypt(const void* data_in, void* data_out, int len_in)
	{
		if (!m_crypto_flag || 0 == len_in)
		{
			return false;
		}

		m_crypto.decrypt(data_in, data_out, len_in);
		return true;
	}

	void wclient_session::handle_test(wnet_msg* msg_ptr)
	{
		LOG_DEBUG.append(msg_ptr->get_buf(), msg_ptr->get_size());
	}

}
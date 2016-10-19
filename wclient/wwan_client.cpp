#include "wclient_session.h"
#include "wwan_client.h"
#include "wnet_client.h"
#include "wnet_msg_queue.h"
#include "wcfg.h"
#include <functional>
#include "wlog.h"
#include "wnet_msg.h"
#include "wmsg_dispatch.h"

namespace wang
{
	wwan_client g_wan_client;

	wwan_client::wwan_client() : m_client_ptr(nullptr)
		, m_session_ptr(nullptr)
		, m_max_session_num(0)
		, m_msg_queue_ptr(nullptr)
		, m_update_timer(0)
		, m_active_session_num(0)
	{

	}

	wwan_client::~wwan_client()
	{
	}

	bool	wwan_client::init()
	{
		m_client_ptr = new wnet_client();

		m_max_session_num = g_cfg.get_uint32(ECI_LanClientMaxNum);

		wnet_session_mgr& session_mgr = m_client_ptr->get_session_mgr();
		session_mgr.set_max_session(m_max_session_num);
		session_mgr.set_max_received_msg_size(g_cfg.get_uint32(ECI_LanMaxRecvSize));
		session_mgr.set_send_buff_size(g_cfg.get_uint32(ECI_LanSendBufSize));
		session_mgr.set_recv_buff_size(g_cfg.get_uint32(ECI_LanRecvBufSize));
		session_mgr.set_wan();
		session_mgr.set_msg_id_key(g_cfg.get_uint32(ECI_MsgIdKey));
		session_mgr.set_msg_size_key(g_cfg.get_uint32(ECI_MsgSizeKey));
		session_mgr.set_disconnect_callback(std::bind(&wwan_client::on_disconnect, this, std::placeholders::_1));
		session_mgr.set_msg_callback(std::bind(&wwan_client::on_msg_receive, this, std::placeholders::_1));
		m_client_ptr->set_connect_callback(std::bind(&wwan_client::on_connect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_client_ptr->set_reconnet_second(5);
		if (!m_client_ptr->init())
		{
			return false;
		}

		m_session_ptr = new wclient_session[m_max_session_num];
		if (!m_session_ptr)
		{
			return false;
		}

		for (uint32 i = 0; i < m_max_session_num; i++)
		{
			m_session_ptr[i].init();
		}

		m_msg_queue_ptr = new wnet_msg_queue();

		return true;
	}

	void wwan_client::start()
	{
		m_client_ptr->startup();
		m_client_ptr->connect_to(g_cfg.get_string(ECI_WanIp), g_cfg.get_int32(ECI_WanPort), 0);
	}

	void wwan_client::shutdown()
	{
		m_client_ptr->shutdown();
	}

	void wwan_client::destroy()
	{
		if (m_client_ptr)
		{
			m_client_ptr->destroy();
			delete m_client_ptr;
			m_client_ptr = nullptr;
		}

		if (m_session_ptr)
		{
			for (uint32 i = 0; i < m_max_session_num; ++i)
			{
				m_session_ptr[i].destroy();
			}
			delete[] m_session_ptr;
			m_session_ptr = nullptr;
		}

		m_max_session_num = 0;

		if (m_msg_queue_ptr)
		{
			m_msg_queue_ptr->clear();
			delete m_msg_queue_ptr;
			m_msg_queue_ptr = nullptr;
		}

		m_update_timer = 0;
		m_active_session_num = 0;
	}

	void wwan_client::update(uint32 uDeltaTime)
	{
		_process_msg();

		if (m_active_session_num > 0)
		{
			uint32 active_session_num = m_active_session_num;
			for (uint32 i = 0; i < m_max_session_num; ++i)
			{
				if (m_session_ptr[i].is_used())
				{
					m_session_ptr[i].update(uDeltaTime);
					--active_session_num;
					if (active_session_num == 0)
					{
						break;
					}
				}
			}
		}
	}

	void wwan_client::close(uint32 session_id)
	{
		m_client_ptr->close(session_id);
	}

	bool wwan_client::send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
	{
		return m_client_ptr->send_msg(session_id, msg_id, msg_ptr, size);
	}

	wclient_session* wwan_client::get_wan_session(uint32 session_id)
	{
		uint32 index = session_id & 0x0000FFFF;
		if (index < m_max_session_num && m_session_ptr[index].get_id() == session_id)
		{
			return &m_session_ptr[index];
		}
		return nullptr;
	}

	wclient_session* wwan_client::get_wan_session_by_index(uint32 index)
	{
		if (index < m_max_session_num)
		{
			return &m_session_ptr[index];
		}
		return nullptr;
	}

	void wwan_client::on_msg_receive(wnet_msg* msg_ptr)
	{
		m_msg_queue_ptr->enqueue(msg_ptr);
	}

	void wwan_client::on_connect(uint32 session_id, uint32 , uint32 error_code)
	{
		if (error_code != 0)
		{
			LOG_ERROR << "failed error_code " << error_code;
			return;
		}
		uint32 index = session_id & 0x0000FFFF;
		if (index >= m_max_session_num)
		{
			LOG_ERROR << "invalid index " << index;
			return;
		}

		wclient_session* p = &m_session_ptr[index];
		if (p->is_used())
		{
			LOG_ERROR << "lan session is used, id = "<< session_id << ", index = " << index;
			return;
		}

		p->on_connected(session_id);
		++m_active_session_num;
	}

	void wwan_client::on_disconnect(uint32 session_id)
	{
		uint32 index = session_id & 0x0000FFFF;
		uint32 serial = (session_id & 0xFFFF0000) >> 16;
		wclient_session* p = get_wan_session(session_id);
		if (p)
		{
			if (m_active_session_num > 0)
			{
				--m_active_session_num;
			}
			else
			{
				LOG_ERROR << "m_active_session_num is zero";
			}
			
			p->on_disconnected();
			
			LOG_DEBUG << "session_index=" << index << " serial=" << serial;
		}
		else
		{
			LOG_WARN << "not found, session_index=" << index << " serial=" << serial;
		}
	}

	void wwan_client::_process_msg()
	{
		wnet_msg *msg_ptr = m_msg_queue_ptr->dequeue();

		while(msg_ptr)
		{
			wclient_session* session_ptr = get_wan_session(msg_ptr->get_session_id());
			if (session_ptr)
			{
				const wmsg_dispatch::wan_handler* handler_ptr = g_msg_dispatch.get_wan_handler(msg_ptr->get_msg_id());
				if (!handler_ptr)
				{
					LOG_ERROR << "unknown msg_id=" << msg_ptr->get_msg_id();
				}
				else if (handler_ptr->status != 0 && (handler_ptr->status & session_ptr->get_status()) == 0)
				{
					LOG_ERROR << "invalid status, status= "<< handler_ptr->status << ", msg_id = " << msg_ptr->get_msg_id();
				}
				else
				{
					(session_ptr->*(handler_ptr->handler))(msg_ptr);
				}
			}
			wnet_msg *p = msg_ptr;
			msg_ptr = msg_ptr->get_next();
			p->free_me();
		}
	}
}
#include "wwan_session.h"
#include "wwan_server.h"
#include "wnet_server.h"
#include "wnet_msg_queue.h"
#include "wcfg.h"
#include <functional>
#include "wlog.h"
#include "wnet_msg.h"
#include "wmsg_dispatch.h"

namespace wang
{
	wwan_server g_wan_server;

	wwan_server::wwan_server() : m_server_ptr(nullptr)
		, m_session_ptr(nullptr)
		, m_max_session_num(0)
		, m_msg_queue_ptr(nullptr)
		, m_update_timer(0)
		, m_active_session_num(0)
	{

	}

	wwan_server::~wwan_server()
	{
	}

	bool	wwan_server::init()
	{
		m_server_ptr = new wnet_server();

		m_max_session_num = g_cfg.get_uint32(ECI_WanClientMaxNum);

		wnet_session_mgr& session_mgr = m_server_ptr->get_session_mgr();
		session_mgr.set_max_session(m_max_session_num);
		session_mgr.set_max_received_msg_size(g_cfg.get_uint32(ECI_WanMaxRecvSize));
		session_mgr.set_send_buff_size(g_cfg.get_uint32(ECI_WanSendBufSize));
		session_mgr.set_recv_buff_size(g_cfg.get_uint32(ECI_WanRecvBufSize));
		session_mgr.set_wan();
		session_mgr.set_msg_id_key(g_cfg.get_uint32(ECI_MsgIdKey));
		session_mgr.set_msg_size_key(g_cfg.get_uint32(ECI_MsgSizeKey));
		session_mgr.set_disconnect_callback(std::bind(&wwan_server::on_disconnect, this, std::placeholders::_1));
		session_mgr.set_msg_callback(std::bind(&wwan_server::on_msg_receive, this, std::placeholders::_1));
		m_server_ptr->set_listen_callback(std::bind(&wwan_server::on_connect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		if (!m_server_ptr->init())
		{
			return false;
		}

		m_session_ptr = new wwan_session[m_max_session_num];
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

	void wwan_server::start()
	{
		m_server_ptr->startup(g_cfg.get_string(ECI_WanIp), g_cfg.get_int32(ECI_WanPort));
	}

	void wwan_server::shutdown()
	{
		m_server_ptr->shutdown();
	}

	void wwan_server::destroy()
	{
		if (m_server_ptr)
		{
			m_server_ptr->destroy();
			delete m_server_ptr;
			m_server_ptr = nullptr;
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

	void wwan_server::update(uint32 uDeltaTime)
	{
		static const uint32 UPDATE_TIMER = 15 * 1000;

		_process_msg();

		m_update_timer += uDeltaTime;

		if (m_update_timer >= UPDATE_TIMER)
		{
			m_update_timer -= UPDATE_TIMER;

			for (uint32 i = 0; i < m_max_session_num; ++i)
			{
				if (m_session_ptr[i].is_used())
				{
					m_session_ptr[i].update(UPDATE_TIMER);
				}
			}
		}
	}

	void wwan_server::close(uint32 session_id)
	{
		m_server_ptr->close(session_id);
	}

	bool wwan_server::send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
	{
		return m_server_ptr->send_msg(session_id, msg_id, msg_ptr, size);
	}

	wwan_session* wwan_server::get_wan_session(uint32 session_id)
	{
		uint32 index = session_id & 0x0000FFFF;
		if (index < m_max_session_num && m_session_ptr[index].get_id() == session_id)
		{
			return &m_session_ptr[index];
		}
		return nullptr;
	}

	void wwan_server::on_msg_receive(wnet_msg* msg_ptr)
	{
		m_msg_queue_ptr->enqueue(msg_ptr);
	}

	void wwan_server::on_connect(uint32 session_id, uint32 address, uint32 port)
	{
		uint32 index = session_id & 0x0000FFFF;
		if (index >= m_max_session_num)
		{
			LOG_ERROR << "invalid index " << index;
			return;
		}

		wwan_session* p = &m_session_ptr[index];
		if (p->is_used())
		{
			LOG_ERROR << "lan session is used, id = "<< session_id << ", index = " << index;
			return;
		}

		p->on_connected(session_id, address);
		++m_active_session_num;
		{
			boost::asio::ip::address_v4 addr(address);
			std::string ip_str = addr.to_string();
			uint32 serial = (session_id & 0xFFFF0000) >> 16;
			LOG_DEBUG << "server session_index=" << index << " serial=" << serial << " connected from ip=" << ip_str << " port=" << port;
		}
	}

	void wwan_server::on_disconnect(uint32 session_id)
	{
		uint32 index = session_id & 0x0000FFFF;
		uint32 serial = (session_id & 0xFFFF0000) >> 16;
		wwan_session* p = get_wan_session(session_id);
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

	void wwan_server::_process_msg()
	{
		wnet_msg *msg_ptr = m_msg_queue_ptr->dequeue();

		while(msg_ptr)
		{
			wwan_session* session_ptr = get_wan_session(msg_ptr->get_session_id());
			if (session_ptr)
			{
				session_ptr->inc_msg();

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
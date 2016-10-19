#include "wnet_client.h"
#include "wnet_session.h"
#include "wlog.h"
#include <boost/bind.hpp>

namespace wang {

	wnet_client::wconnect_para::wconnect_para(boost::asio::io_service& io_service)
		: m_socket(io_service)
		, m_timer(io_service)
	{}

	wnet_client::wnet_client() : m_reconnect_second(0)
		, m_connect_callback(nullptr)
	{
		
	}

	wnet_client::~wnet_client()
	{
		
	}

	bool wnet_client::init()
	{
		if (!m_session_mgr.init())
		{
			return false;
		}
		const uint32 max_session = m_session_mgr.get_max_session();
		m_paras.reserve(max_session);

		boost::asio::io_service& io_service = m_session_mgr.get_service();
		for (uint32 i = 0; i < max_session; ++i)
		{
			m_paras.emplace_back(new wconnect_para(io_service));
		}

		if (m_paras.size() != max_session)
		{
			LOG_ERROR << "new socket failed";
			return false;
		}

		if (!m_available_paras.init(max_session))
		{
			LOG_ERROR << "m_available_paras init failed";
			return false;
		}

		for (wpara_ptr& session : m_paras)
		{
			m_available_paras.push(session.get());
		}

		return true;
	}

	bool wnet_client::startup()
	{
		m_session_mgr.start();

		return true;
	}

	void wnet_client::shutdown()
	{
		m_session_mgr.shutdown();
	}

	void wnet_client::destroy()
	{
		m_session_mgr.destroy();
		m_paras.clear();
	}

	bool wnet_client::connect_to(const std::string& ip_address, uint16 port, uint32 para)
	{
		wconnect_para* para_ptr = NULL;
		if (!m_available_paras.safe_pop(para_ptr))
		{
			return false;
		}
		para_ptr->m_para = para;
		para_ptr->m_endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_address), port);

		para_ptr->m_socket.async_connect(para_ptr->m_endpoint, boost::bind(&wnet_client::handle_connect, this, boost::asio::placeholders::error, para_ptr));
		
		return true;
	}

	void wnet_client::handle_connect(const boost::system::error_code& error, wconnect_para* para_ptr)
	{
		if (error)
		{
			if (para_ptr->m_socket.is_open())
			{
				boost::system::error_code ec;
				para_ptr->m_socket.close(ec);
			}

			if (m_reconnect_second != 0)
			{
				//重连
				para_ptr->m_timer.expires_from_now(boost::posix_time::seconds(m_reconnect_second));
				para_ptr->m_timer.async_wait(boost::bind(&wnet_client::handle_reconnect, this, para_ptr));
			}
			else
			{
				//链接失败回调
				m_connect_callback(0, para_ptr->m_para, error.value());
				m_available_paras.safe_push(para_ptr);
			}
			return;
		}
		else
		{
			wnet_session* session_ptr = m_session_mgr.get_available_session();

			if (!session_ptr)
			{
				if (para_ptr->m_socket.is_open())
				{
					boost::system::error_code ec;
					para_ptr->m_socket.close(ec);
				}
				m_available_paras.safe_push(para_ptr);
				return;
			}
			
			session_ptr->open(std::move(para_ptr->m_socket));
			m_connect_callback(session_ptr->get_id(), para_ptr->m_para, 0);
			session_ptr->begin_read();

			m_available_paras.safe_push(para_ptr);
		}
	}

	void wnet_client::handle_reconnect(wconnect_para* para_ptr)
	{
		if (m_reconnect_second != 0)
		{
			para_ptr->m_socket.async_connect(para_ptr->m_endpoint, boost::bind(&wnet_client::handle_connect, this, boost::asio::placeholders::error, para_ptr));
		}
		else
		{
			m_available_paras.safe_push(para_ptr);
		}
	}
} // namespace wang

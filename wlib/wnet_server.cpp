#include "wnet_server.h"
#include "wnet_session.h"
#include "wlog.h"
#include <boost/bind.hpp>

namespace wang {

	wnet_server::wnet_server() : m_listen_callback(nullptr), m_acceptor(m_session_mgr.get_service()), m_socket(m_session_mgr.get_service())
	{
		
	}

	wnet_server::~wnet_server()
	{
		
	}

	bool wnet_server::init()
	{
		return m_session_mgr.init();
	}

	bool wnet_server::startup(const std::string& ip, uint16 port)
	{
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);

		boost::system::error_code ec;
		m_acceptor.open(endpoint.protocol(), ec);

		if (ec)
		{
			LOG_ERROR << "open socket error " << ec.value();
			return false;
		}

		// reuse造成多次绑定成功，可能找不到正确可用的端口
		// 如果两台服务器开在同一台机器会使用同一个端口，造成混乱		
		{
			m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);

			if (ec)
			{
				LOG_ERROR << "set_option reuse_address error, " << ec.value();
				return false;
			}
		}

		m_acceptor.bind(endpoint, ec);

		if (ec)
		{
			LOG_ERROR << "bind error, " << ec.value();
			return false;
		}

		m_acceptor.listen(boost::asio::socket_base::max_connections, ec);

		if (ec)
		{
			LOG_ERROR << "listen error, " << ec.value() << ", port = " << port;
			return false;
		}

		// 投递异步AcceptEx请求
		_async_accept();

		m_session_mgr.start();

		return true;
	}

	void wnet_server::shutdown()
	{
		if (m_acceptor.is_open())
		{
			boost::system::error_code ec;
			m_acceptor.cancel(ec);
			m_acceptor.close(ec);
		}

		m_session_mgr.shutdown();
	}

	void wnet_server::destroy()
	{
		m_session_mgr.destroy();
	}

	void wnet_server::_async_accept()
	{
		m_acceptor.async_accept(m_socket, boost::bind(&wnet_server::handle_accept, this, boost::asio::placeholders::error));
	}

	void wnet_server::handle_accept(const boost::system::error_code& error)
	{
		if (error)
		{
			if (m_socket.is_open())
			{
				boost::system::error_code ec;
				m_socket.close(ec);
			}
			_async_accept();
			return;
		}

		boost::asio::ip::tcp::socket socket(std::move(m_socket));
		_async_accept();

		wnet_session* p = m_session_mgr.get_available_session();
		if (p)
		{
			p->open(std::move(socket));

			// 回调
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint _endpoint = m_socket.remote_endpoint(ec);
			m_listen_callback(p->get_id()
				, (uint32)_endpoint.address().to_v4().to_ulong()
				, _endpoint.port());

			p->begin_read();
		}
	}

} // namespace wang

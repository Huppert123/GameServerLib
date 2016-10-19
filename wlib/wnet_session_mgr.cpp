#include "wnet_session_mgr.h"
#include "wnet_mem_pool.h"
#include "wlog.h"
#include "wnet_session.h"
#include "wnet_ex_type.h"

namespace wang
{
	wnet_session_mgr::wnet_session_mgr()
		: m_flag_wan(false)
		, m_max_session(1)
		, m_msg_id_key(0)
		, m_msg_size_key(0)
		, m_max_received_msg_size(100)
		, m_send_buff_size(100)
		, m_recv_buff_size(100)
		, m_pool_size(10)
		, m_thread_num(1)
		, m_msg_callback(nullptr)
	//	, m_listen_callback(nullptr)
	//	, m_connect_callback(nullptr)
		, m_disconnect_callback(nullptr)
		, m_pool_ptr(nullptr)
		, m_work(m_io_service)
	{
		
	}

	wnet_session_mgr::~wnet_session_mgr()
	{
		
	}

	void wnet_session_mgr::destroy()
	{
		if (m_pool_ptr)
		{
			delete m_pool_ptr;
			m_pool_ptr = nullptr;
		}
		m_available_sessions.clear();
		m_sessions.clear();
	}

	bool wnet_session_mgr::init()
	{
		//check
		if (m_max_session < 1)
		{
			LOG_ERROR << "invalid para max_session=" << m_max_session;
			return false;
		}

		if (m_max_received_msg_size < MSG_HEADER_SIZE || m_max_received_msg_size > 100 * 1024 * 1024)
		{
			LOG_ERROR << "invalid para max_received_msg_size=" << m_max_received_msg_size;
			return false;
		}

		if (m_send_buff_size < MSG_HEADER_SIZE || m_send_buff_size > 50 * 1024 * 1024)
		{
			LOG_ERROR << "invalid para send_buff_size=" << m_send_buff_size;
			return false;
		}

		if (m_thread_num < 1 || m_thread_num > 10)
		{
			LOG_ERROR << "invalid para thread_num=" << m_thread_num;
			return false;
		}

		m_pool_ptr = new wnet_mem_pool(m_pool_size);
		if (!m_pool_ptr)
		{
			LOG_ERROR << "memory error";
			return false;
		}		
				
		m_sessions.reserve(m_max_session);
		for (uint32 i = 0; i < m_max_session; ++i)
		{
			//m_sessions.emplace_back(std::make_unique<wnet_session>(m_io_service, *this, i));
			m_sessions.emplace_back(new wnet_session(m_io_service, *this, i));
		}

		if (m_sessions.size() != m_max_session)
		{
			LOG_ERROR << "new session failed";
			return false;
		}

		if (!m_available_sessions.init(m_max_session))
		{
			LOG_ERROR << "m_available_sessions init failed";
			return false;
		}

		for (wsession_ptr& session : m_sessions)
		{
			if (!session->init())
			{
				return false;
			}
			m_available_sessions.push(session.get());
		}

		return true;
	}

	void wnet_session_mgr::start()
	{
		// 创建完成包处理线程
		for (uint32 i = 0; i < m_thread_num; ++i)
		{
			m_threads.emplace_back(&wnet_session_mgr::_worker_thread, this);
		}
	}

	void wnet_session_mgr::shutdown()
	{
		if(!m_io_service.stopped())
		{
			// 关闭工作线程
			m_io_service.stop();
		}

		for (std::thread& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
		m_threads.clear();
	}

	void* wnet_session_mgr::alloc(uint32 bytes)
	{
		return m_pool_ptr->alloc(bytes);
	}
	void wnet_session_mgr::free(void* const ptr)
	{
		return m_pool_ptr->free(ptr);
	}

	// 取得一个可用的会话
	wnet_session* wnet_session_mgr::get_available_session()
	{
		wnet_session* temp_ptr = NULL;
		if (!m_available_sessions.safe_pop(temp_ptr))
		{
			LOG_ERROR << " no available_session";
			return NULL;
		}
		return temp_ptr;
	}

	// 归还一个关闭的会话
	void wnet_session_mgr::return_session(wnet_session* session_ptr)
	{	
		m_available_sessions.safe_push(session_ptr);
	}

	bool wnet_session_mgr::send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
	{
		if ((session_id & 0X0000FFFF) >= m_sessions.size())
		{
			return false;
		}

		return m_sessions[session_id & 0X0000FFFF]->send_msg(session_id, msg_id, msg_ptr, size);
	}

	/**
	* 关闭指定连接
	* @param session_id	连接id
	*/
	void wnet_session_mgr::close(uint32 session_id)
	{
		if ((session_id & 0X0000FFFF) >= m_sessions.size())
		{
			return;
		}

		return m_sessions[session_id & 0X0000FFFF]->close(session_id);
	}

	void wnet_session_mgr::handle_close(wnet_session* session_ptr)
	{
		uint32 session_id = session_ptr->get_id();
		session_ptr->reset();

		// 回调上层关闭函数
		m_disconnect_callback(session_id);
		// 将session归还个连接管理池
		return_session(session_ptr);
	}

	void wnet_session_mgr::_worker_thread()
	{
		for (;;)
		{
			try
			{
				m_io_service.run();
				LOG_INFO << "work thread exit";
				break;
			}
			catch (boost::system::system_error& e)
			{
				LOG_ERROR << "work thread exception, code = " << e.code().value();
			}
		}
	}
}
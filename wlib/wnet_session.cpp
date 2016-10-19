#include "wnet_session.h"
#include "wlog.h"
#include "wnet_session_mgr.h"
#include "wnet_msg.h"
#include <boost/bind.hpp>
#include "wnet_ex_type.h"


namespace wang {

	wnet_session::wnet_session(boost::asio::io_service& io_service, wnet_session_mgr& session_mgr, uint16 index)
		: m_session_id(0X00010000 | index)
		, m_socket(io_service)
		, m_session_mgr(session_mgr)
		, m_status(ENSS_NONE)
		, m_is_writing(false)
		, m_is_reading(false)
		, m_recv_buf_ptr(nullptr)
		, m_msg_ptr(nullptr)
		, m_data_buf_ptr(nullptr)
		, m_sending_buf_ptr(nullptr)
		, m_wait_head_ptr(nullptr)
		, m_wait_tail_ptr(nullptr)
	{
		
	}

	wnet_session::~wnet_session()
	{
		destroy();
	}

	bool wnet_session::init()
	{
		// alloc memory
		m_recv_buf_ptr = (wreceive_buffer*)malloc(sizeof(wreceive_buffer) + m_session_mgr.get_recv_buff_size() - 1);
		if (!m_recv_buf_ptr)
		{
			LOG_ERROR << "alloc m_data_buf_ptr memory error!";
			return false;
		}
		//m_recv_buf_ptr->msg_size = 0;
		m_recv_buf_ptr->recv_size = 0;
		m_recv_buf_ptr->expect_size = 0;
		m_recv_buf_ptr->max_size = m_session_mgr.get_recv_buff_size();


		m_data_buf_ptr = (wsend_buffer*)malloc(sizeof(wsend_buffer) + m_session_mgr.get_send_buff_size() - 1);
		if (!m_data_buf_ptr)
		{
			LOG_ERROR << "alloc m_data_buf_ptr memory error!";
			return false;
		}
		m_data_buf_ptr->size = 0;
		m_data_buf_ptr->send_size = 0;
		m_data_buf_ptr->max_size = m_session_mgr.get_send_buff_size();

		m_sending_buf_ptr = (wsend_buffer*)malloc(sizeof(wsend_buffer) + m_session_mgr.get_send_buff_size() - 1);
		if (!m_sending_buf_ptr)
		{
			LOG_ERROR << "alloc m_data_buf_ptr memory error!";
			return false;
		}
		m_sending_buf_ptr->size = 0;
		m_sending_buf_ptr->send_size = 0;
		m_sending_buf_ptr->max_size = m_session_mgr.get_send_buff_size();

		m_status = ENSS_INIT;
		return true;
	}

	void wnet_session::destroy()
	{
		m_status = ENSS_NONE;

		if (m_recv_buf_ptr)
		{
			free(m_recv_buf_ptr);
			m_recv_buf_ptr = NULL;
		}

		if (m_data_buf_ptr)
		{
			free(m_data_buf_ptr);
			m_data_buf_ptr = NULL;
		}
		if (m_sending_buf_ptr)
		{
			free(m_sending_buf_ptr);
			m_sending_buf_ptr = NULL;
		}

		if (m_msg_ptr)
		{
			m_msg_ptr->free_me();
			m_msg_ptr = nullptr;
		}
	}
	
	void wnet_session::reset()
	{
		if ((m_session_id & 0XFFFF0000) == 0XFFFF0000)
		{
			m_session_id += 0X00020000;
		}
		else
		{
			m_session_id += 0X00010000;
		}

		m_is_writing = false;
		m_is_reading = false;

		if (m_recv_buf_ptr)
		{
			//m_recv_buf_ptr->msg_size = 0;
			m_recv_buf_ptr->recv_size = 0;
			m_recv_buf_ptr->expect_size = 0;
		}

		if (m_data_buf_ptr)
		{
			m_data_buf_ptr->size = 0;
			m_data_buf_ptr->send_size = 0;
		}

		if (m_sending_buf_ptr)
		{
			m_sending_buf_ptr->size = 0;
			m_sending_buf_ptr->send_size = 0;
		}

		if (m_wait_head_ptr)
		{
			wwait_buffer* p = m_wait_head_ptr;
			m_wait_head_ptr = NULL;
			m_wait_tail_ptr = NULL;
			while (p)
			{
				wwait_buffer* q = p;
				p = p->next_ptr;
				m_session_mgr.free(q);
			}
		}
	}

	void wnet_session::open(boost::asio::ip::tcp::socket&& socket)
	{	
		m_socket = std::move(socket);
		m_status = ENSS_OPEN;
		m_is_reading.store(true);
	}

	void wnet_session::begin_read()
	{
		// 可能回调函数调用了close
		if (ENSS_OPEN != m_status)
		{
			m_is_reading.store(false);
			_close();
			return;
		}

		// 投递读请求
		//m_recv_buf_ptr->expect_size = sizeof(m_recv_buf_ptr->msg_size);
		m_socket.async_read_some(boost::asio::buffer(&m_recv_buf_ptr->buf, m_recv_buf_ptr->max_size),
			boost::bind(&wnet_session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	bool wnet_session::send_msg(uint32 session_id, uint16 msg_id, const void* msg_ptr, uint32 size)
	{
		// 检查参数及连接的有效性,m_session_id和m_status多线程访问
		if (session_id != m_session_id || 0 == size || !msg_ptr || ENSS_OPEN != m_status)
		{
			return false;
		}

		const uint16 code_msg_id = msg_id ^ m_session_mgr.get_msg_id_key();
		const uint32 code_msg_size = size ^ m_session_mgr.get_msg_size_key();

		{// 括号限制m_mutex保护的范围，避免递归
			// 获得锁
			std::lock_guard<std::mutex> lock(m_mutex);

			if (session_id != m_session_id || ENSS_OPEN != m_status)
			{
				return false;
			}

			if (m_wait_head_ptr)
			{
				return push_to_queue(msg_ptr, size, code_msg_id, code_msg_size);
			}

			if (m_data_buf_ptr->avail() >= sizeof(MSG_HEADER_SIZE))
			{
				m_data_buf_ptr->append(&code_msg_id, sizeof(code_msg_id));
				m_data_buf_ptr->append(&code_msg_size, sizeof(code_msg_size));
				uint32 writed_byte = m_data_buf_ptr->append(msg_ptr, size);
				if (writed_byte < size)
				{
					if (!push_to_queue((char*)msg_ptr + writed_byte, size - writed_byte, 0, 0))
					{
						m_data_buf_ptr->size -= (sizeof(size) + writed_byte);
						return false;
					}
				}
			}
			else if (!push_to_queue(msg_ptr, size, code_msg_id, code_msg_size))
			{
				return false;
			}

			
			bool is_writing = false;
			if (!m_is_writing.compare_exchange_strong(is_writing, true))
			{
				return true;
			}

			std::swap(m_data_buf_ptr, m_sending_buf_ptr);
		}
		m_socket.async_write_some(boost::asio::buffer(m_sending_buf_ptr->buf, m_sending_buf_ptr->size),
			boost::bind(&wnet_session::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		
		return true;
	}

	void wnet_session::close(uint32 session_id)
	{
		// 检查参数及连接的有效性,m_session_id和m_status多线程访问
		if (session_id != m_session_id || ENSS_OPEN != m_status)
		{
			return;
		}

		{// 括号保护m_mutex访问避免递归，helper析构里可能访问m_mutex
			std::lock_guard<std::mutex> guard(m_mutex);

			if (session_id != m_session_id || ENSS_OPEN != m_status)
			{
				return;
			}

			m_status = ENSS_SHUT;

			boost::system::error_code ec;
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			//m_socket.close(ec);
		}

		// 尝试释放资源
		_release();
	}
	
	void wnet_session::_release()
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		if (ENSS_SHUT != m_status || m_is_writing || m_is_reading)
		{
			return;
		}

		// 设置session状态
		m_status = ENSS_CLOSE;

		if (m_socket.is_open())
		{
			boost::system::error_code ec;
			m_socket.close(ec);
		}

		m_session_mgr.get_service().post(boost::bind(&wnet_session_mgr::handle_close, &m_session_mgr, this));

	}

	void wnet_session::_close()
	{
		{// 括号保护m_mutex访问避免递归，helper析构里可能访问m_mutex
			std::lock_guard<std::mutex> guard(m_mutex);

			if (ENSS_OPEN == m_status)
			{
				m_status = ENSS_SHUT;

				boost::system::error_code ec;
				m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
				//m_socket.cancel(ec);
			}
		}

		// 尝试释放资源
		_release();
	}

	void wnet_session::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if (error || 0 == bytes_transferred)
		{
			m_is_reading.store(false);
			_close();
			return;
		}

		// 可能外部调用了close，pool资源会采用统一的close_helper释放
		if (ENSS_OPEN != m_status)
		{
			m_is_reading.store(false);
			_close();
			return;
		}

		m_recv_buf_ptr->recv_size += bytes_transferred;
		
		if (m_msg_ptr || m_recv_buf_ptr->recv_size >= MSG_HEADER_SIZE)
		{
			if (!parse_msg())
			{
				m_is_reading.store(false);
				_close();
				return;
			}
		}
		m_socket.async_read_some(boost::asio::buffer(m_recv_buf_ptr->buf + m_recv_buf_ptr->recv_size, m_recv_buf_ptr->max_size - m_recv_buf_ptr->recv_size),
			boost::bind(&wnet_session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void wnet_session::handle_write(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if (error || 0 == bytes_transferred)
		{
			m_is_writing.store(false);
			_close();
			return;
		}

		if (ENSS_OPEN != m_status)
		{
			m_is_writing.store(false);
			_close();
			return;
		}

		m_sending_buf_ptr->send_size += bytes_transferred;
		
		if (m_sending_buf_ptr->send_size == m_sending_buf_ptr->size)
		{
			m_sending_buf_ptr->size = 0;
			m_sending_buf_ptr->send_size = 0;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (!m_data_buf_ptr->empty())
				{
					std::swap(m_sending_buf_ptr, m_data_buf_ptr);
				}
				else if (m_wait_head_ptr)
				{	
					queue_to_buf();
				}
			}
			if (m_sending_buf_ptr->empty())
			{
				m_is_writing.store(false);
				return;
			}
		}
		// 投递写请求
		m_socket.async_write_some(boost::asio::buffer(m_sending_buf_ptr->buf + m_sending_buf_ptr->send_size, m_sending_buf_ptr->size - m_sending_buf_ptr->send_size),
			boost::bind(&wnet_session::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
		
	bool wnet_session::push_to_queue(const void* msg_ptr, uint32 msg_size, uint16 code_msg_id, uint32 code_msg_size)
	{
		uint32 total_size = msg_size;
		if (code_msg_size > 0)
		{
			total_size += MSG_HEADER_SIZE;
		}
		wwait_buffer* p = (wwait_buffer*)m_session_mgr.alloc(sizeof(wwait_buffer) + total_size - sizeof(((wwait_buffer*)0)->buf));
		if (!p)
		{
			LOG_ERROR << "alloc failed";
			return false;
		}
		p->next_ptr = NULL;
		p->send_size = 0;
		p->total_size = total_size;

		char* buf_ptr = p->buf;

		if (code_msg_size > 0)
		{
			memcpy(buf_ptr, &code_msg_id, sizeof(code_msg_id));
			memcpy(buf_ptr, &code_msg_size, sizeof(code_msg_size));
			buf_ptr += MSG_HEADER_SIZE;
		}

		memcpy(buf_ptr, msg_ptr, msg_size);

		if (!m_wait_tail_ptr)
		{
			m_wait_head_ptr = m_wait_tail_ptr = p;
		}
		else
		{
			m_wait_tail_ptr->next_ptr = p;
			m_wait_tail_ptr = p;
		}

		return true;
	}

	void wnet_session::queue_to_buf()
	{
		while (m_wait_head_ptr)
		{
			const uint32 remain_size = m_wait_head_ptr->total_size - m_wait_head_ptr->send_size;
			const uint32 writed_byte = m_sending_buf_ptr->append(m_wait_head_ptr->buf + m_wait_head_ptr->send_size, remain_size);
			if (writed_byte == 0)
			{
				break;
			}

			if (writed_byte == remain_size)
			{
				//this msg send over
				wwait_buffer* q = m_wait_head_ptr;
				m_wait_head_ptr = m_wait_head_ptr->next_ptr;
				m_session_mgr.free(q);
			}
			else
			{
				//buf is full
				m_wait_head_ptr->send_size += writed_byte;
				break;
			}
		}

		if (!m_wait_head_ptr)
		{
			m_wait_tail_ptr = NULL;
		}
	}

	bool wnet_session::parse_msg()
	{	
		uint32 parse_size = 0;
		while (true)
		{
			if (!m_msg_ptr)
			{
				if (m_recv_buf_ptr->recv_size - parse_size < MSG_HEADER_SIZE)
				{
					//不够一个消息	头	
					break;
				}
				else
				{
					wmsg_header* header_ptr = reinterpret_cast<wmsg_header*>(m_recv_buf_ptr->buf);
					m_recv_buf_ptr->expect_size = header_ptr->msg_size ^ m_session_mgr.get_msg_size_key();
					if (m_recv_buf_ptr->expect_size > m_session_mgr.get_max_received_msg_size())
					{
						LOG_ERROR << "msg size too big " << m_recv_buf_ptr->expect_size << ", max_size =" << m_session_mgr.get_max_received_msg_size();
						return false;
					}

					m_msg_ptr = wnet_msg::alloc_me(m_session_mgr.get_pool_ptr(), m_recv_buf_ptr->expect_size);
					if (!m_msg_ptr)
					{
						LOG_ERROR << "alloc msg failed";
						return false;
					}
					m_msg_ptr->set_session_id(m_session_id);
					m_msg_ptr->set_msg_id(header_ptr->msg_id ^ m_session_mgr.get_msg_id_key());
					parse_size += MSG_HEADER_SIZE;
				}
			}

			if (m_recv_buf_ptr->recv_size - parse_size >= m_recv_buf_ptr->expect_size)
			{
				//一个完成消息
				if (m_recv_buf_ptr->expect_size > 0)
				{
					m_msg_ptr->append(m_recv_buf_ptr->buf + parse_size, m_recv_buf_ptr->expect_size);
					parse_size += m_recv_buf_ptr->expect_size;
					m_recv_buf_ptr->expect_size = 0;
				}
				
				m_session_mgr.get_msg_callback()(m_msg_ptr);
				m_msg_ptr = NULL;
			}
			else
			{
				// 不足一个消息
				m_msg_ptr->append(m_recv_buf_ptr->buf + parse_size, m_recv_buf_ptr->recv_size - parse_size);
				m_recv_buf_ptr->expect_size -= (m_recv_buf_ptr->recv_size - parse_size);
				parse_size = m_recv_buf_ptr->recv_size;
				break;
			}
		}
		if (parse_size < m_recv_buf_ptr->recv_size)
		{
			m_recv_buf_ptr->recv_size -= parse_size;
			memmove(m_recv_buf_ptr->buf, m_recv_buf_ptr->buf + parse_size, m_recv_buf_ptr->recv_size);
		}
		else
		{
			m_recv_buf_ptr->recv_size = 0;
		}
		return true;
	}

	uint32 wnet_session::wsend_buffer::append(const void* p, uint32 len)
	{
		if (len > avail())
		{
			len = avail();
		}
		
		memcpy(get_cur_buf(), p, len);
		size += len;
		return len;
	}

} // namespace nexus

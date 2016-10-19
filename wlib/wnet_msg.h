#ifndef _W_NET_MSG_H_
#define _W_NET_MSG_H_

#include "wnet_type.h"

namespace wang
{
	class wnet_mem_pool;

	class wnet_msg
	{
	private:
		explicit wnet_msg(wnet_mem_pool* pool_ptr);
		
	public:
		static wnet_msg* alloc_me(wnet_mem_pool* pool_ptr, uint32 size);
		void free_me();

	public:
		wnet_msg* get_next() { return m_next_ptr; }
		void set_next(wnet_msg* value) { m_next_ptr = value; }

		uint32 get_session_id() const { return m_session_id; }
		void set_session_id(uint32 value) { m_session_id = value; }

		uint16 get_msg_id() const { return m_msg_id; }
		void set_msg_id(uint16 value) { m_msg_id = value; }

		uint32 get_size() const { return m_size; }
		char* get_buf() { return m_buffer; }

		void append(const void* p, uint32 len);

	private:
		wnet_mem_pool*	m_pool_ptr;
		wnet_msg*		m_next_ptr;
		uint32			m_session_id;
		uint16			m_msg_id;
		uint32			m_size;
		char			m_buffer[sizeof(uint32)];
	};
}

#endif //_W_NET_EX_TYPE_H_

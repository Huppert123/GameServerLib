#include "wnet_msg.h"
#include "wnet_mem_pool.h"
#include <string.h>

namespace wang
{
	wnet_msg::wnet_msg(wnet_mem_pool* pool_ptr)
		: m_pool_ptr(pool_ptr)
		, m_next_ptr(nullptr)
		, m_session_id(0)
		, m_size(0)
	{}
	
	wnet_msg* wnet_msg::alloc_me(wnet_mem_pool* pool_ptr, uint32 size)
	{
		void* p = pool_ptr->alloc(sizeof(wnet_mem_pool) + size - sizeof(((wnet_msg*)0)->m_buffer));
		if (!p)
		{
			return nullptr;
		}
		wnet_msg* ret = new (p) wnet_msg(pool_ptr);
		return ret;
	}

	void wnet_msg::free_me()
	{
		m_pool_ptr->free(this);
	}

	void wnet_msg::append(const void* p, uint32 len)
	{
		memcpy(m_buffer + m_size, p, len);
		m_size += len;
	}
}

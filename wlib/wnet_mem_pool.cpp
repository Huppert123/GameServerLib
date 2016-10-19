#include "wnet_mem_pool.h"
#include <string.h>

namespace wang
{
	wnet_mem_pool::wnet_mem_pool(uint32 max_pool_size_mb)
		: m_max_size(max_pool_size_mb * 1024 * 1024)
		, m_malloc(0)
		, m_gc_count(0)
		, m_current_size(0)
	{
		memset(m_pool, 0, sizeof(m_pool));
	}

	//-----------------------------------------------------------------------------
	// destruction
	//-----------------------------------------------------------------------------


	wnet_mem_pool::~wnet_mem_pool()
	{
		destroy();
	}

	void wnet_mem_pool::destroy()
	{
		for (int32 i = 0; i < 16; ++i)
		{
			while (m_pool[i].first_ptr)
			{
				nnode* pNode = m_pool[i].first_ptr;
				m_pool[i].first_ptr = m_pool[i].first_ptr->next_ptr;
				::free(pNode);
			}
		}
		memset(m_pool, 0, sizeof(m_pool));
	}

	//-----------------------------------------------------------------------------
	// 分配
	//-----------------------------------------------------------------------------

	void* wnet_mem_pool::alloc(uint32 bytes)
	{
		uint32 realSize = 0;
		int32 index = size2index(bytes, realSize);

		if (-1 != index)
		{
			// 提前尝试
			if (m_pool[index].first_ptr)
			{
				guard_type guard(m_lock);

				// 池里有，就从池里分配
				if (m_pool[index].first_ptr)
				{
					nnode* pNode = m_pool[index].first_ptr;
					m_pool[index].first_ptr = m_pool[index].first_ptr->next_ptr;

					if (m_pool[index].first_ptr)
					{
						m_pool[index].first_ptr->pre_ptr = NULL;
					}
					else
					{
						m_pool[index].last_ptr = NULL;
					}

					m_current_size -= realSize;
					++pNode->use_count;
					return pNode->mem_ptr;
				}
			}
		}

		++m_malloc;
		nnode* pNodePtr = reinterpret_cast<nnode*>(::malloc(realSize + sizeof(nnode) - sizeof(uint32)));

		if (!pNodePtr)
		{
			return NULL;
		}

		pNodePtr->index = index;
		pNodePtr->size = realSize;
		pNodePtr->next_ptr = NULL;
		pNodePtr->pre_ptr = NULL;
		pNodePtr->use_count = 0;
		return pNodePtr->mem_ptr;			// 从实际内存中分配
	}

	//-----------------------------------------------------------------------------
	// 释放
	//-----------------------------------------------------------------------------

	void wnet_mem_pool::free(void* const pMem)
	{
		nnode* pNodePtr = reinterpret_cast<nnode*>(((char*)pMem) - sizeof(nnode) + sizeof(uint32));

		if (-1 != pNodePtr->index)
		{
			guard_type guard(m_lock);

			if (pNodePtr->size + m_current_size > m_max_size && pNodePtr->use_count > 0)
			{
				// 垃圾收集
				garbage_collect(pNodePtr->size * 2, pNodePtr->use_count);
			}

			// 内存池可以容纳
			if (pNodePtr->size + m_current_size <= m_max_size)
			{
				pNodePtr->pre_ptr = NULL;
				pNodePtr->next_ptr = m_pool[pNodePtr->index].first_ptr;

				if (m_pool[pNodePtr->index].first_ptr)
				{
					m_pool[pNodePtr->index].first_ptr->pre_ptr = pNodePtr;
				}
				else
				{
					m_pool[pNodePtr->index].last_ptr = pNodePtr;
				}

				m_pool[pNodePtr->index].first_ptr = pNodePtr;
				m_current_size += pNodePtr->size;
				return;
			}
		}

		::free(pNodePtr);
	}


	//-----------------------------------------------------------------------------
	// 垃圾收集
	//-----------------------------------------------------------------------------

	void wnet_mem_pool::garbage_collect(uint32 expectSize, uint32 useCount)
	{
		++m_gc_count;
		uint32 freeSize = 0;

		// 从最大的开始回收
		for (int32 i = 15; i >= 0; --i)
		{
			if (!m_pool[i].first_ptr)
			{
				continue;
			}

			// 从最后开始释放，因为后面的Node使用次数少
			nnode* pNode = m_pool[i].last_ptr;

			while (pNode)
			{
				nnode* pTempNode = pNode;
				pNode = pNode->pre_ptr;

				// 释放此节点
				if (pTempNode->use_count < useCount)
				{
					if (pNode)
					{
						pNode->next_ptr = pTempNode->next_ptr;
					}

					if (pTempNode->next_ptr)
					{
						pTempNode->next_ptr->pre_ptr = pNode;
					}

					if (m_pool[i].last_ptr == pTempNode)
					{
						m_pool[i].last_ptr = pNode;
					}

					if (m_pool[i].first_ptr == pTempNode)
					{
						m_pool[i].first_ptr = pTempNode->next_ptr;
					}

					m_current_size -= pTempNode->size;
					freeSize += pTempNode->size;
					::free(pTempNode);
				}

				if (freeSize >= expectSize)
				{
					return;
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// 返回最匹配的大小
	//-----------------------------------------------------------------------------
	int32 wnet_mem_pool::size2index(uint32 nSize, uint32& realSize)
	{
		if (nSize <= 32) { realSize = 32;			return 0; }
		if (nSize <= 64) { realSize = 64;			return 1; }
		if (nSize <= 128) { realSize = 128;			return 2; }
		if (nSize <= 256) { realSize = 256;			return 3; }
		if (nSize <= 512) { realSize = 512;			return 4; }
		if (nSize <= 1024) { realSize = 1024;			return 5; }
		if (nSize <= 2 * 1024) { realSize = 2 * 1024;		return 6; }
		if (nSize <= 4 * 1024) { realSize = 4 * 1024;		return 7; }
		if (nSize <= 8 * 1024) { realSize = 8 * 1024;		return 8; }
		if (nSize <= 16 * 1024) { realSize = 16 * 1024;		return 9; }
		if (nSize <= 32 * 1024) { realSize = 32 * 1024;		return 10; }
		if (nSize <= 64 * 1024) { realSize = 64 * 1024;		return 11; }
		if (nSize <= 128 * 1024) { realSize = 128 * 1024;	return 12; }
		if (nSize <= 256 * 1024) { realSize = 256 * 1024;	return 13; }
		if (nSize <= 512 * 1024) { realSize = 512 * 1024;	return 14; }
		if (nSize <= 1024 * 1024) { realSize = 1024 * 1024;	return 15; }
		realSize = nSize;
		return -1;
	}

}

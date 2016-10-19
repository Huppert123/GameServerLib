#ifndef _W_NET_MEM_POOL_H_
#define _W_NET_MEM_POOL_H_

#include <mutex>
#include "wnet_type.h"

namespace wang
{
	class wnet_mem_pool
	{
	public:
		typedef std::mutex lock_type;
		typedef std::lock_guard<lock_type> guard_type;

	public:
		wnet_mem_pool(uint32 max_pool_size_mb);
		~wnet_mem_pool();

		void destroy();

		void*	alloc(uint32 bytes);
		void	free(void* const ptr);

		uint32	get_size()		const { return m_current_size; }
		uint32	get_malloc()	const { return m_malloc; }
		uint32	get_gc()		const { return m_gc_count; }

	private:
		// 垃圾收集
		inline void garbage_collect(uint32 expectSize, uint32 useCount);
		// 返回最匹配的大小
		inline int32 size2index(uint32 nSize, uint32& realSize);

	private:
		wnet_mem_pool(const wnet_mem_pool& rht);
		wnet_mem_pool& operator=(const wnet_mem_pool& rht);

	private:
		// 内存块头描述
		struct nnode
		{
			nnode*		next_ptr;
			nnode*		pre_ptr;
			int32		index;
			uint32		size;
			uint32		use_count;
			uint32		mem_ptr[1];							// 实际内存空间
		};

		struct
		{
			nnode*		first_ptr;
			nnode*		last_ptr;
		}m_pool[16];

		uint32			m_max_size;						// 外部设定的最大允许空闲内存
		uint32			m_malloc;						// 统计用，实际Malloc次数
		uint32			m_gc_count;						// 统计用，实际垃圾收集次数

		uint32  		m_current_size;					// 内存池中空闲内存总数

		lock_type		m_lock;
	};


}
#endif // _W_NET_MEM_POOL_H_

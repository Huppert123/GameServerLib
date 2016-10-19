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
		// �����ռ�
		inline void garbage_collect(uint32 expectSize, uint32 useCount);
		// ������ƥ��Ĵ�С
		inline int32 size2index(uint32 nSize, uint32& realSize);

	private:
		wnet_mem_pool(const wnet_mem_pool& rht);
		wnet_mem_pool& operator=(const wnet_mem_pool& rht);

	private:
		// �ڴ��ͷ����
		struct nnode
		{
			nnode*		next_ptr;
			nnode*		pre_ptr;
			int32		index;
			uint32		size;
			uint32		use_count;
			uint32		mem_ptr[1];							// ʵ���ڴ�ռ�
		};

		struct
		{
			nnode*		first_ptr;
			nnode*		last_ptr;
		}m_pool[16];

		uint32			m_max_size;						// �ⲿ�趨�������������ڴ�
		uint32			m_malloc;						// ͳ���ã�ʵ��Malloc����
		uint32			m_gc_count;						// ͳ���ã�ʵ�������ռ�����

		uint32  		m_current_size;					// �ڴ���п����ڴ�����

		lock_type		m_lock;
	};


}
#endif // _W_NET_MEM_POOL_H_

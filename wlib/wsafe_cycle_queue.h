/********************************************************************
	created:	2015/04/10

	author:		wanghaibo
	desc:		cycle queue with fix size
*********************************************************************/

#ifndef _W_SAFE_CYCLE_QUEUE_H_
#define _W_SAFE_CYCLE_QUEUE_H_

#include <mutex>
#include "wcycle_queue.h"

namespace wang
{
	template<typename T>
	class wsafe_cycle_queue : public wcycle_queue<T>
	{
		//-- member function
	public:		
		bool safe_push(const T& data);
		bool safe_pop(T& data);

	private:
		std::mutex m_mutex;
	};
	
	template<typename T>
	bool wsafe_cycle_queue<T>::safe_push(const T& data)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return this->push(data);
	}

	template<typename T>
	bool wsafe_cycle_queue<T>::safe_pop(T& data)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return this->pop(data);
	}
}
#endif
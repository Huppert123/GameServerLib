#include "wthread_pool.h"

namespace wang
{	
	wthread_pool::wthread_pool() : m_stop{ false }
	{
		
	}

	wthread_pool::~wthread_pool()
	{
		shutdown();		
	}

	void wthread_pool::start(size_t size)
	{
		size = size < 1 ? 1 : size;
		m_threads.reserve(size);
		for (size_t i = 0; i < size; ++i)
		{
			m_threads.emplace_back(&wthread_pool::schedual, this);    // push_back(std::thread{...})
		}
	}

	void wthread_pool::post_task(wTask task)
	{
		// 添加任务到队列
		{
			std::lock_guard<std::mutex> lock(m_lock);
			if (m_stop)
			{
				return;
			}
			m_tasks.push(task);
		}
		m_condition.notify_one();    // 唤醒线程执行
	}

	void wthread_pool::shutdown()
	{
		{
			std::lock_guard<std::mutex> lock(m_lock);
			if (m_stop)
			{
				return;
			}
			m_stop = true;
		}

		m_condition.notify_all();
		for (auto& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
		m_threads.clear();
	}

	// 任务调度
	void wthread_pool::schedual()
	{
		wTask task;
		while (true)
		{
			//get task
			{
				std::unique_lock<std::mutex> lock{ m_lock };
				m_condition.wait(lock, [this]() { return !m_tasks.empty() || m_stop; });
				if (m_tasks.empty())
				{
					break;
				}
				task = m_tasks.front();    // 取一个 task
				m_tasks.pop();
			}
			task();
		}
	}
}


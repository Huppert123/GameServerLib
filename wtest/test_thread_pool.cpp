#include "test_util.h"
#include "wthread_pool.h"

void thread_func1(int value)
{
	LOG_INFO << "thread_func1, value=" << value;
}

struct thread_funcs {

	void func1(int value)
	{
		LOG_INFO << "thread_funcs func1, value=" << value;
	}
};


void test_thread_pool()
{
	TEST_BEGIN;

	wang::wthread_pool executor;
	executor.start(10);

	thread_funcs ht;
	for (int i = 0; i < 100; ++i)
	{
		executor.post_task(std::bind(thread_func1, i));
		executor.post_task(std::bind(&thread_funcs::func1, &ht, i));
	}	

	executor.shutdown();

	TEST_PASS;
}
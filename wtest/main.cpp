#include <iostream>
#include <string>
#include "wlog.h"

void test_int2str();
void test_log();
void test_thread_pool();
void test_cycle_buffer();

void test_all()
{
	test_int2str();
	test_log();
	test_thread_pool();
	test_cycle_buffer();
}

int main(int argc, char * argv[])
{
	try
	{
		std::string app_name(argv[0]);
		app_name.append(".log");

		wang::wlog::init(app_name, 50, true);

		test_all();

		wang::wlog::destroy();
	}
	catch (std::exception)
	{
		return 1;
	}
	return 0;
}
#include "wlog.h"
#include "test_util.h"
#include <limits>

void test_log_type()
{
	TEST_BEGIN;
	
	LOG_INFO << "bool true=" << true;
	
	LOG_INFO << "bool false=" << false;

	LOG_INFO << "char a=" << 'a';

	LOG_INFO << "signed char max=" << std::numeric_limits<signed char>::max();
	LOG_INFO << "unsigned char max=" << std::numeric_limits<unsigned char>::max();

	LOG_INFO << "signed short max=" << std::numeric_limits<signed short>::max();
	LOG_INFO << "unsigned short max=" << std::numeric_limits<unsigned short>::max();

	LOG_INFO << "signed int max=" << std::numeric_limits<signed int>::max();
	LOG_INFO << "unsigned int max=" << std::numeric_limits<unsigned int>::max();

	LOG_INFO << "signed long max=" << std::numeric_limits<signed long>::max();
	LOG_INFO << "unsigned long max=" << std::numeric_limits<unsigned long>::max();

	LOG_INFO << "signed long long max=" << std::numeric_limits<signed long long>::max();
	LOG_INFO << "unsigned long long max=" << std::numeric_limits<unsigned long long>::max();

	LOG_INFO << "float max=" << std::numeric_limits<float>::max();
	LOG_INFO << "double long max=" << std::numeric_limits<double>::max();

	TEST_PASS;
}

void test_normal_log_level()
{
	TEST_BEGIN;

	LOG_DEBUG << "this is debug log";
	LOG_INFO << "this is info log";
	LOG_WARN << "this is warning log";
	LOG_ERROR << "this is error log";
	LOG_FATAL << "this is fatal error log";

	TEST_PASS;
}

void test_simple_log_level()
{
	TEST_BEGIN;

	SLOG << "this is simple log";

	TEST_PASS;
}

void test_prefix_log_level()
{
	TEST_BEGIN;

	LOG_DEBUG << "this is debug log";
	LOG_INFO << "this is info log";
	LOG_WARN << "this is warning log";
	LOG_ERROR << "this is error log";
	LOG_FATAL << "this is fatal error log";

	TEST_PASS;
}

void test_log()
{
	test_log_type();
	test_normal_log_level();
	test_simple_log_level();
	test_prefix_log_level();
}
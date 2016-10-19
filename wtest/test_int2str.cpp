#include "wint2str.h"
#include "test_util.h"

void test_int2str_dec()
{
	TEST_BEGIN;

	char buf[60];

#define MY_TEST(TYPE, VALUE) \
wang::int2str_dec(buf, static_cast < TYPE >(VALUE));\
ASSERT_TEST(std::string(buf), std::string(#VALUE))

	MY_TEST(char, 100);
	MY_TEST(char, -100);
	MY_TEST(unsigned char, 100);

	MY_TEST(short, 30000);
	MY_TEST(short, -30000);
	MY_TEST(unsigned short, 30000);

	MY_TEST(int, 500000);
	MY_TEST(int, -500000);
	MY_TEST(unsigned int, 500000);

	MY_TEST(long, 1000000);
	MY_TEST(long, -1000000);
	MY_TEST(unsigned long, 1000000);

	MY_TEST(long long, 10000000000);
	MY_TEST(long long, -10000000000);
	MY_TEST(unsigned long long, 10000000000);

#undef MY_TEST

	TEST_PASS;
}

void test_int2str_hex()
{
	TEST_BEGIN;

	char buf[60];
	std::string str;

#define MY_TEST(TYPE, VALUE) \
wang::int2str_hex(buf, static_cast < TYPE >(VALUE));\
str.clear();\
str.append("0x");\
str.append(buf);\
ASSERT_TEST(str, std::string(#VALUE))

	MY_TEST(char, 0x77);
	MY_TEST(unsigned char, 0x98);

	MY_TEST(short, 0x7777);
	MY_TEST(unsigned short, 0x9898);

	MY_TEST(int, 0x77777777);
	MY_TEST(unsigned int, 0x98989898);

	MY_TEST(long, 0x77777777);
	MY_TEST(unsigned long, 0x98989898);

	MY_TEST(long long, 0x7777777777777777);
	MY_TEST(unsigned long long, 0x9898989898989898);

#undef MY_TEST

	TEST_PASS;
}

void test_int2str()
{
	test_int2str_dec();
	test_int2str_hex();
}
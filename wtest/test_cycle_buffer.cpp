#include "wlog.h"
#include "test_util.h"
#include "wcycle_buffer.h"
#include <string>
#include <ctime>
#include <string.h>

void test_cycle_buffer()
{
	TEST_BEGIN;
	
	char src_buf[11] = {0};
	
	wang::wcycle_buffer cbuf(src_buf, sizeof(src_buf) - 1);

	char src_digit[128] = {0};

	int index = 0;
	for (int i = '0'; i <= '9'; ++i)
	{
		src_digit[index++] = i;
	}
	for (int i = 'a'; i <= 'z'; ++i)
	{
		src_digit[index++] = i;
	}
	for (int i = 'A'; i <= 'Z'; ++i)
	{
		src_digit[index++] = i;
	}

	char dst_digit[sizeof(src_buf)] = { 0 };
	char dst_digit1[sizeof(dst_digit)] = { 0 };

	srand(static_cast<int>(time(NULL)));

	for (int i = 0; i < 100; ++i)
	{
		int total_len = 0;
		do
		{
			int start_i = rand() % (index - 10);
			int len = rand() % 10;
			if (len == 0)
			{
				len = 10;
			}
			if (cbuf.write(src_digit + start_i, len))
			{
				memcpy(dst_digit1 + total_len, src_digit + start_i, len);
				total_len += len;
			}
			else
			{
				break;
			}
		}while (true);
		cbuf.read(dst_digit, total_len);
		ASSERT_TEST(std::string(dst_digit1, total_len), std::string(dst_digit, dst_digit + total_len));
	}

	/*{
		cbuf.write(src_digit, 1);
		cbuf.read(dst_digit, 1);
		ASSERT_TEST(src_digit[0], dst_digit[0]);
	}
	

	{
		cbuf.write(src_digit, 10);
		cbuf.read(dst_digit, 10);
		ASSERT_TEST(std::string(src_digit), std::string(dst_digit, dst_digit + 10));
	}	*/

	TEST_PASS;
}

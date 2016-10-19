#ifndef _W_TEST_UTIL_H__ 
#define _W_TEST_UTIL_H__

#include <iostream>
#include <string>
#include "wlog.h"

template<typename T>
void assert_test(const T& src, const T& dst, const char * func, int line)
{
	if (src == dst)
	{
		SLOG << src << "=" << dst << " pass";
	}
	else
	{
		SLOG << "func:" << func << " line:" << line
			<< src << "!=" << dst << " failed";
		throw std::exception();
	}
}

#define ASSERT_TEST(SRC, DST) assert_test((SRC), (DST), __FUNCTION__, __LINE__)

#define ASSERT_TEST1(EXP) \
if( (EXP) ) {\
	SLOG << #EXP << " pass";\
} else {\
	LOG_ERROR << #EXP << " failed";\
}

#define TEST_BEGIN SLOG << __FUNCTION__ << " begin"

#define TEST_PASS SLOG << __FUNCTION__ << " pass"

#endif // _W_TEST_UTIL_H__
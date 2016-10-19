#include "wutil.h"
#include <thread>
#include <chrono>

namespace wang {

	const std::string g_null_str;

	void wsleep(uint32 ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
} // namespace wang

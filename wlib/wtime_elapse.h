#ifndef _W_TIME_ELAPSE_H_
#define _W_TIME_ELAPSE_H_

#include "wnet_type.h"
#include "wtime_api.h"

namespace wang {
   
	class wtime_elapse
	{
	public:
		wtime_elapse() : m_cur_time(0)
		{
			m_cur_time = wtime_api::get_time_ms();
		}

		uint32 get_elapse()
		{
			int64 last_time = m_cur_time;
			m_cur_time = wtime_api::get_time_ms();
			if (m_cur_time >= last_time)
			{
				return static_cast<uint32>(m_cur_time - last_time); 
			}
			else
			{
				return 1;
			}
		}

	private:
		int64	m_cur_time;
	};

} // namespace wang

#endif // _W_TIME_ELAPSE_H_

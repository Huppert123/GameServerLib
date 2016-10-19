#ifndef _W_MAIN_THREAD_H_
#define _W_MAIN_THREAD_H_

#include <thread>

namespace wang
{
	class wmain_thread
	{
		//-- constructor/destructor
	public:
		wmain_thread();
		~wmain_thread();

		//-- implement member function
	public:
		bool init(const char* log_filename, const char* config_filename);
		
		bool work();

		void destroy();

		void stop() { m_stop = true; }

	private:
		void _update_thread();
		void _process_input();

	private:
		volatile bool m_stop;
		volatile bool m_update_thread_stop;
		std::thread m_update_thread;
	};

}
#endif


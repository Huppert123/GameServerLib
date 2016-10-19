#ifndef _W_MAIN_THREAD_H_
#define _W_MAIN_THREAD_H_

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
		volatile bool m_stop;
	};

}
#endif


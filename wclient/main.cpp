#include <string>
#include <signal.h>
#include "wlog.h"
#include "wmain_thread.h"


using namespace wang;
using namespace std;

wmain_thread g_main;

void StopMain(int i)
{
	g_main.stop();
}

void RegisterSignal()
{
	signal(SIGINT, StopMain);
	signal(SIGTERM, StopMain);
}

int main(int argc, char * argv[])
{
	RegisterSignal();
	try
	{
		std::string app_name(argv[0]);
		app_name.append(".log");

		const char* cfg_ptr = "cfg.txt";
		if (argc > 1)
		{
			cfg_ptr = argv[1];
		}

		if (g_main.init(app_name.c_str(), cfg_ptr))
		{
			g_main.work();
		}
	}
	catch (std::exception)
	{
		
	}
	g_main.destroy();
	return 0;
}
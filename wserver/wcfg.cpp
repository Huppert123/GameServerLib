#include "wcfg.h"
#include "wlog.h"

namespace wang {

	wcfg g_cfg;

	wcfg::wcfg()
	{
	}

	wcfg::~wcfg()
	{
	}

	bool wcfg::init(const char* config_filename)
	{
		if (false == wserver_cfg::init(ECI_Max, config_filename))
		{
			LOG_ERROR << "init config, set values count error, values_count = " << ECI_Max;
			return false;
		}		

		return true;
	}

	void wcfg::destroy()
	{
		wserver_cfg::destroy();
	}

	bool wcfg::init_more()
	{
		set_uint32(ECI_MsgIdKey, "msg_id_key", 0x5680);
		set_uint32(ECI_MsgSizeKey, "msg_size_key", 0x6C078965);
		set_uint32(ECI_MsgCrypto, "msg_cropto", 0);
		return true;
	}
} // namespace wang

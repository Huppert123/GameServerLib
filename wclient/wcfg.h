#ifndef _W_CFG_H_
#define _W_CFG_H_

#include "wserver_cfg.h"

namespace wang {

	enum EConfigIndex
	{
		ECI_None			= ECI_ServerMax - 1,

		ECI_MsgIdKey,
		ECI_MsgSizeKey,
		ECI_MsgCrypto,
		ECI_Max,
	};

	class wcfg : public wserver_cfg
	{
	public:
		wcfg();
		~wcfg();

		bool init(const char* config_filename);
		void destroy();

	protected:
		virtual bool init_more();
	};

	extern wcfg g_cfg;

} // namespace wang

#endif // _NGAME_NCFG_H_

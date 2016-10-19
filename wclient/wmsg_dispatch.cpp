#include "wmsg_dispatch.h"
#include "wmsg_register.h"
#include "wclient_session.h"

namespace wang
{
	wmsg_dispatch g_msg_dispatch;

	wmsg_dispatch::wmsg_dispatch()
	{

	}
	wmsg_dispatch::~wmsg_dispatch()
	{

	}
	
	bool wmsg_dispatch::init()
	{
		if(!m_wan_msg_handler.init(1, 10))
		{
			return false;
		}

		if(!register_msg())
		{
			return false;
		}

		return true;
	}
	void wmsg_dispatch::destroy()
	{
		m_wan_msg_handler.destroy();
	}
	bool wmsg_dispatch::register_msg()
	{
		REGISTER_RECEIVE_MSG(m_wan_msg_handler, 1, &wclient_session::handle_test, 0);
		return true;
	}

}
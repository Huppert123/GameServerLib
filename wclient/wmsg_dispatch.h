#ifndef _W_MSG_DISPATCH_H_
#define _W_MSG_DISPATCH_H_

#include "wnet_type.h"
#include "wmsg_handler.h"

namespace wang
{
	class wclient_session;
	class wnet_msg;

	class wmsg_dispatch
	{
	public:
		typedef void (wclient_session::*wan_msg_handler_func)(wnet_msg* msg_ptr);
		typedef wmsg_handler<wan_msg_handler_func> wan_msg_handler;
		typedef wan_msg_handler::whandler wan_handler;

	public:
		wmsg_dispatch();
		~wmsg_dispatch();

		bool init();
		void destroy();

		const wan_handler* get_wan_handler(uint16 msg_id) const { return m_wan_msg_handler.get_handler(msg_id); }

	private:
		bool	register_msg();
		wan_msg_handler m_wan_msg_handler;
	};

	extern wmsg_dispatch g_msg_dispatch;
}
#endif

#ifndef _W_NET_EX_TYPE_H_
#define _W_NET_EX_TYPE_H_

#include "wnet_type.h"

namespace wang
{
#pragma pack(push,1)
	struct wmsg_header
	{
		uint16 msg_id;
		uint32 msg_size;
	};

#pragma pack(pop)

	static const uint32 MSG_HEADER_SIZE = sizeof(wmsg_header);
}

#endif //_W_NET_EX_TYPE_H_

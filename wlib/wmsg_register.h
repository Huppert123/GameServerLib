#ifndef _W_MSG_REGISTER_H_
#define _W_MSG_REGISTER_H_

#include "wnet_type.h"

#define  REGISTER_RECEIVE_MSG(MGR, MSG_ID, MSG_HANDLER, STATUS) if(!MGR.register_msg(MSG_ID, MSG_HANDLER, STATUS)) return false;

namespace wang
{	

}

#endif // _W_MSG_REGISTER_H_

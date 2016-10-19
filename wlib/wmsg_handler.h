/********************************************************************
	created:	copyright fast time Inc

	author:		WangHaibo

	purpose:	msg create realse execute
*********************************************************************/


#ifndef _W_MSG_HANDLER_H_
#define _W_MSG_HANDLER_H_

#include "wnet_type.h"
#include "wlog.h"

namespace wang
{
	template<typename T>
	class wmsg_handler
	{		   
		//-- member data
	public:
		typedef T handler_type;
	
		struct whandler
		{	
			uint32					status;
			handler_type			handler;

			whandler() : status(0), handler(NULL) {}
		};

		//-- constructor/destructor
	public:
		//[min_msg_id, max_msg_id)
		wmsg_handler() : m_handlers(NULL), m_min_msg_id(0), m_max_msg_id(0){}
		~wmsg_handler() { destroy(); }

		//-- member function
	public:
		bool init(uint32 min_msg_id, uint32 max_msg_id);
		void destroy();
		bool register_msg(uint16 msg_id, handler_type handler, uint32 status);
		whandler* get_handler(uint16 msg_id) const;

	private:
		bool invalid_msg_id(uint16 msg_id) const
		{ 
			return (msg_id < m_min_msg_id || msg_id >= m_max_msg_id); 
		}

		uint32 get_msg_index(uint16 msg_id) const
		{ 
			return msg_id - m_min_msg_id; 
		}

	private:
		wmsg_handler(const wmsg_handler& rht);
		wmsg_handler& operator=(const wmsg_handler& rht);

	private:
		whandler*		m_handlers;
		uint32			m_min_msg_id;
		uint32			m_max_msg_id;
	};

	template<typename T>
	bool wmsg_handler<T>::init(uint32 min_msg_id, uint32 max_msg_id)
	{
		if(min_msg_id >= max_msg_id)
		{
			return false;
		}	

		uint32 msg_num = max_msg_id - min_msg_id;

		m_handlers = new whandler[msg_num];
		if(!m_handlers)
		{
			return false;
		}
		m_min_msg_id = min_msg_id;
		m_max_msg_id = max_msg_id;
		return true;
	}

	template<typename T>
	void wmsg_handler<T>::destroy()
	{
		if (m_handlers)
		{
			delete[]  m_handlers;
			m_handlers = NULL;
		}
		m_min_msg_id = 0;
		m_max_msg_id = 0;
	}

	template<typename T>
	bool wmsg_handler<T>::register_msg(uint16 msg_id, handler_type handler, uint32 status)
	{
		if(!m_handlers)
		{
			return false;
		}

		if (invalid_msg_id(msg_id))
		{
			LOG_ERROR << "invalid msg " << msg_id;
			return false;
		}

		uint32 msg_index = get_msg_index(msg_id);

		if (m_handlers[msg_index].handler != NULL)
		{
			LOG_ERROR << "msg has been register " << msg_id;
			return false;
		}
		m_handlers[msg_index].status = status;
		m_handlers[msg_index].handler = handler;

		return true;
	}

	template<typename T>
	typename wmsg_handler<T>::whandler* wmsg_handler<T>::get_handler(uint16 msg_id) const
	{
		if (invalid_msg_id(msg_id))
		{
			return NULL;
		}
		uint32 msgIndex = get_msg_index(msg_id);
		return &m_handlers[msgIndex];
	}
}
#endif														
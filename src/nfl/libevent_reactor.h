#ifndef __LIBEVENT_REACTOR_H__
#define __LIBEVENT_REACTOR_H__

#include "nfl_base.h"
#include "cq_base.h"
#include "event_loop_thread_base.h"

namespace nfl
{

/** 
 * @class	libevent_reactor
 * 
 * @brief	
 *		该类由无锁event_base与notify配合，并运行在实际的线程中
 *	运行时跨线程操作由notify通知，中断的eventloop会调用process_notify
 *
 * detail...
 * 
 * @author	mklong
 * @date	2014/6/12
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */

	class libevent_reactor: public event_loop_thread_base
	{
	public:
		libevent_reactor();


/** 
 * @brief 	open
 * 
 * 	调用基类open初始化，并初始化本类部分
 * 
 * @author	mklong
 * @date	2014/6/12
 * @return	int	- Return 0 in success, otherwise return error code.
 * @see		
 */
		virtual int open();

/** 
 * @brief 	close
 * 
 * 	逆初始化本类，并调用基类close逆初始化
 * 
 * @author	mklong
 * @date	2014/6/12
 * @return	int	- Return 0 in success, otherwise return error code.
 * @see		
 */
		virtual int close();

/** 
 * @brief 	notify_eh
 * 
 * 	跨线程通知event_handler,会回调event_handler的handle_exception
 * mask会设置到event_handler::notify_mask_,由此可以触发其他事件回调.
 *	调用者必须确保eh在当前event_loop中
 *
 * @author	mklong
 * @date	2014/6/12
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	event_handler*eh	- [in] 
 * @param	short mask = EH_MASK::EXCEPT_MASK	- [in] 
 * @see		
 */
		int notify_eh(event_handler*eh,short mask = nfl::EXCEPT_MASK);


/** 
 * @brief 	add_eh
 * 
 * 	跨线程添加事件
 * 
 * @author	mklong
 * @date	2014/6/12
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	event_handler*eh	- [in] 
 * @see		
 */
		int add_eh(event_handler*eh);

	protected:

/** 
 * @brief 	process_notify
 * 
 * 	实现了通知事件的处理，主要有新增event_handler，触发已存在的event_handler，
 * 通知event_loop线程退出
 * 
 * @author	mklong
 * @date	2014/6/12
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE s	- [in] 
 * @see		
 */
		virtual int process_notify(ACE_HANDLE s);

	private:

		//事件通知队列
		cq_base<event_handler,ACE_SYNCH> cq_eh_;

	};


	/************************************************************************/
	/*        inline func                                                               */
	/************************************************************************/


	ACE_INLINE int libevent_reactor::notify_eh( event_handler*eh,short mask /*= EH_MASK::EXCEPT_MASK*/ )
	{
		eh->notify_mask(mask);
		cq_eh_.cq_push(eh);

		char c = NOTIFY_EVENT;
		//返回1，说明写满了，由于通知模式只起到触发的作用，所以
		//当前通知满，是可以起到触发作用的

		return this->notify_event(&c) >= 0?0:-1;
	}

	ACE_INLINE int libevent_reactor::add_eh( event_handler*eh )
	{
		return notify_eh(eh,OPEN_MASK);
	}

}

#endif /* __LIBEVENT_REACTOR_H__ */

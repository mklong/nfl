#ifndef __EVENT_LOOP_THREAD_BASE_H__
#define __EVENT_LOOP_THREAD_BASE_H__


#include "nfl_base.h"
#include "notify_base.h"
#include "libevent_handler.h"

namespace nfl
{

/** 
 * @class	event_loop_thread_base
 * 
 * @brief	
 *		event loop线程基类
 * detail...
 *		  以libevent的event_base为基础，封装成一个线程	一个event_base的模型，
 * 由于将event_base初始化为EVENT_BASE_FLAG_NOLOCK 无锁的模式，所以多线
 * 程使用libevent api并不是线程安全的.
 *		为了提升效率的 的同时，不损失功能性，给每个event loop线程配置了一个事件
 * 通知器nofity_base，使用该组件可以无锁的打断event loop，传递事件通知，并无线
 * 程安全问题。libevnet本身也是使用这种机制来处理信号的打断，可惜作者设计上并没有将
 * 此用到其他通知。
 *
 *
 * @author	mklong
 * @date	2014/6/11
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */

class event_loop_thread_base :public ACE_Task_Base
{
public:
	enum NOTIFY_FLAG{
		NOTIFY_EVENT = 'n',
		END_EVENT_LOOP = 'e',
	};

	event_loop_thread_base();


/** 
 * @brief 	open
 * 
 * 		初始化event_base与notify
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @see		
 */
	virtual int open();

/** 
 * @brief 	close
 * 
 * 	逆初始化event_base与notify
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @see		
 */
	virtual int close();


/** 
 * @brief 	end_event_loop
 * 
 *	跨线程中断event loop，通过notify写入退出事件
 * event loop线程收到后，需要调用end_event_loop_cb
 * 来退出。
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	int	- Return 0 in success, otherwise return error code.
 * @see		
 */
	int end_event_loop();


/** 
 * @brief 	end_event_loop_cb
 * 
 * 	用于event loop线程内部终止循环，使得event loop 线程退出
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	int	- Return 0 in success, otherwise return error code.
 * @see		
 */
	int end_event_loop_cb();


/** 
 * @brief 	notify_cb
 * 
 * 	传递给notify的使用，作为通知回调函数，由于是静态函数，所以会传类指针作为参数
 *  当notify触发该函数时，函数调用process_notify，派生类根据具体情况重写
 *	 process_notify
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	No return value.
 * @param	evutil_socket_t s	- [in] 
 * @param	short what	- [in] 
 * @param	void *ptr	- [in] 
 * @see		
 */
	static void notify_cb(evutil_socket_t s, short what, void *ptr);


/** 
 * @brief 	notify_event
 * 
 * 	跨线程通知事件，建议传固定长度字节，
 *	以便于在处理回调的时候每次收取固定的长度，
 *  NOTIFY_FLAG是建议的几个事件参数
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	const char* msg	- [in] 
 * @param	size_t len	- [in] 
 * @see		
 */
	int notify_event(const char* msg,size_t len =1);

/** 
 * @brief 	base
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/6/12
 * @return	libevent_base * 
 * @see		
 */
	libevent_base * base()const;

protected:

/** 
 * @brief 	process_notify
 * 
 * 		处理通知回调，派生类需实现
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE s	- [in] 
 * @see		
 */
	virtual int process_notify(ACE_HANDLE s);

/** 
 * @brief 	svc
 * 
 * 	event loop 线程函数，默认直接执行event_loop
 * 
 * @author	mklong
 * @date	2014/6/11
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @see		
 */
	virtual int svc();

protected:

	libevent_base * base_;
	nofity_base notify_;
};




/************************************************************************/
/* Class Implementation                                                                     */
/************************************************************************/

ACE_INLINE int event_loop_thread_base::end_event_loop_cb()
{
	//exit 
	return event_base_loopexit(base_,NULL);
}

ACE_INLINE libevent_base * event_loop_thread_base::base() const
{
	return this->base_;
}

ACE_INLINE int event_loop_thread_base::notify_event(const char* msg,size_t len)
{
	return this->notify_.notify(msg,len);
}

}

#endif /* __EVENT_LOOP_THREAD_BASE_H__ */

/** 
 * @file	libevent_handler.h
 * @brief	
 *	参考ACE event_handler封装libevent中的event
 *
 * detail...
 *  将各种事件的回调分离成虚函数，方便派生类处理
 *
 * @author	mklong
 * @version	1.0
 * @date	2014/5/30
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>2014/5/30	<td>mklong	<td>Create this file</tr>
 * </table>
 * 
 */
#ifndef __LIBEVENT_HANDLER_H__
#define __LIBEVENT_HANDLER_H__

#include "nfl_base.h"
#include "cq_base.h"
#include "ace/INET_Addr.h"
//#include "libevent_reactor.h"

namespace nfl
{

	class event_handler : public cq_item_base
	{
	public:
	
		event_handler();

		virtual ~event_handler();

/** 
 * @brief 	open
 * 
 * 	默认将event_handler注册到reactor中，注意设置reactor（）和handle（）
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @see		
 */
		virtual int open();

/** 
 * @brief 	close
 * 
 * 	将event_handler从reactor移除，关闭handle，并调用基类cleanup函数
 * 将对象归还到缓存队列中，或者直接释放内存。
 *
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @see		
 */
		virtual int close();

/** 
 * @brief 	handle_events
 * 
 * 	事件回调函数，通过mask，内部再调用handle_input，handle_output等读写回调
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	No return value.
 * @param	ACE_HANDLE fd	- [in] 
 * @param	short mask	- [in] 
 * @see		
 */
		virtual void handle_events(ACE_HANDLE fd,short mask);

	public:

/** 
 * @brief 	reactor
 * 
 * 	设置reactor
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	No return value.
 * @param	 libevent_reactor *const reactor	- [in] 
 * @see		
 */
		void reactor (libevent_reactor *const reactor);

/** 
 * @brief 	reactor
 * 
 * 	返回reactor
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	libevent_reactor * 
 * @see		
 */
		libevent_reactor * reactor(void) const;

/** 
 * @brief 	notify_mask
 * 
 * 	notify通知事件所使用的中间变量
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	No return value.
 * @param	short mask	- [in] 
 * @see		
 */
		void notify_mask (short mask);

/** 
 * @brief 	notify_mask
 * 
 * 	获取notify mask
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	short 
 * @see		
 */
		short notify_mask ()const;

/** 
 * @brief 	handle
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return ACE_HANDLE 
 * @see		
 */
		ACE_HANDLE handle (void) const;

/** 
 * @brief 	handle
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	No return value.
 * @param	ACE_HANDLE fd	- [in] 
 * @see		
 */
		void handle (ACE_HANDLE fd);


/** 
 * @brief 	active
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/7/14
 * @return	bool	- Return TRUE in success, otherwise return FALSE.
 * @see		
 */
		bool active()const;


/** 
 * @brief 	remote_addr
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/7/14
 * @return	No return value.
 * @param	const ACE_INET_Addr *addr	- [in] 
 * @see		
 */
		void remote_addr(const ACE_INET_Addr *addr);
		

/** 
 * @brief 	*remote_addr
 * 
 * 	
 * 
 * @author	mklong
 * @date	2014/7/14
 * @return	const ACE_INET_Addr 
 * @see		
 */
		const ACE_INET_Addr *remote_addr() const;

	protected:

/** 
 * @brief 	event_cb
 * 
 * 	设置到event中的静态回调函数，通过arg传入this指针，调用
 *  this->handle_events,来执行对象的事件回调
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	No return value.
 * @param	evutil_socket_t fd	- [in] 
 * @param	short flag	- [in] 
 * @param	void * arg	- [in] 
 * @see		
 */
		static void event_cb(evutil_socket_t fd,short flag,void * arg);

/** 
 * @brief 	handle_input
 * 
 * 	I/O 读事件回调，返回-1，会回调到handle_close
 *  通过fd的取值可以判断该回调是由I/O 触发，还是notify类型触发，
 *  后者触发，fd取值为ACE_INVALID_HANDLE
 *
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE fd = ACE_INVALID_HANDLE	- [in] 
 * @see		
 */
		virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

/** 
 * @brief 	handle_output
 * 
 * 	I/O 写事件回调，参考handle_input
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE fd = ACE_INVALID_HANDLE	- [in] 
 * @see		
 */
		virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);

/** 
 * @brief 	handle_timeout
 * 
 * 	定时器回调
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE fd = ACE_INVALID_HANDLE	- [in] 
 * @see		
 */
		virtual int handle_timeout (ACE_HANDLE fd = ACE_INVALID_HANDLE);


/** 
 * @brief 	handle_exception
 * 
 * 	非I/O回调，仅用于notify类型的通知回调
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE fd = ACE_INVALID_HANDLE	- [in] 
 * @see		
 */
		virtual int handle_exception (ACE_HANDLE fd = ACE_INVALID_HANDLE);


/** 
 * @brief 	handle_close
 * 
 * 	非I/O回调,用于统一处理清理逻辑
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	virtual int	- Return 0 in success, otherwise return error code.
 * @param	ACE_HANDLE fd	- [in] 
 * @param	short close_mask	- [in] 
 * @see		
 */
		virtual int handle_close (ACE_HANDLE fd,short close_mask);

			
/** 
 * @brief 	eh_add
 * 
 * 	注册事件到reactor
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	short flag	- [in] 
 * @see		
 */
		int eh_add(short flag,struct timeval *tv = NULL);

/** 
 * @brief 	eh_cancel_mask
 * 
 * 	取消某些事件，将原来的mask中移除@flag,再通过eh_add重新注册
 *  仅适合含有PERSIST_MASK的情况下，否则事件都是一次性触发
 *
 * @author	mklong
 * @date	2014/6/18
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	short flag	- [in] 
 * @see		
 */
		int eh_cancel_mask(short flag,struct timeval *tv = NULL);


/** 
 * @brief 	eh_modify_mask
 * 
 * 	用@flag来覆盖当前已有注册事件
 * 仅适合含有PERSIST_MASK的情况下
 *
 * @author	mklong
 * @date	2014/7/9
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	short flag	- [in] 
 * @param	struct timeval *tv = NULL	- [in] 
 * @see		
 */
		int eh_modify_mask(short flag,struct timeval *tv = NULL);

/** 
 * @brief 	eh_wakeup_mask
 * 
 * 	新增事件，在原来的mask基础上新增@flag，再通过eh_add重新注册
 * 仅适合含有PERSIST_MASK的情况下
 *
 * @author	mklong
 * @date	2014/6/18
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	short flag	- [in] 
 * @see		
 */
		int eh_wakeup_mask(short flag,struct timeval *tv = NULL);

/** 
 * @brief 	eh_del
 * 
 * 	移除所有事件
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	int	- Return 0 in success, otherwise return error code.
 * @see		
 */
		int eh_del();


/** 
 * @brief 	eh_timer_update
 * 
 * 	单独更新定时器，不会对已注册I/O事件做修改
 * 仅适合含有PERSIST_MASK的情况下
 *
 * @author	mklong
 * @date	2014/6/24
 * @return	int	- Return 0 in success, otherwise return error code.
 * @param	struct timeval *tv	- [in] 
 * @see		
 */
		int eh_timer_update(struct timeval *tv);

/** 
 * @brief 	eh_mask
 * 
 * 	查看当前已注册事件的mask
 * 
 * @author	mklong
 * @date	2014/6/18
 * @return	short 
 * @see		
 */
		short eh_mask()const;

	protected:
		short notify_mask_;
		ACE_HANDLE fd_;

		//libevent io event
		bool active_;
		libevent_event event_;
		libevent_reactor * reactor_;
		ACE_INET_Addr addr_;
	};



	/************************************************************************/
	/* inline   func                                                              */
	/************************************************************************/

	ACE_INLINE void event_handler::reactor( libevent_reactor *const reactor )
	{
		this->reactor_ = reactor;
	}

	ACE_INLINE libevent_reactor * event_handler::reactor( void ) const
	{
		return this->reactor_;
	}

	ACE_INLINE void event_handler::notify_mask( short mask )
	{
		this->notify_mask_ = mask;
	}

	ACE_INLINE short event_handler::notify_mask() const
	{
		return this->notify_mask_;
	}

	ACE_INLINE ACE_HANDLE event_handler::handle( void ) const
	{
		return this->fd_;
	}

	ACE_INLINE void event_handler::handle( ACE_HANDLE fd )
	{
		this->fd_ = fd;
	}

	ACE_INLINE bool event_handler::active() const
	{
		return this->active_;
	}

	ACE_INLINE int event_handler::eh_del()
	{
		if (event_del(&event_) == 0){
			this->active_ = false;
			return 0;
		}else{
			return -1;
		}
	}

	ACE_INLINE int event_handler::eh_cancel_mask( short flag,struct timeval *tv)
	{
		short cur_mask = this->eh_mask();

		if (ACE_BIT_ENABLED(cur_mask,flag)){
			ACE_CLR_BITS(cur_mask,flag);
		
			if (event_del(&event_))
				return -1;
			
			//无需注册
			if (cur_mask == NULL && tv == NULL)
				return 0;

			return eh_add(cur_mask,tv);

		}else{
			//cur_mask & flag == 0
			return 1;
		}
	}

	ACE_INLINE int event_handler::eh_wakeup_mask( short flag ,struct timeval *tv /*= NULL*/)
	{
		short cur_mask = this->eh_mask();

		if (ACE_BIT_ENABLED(cur_mask,flag)){
			//already in cur_mask
				return 1;
		}else{
			//cur_mask & flag == 0
			ACE_SET_BITS(cur_mask,flag);

			if (event_del(&event_))
				return -1;

			return eh_add(cur_mask,tv);
		}
	}

	ACE_INLINE int event_handler::eh_modify_mask(short flag,struct timeval *tv /*= NULL*/)
	{
		short cur_mask = this->eh_mask();
		if (cur_mask == flag){
				return 1;
		}else{
			if (event_del(&event_))
				return -1;

			return eh_add(flag,tv);
		}
	}

	ACE_INLINE short event_handler::eh_mask() const
	{
		return event_get_events(&event_);
	}

	ACE_INLINE int event_handler::eh_timer_update( struct timeval *tv )
	{
		return event_add(&event_,tv);
	}

	ACE_INLINE	void event_handler::remote_addr(const ACE_INET_Addr *addr)
	{
		addr_ .set(*addr);
	}
		
	ACE_INLINE	const ACE_INET_Addr *event_handler::remote_addr() const
	{
		return &addr_;
	}

}


#endif /* __LIBEVENT_HANDLER_H__ */

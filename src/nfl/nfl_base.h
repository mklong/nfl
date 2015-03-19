#ifndef __NFL_BASE_H__
#define __NFL_BASE_H__
/** 
 * @file	nfl_base.h
 * @brief	
 *		
 * detail...
 *
 *		整个项目的大体思路是参考memcached的思路，来构建
 *	基于libevent多线程版本的eventloop。基本的理念是解决C
 *	代码复用的问题，同时保持高效。本库对memcached中的框架部
 *  分进行了抽象，由于C代码中对内存的管理等要求更高，对抽
 *  象造成了一定的难度，所以库中内存管理看上去并不优美，通常
 *	会跨类操作。为了不牺牲封装带来的性能损耗，本库并未使用太多
 *  c++运行时高级特性，内部类大部分都是使用静态函数来协作。
 *  
 * @author	mklong
 * @version	1.0
 * @date	2014/2/21
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>2014/2/21	<td>mklong	<td>Create this file</tr>
 * </table>
 * 
 */

//libevent include
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/util.h"

//ace include
#include "ace/Task.h"
#include "ace/OS_NS_unistd.h"

#include "nfl_os.h"


namespace nfl
{
	//class Declaration
 	class event_handler;
 	class libevent_reactor;

	//libevent event callback  

	typedef void (*ev_cb_fn)(evutil_socket_t, short, void *arg);

	//dispather
	typedef int (*ev_dispatcher_fn)(event_handler*eh,void *ptr);

	//typedef
	typedef struct event_base libevent_base;
	typedef struct event libevent_event;


//disable copy constructor
class nfl_nocopy
{
public:
	/// Default constructor
	nfl_nocopy (void){};

private:
	nfl_nocopy (const nfl_nocopy &);
	nfl_nocopy &operator= (const nfl_nocopy &);
};


//event handler mask
enum EH_MASK
{	
	//EV_TIMEOUT	0x00
	NULL_MASK = 0x00,
	//EV_TIMEOUT	0x01
	TIMER_MASK = 0x01,
	//EV_READ		0x02
	READ_MASK = 0x02,
	//EV_WRITE	0x04
	WRITE_MASK = 0x04,
	//EV_SIGNAL	0x08
	SIGNAL_MASK = 0x08,
	//EV_PERSIST	0x10
	PERSIST_MASK = 0x10,
	//EV_ET       0x20
	ET_MASK = 0x20,

	//nfl mask
	EXCEPT_MASK = 0x40,
	CLOSE_MASK = 0x80,
	OPEN_MASK = 0x100,

};

}




#endif /* __NFL_BASE_H__ */

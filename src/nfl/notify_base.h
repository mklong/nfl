#ifndef __NOTIFY_BASE_H__
#define __NOTIFY_BASE_H__

#include "nfl_base.h"


namespace nfl
{
/** 
 * @class	nofity_base
 * 
 * @brief	
 *		多线程通知
 * detail...
 *		利用此类跨线程无锁打断eventloop线程，并传递具体通知内容。
 *      可以配合队列来传递更复杂的通知。
 *    
 * @author	mklong
 * @date	2014/6/10
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
	class nofity_base : public nfl_nocopy
	{
	public:

		nofity_base():cb_(NULL),
			  cb_arg_(NULL)
		{
			socket_pair_[0] = -1;
			socket_pair_[1] = -1;
		}

		int open(libevent_base *bs)
		{
			if (cb_ == NULL || cb_arg_ == NULL){
				return -1;
			}
			
			//notify pipe (socketpair) init
			if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair_)<0){
				return -1;
			}

			evutil_make_socket_nonblocking(socket_pair_[0]);
			evutil_make_socket_nonblocking(socket_pair_[1]);

			//在linux下如果是socketpair需要扩大SNDBUF和RCVBUF,防止写满丢数据
			int sndbuf = 128*1024;
			ACE_OS::setsockopt((ACE_HANDLE)socket_pair_[1],SOL_SOCKET,SO_SNDBUF,(const char *)&sndbuf,sizeof(int));

			int rcvbuf = 128*1024;
			ACE_OS::setsockopt((ACE_HANDLE)socket_pair_[0],SOL_SOCKET,SO_RCVBUF,(const char *)&rcvbuf,sizeof(int));

			event_assign(&notify_event_,bs,socket_pair_[0],EV_READ | EV_PERSIST, notify_cb, this);
			event_add(&notify_event_,NULL);
			return 0;
		}

		int close()
		{
			//close socketpair
			if (socket_pair_[0] != -1){
				event_del(&notify_event_);
				EVUTIL_CLOSESOCKET(socket_pair_[0]);

				if (socket_pair_[1] != -1){
					EVUTIL_CLOSESOCKET(socket_pair_[1]);
				}
				socket_pair_[0] = -1;
				socket_pair_[1] = -1;
			}
			return 0;
		}

		void set_notify_cb(ev_cb_fn cb,void *arg)
		{
			cb_ = cb;
			cb_arg_ = arg;
		}

		int notify(const char* msg,size_t len)
		{
			if (socket_pair_[1] == -1){
				return -1;
			}
			
			/*
			*	大压力写的情况下容易写满SNDBUF,可以通过2中手段规避
			*  1）扩大SNDBUF
			*	2）写满while循环写
			*/
			
			int ret = send(socket_pair_[1],msg,len,0);
			if (ret <0){
				if(errno == EAGAIN){
					return 1;
				}else{
					return -1;
				}
			}

			return 0;
		}

		static void notify_cb(evutil_socket_t s, short what, void *ptr)
		{
			nofity_base * nb = (nofity_base*) ptr;
			if (nb->cb_ == NULL){
				return;
			}else{
				nb->cb_(s,what,nb->cb_arg_);
			}
		}

	private:
		//cb
		ev_cb_fn cb_;
		void * cb_arg_;

		libevent_event notify_event_;
		evutil_socket_t socket_pair_[2];
	};

}

#endif /* __NOTIFY_BASE_H__ */

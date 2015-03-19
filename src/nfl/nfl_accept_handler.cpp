#ifndef __NFL_ACCEPT_HANDLER_CPP__
#define __NFL_ACCEPT_HANDLER_CPP__
#include "nfl_accept_handler.h"
#include "ace/INET_Addr.h"
//#include "log/srlog.h"

#ifndef WIN32
#include <netinet/tcp.h>
#endif

namespace nfl
{

	template<class EVENT_HANDLER>
	accept_handler<EVENT_HANDLER>::accept_handler():dispatcher_(NULL),
		arg_(NULL),
		rcvbuf_(accept_cfg::UNSET_VALUE),
		sndbuf_(accept_cfg::UNSET_VALUE),
		accept_count_(0)
	{

	}

	template<class EVENT_HANDLER>
	int accept_handler<EVENT_HANDLER>::open( accept_cfg *cfg )
	{
		int ret = 0;

		cq_eh_.open();

		if (cfg->listen_fd != ACE_INVALID_HANDLE){
			this->handle(cfg->listen_fd);
		}else{
			ACE_HANDLE s = ACE_OS::socket (cfg->addr.get_type (),SOCK_STREAM,0);
			if (s == ACE_INVALID_HANDLE){
				return -1;
			}

			if (cfg->reuse_addr != accept_cfg::UNSET_VALUE){
				
				ret = ACE_OS::setsockopt(s,SOL_SOCKET,SO_REUSEADDR,
					(const char *)&cfg->reuse_addr,sizeof(int));
				if (ret == -1){
					ACE_OS::closesocket(s);
					return -1;
				}
			}

#ifndef WIN32
			if (cfg->deferred_accept != false){

				ret = ACE_OS::setsockopt(s,IPPROTO_TCP,TCP_DEFER_ACCEPT,
					(const char *)&cfg->deferred_timeout,sizeof(int));
				if (ret == -1){
					ACE_OS::closesocket(s);
					return -1;
				}
			}
#endif

			 sockaddr_in local_inet_addr;
			 ACE_OS::memset (reinterpret_cast<void *> (&local_inet_addr),
				 0,sizeof local_inet_addr);

			 if (static_cast<ACE_Addr>(cfg->addr) == ACE_Addr::sap_any){
				 local_inet_addr.sin_port = 0;
			 } else{
				  local_inet_addr = *reinterpret_cast<sockaddr_in *> (cfg->addr.get_addr ());
			 }
				
			 if (local_inet_addr.sin_port == 0)
			 {
				ret = ACE::bind_port (s,ACE_NTOHL (ACE_UINT32 (local_inet_addr.sin_addr.s_addr))); 
				if (ret == -1){
					ACE_OS::closesocket(s);
					return -1;
				}
			 }else{

				ret = ACE_OS::bind (s,reinterpret_cast<sockaddr *> (&local_inet_addr),
					sizeof local_inet_addr);
				if (ret == -1){
					ACE_OS::closesocket(s);
					return -1;
				}
			 }
			 
			 ret =  ACE_OS::listen (s,cfg->backlog);
			 if (ret == -1){
				 ACE_OS::closesocket(s);
				 return -1;
			 }
			 
			 int val;
			 ACE::record_and_set_non_blocking_mode (s,val);
			
			 this->handle(s);
		}
		
		if(cfg->sndbuf != accept_cfg::UNSET_VALUE){
			sndbuf_ = cfg->sndbuf ;
		}

		if(cfg->rcvbuf != accept_cfg::UNSET_VALUE){
			rcvbuf_ = cfg->rcvbuf; 
		}

		if (cfg->keepalive){
			keepalive_ = cfg->keepalive;
		}
		
		return 0;
	}

	template<class EVENT_HANDLER>
	int accept_handler<EVENT_HANDLER>::handle_input( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
	{
		int ret = 0;
		int loop = 0;
		ACE_HANDLE s;
		while(1)
		{
			loop ++;
			ACE_INET_Addr remote_addr;
			int len = remote_addr.get_addr_size();

			s = ACE_OS::accept(fd,(sockaddr*)remote_addr.get_addr(),&len);	
			if (s == ACE_INVALID_HANDLE){
				int err = errno;
				if (err == EWOULDBLOCK){
					break;
				}
				else if (err == EMFILE || err ==ENFILE){
					//too many open files
					//stop accept for a while
					//srlog_warn("accept_handler::handle_input | too many open files.");
					break;
				}
				else
				{
					//other errors
					//srlog_warn("accept_handler::handle_input | accept errno : %d.",err);
					break;
				}

			}	

			accept_count_ ++;

			//set socket opt
			int val;
			ACE::record_and_set_non_blocking_mode (s,val);

			if(sndbuf_ != accept_cfg::UNSET_VALUE){
				ret = ACE_OS::setsockopt(s,SOL_SOCKET,SO_SNDBUF,
					(const char *)&sndbuf_,sizeof(int));

			}

			if(rcvbuf_ != accept_cfg::UNSET_VALUE){
				ret = ACE_OS::setsockopt(s,SOL_SOCKET,SO_RCVBUF,
					(const char *)&rcvbuf_,sizeof(int));
			}

			if (keepalive_){
				int keepalive = 1;
				ret = ACE_OS::setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,
					(const char *)&keepalive,sizeof(int));
			}
			
			//eh 在close中调用eh->cleanup会把对象还给cq_eh_
			EVENT_HANDLER * eh = cq_eh_.cqi_new();
			if (eh == NULL){
				//error
				//srlog_warn("accept_handler::handle_input | cqi_new failed.");
				ACE_OS::closesocket(s);
				return 0;
			}
			
			eh->handle(s);
			
			eh->remote_addr(&remote_addr);

			if (dispatcher_){
				ret = dispatcher_(eh,arg_);
				if (ret){
					//srlog_warn("accept_handler::handle_input | dispatcher failed.");
				}
			}else{
				eh->reactor(this->reactor());
				ret = eh->open();
				if (ret !=0){
					eh->close();
				}
			}	
		}

		return 0;
	}

	template<class EVENT_HANDLER>
	int accept_handler<EVENT_HANDLER>::handle_close( ACE_HANDLE fd,short close_mask )
	{
		if (this->active() ){
			eh_del();
			//log error and ignore it
		}
		
		if (this->handle() != ACE_INVALID_HANDLE){
			ACE_OS::closesocket(this->handle());
			this->handle(ACE_INVALID_HANDLE);
		}

		return 0;
	}


	template<class EVENT_HANDLER>
	void accept_handler<EVENT_HANDLER>::set_dispatcher( ev_dispatcher_fn func,void * arg )
	{
		dispatcher_ = func;
		arg_ = arg;
	}

	template<class EVENT_HANDLER>
	int accept_handler<EVENT_HANDLER>::close()
	{
		cq_eh_.close();
		return event_handler::close();
	}

	template<class EVENT_HANDLER>
	size_t nfl::accept_handler<EVENT_HANDLER>::accept_count() const
	{
		return this->accept_count_;
	}

}

#endif /* __NFL_ACCEPT_HANDLER_CPP__ */

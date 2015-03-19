#include "libevent_handler.h"

#include "libevent_reactor.h"

namespace nfl
{

	event_handler::event_handler():notify_mask_(NULL_MASK),
		fd_(ACE_INVALID_HANDLE),
		active_(false),
		reactor_(NULL)
	{

	}

	event_handler::~event_handler()
	{

	}

	int event_handler::open()
	{
		notify_mask_ = NULL_MASK;
		active_ = false;
		return eh_add(READ_MASK | PERSIST_MASK);
	}

	int event_handler::close()
	{
		if (this->active()){
			eh_del();
		}

		if (fd_ != ACE_INVALID_HANDLE){
			ACE_OS::closesocket(fd_);
			fd_ = ACE_INVALID_HANDLE;
		}

		reactor_ = NULL;
		this->cleanup();
		return 0;
	}

	int event_handler::handle_input( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
	{
		return -1;
	}

	int event_handler::handle_output( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
	{
		return -1;
	}

	int event_handler::handle_exception( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
	{
		return -1;
	}

	int event_handler::handle_timeout( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
	{
		return -1;
	}

	int event_handler::handle_close( ACE_HANDLE fd,short close_mask )
	{
		if (this->active()){
			eh_del();
		}

		if (fd_ != ACE_INVALID_HANDLE){
			//shutdown
			ACE_OS::closesocket(fd_);
			fd_ = ACE_INVALID_HANDLE;
		}

		return 0;
	}

	void event_handler::event_cb( evutil_socket_t fd,short flag,void * arg )
	{
		event_handler *eh = (event_handler *)arg;
		eh->handle_events((ACE_HANDLE)fd,flag);
	}

	void event_handler::handle_events( ACE_HANDLE fd,short mask )
	{
		int ret = 0;
		short flag = NULL_MASK;

		if (mask & WRITE_MASK ){
			flag = WRITE_MASK;
			ret = this->handle_output(fd);
			if (ret == -1){
				goto failed;
			}
		}

		if (mask & READ_MASK ){
			flag = READ_MASK;
			ret = this->handle_input(fd);
			if (ret == -1){
				goto failed;
			}
		}

		if (mask & TIMER_MASK ){
			flag = TIMER_MASK;
			ret = this->handle_timeout(fd);
			if (ret == -1){
				goto failed;
			}
		}

		if (mask & EXCEPT_MASK ){
			flag = EXCEPT_MASK;
			ret = this->handle_exception(fd);
			if (ret == -1){
				goto failed;
			}
		}

		if (mask & CLOSE_MASK ){
			flag = CLOSE_MASK;
			ret = this->handle_close(fd,flag);
			if (ret == -1){
				goto failed;
			}
		}

		return;
failed:

		if (flag != CLOSE_MASK){
			this->handle_close(fd,mask);
		}

	}

	int event_handler::eh_add( short flag ,struct timeval *tv /*= NULL*/)
	{
		if ( event_assign(&event_,this->reactor()->base(),
			(evutil_socket_t)this->handle(),flag,event_handler::event_cb,this) == -1){
				return -1;
		}

		if (event_add(&event_,tv) == -1){
			return -1;
		}

		this->active_ = true;
		return 0;	 
	}



}
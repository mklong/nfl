#include "nfl_core_threads.h"

namespace nfl
{

	sock_thread_pool::sock_thread_pool():ref_(0),
		n_threads_(1),
		cur_pos_(0),
		non_accept_mode_(false)
	{
	}

	int sock_thread_pool::open( size_t thread_num /*= 0*/ )
	{
		ACE_GUARD_RETURN (ACE_Thread_Mutex,lock,this->lock_,-1);

		//unset
		if (thread_num == 0){
			nfl_os::init();
			thread_num = nfl_os::CpuNum;
		}

		//open count
		if (++ref_ != 1){
			//already open
			if (thread_num == n_threads_){
				return 0;
			}else{
				return -1;
			}
		}

		n_threads_ = thread_num;

		//
		for (int i = 0;i <n_threads_;i++ ){
			libevent_reactor *lr = new libevent_reactor();

			lr->open();
			work_threads_.push_back(lr);
		}

		return 0;
	}

	int sock_thread_pool::close()
	{
		ACE_GUARD_RETURN (ACE_Thread_Mutex,lock,this->lock_,-1);
		if (--ref_ != 0){
			//close count
			return 0;
		}

		if (!work_threads_.empty()){

			for (int i = 0; i < work_threads_.size(); i++ ){
				work_threads_[i]->close();
				delete work_threads_[i];
			}

			work_threads_.clear();
		}

		return 0;
	}

	int sock_thread_pool::event_dispatcher( event_handler*eh,void *ptr )
	{
		sock_thread_pool* tp = (sock_thread_pool*)ptr;
		return tp->dispatch(eh);
	}

	nfl::ev_dispatcher_fn sock_thread_pool::get_dispatcher()
	{
		return sock_thread_pool::event_dispatcher;
	}

	libevent_reactor* sock_thread_pool::get_accept_reactor() const
	{
		if (!work_threads_.empty()){
			return work_threads_[0];
		}
		return NULL;
	}

	void sock_thread_pool::non_accept_mode()
	{
		non_accept_mode_ = true;
	}


}
#include "event_loop_thread_base.h"
#include "os_utils.h"

namespace nfl
{

	event_loop_thread_base::event_loop_thread_base()
		:base_(NULL)
	{
	}

	int event_loop_thread_base::open()
	{
		//new event base with no locks
		struct event_config * cfg = event_config_new();
		if (cfg == NULL){
			return -1;
		}

		event_config_set_flag(cfg, EVENT_BASE_FLAG_NOLOCK);

		base_ = event_base_new_with_config(cfg);
		event_config_free(cfg);

		if (base_ == NULL){
			return -1;
		}

		//notify init
		notify_.set_notify_cb(notify_cb,this);
		int ret = notify_.open(base_);
		if (ret){
			return ret;
		}
		return this->activate();
	}

	int event_loop_thread_base::close()
	{
		if (this->thr_count()>0){
			end_event_loop();
			this->wait();
		}

		notify_.close();

		if (base_ != NULL){
			event_base_free(base_);
			base_ = NULL;
		}
		return 0;
	}

	int event_loop_thread_base::end_event_loop()
	{
		char c = END_EVENT_LOOP;
		int ret = notify_.notify(&c,1);
		if (ret != 0){
			return ret;
		}

		return 0;
	}

	int event_loop_thread_base::svc()
	{
		// run event loop
		nfl_utils::set_thread_name("nfl::sockets");
		return ::event_base_loop(base_, 0);
	}

	int event_loop_thread_base::process_notify( ACE_HANDLE s )
	{
		return 0;
	}

	void event_loop_thread_base::notify_cb( evutil_socket_t s, short what, void *ptr )
	{
		event_loop_thread_base* t = (event_loop_thread_base*)ptr;
		t->process_notify((ACE_HANDLE)s);
		return;
	}

}
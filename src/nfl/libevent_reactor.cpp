
#include "libevent_reactor.h"


namespace nfl
{
	libevent_reactor::libevent_reactor()
	{

	}

	int libevent_reactor::open()
	{
		int ret = event_loop_thread_base::open();
		if (ret){
			return ret;
		}

		nfl_os::init();

		cq_eh_.open(nfl_os::PageSize);

		return 0;
	}

	int libevent_reactor::close()
	{
		cq_eh_.close();
		return event_loop_thread_base::close();
	}

	int libevent_reactor::process_notify( ACE_HANDLE s )
	{
		char c;
		int ret = 0;
		int loop_count = 0;
		event_handler *eh = NULL;

		while(1)
		{
			//read char 
			if (ACE_OS::recv(s,&c,1,0) != 1){
				//
				if (errno == EWOULDBLOCK){
					//no events
					break;
				}else{
					//unkown error
					ret = -1;
					break;
				}
			}

			//统计通知频率
			loop_count++;

			switch (c)
			{
			case NOTIFY_EVENT:
				while(1){
					eh = cq_eh_.cq_pop();
					if (eh == NULL){
						break;
					}else{
						if (eh->notify_mask() == OPEN_MASK){
							eh->reactor(this);
							ret = eh->open();
							if (ret != 0){
								eh->close();
							}
						}else{
							eh->handle_events(eh->handle(),eh->notify_mask());
						}
						
					}
				}

				break;

			case END_EVENT_LOOP:
				//end event loop
				end_event_loop_cb();
				break;
			default:
				ret = -1;
				break;
			}
		}

		if(ret == -1){
			//log serious error
		}

		return 0;
	}


}
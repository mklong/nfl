#include "os_utils.h"

#ifndef WIN32
#include <sys/prctl.h>
#endif

namespace nfl_utils
{
	int set_thread_name(const char* name)
	{
#ifdef WIN32
		return -1;
#else
		return prctl(PR_SET_NAME,name) ;
#endif
	}

}
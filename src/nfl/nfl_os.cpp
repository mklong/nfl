#include "nfl_os.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_sys_resource.h"

namespace nfl_os
{
	long PageSize = DEFAULT_PAGESIZE;
	long CpuNum = DEFAULT_CPUNUM;
	long MaxFd = DEFAULT_MAXFD;

	int init()
	{
		static int called = 0;
		if (++called !=1){
			//already init
			return 0;
		}

		PageSize = ACE_OS::getpagesize();
		if (PageSize < 1){
			PageSize = DEFAULT_PAGESIZE;
		}

		CpuNum = ACE_OS::num_processors();
		if (CpuNum < 1){
			CpuNum = DEFAULT_CPUNUM;
		}

#ifdef WIN32
		MaxFd = DEFAULT_MAXFD;
#else
		struct rlimit lm;

		if (ACE_OS::getrlimit(RLIMIT_NOFILE,&lm) == -1){
			MaxFd = DEFAULT_MAXFD;
		}else{
			MaxFd = lm.rlim_cur;
		}

#endif
		return 0;
	}

}


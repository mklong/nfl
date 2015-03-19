#ifndef __NFL_OS_H__
#define __NFL_OS_H__


// os attribute
namespace nfl_os
{
	int init();
	enum {
		DEFAULT_PAGESIZE = 4096,
		DEFAULT_CPUNUM = 1,
		DEFAULT_MAXFD = 1024
	};
	extern	long PageSize;
	extern	long CpuNum;
	extern	long MaxFd;

}


#endif /* __NFL_OS_H__ */

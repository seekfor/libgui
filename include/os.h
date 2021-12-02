#ifndef __OS_H__
#define __OS_H__

#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/sockios.h>
#include <dirent.h>
#include <linux/if.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <sys/statfs.h>
#define closesocket close
typedef void* (*thread_route)(void* args);
typedef void* handle_t;

#define PTHREAD_LOW		0x00000000
#define PTHREAD_HIGH		0x40000000
#define PTHREAD_DETACHED	0x80000000



#ifdef __cplusplus
extern "C"
{
#endif

	void osInit(int num,int stacksize);
	void osUninit();
	void osSleep(int ms);
	void* osMalloc(int size);
	void osFree(void* mem);
	unsigned int osGetTickCount();

	int osDefaultSignal();
	void* osCreateThread(int prior,thread_route route,void* args,int stack,char* name);
	int osThreadIsExit(void* hdl);
	int osJoinThread(void* thread);
	int osDestroyThread(void* thread,int wait);

	void* osCreateSemaphore(char* name,int level);
	int osTryWaitSemaphore(void* sem);
	int osWaitSemaphore(void* sem);
	int osWaitSemaphoreTimeout(void* sem,int ms);
	int osPostSemaphore(void* sem);
	int osDestroySemaphore(void* sem);

#ifdef __cplusplus
}
#endif


#endif


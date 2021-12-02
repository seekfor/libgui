#include <common.h>


extern int threadInit(int threadnum,int statcksize);

static unsigned int now = 0;
static void* tickthd = NULL;

static unsigned int os_get_tickcount()
{
	struct timeval tv;
	unsigned int ret;
	gettimeofday(&tv,NULL);
	ret = tv.tv_sec * 1000ll + tv.tv_usec / 1000;
	return ret;
}

static void* os_tick_thread(void* args)
{
	unsigned int oldtick,newtick;
	while(!osThreadIsExit(tickthd))
	{
		oldtick = os_get_tickcount();
		osSleep(10);
		newtick = os_get_tickcount();
		if(newtick > oldtick)
		{
			now += (newtick - oldtick);
		}
		else
		{
			now += 10;
		}
	}
	return args;

}

void osInit(int numthread,int stacksize)
{
	srand(os_get_tickcount());
	threadInit(numthread,stacksize);
	tickthd = osCreateThread(100,os_tick_thread,(void*)0,128000,"os_tick_thread");
}

void osUninit()
{
	osJoinThread(tickthd);
}


unsigned int osGetTickCount()
{
	return now;
}

void osSleep(int ms)
{
	usleep(ms * 1000);
}

void* osCreateSemaphore(char* name,int level)
{
	sem_t* ret;
	ret = (sem_t*)osMalloc(sizeof(sem_t) + strlen(name));
	if(ret)
	{
		sem_init(ret,0,level);
	}
	return ret;
}

int osTryWaitSemaphore(void* sem)
{
	if(!sem)
	{
		return -1;
	}
	return sem_trywait((sem_t*)sem);
}

int osWaitSemaphore(void* sem)
{
	if(!sem)
	{
		return -1;
	}
	return sem_wait((sem_t*)sem);
}

int osWaitSemaphoreTimeout(void* sem,int ms)
{
	struct timespec ts;
	if(!sem)
	{
		return -1;
	}
	clock_gettime(CLOCK_REALTIME,&ts);
	ts.tv_sec += ms / 1000;
	ts.tv_nsec += (ms % 1000) * 1000 * 1000;
	return sem_timedwait((sem_t*)sem,&ts);
}

int osPostSemaphore(void* sem)
{
	if(!sem)
	{
		return -1;
	}
	return sem_post((sem_t*)sem);
}

int osDestroySemaphore(void* sem)
{
	if(sem)
	{
		sem_destroy((sem_t*)sem);
		osFree(sem);
	}
	return 0;
}

void* osMalloc(int size)
{
	void* ret;
  	if(size & 0xfff)
	{
		size += 4096 - (size & 0xfff);
	}
	ret = malloc(size);
	return ret;
}

void osFree(void* mem)
{
	free(mem);
}



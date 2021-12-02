#ifndef __COMMON_H__
#define __COMMON_H__

#include "config.h"
#include "os.h"
#include "network.h"
#include "confile.h"
#include "utils.h"
#include "gui.h"

#ifdef __cplusplus
extern "C"
{
#endif
	int libguiInit(char* dev);
	int libguiUninit();
#ifdef __cplusplus
}
#endif


#endif


#include <common.h>

int libguiInit(char* dev)
{
	osInit(0,0);
	guiInit();
	fbInit(dev,CFG_DISPLAY_WIDTH,CFG_DISPLAY_HEIGHT);
	hidInit();
	return 0;
}

int libguiRun(char* confile)
{
	return guiRun(confile);
}


int libguiUninit()
{
	guiUninit();
	fbUninit();
	hidUninit();
	osUninit();
	return 0;
}

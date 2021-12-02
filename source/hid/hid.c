#include <common.h>
#include <linux/input.h>

#define MAX_HIDS		CFG_MAX_HIDS

#define CURSOR_W		13
#define CURSOR_H		20

#define CFG_FLIP

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


typedef struct
{
	int visible;

	int deltax[10];
	int deltay[10];
	int x[10];
	int y[10];

	int minx;
	int miny;
	int maxx;
	int maxy;
	int speed;

	int* img;
	int* scr;


	int button[10];

	int deltaz;
	int z;

}hw_cursor_t;


static int globalvisible = 1;




static int hid_get_abs(int fd,int* xmax,int* ymax)
{
	int yalv;
	uint8_t abs_b[ABS_MAX/8 + 1];
	struct input_absinfo abs_feat;
	*xmax = 1920;
	*ymax = 1080;
	ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_b)), abs_b);
	for (yalv = 0; yalv < ABS_MAX; yalv++) 
	{	
    		if (test_bit(yalv, abs_b)) 
    		{
        		if(ioctl(fd, EVIOCGABS(yalv), &abs_feat)) 
			{
				continue;
        		}
        		switch ( yalv)
            		{
            		case ABS_X :
				*xmax = abs_feat.maximum + 1;
                		break;
            		case ABS_Y :
				*ymax = abs_feat.maximum + 1;
                		break;
            		default:
				break;
            		}
    		}
	}
	return 0;
}

int arrow[] = 
{
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_BLACK,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,COLOR_BLACK,0x0000,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,COLOR_BLACK,COLOR_WHITE,COLOR_WHITE,COLOR_BLACK,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,COLOR_BLACK,COLOR_BLACK,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

};

static int scr[CURSOR_W * CURSOR_H];
static void* thread[MAX_HIDS + 1];
static void* mutex = NULL;
static hw_cursor_t cursor = {0,{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2,CFG_DISPLAY_WIDTH / 2},{CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2,CFG_DISPLAY_HEIGHT / 2},0,0,CFG_DISPLAY_WIDTH,CFG_DISPLAY_HEIGHT,1,arrow,scr,{0,0,0,0,0,0,0,0,0,0},0,0};

static void hid_limit_xyz()
{
	int i;
	if(cursor.z > 100)
	{
		cursor.z = 100;
	}
	else if(cursor.z < 0)
	{
		cursor.z = 0;
	}
	for(i = 0; i < 10; i ++)
	{
		if(cursor.x[i] < cursor.minx)
		{
			cursor.x[i] = cursor.minx;
		}		
		else if(cursor.x[i] > (cursor.maxx - CURSOR_W))
		{
			cursor.x[i] = cursor.maxx - CURSOR_W;
		}
		if(cursor.y[i] < cursor.miny)
		{
			cursor.y[i] = cursor.miny;
		}		
		else if(cursor.y[i] > (cursor.maxy - CURSOR_H))
		{
			cursor.y[i] = cursor.maxy - CURSOR_H;
		}
	}
}


static void* mouse_thread(void* args)
{
	int idx = (int)args;
	int fd = -1;
	int type = 0;
		
	int sel = 0;



	unsigned char buf[32];
	int size;
	int abs_move;
	int delta_x,delta_y,delta_z;

	int xres = 4096,yres = 4096;

	short int* vram;
	int padw;
	unsigned short id[4];
	int hide = 0;
	if(idx == 0)
	{
		vram = (short int*)fbGet(1,&padw);
		if(vram)
		{
			gdiGetBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,scr);
			if(cursor.visible)
			{
				gdiPutBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.img,1);
			}
		}
	}
	else if(idx == MAX_HIDS)
	{
		type = 1;
	}
	osSleep(500);


	while(!osThreadIsExit(thread[idx]))
	{
		int i;
		struct input_event ev[32];
		if(fd < 0)
		{
			if(type == 0)
			{
				sprintf((char*)buf,"/dev/input/event%d",idx);
				fd = open((char*)buf,O_RDONLY);
				if(fd >= 0)
				{
					ioctl(fd, EVIOCGID, id);
					if((id[ID_VENDOR] == 0x0eef) || (id[ID_VENDOR] == 0x222a))
					{
						hide = 1;
					}
					else
					{
						hide = 0;
					}
					hid_get_abs(fd,&xres,&yres);
				}
			}
			else
			{	
				fd = open("/dev/input/mice",O_RDONLY);
			}
			osSleep(100);
			continue;
		}
		abs_move = 0;
		delta_x = delta_y = delta_z = 0;

		if(type == 1)
		{
			size = read(fd,buf,4);
			if(size < 0)
			{
				close(fd);
				fd = -1;
				type = 0;
				osSleep(100);
				continue;
			}
			if(!(buf[1] & 0x80))
			{
				delta_x = (unsigned int)buf[1];
			}
			else
			{
				delta_x = -(255 - buf[1]);
			}
			if(!(buf[2] & 0x80))
			{
				delta_y = (unsigned int)buf[2];
			}
			else
			{
				delta_y = -(255 - buf[2]);
			}
			delta_y = -delta_y;
		}
		else
		{
			int len;
			len = read(fd,ev,sizeof(ev));
			if((len <= 0) || (len < sizeof(struct input_event)))
			{
				close(fd);
				fd = -1;
				continue;
			}
			abs_move = 0;
			delta_x = delta_y = delta_z = 0;
			buf[0] = cursor.button[sel];
			for(i = 0; i < len / sizeof(struct input_event); i ++)
			{
				//printf("type = %d(REL = %d,KEY = %d,ABS = %d),code = %d,value = %d\n",ev[i].type,EV_REL,EV_KEY,EV_ABS,ev[i].code,ev[i].value);
				if(ev[i].type == EV_REL)
				{
					cursor.visible = globalvisible;
					if(ev[i].code == 0) /*delta X*/
					{
						delta_x = ev[i].value;
					}
					else if(ev[i].code == 1) /*delta Y*/
					{
						delta_y = ev[i].value;
					}
					else if(ev[i].code == 8) /*delta Z*/
					{
						delta_z = ev[i].value;
					}
					continue;
				}
				else if(ev[i].type == EV_KEY)
				{
					switch(ev[i].code)
					{
					case 330:/*Pen pressed*/
						buf[0] = ev[i].value;
						break;
					case 272:/*LBUTTON*/
						buf[0] = ev[i].value;
						break;
					case 273:/*RBUTTON*/
						buf[0] = (ev[i].value << 1);
						break; 
					}
					continue;	
				}	
				else if(ev[i].type == EV_ABS)
				{
					cursor.visible = 0;
					switch(ev[i].code)
					{
					case 53:
						cursor.x[sel] = ev[i].value * CFG_DISPLAY_WIDTH / xres;
						break;
					case 54:
						cursor.y[sel] = ev[i].value * CFG_DISPLAY_HEIGHT / yres;
#ifdef CFG_FLIP
						cursor.y[sel] = CFG_DISPLAY_HEIGHT - cursor.y[sel];
#endif
						break;
					case 24:
						buf[0] = ev[i].value;
						break;
					case 0:
						cursor.x[sel] = ev[i].value * CFG_DISPLAY_WIDTH / xres;
						abs_move = 1;
						break;
					case 1:
						cursor.y[sel] = ev[i].value * CFG_DISPLAY_HEIGHT / yres;
#ifdef CFG_FLIP
						cursor.y[sel] = CFG_DISPLAY_HEIGHT - cursor.y[sel];
#endif
						abs_move = 1;
						break;
					case 47:
						sel = ev[i].value;
						if(!sel)
						{
							buf[0] = cursor.button[sel];
						}
						break;
					case 57:
						if(!sel)
						{
							if(ev[i].value < 0)
							{
								buf[0] &= 0xfe;
							}
							else
							{
								buf[0] |= 0x01;
							}
						}
						break;
					default:
						continue;
					}
					continue;
				}
				else
				{
				}
				if(sel)
				{
					continue;
				}
				if(delta_x || delta_y || abs_move)
				{
					cursor.deltax[sel] = delta_x;
					cursor.deltay[sel] = delta_y;
					osWaitSemaphore(mutex);
					if(cursor.visible && !hide && !sel)
					{
						/*restore old screen*/
						vram = (short int*)fbGet(1,&padw);
						gdiPutBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.scr,0);
					}
					osPostSemaphore(mutex);
					cursor.x[sel] += delta_x * cursor.speed;
					cursor.y[sel] += delta_y * cursor.speed;
				}	
				if(delta_z)
				{
					cursor.deltaz = delta_z;
					cursor.z += delta_z * cursor.speed;
				}
				hid_limit_xyz();
				if(!sel && (delta_x || delta_y || abs_move || delta_z))
				{
					osWaitSemaphore(mutex);
					vram = (short int*)fbGet(1,&padw);
					gdiGetBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,scr);
					if(cursor.visible && !hide)
					{
						gdiPutBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.img,1);
					}
					osPostSemaphore(mutex);
					guiSendMessage(guiGetActiveWindow(),WM_MOUSEMOVE,(cursor.y[0] << 16) | cursor.x[0],buf[0] & 0x07);
				}
				//printf("sel = %d:buf[0] = %x,button = %x,x = %d,y = %d\r\n",sel,buf[0],cursor.button[0],cursor.x[0],cursor.y[0]);
				if((cursor.button[sel] & 0x01) != (buf[0] & 0x01))
				{
					if(buf[0] & 0x01)
					{
						if(!sel)
						{
							guiSendMessage(guiGetActiveWindow(),WM_LBUTTONDOWN,(cursor.y[sel] << 16) | cursor.x[sel],buf[0] & 0x07);
						}
					}	
					else
					{
						if(sel)
						{
							guiSendMessage(guiGetActiveWindow(),WM_LBUTTONDOWN,(cursor.y[sel] << 16) | cursor.x[sel],0x01 | (buf[0] & 0x07));
						}
						guiSendMessage(guiGetActiveWindow(),WM_LBUTTONUP,(cursor.y[sel] << 16) | cursor.x[sel],buf[0] & 0x07);
					}
				}
				if((cursor.button[sel] & 0x02) != (buf[0] & 0x02))
				{
					if(buf[0] & 0x02)
					{
						guiSendMessage(guiGetActiveWindow(),WM_RBUTTONDOWN,(cursor.y[sel] << 16) | cursor.x[sel],buf[0] & 0x07);
					}
					else
					{
						guiSendMessage(guiGetActiveWindow(),WM_RBUTTONUP,(cursor.y[sel] << 16) | cursor.x[sel],buf[0] & 0x07);
					}
				}
				cursor.button[sel] = buf[0] & 0x07;
			}
		}	
	}
	return args;
}

int mouseRepaint()
{
	short int* vram;
	int padw;
	osWaitSemaphore(mutex);
	vram = (short int*)fbGet(0,&padw);
	if(vram && cursor.scr && cursor.visible)
	{
		gdiGetBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.scr);
		gdiPutBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.img,1);
	}
	osPostSemaphore(mutex);
	return 0;
}

int mouseSetRange(int minx,int miny,int maxx,int maxy)
{
	cursor.minx = minx;
	cursor.miny = miny;
	cursor.maxx = maxx;
	cursor.maxy = maxy;
	return 0;
}

void mouseShow(int visible)
{
	int padw;
	short int* vram;
	osWaitSemaphore(mutex);
	vram = (short int*)fbGet(1,&padw);
	if(vram)
	{
		gdiGetBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,scr);
		if(visible)
		{
			gdiPutBitmap((int*)vram,cursor.x[0],cursor.y[0],padw,CURSOR_W,CURSOR_H,cursor.img,1);
		}
	}
	globalvisible = visible;
	osPostSemaphore(mutex);
}


int mouseGetX()
{
	return cursor.x[0];
}

int mouseGetY()
{
	return cursor.y[0];
}

int mouseGetZ()
{
	return cursor.z;
}

int mouseGetDeltaX()
{
	return cursor.deltax[0];
}

int mouseGetDeltaY()
{
	return cursor.deltay[0];
}

int mouseGetDeltaZ()
{
	int ret = cursor.deltaz;
	cursor.deltaz = 0;
	return ret;
}


int mouseGetBTN()
{
	return cursor.button[0] & 0x07;
}

int hidAdjust(int xmax,int ymax,int inv)
{
	return 0;
}

int hidInit()
{
	int i;
	mutex = osCreateSemaphore("MTX-HID",1);
	for(i = 0; i < MAX_HIDS; i ++)
	{
		thread[i] = osCreateThread(100,mouse_thread,(void*)i,128 * 1024,"hid-event thread");
	}
	return 0;
}

int hidUninit()
{
	int i;
	for(i = 0; i < MAX_HIDS + 1; i ++)
	{
		if(thread[i])
		{
			osJoinThread(thread[i]);
		}
	}
	osDestroySemaphore(mutex);
	return 0;
}

#include <common.h>

#include <linux/fb.h>
#include <sys/mman.h>

static int fp = -1;
static int* fbp = NULL;
static int* fbase = NULL;

static long screensize = 0;
static int padw = 0;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;

int* fbGet(int mode,int* w)
{
	int line = padw;
	if(w)
	{
		*w = line;
	}
	return fbp;
}


int fbSync()
{
	return ioctl (fp,FBIOGET_VBLANK);
}

int fbFlip()
{
	mouseRepaint();
	memcpy(fbase,fbp,vinfo.yres * padw * (CFG_DISPLAY_BPP / 8));
	return ioctl(fp,FBIOPAN_DISPLAY, &vinfo);
}

int fbClear(short int* img)
{
	if(!img)
	{
		memset(fbp,0x00,padw * vinfo.yres * (CFG_DISPLAY_BPP / 8));
	}
	else
	{
		memcpy(fbp,img,padw * vinfo.yres * (CFG_DISPLAY_BPP / 8));
	}
	return 0;
}

int fbInit(char* dev,int w,int h)
{
	struct fb_bitfield a32 = {24,8,0};
	struct fb_bitfield r32 = {16,8,0};
	struct fb_bitfield g32 = {8,8,0};
	struct fb_bitfield b32 = {0,8,0};
	fp = open (dev,O_RDWR);
	if (fp < 0)
	{
		return -1;
	}
	if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo))
	{
		return -2;
	}

	if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo))
	{
		return -3;
	}

	vinfo.xres_virtual = w;
	vinfo.yres_virtual = h;
	vinfo.xres = w;
	vinfo.yres = h;
	vinfo.activate = FB_ACTIVATE_NOW;
	vinfo.bits_per_pixel = CFG_DISPLAY_BPP;
	
	vinfo.xoffset = 0;
	vinfo.yoffset = 0;
	vinfo.red = r32;
	vinfo.green = g32;
	vinfo.blue = b32;
	vinfo.transp = a32;

	ioctl(fp,FBIOPUT_VSCREENINFO,&vinfo);
	ioctl(fp,FBIOGET_VSCREENINFO,&vinfo);

	if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo))
	{
		return -3;
	}
	padw = finfo.line_length / (CFG_DISPLAY_BPP / 8);
	screensize = finfo.smem_len;
	fbase = (int*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp,0);
	fbp = osMalloc(finfo.line_length * vinfo.yres);
	memset(fbp,0x00,screensize);
	return 0;
}

int fbUninit()
{
	munmap(fbase,screensize);
	osFree(fbp);
	close(fp);
	return 0;
}


int fbFill(int x,int y,int w,int h,int color)
{
	int i,j;
	int* dst = fbp + padw * y + x;
	for(i = 0; i < h; i ++)
	{
		for(j = 0; j < w; j ++)
		{
			dst[j] = color;
		}
		dst += padw;	
	}
	return 0;
}

int fbDrawHLine(int x,int y,int w,int color)
{
	return gdiDrawHLine(fbp,x,y,padw,w,color);
}

int fbDrawVLine(int x,int y,int h,int color)
{
	return gdiDrawVLine(fbp,x,y,padw,h,color);
}


int fbDrawRectangle(int x,int y,int w,int h,int color)
{
	return gdiDrawRectangle(fbp,x,y,padw,w,h,color);
}

int fbDrawTextEx(int x,int y,int w,int h,int forceground,int background,int xscale,int yscale,char* msg)
{
	return gdiDrawTextEx(fbp,padw,x,y,w,h,forceground,background,xscale,yscale,msg);
}

int fbDrawText(int x,int y,int forceground,int background,int xscale,int yscale,char* msg)
{
	return gdiDrawText(fbp,padw,x,y,forceground,background,xscale,yscale,msg);
}

int fbPutBitmap(int x,int y,int w,int h,short int* bmp,int transparent)
{
	if(!x && !y && (w == padw) && !transparent)
	{
		return fbClear(bmp);
	}
	return gdiPutBitmap(fbp,x,y,padw,w,h,(int*)bmp,transparent);
}





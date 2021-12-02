#ifndef __GUI_H__
#define __GUI_H__


#define COLOR_BLACK     0xff101010
#define COLOR_RED       0xffff0000
#define COLOR_GREEN     0xff00ff00
#define COLOR_BLUE      0xff0000ff
#define COLOR_YELLOW    0xffffff00
#define COLOR_WHITE     0xffffffff
#define COLOR_GRAY      0xff808080
#define COLOR_CANON     0xffaccfff




#define WM_MOUSEMOVE		0x0000
#define WM_LBUTTONDOWN		0x0001
#define WM_LBUTTONUP		0x0002
#define WM_RBUTTONDOWN		0x0003
#define WM_RBUTTONUP		0x0004
#define WM_MBUTTONDOWN		0x0005
#define WM_MBUTTONUP		0x0006
#define WM_LBUTTONDBLCLK	0x0007
#define WM_CLOSE		0x0008
#define WM_COMMAND		0x0009
#define WM_TIMER		0x000a
#define WM_INIT			0x000b
#define WM_KEYDOWN		0x000c
#define WM_KEYUP		0x000d


#define GUI_MAX_OBJS		1024
#define GUI_LAST_NUM		1023
#define COLOR_SEL		0x47ffc5

#define OBJ_STYLE_BITMAP	0
#define	OBJ_STYLE_TEXT		1
#define OBJ_STYLE_TRACKBAR	2
#define OBJ_STYLE_CHECKBOX	3
#define OBJ_STYLE_COMBOX	4
#define OBJ_STYLE_BUTTON	5
#define OBJ_STYLE_EDIT		6
#define OBJ_STYLE_RADIOBOX	7
#define OBJ_STYLE_LISTBOX	8
#define OBJ_STYLE_TOUCH		9
#define OBJ_STYLE_IMB		10
#define OBJ_STYLE_DROPBOX	11
#define OBJ_STYLE_FILELIST	12
#define OBJ_STYLE_ICONLIST	13
#define OBJ_STYLE_ICONITEM	14
#define OBJ_STYLE_WIN		4096


#define OBJ_ATTR_MODIFY		0x0000001
#define OBJ_ATTR_READONLY	0x0000002
#define OBJ_ATTR_DISABLED	0x0000004
#define OBJ_ATTR_MOVEABLE	0x0000008
#define OBJ_ATTR_HIDE		0x0000010
#define OBJ_ATTR_NUMONLY	0x0000020
#define OBJ_ATTR_DROP		0x0000040
#define OBJ_ATTR_DBLMSG		0x0000080
#define OBJ_ATTR_TEXT		0x0000100
#define OBJ_ATTR_STEP		0x0000200
#define OBJ_ATTR_PASSWORD	0x0000400
#define OBJ_ATTR_DATE		0x0000800
#define OBJ_ATTR_TIME		0x0001000
#define OBJ_ATTR_SCROLL		0x0002000
#define OBJ_ATTR_LRMIRROR	0x0004000
#define OBJ_ATTR_UDMIRROR	0x0008000
#define OBJ_ATTR_TRANSPARENT	0x0010000
#define OBJ_ATTR_NOCHS		0x0020000
#define OBJ_ATTR_MULTILINE	0x0040000
#define OBJ_ATTR_WANTINPUT	0x0080000
#define OBJ_ATTR_CLEARMCI	0x0100000
#define OBJ_ATTR_CHECKED	0x0200000
#define OBJ_ATTR_UPDATE		0x0400000


#define MB_STYLE_YESNO			0x01
#define MB_STYLE_OKCANCEL		0x02
#define MB_STYLE_CANCEL			0x03
#define MB_STYLE_OK			0x04
#define MB_STYLE_YES			0x05
#define MB_STYLE_ICONASK		0x80
#define MB_STYLE_ICONHAND		0x40
#define MB_STYLE_ICONINFO		0x20


#define FILE_FMT_RETURN			0
#define FILE_FMT_DIR			1
#define FILE_FMT_DISK			2
#define FILE_FMT_UDISK			3
#define FILE_FMT_RDISK			4
#define FILE_FMT_MP4			5
#define FILE_FMT_TS			6
#define FILE_FMT_BMP			7
#define FILE_FMT_JPG			8
#define FILE_FMT_WAV			9
#define FILE_FMT_OTHER			10
#define FILE_FMT_CAM			11
#define FILE_FMT_ZIGBEE			12
#define FILE_FMT_PNG			13
#define FILE_FMT_H265			14
#define FILE_FMT_FLV			15
#define FILE_FMT_HIDE			255


#define OBJ_STATE_NORMAL	0
#define OBJ_STATE_CLICK		1
#define OBJ_STATE_DISABLE	2


typedef struct
{
	char filename[256];
	int x;
	int y;
	int w;
	int h;
	void* buf;
}gui_bmp_t;

typedef struct
{
	char* str;
	int color;
}gui_color_t;

typedef struct
{
	void* parent;
	void* private;
	int cmd;
	int id;
	int style;
	int attr;
	int caret;
	int inited;
	int x;
	int y;
	int w;
	int h;
	int font;
	int oldx;
	int oldy;
	int oldw;
	int oldh;
	int xscale;
	int yscale;
	int offsetx;
	int offsety;
	int color;
	int background;
	char caption[256];
	char url[64];

	gui_bmp_t normal;
	gui_bmp_t click;
	gui_bmp_t disable;
	gui_bmp_t layer;

	int changed;
	int state;
	int visible;
	int min;
	int max;
	int scroll[32];
	int current;
	int sel;
	int pg;
	int type;
	int row;

	int maxsize;
	char text[32][1024];
	char userdata[32][1024];
	int ext[32];
	
	char en[256];
	char cn[1024];


	char cwd[256];
	char fmt[32];

	void* lb;	
}gui_obj_t;

#define MCI_ATTR_DROPVIDEO	0x01
#define MCI_ATTR_DROPLOGO	0x02
#define MCI_ATTR_DROPCT		0x04


typedef struct
{
	int visible;
	int mode;
	int x;
	int y;
	int w;
	int h;
	int xscale;
	int yscale;
	int duration;
	int color;
	char text[256];
	void* bmp;
}sprite_t;


typedef struct
{
	int visible;
	int minx;
	int miny;
	int maxx;
	int maxy;
	int norb;
}cursor_t;

typedef struct
{
	time_t time;
	int seconds;
	char section[64];
}timeout_t;


typedef struct
{
	int x;
	int y;
	int w;
	int h;
	int vpss;
	int chn;

	int attr;
}mci_t;


typedef struct
{
	gui_obj_t obj;
	int offset_x;
	int offset_y;
	int id;
	int cmdbase;
	int visible;
	int font;
	int delay;
	char caption[256];
	char section[256];
	char keyboard[256];
	char nextpage[256];
	cursor_t cursor;		
	mci_t mci[8];
	mci_t xmci[8];
	timeout_t timeout;	

	gui_obj_t* objs[GUI_MAX_OBJS];

	void* parent;
	void* conf;
	void* tmr;
	
	char private[1024];
	char cwd[256];

	char page[32][256];
	int pageidx;
	int pagemask;

	int iconx;
	int icony;
	int iconw;
	int iconh;
	int textw;
	int texth;

	int sepw[8];
	int seph[8];
	int cbcmd;

	int pg;
}window_t;

typedef int (*GUICALLBACK)(void* hwnd,int msg,int wparam,int lparam);


typedef int (*lpfn_std_msg)(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient);

#define gdiDrawText(vram,w,x,y,forceground,background,xscale,yscale,msg) gdiDrawTextEx(vram,w,x,y,0,0,forceground,background,xscale,yscale,msg)

#ifdef __cplusplus
extern "C"
{
#endif

	/*LIB level API*/
	int libguiInit(char* dev);
	int libguiRun(char* confile);
	int libguiUninit();


	/*FB Level API*/
        int fbInit(char* dev,int w,int h);
        int fbSync();
        int fbClear(short int* bmp);
        int fbFill(int x,int y,int w,int h,int color);
        int fbPutBitmap(int x,int y,int w,int h,short int* bmp,int transparent);
        int fbFlip();
        int* fbGet(int mode,int* w);
        int fbDrawTextEx(int x,int y,int w,int h,int forceground,int background,int xscale,int yscale,char* msg);
        int fbDrawText(int x,int y,int forceground,int background,int xscale,int yscale,char* msg);
        int fbDrawRectangle(int x,int y,int w,int h,int color);
        int fbDrawHLine(int x,int y,int w,int color);
        int fbDrawVLine(int x,int y,int h,int color);
        int fbUninit();

	/*GDI Level API*/
        int gdiDrawTextEx(int* vram,int w,int x,int y,int tw,int th,int forceground,int background,int xscale,int yscale,char* msg);
        int gdiPutBitmap(int* vram,int x,int y,int padw,int w,int h,int* bmp,int transparent);
        int gdiGetBitmap(int* vram,int x,int y,int padw,int w,int h,int* bmp);
        int gdiFillRect(int* vram,int x,int y,int padw,int w,int h,int color);
        int gdiDrawHLine(int* vram,int x,int y,int padw,int w,int color);
        int gdiDrawVLine(int* vram,int x,int y,int padw,int h,int color);
        int gdiDrawRectangle(int* vram,int x,int y,int padw,int w,int h,int color);
        int yuvDrawText(char* Y,char* UV,int w,int x,int y,int tw,int th,int forceground,int background,int xscale,int yscale,char* msg);
	int gdiSetTextMode(int tmode);
        int gdiSetFont(int tfont);
        int gdiGetTextWidth(char* text,int xscale);
        int gdiGetTextHeight(int yscale);
        int gdiGetTextWidthEx(char* text,int font,int xscale);
        int gdiGetTextHeightEx(int font,int yscale);


	/*Window Level API*/
	window_t* guiGetActiveWindow();
	window_t* guiGetDesktopWindow();
	window_t* guiGetKeyboardWindow();
	window_t* guiMessageBox(window_t* parent,char* caption,char* msg,int style,int id);
	window_t* guiNewWindow(window_t* parent,char* section,int ox,int oy);
	int guiGoto(window_t* win,char* page);
	int guiDestroy(window_t* win);
	int guiMoveWindow(window_t* win,int x,int y);
	int guiShowWindow(window_t* win,int show);
	int guiLoadDialog(window_t* win,char* page,int left,int top);
	/*Object Level API*/
	gui_obj_t* guiFindObject(window_t* win,int cmd);
	int guiHideObject(gui_obj_t* obj);
	int guiShowObject(gui_obj_t* obj);
	int guiCheckRadioBox(window_t* win,int min,int max,int sel);
	int guiGetRadioBoxChecked(window_t* win,int min,int max);
	int guiEnableObject(window_t* win,int cmd,int en);
	int guiSetObjectState(window_t* win,int cmd,int state);
	int guiGetObjectState(window_t* win,int cmd);
	int guiGetObjectInt(window_t* win,int cmd);
	float guiGetObjectFloat(window_t* win,int cmd);
	char* guiGetObjectText(window_t* win,int cmd);
	int guiGetObjectSel(window_t* win,int cmd);
	int guiSetObjectValue(window_t* win,int cmd,int value);
	int guiSetObjectText(window_t* win,int cmd,char* text);
	int guiResetContent(window_t* win,int cmd);
	/*MSG Level API*/
	int guiShowTips(int x,int y,int duration,char* text,int color);
	int guiSendMessage(void* hwnd,int msg,int wParam,int lParam);
	int guiPostMessage(void* hwnd,int msg,int wparam,int lparam);

	/*HID level API*/
        int hidInit();
        int hidGetReport(int* rd,char** buf);
        void mouseShow(int visible);
        int mouseGetX();
        int mouseGetY();
        int mouseGetZ();
        int mouseGetDeltaX();
        int mouseGetDeltaY();
        int mouseGetDeltaZ();
        int mouseGetBTN();
        int mouseSetRange(int minx,int miny,int maxx,int maxy);
        int mouseRepaint();
        int hidUninit();

	/*Image level API*/
	int read_BMP_file(char* filename,int*w ,int* h,int* buf);
        int read_JPEG_file(char* filename,int* w,int* h,int* buf);
        int read_PNG_file(char* filename,int* w,int* h,int* buf);

	/*GUI Level API*/
	int guiInit();
	int guiSetMessageCallback(GUICALLBACK callback);
	int guiRun(char* confile);
	int guiLock();
	void guiUnlock();
	int guiUninit();

#ifdef __cplusplus
}
#endif










#endif

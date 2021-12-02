#include <common.h>

static void* 		mutex = NULL;
static int 		gExit = 0;

static window_t* 	hDesktop = NULL;
static window_t* 	hActive = NULL;
static window_t* 	hKeyboard = NULL;
static window_t* 	hMsgbox = NULL;

static gui_obj_t*	hDrop = NULL;
static gui_obj_t* 	hFocus = NULL;

static sprite_t 	sprite = {0,0,0,0,400,24,1,1,500,COLOR_YELLOW,""};

static uint32_t 	tick1s = 0;
static uint32_t 	tick500ms = 0;


static GUICALLBACK guiCustomerCallback = (GUICALLBACK)NULL;

static gui_obj_t* gui_new_object(window_t* win,int id,char* type,int x,int y,int w,int h,int xscale,int yscale,char* color,char* background,char* caption,char* url,char* normal,char*click,char* disable,char* layer,int attr,int maxsize,int font);
static int gui_draw_objects(window_t* win);
static void gui_destroy_objects(window_t* win,int start);


static void gui_draw_sprite()
{
	if(!sprite.visible)
	{
		return;
	}
	switch(sprite.mode)
	{
		case 0:/*text mode*/
			fbDrawTextEx(sprite.x,sprite.y,sprite.w,sprite.h,sprite.color,0x0000,sprite.xscale,sprite.yscale,sprite.text);
			sprite.y -= 4;
			break;
		case 1:/*bitmap mode*/
			fbPutBitmap(sprite.x,sprite.y,sprite.w,sprite.h,sprite.bmp,0);
			break;
	}
	sprite.duration -= 4;
	if((sprite.duration <= 0) || (sprite.y <= 0))
	{
		sprite.visible = 0;
	}
}

static void gui_get_edit_scroll(gui_obj_t* obj,int row)
{
	int pos;
	int len = strlen(obj->text[row]);
	if(len > obj->maxsize)
	{
		pos = len - obj->maxsize;
		if(obj->text[row][pos] & 0x80)
		{
			if(pos)
			{
				pos --;	
			}
		}
		obj->scroll[row] = pos;
	}
	else
	{
		obj->scroll[row] = 0;
	}
}

static int gui_set_sprite_text(int x,int y,int duration,char* text)
{
	memset(sprite.text,0,256);
	strncpy(sprite.text,text,256);
	sprite.mode = 0;
	sprite.x = x;
	sprite.y = y;
	sprite.w = gdiGetTextWidth(sprite.text,sprite.xscale);
	sprite.h = gdiGetTextHeight(sprite.yscale);
	if((sprite.x + sprite.w) > 1920)
	{
		sprite.x = 1920 - sprite.w;
	}
	if((sprite.y + sprite.h) > 1080)
	{
		sprite.y = 1080 - sprite.h;
	}
	sprite.duration = duration;
	sprite.visible = 1;
	return 0;
}

static int gui_get_filetype(char* filename)
{
	unsigned char buf[256];
	int fd = open(filename,O_RDONLY);
	if(fd < 0)
	{
		return 0;
	}
	memset(buf,0,256);
	read(fd,buf,256);
	close(fd);
	if(!memcmp(buf,"\x42\x4d",2))
	{
		return 1;/*BMP*/
	}
	else if(!memcmp(buf,"\xff\xd8",2))
	{
		return 2;/*JPG*/
	}
	else if(!memcmp(buf,"\x89\x50",2))
	{
		return 3;/*PNG*/
	}
	return 0;	
}

static int gui_load_bitmap(gui_bmp_t* bmp,char* filename)
{
	if(!filename || !filename[0])
	{
		return -1;
	}

	if(!strcmp(bmp->filename,filename))
	{
		return 0;
	}
	strncpy(bmp->filename,filename,256);
	if(bmp->buf)
	{
		osFree(bmp->buf);
		bmp->buf = NULL;
	}
#ifdef CFG_BMP_SUPPORT
	if(strstr(filename,".bmp") && (gui_get_filetype(filename) == 1))
	{
		if(0 == read_BMP_file(filename,&bmp->w,&bmp->h,NULL))
		{
			bmp->buf = (char*)osMalloc(bmp->w * bmp->h * sizeof(int));
			return (read_BMP_file(filename,&bmp->w,&bmp->h,bmp->buf) > 0)?0:-1;
		}
	}
#endif
#ifdef CFG_JPG_SUPPORT
	else if(strstr(filename,".jpg") && (gui_get_filetype(filename) == 2))
	{
		if(0 == read_JPEG_file(filename,&bmp->w,&bmp->h,NULL))
		{
			bmp->buf = (char*)osMalloc(bmp->w * bmp->h * sizeof(int));
			return read_JPEG_file (filename,&bmp->w,&bmp->h,bmp->buf);
		}
	}
#endif
#ifdef CFG_PNG_SUPPORT
	else if(strstr(filename,".png") && (gui_get_filetype(filename) == 3))
	{
		if(0 == read_PNG_file(filename,&bmp->w,&bmp->h,NULL))
		{
			bmp->buf = (char*)osMalloc(bmp->w * bmp->h * sizeof(int));
			return read_PNG_file (filename,&bmp->w,&bmp->h,bmp->buf);
		}
	}
#endif
	return -1;
}

static int gui_free_bitmap(gui_bmp_t* bmp)
{
	if(bmp && bmp->buf)
	{
		osFree(bmp->buf);
		memset(bmp,0,sizeof(gui_bmp_t));
	}
	return 0;
}

static int gui_get_style(char* type)
{
	int i;
	typedef struct
	{
		char* str;
		int style;
	}type2style_t;
	type2style_t styles[] = 
	{
		{"BUTTON",OBJ_STYLE_BUTTON},
		{"TEXT",OBJ_STYLE_TEXT},
		{"TRACKBAR",OBJ_STYLE_TRACKBAR},
		{"COMBOX",OBJ_STYLE_COMBOX},
		{"EDIT",OBJ_STYLE_EDIT},
		{"BITMAP",OBJ_STYLE_BITMAP},
		{"CHECKBOX",OBJ_STYLE_CHECKBOX},
		{"RADIOBOX",OBJ_STYLE_RADIOBOX},
		{"LISTBOX",OBJ_STYLE_LISTBOX},
		{"TOUCH",OBJ_STYLE_TOUCH},
		{"IMB",OBJ_STYLE_IMB},
		{"DROPBOX",OBJ_STYLE_DROPBOX},
		{"FILELIST",OBJ_STYLE_FILELIST},
		{"ICONLIST",OBJ_STYLE_ICONLIST},
		{"ICONITEM",OBJ_STYLE_ICONITEM},
	};
	if(!type || !type[0])
	{
		return OBJ_STYLE_TEXT;
	}

	for(i = 0; i < sizeof(styles) / sizeof(type2style_t); i ++)
	{
		if(!strcmp(type,styles[i].str))
		{
			return styles[i].style;
		}
	}
	return OBJ_STYLE_TEXT;
}

static gui_color_t colors[] = 
{
	{"black",COLOR_BLACK},
	{"red",COLOR_RED},
	{"green",COLOR_GREEN},
	{"blue",COLOR_BLUE},
	{"yellow",COLOR_YELLOW},
	{"white",COLOR_WHITE},
	{"gray",COLOR_GRAY},
	{"canon",COLOR_CANON},
	{"none",0x0000},
};

static gui_color_t colors2[] = 
{
	{"white",COLOR_WHITE},
	{"yellow",COLOR_YELLOW},
	{"green",COLOR_GREEN},
	{"blue",COLOR_BLUE},
	{"red",COLOR_RED},
	{"gray",COLOR_GRAY},
	{"black",COLOR_BLACK},
	{"canon",COLOR_CANON},
	{"none",0x0000},
};




static int gui_color2index(int color,int wh)
{
	int i;
	gui_color_t* ct = wh?colors2:colors;
	if(color == 0)
	{
		return 0;
	}
	for(i = 0; i < sizeof(colors) / sizeof(gui_color_t); i ++,ct ++)
	{
		if(color == ct->color)
		{
			return i;
		}
	}
	return 0;
}



static char* gui_color2string(int color)
{
	int i;
	for(i = 0; i < sizeof(colors) / sizeof(gui_color_t); i ++)
	{
		if(color == colors[i].color)
		{
			return colors[i].str;
		}
	}
	return "black";
}

static int gui_string2color(char* color)
{
	int i;
	for(i = 0; i < sizeof(colors) / sizeof(gui_color_t); i ++)
	{
		if(!color)
		{
			break;
		}
		if(!strcmp(color,colors[i].str))
		{
			return colors[i].color;
		}
	}
	return COLOR_BLACK;
}

static void gui_delete_object(gui_obj_t* obj)
{
	if(obj->normal.buf)
	{
		osFree(obj->normal.buf);
	}
	if(obj->click.buf)
	{
		osFree(obj->click.buf);
	}
	if(obj->disable.buf)
	{
		osFree(obj->disable.buf);
	}
	if(obj->layer.buf)
	{
		osFree(obj->layer.buf);
	}
	if(obj == hFocus)
	{
		hFocus = NULL;
	}
	if(obj == hDrop)
	{
		hDrop = NULL;
	}
	osFree(obj);
}


static void gui_destroy_objects(window_t* win,int start)
{
	int i;
	gui_obj_t* obj;
	
	if(start == 0)
	{
		memset(&win->cursor,0,sizeof(win->cursor));
		memset(&win->mci,0,sizeof(win->mci));
		memset(&win->timeout,0,sizeof(win->timeout));
	}

	for(i = start; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj)
		{
			continue;
		}
		gui_delete_object(obj);
		win->objs[i] = NULL;
	}
}


static gui_obj_t* gui_find_object(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return NULL;
	}
	for(i = 0; i < GUI_MAX_OBJS - 1; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		return obj;
	}
	return NULL;
}

static gui_obj_t* gui_new_object(window_t* win,int id,char* type,int x,int y,int w,int h,int xscale,int yscale,char* color,char* background,char* caption,char* url,char* normal,char*click,char* disable,char* layer,int attr,int maxsize,int font)
{
	gui_obj_t* obj;
	obj = (gui_obj_t*)osMalloc(sizeof(gui_obj_t));
	if(!obj)
	{
		return obj;
	}
	memset(obj,0,sizeof(gui_obj_t));
	if(maxsize == 0)
	{
		maxsize = w / gdiGetTextWidth(" ",xscale);
	}
	if(font < 0)
	{
		obj->font = win->font;
	}
	else
	{
		obj->font = font;
	}
	obj->maxsize = maxsize;
	obj->attr = attr;
	obj->parent = win;
	obj->cmd = id;
	obj->id = id;
	if(x < 0)
	{
		x = (1920 - w) / 2;
	}
	if(y < 0)
	{
		y = (1080 - h) / 2;
	}
	obj->x = x;
	obj->y = y;
	obj->w = w;
	obj->h = h;
	obj->sel = 0;
	obj->oldx = x;
	obj->oldy = y;
	obj->oldw = w;
	obj->oldh = h;
	obj->xscale = xscale;
	obj->yscale = yscale;
	obj->visible = 1;
	if(attr & OBJ_ATTR_CHECKED)
	{
		obj->state = OBJ_STATE_CLICK;
	}
	else
	{
		obj->state = OBJ_STATE_NORMAL;
	}
	obj->style = gui_get_style(type);
	if(obj->style == OBJ_STYLE_EDIT)
	{
		obj->attr |= OBJ_ATTR_SCROLL;
	}
	strncpy(obj->caption,caption,256);
	strncpy(obj->url,url,256);

	strcpy(obj->cwd,"/mnt/disk");
	strcpy(obj->fmt,".mp4|.ts");

	obj->color = gui_string2color(color);
	obj->background = gui_string2color(background);
	gui_load_bitmap(&obj->normal,normal);
	gui_load_bitmap(&obj->click,click);
	gui_load_bitmap(&obj->disable,disable);
	gui_load_bitmap(&obj->layer,layer);

	obj->min = 0;
	obj->max = 100;
	obj->current = 0;

	if(obj->style == OBJ_STYLE_EDIT)
	{
		if(strcmp(obj->caption," "))
		{
			strcpy(obj->text[0],obj->caption);
		}
		if((attr & OBJ_ATTR_NUMONLY) && !obj->text[0][0])
		{
			strcpy(obj->text[0],"0");
		}
		
	}
	else if(obj->style == OBJ_STYLE_LISTBOX)
	{
		int num = 0;
		char buf[32 * 256];
		char* src = buf;
		utlFormatCopy(src,obj->caption);
		for(num = 0; num < 32 && src; num ++)
		{
			obj->text[num][0] = 0;
			src = utlGetStringItem(obj->text[num],src);
		}
		obj->max = num;
	}

	return obj;
}

static void gui_set_object_color(window_t* win,int color,char* caption)
{
	int i;
	gui_obj_t* obj;
	for(i = 0; i < GUI_MAX_OBJS - 1; i ++)
	{
		obj = win->objs[i];
		if(obj && strstr(obj->caption,caption))
		{
			obj->color = color;
		}
	}

}


static int gui_refresh()
{
	
	fbClear(NULL);
	guiLock();
	gui_draw_objects(hDesktop);
	gui_draw_objects(hKeyboard);
	gui_draw_objects(hMsgbox);
	gui_draw_sprite();
	guiUnlock();

	return 0;
}

static int gui_show()
{
	fbSync();
	fbFlip();
	return 0;
}


static window_t* gui_load_from_file(char* filename)
{
	void* conf;
	window_t* win;
	win = (window_t*)osMalloc(sizeof(window_t));
	if(!win)
	{
		return win;
	}
	memset(win,0,sizeof(window_t));
	strcpy(win->keyboard,"keyboard");
	conf = win->conf = confCreate(filename);
	if(!conf)
	{
		return win;
	}
	win->visible = 1;
	confReadString(conf,"gui","keyboard",win->keyboard);
	confReadString(conf,"gui","home",win->section);
	guiGoto(win,win->section);
	return win;
}


static void gui_draw_context(gui_obj_t* obj)
{
	int j,i;
	gui_bmp_t* bmp;
	int num;
	int pg = 0;
	gui_obj_t* iconlist;
	char msg[4096];
	int x = 0,y = 0;
	int h,m,s;
	window_t* parent = (window_t*)obj->parent;
	if(obj->attr & OBJ_ATTR_HIDE)
	{
		return;
	}
	gdiSetFont(obj->font);
	switch(obj->style)
	{
		case OBJ_STYLE_ICONLIST:
			break;
		case OBJ_STYLE_ICONITEM:
			break;
		case OBJ_STYLE_DROPBOX:
			break;
		case OBJ_STYLE_LISTBOX:
			fbFill(obj->x,obj->y,obj->w,obj->h + 4,obj->background);
			for(j = 0; j < obj->max; j ++)
			{
				if(obj->current == j)
				{
					int color = COLOR_WHITE;
					if(obj->color == COLOR_WHITE)
					{
						color = COLOR_BLACK;
					}
					fbDrawTextEx(obj->x + 2,2 + obj->y + j * gdiGetTextHeightEx(obj->font,obj->yscale),obj->w,obj->h,color,obj->background,obj->xscale,obj->yscale,obj->text[j]);
				}
				else
				{
					fbDrawTextEx(obj->x + 2,2 + obj->y + j * gdiGetTextHeightEx(obj->font,obj->yscale),obj->w,obj->h,obj->color,obj->background,obj->xscale,obj->yscale,obj->text[j]);
				}
			}
			break;
		case OBJ_STYLE_TRACKBAR:
			bmp = &obj->layer;
			if(bmp->buf)
			{
				bmp->x = obj->x + obj->current * obj->w / 100;
				bmp->y = obj->y;
				fbPutBitmap(bmp->x,bmp->y,bmp->w,bmp->h,bmp->buf,1);
			}
			break;
		case OBJ_STYLE_COMBOX:
			{
				int maxc;
				int num = 0;
				char* src = msg;
				char item[32][4096];
				maxc = obj->w / (gdiGetTextWidth(" ",obj->xscale));
				if(maxc & 0x01)
				{
					maxc --;
				}
				if(!obj->inited)
				{
					utlFormatCopy(src,obj->caption);
					for(num = 0; num < 32 && src; num ++)
					{
						item[num][0] = 0;
						src = utlGetStringItem(item[num],src);
						strncpy(obj->text[num],item[num],256);
					}
					obj->inited = num;
				}
				strncpy(item[0],obj->text[obj->current],256);
				item[0][maxc] = 0;
				if((hFocus == obj) && obj->caret)
				{
					sprintf(msg,"%s|",item[0]);
				}
				else
				{
					strncpy(msg,item[0],256);
				}
				fbDrawTextEx(obj->x + 4 + obj->offsetx,obj->y + 2 + obj->offsety,obj->w,obj->h,obj->color,obj->background,obj->xscale,obj->yscale,msg);
			}
			break;
		case OBJ_STYLE_EDIT:
			if((obj->attr & OBJ_ATTR_READONLY) || (obj->attr & OBJ_ATTR_UPDATE))
			{
				strncpy(obj->text[0],obj->caption,256);
				obj->attr &= ~OBJ_ATTR_UPDATE;
			}
			{
				char* src;
				int i;
				for(i = 0; i < 8; i ++)
				{
					src = &obj->text[i][obj->scroll[i]];
					if((hFocus == obj) && obj->caret)
					{
						int n;
						if(obj->attr & OBJ_ATTR_PASSWORD)
						{
							n = strlen(src);
							memset(msg,'*',n);
							if(i == obj->row)
							{
								msg[n] = '|';
								msg[n + 1] = 0;
							}
							else
							{
								msg[n] = 0;
							}
					
						}
						else
						{
							sprintf(msg,"%s",src);
							if(i == obj->row)
							{
								strcat(msg,"|");
							}
						}
					}
					else
					{
						int n;
						if(obj->attr & OBJ_ATTR_PASSWORD)
						{	
							n = strlen(src);
							memset(msg,'*',n);
							msg[n] = 0; 
						}
						else
						{
							strncpy(msg,src,256);
						}
					}
					fbDrawTextEx(obj->x + 2 + obj->offsetx,obj->y + 2 + obj->offsety + i * gdiGetTextHeight(obj->yscale),obj->w,obj->h,obj->color,obj->background,obj->xscale,obj->yscale,msg);
				}
			}
			break;
		case OBJ_STYLE_TEXT:
			strcpy(msg,obj->caption);
			fbDrawTextEx(obj->x,obj->y,obj->w,obj->h,obj->color,obj->background,obj->xscale,obj->yscale,msg);
			break;	
		case OBJ_STYLE_IMB:
			fbDrawTextEx(obj->x,obj->y,obj->w,obj->h,COLOR_RED,obj->background,obj->xscale,obj->yscale,obj->en);
			if(obj->en[0])
			{
				int x = obj->x + gdiGetTextWidth(obj->en,obj->xscale) + 32;
				int max = strlen(obj->cn) / 2;
				if(x < (obj->x + obj->w) )
				{
					int i;
					char tmp[32];
					char* src = &obj->cn[obj->current * 2];
					msg[0] = 0;
					for(i = 0;i < 10; i ++)
					{
						if((obj->current + i) >= max)
						{
							break;
						}
						sprintf(tmp,"%d %c%c  ",(i < 9)? (i + 1):0,src[0],src[1]);
						src += 2;
						strcat(msg,tmp);
					}
					fbDrawTextEx(x,obj->y,obj->w,obj->h,COLOR_WHITE,obj->background,obj->xscale,obj->yscale,msg);
				}	
				
			}
			break;
			
	}

	if(obj->attr & OBJ_ATTR_TEXT)
	{
		int color = obj->color;
		strcpy(msg,obj->caption);
		if(obj->state == OBJ_STATE_CLICK)
		{
			color = (color == COLOR_WHITE)?COLOR_BLACK:COLOR_WHITE;
		}
		fbDrawTextEx(obj->x,obj->y,obj->w,obj->h,color,obj->background,obj->xscale,obj->yscale,msg);
	}

}

static void gui_clear_mci(window_t* win)
{
	int i;
	for(i = 0; i < 8; i ++)
	{
		if(!win->mci[i].w || !win->mci[i].h)
		{
			continue;
		}
		fbFill(win->mci[i].x,win->mci[i].y,win->mci[i].w,win->mci[i].h,0);
	}
}

static int gui_draw_objects(window_t* win)
{
	int i;
	gui_obj_t* obj;
	gui_bmp_t* bmp;
	if(!win || !win->visible)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		if(!win->objs[i])
		{
			continue;
		}
		obj = win->objs[i];
		if(obj->attr & OBJ_ATTR_HIDE)
		{
			continue;
		}
		if(obj->attr & OBJ_ATTR_DISABLED)
		{
			if(obj->disable.buf)
			{
				bmp = &obj->disable;
			}
			else	
			{
				bmp = &obj->normal;
			}
		}
		else
		{
			switch(obj->state)
			{
			default:
			case OBJ_STATE_NORMAL:
				bmp = &obj->normal;
				break;
			case OBJ_STATE_CLICK:
				bmp = &obj->click;
				break;
			case OBJ_STATE_DISABLE:
				bmp = &obj->disable;
				break;
			}
		}
		if(!bmp->buf)
		{
			bmp = &obj->normal;
		}
		if(bmp->buf && (obj->style != OBJ_STYLE_FILELIST) && (obj->style != OBJ_STYLE_ICONLIST) && (obj->style != OBJ_STYLE_ICONITEM))
		{
			int tr = obj->attr & OBJ_ATTR_TRANSPARENT;
			int x = obj->x;
			int y = obj->y;
			if(obj->attr & OBJ_ATTR_TEXT)
			{
				x += gdiGetTextWidth(obj->caption,obj->xscale);;
			}
			if(obj->attr & OBJ_ATTR_STEP)
			{
				int w,h;
				w = bmp->w;
				h = bmp->h;
				int current = obj->current?obj->current:0x01;
				if(obj->attr & OBJ_ATTR_UDMIRROR)
				{
					int* buf = (int*)bmp->buf;
					h = obj->h * current / 100 + 1;
					if(h > obj->h)
					{
						h = obj->h;
					}
					y += obj->h - h;
					buf += (obj->h - h) * w;
					fbPutBitmap(x,y,w,h,(short int*)buf,tr);
				}
				else if(!(obj->attr & OBJ_ATTR_LRMIRROR))
				{
					current = bmp->w * current / 100 + 1;
					if(current > w)
					{
						current = w;
					}
					w |= current << 16;
					fbPutBitmap(x,y,w,bmp->h,bmp->buf,tr);
				}
				else
				{
					current = bmp->w * current / 100 + 1;
					if(current > w)
					{
						current = w;
					}
					w |= (-current << 16);
					fbPutBitmap(x,y,w,bmp->h,bmp->buf,tr);
				}
			}
			else
			{
				fbPutBitmap(x,obj->y,bmp->w,bmp->h,bmp->buf,tr);
			}
		}
		if(obj->attr & OBJ_ATTR_CLEARMCI)
		{
			gui_clear_mci(win);
		}
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		if(!win->objs[i])
		{
			continue;
		}
		obj = win->objs[i];
		gui_draw_context(obj);
	}
	return 0;
}


static int video_std_process(window_t* win,int msg,int wparam,int lparam,int inclient)
{
	return 0;
}

static int button_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	if(!inclient)
	{
		return 0;
	}
	if(msg == WM_LBUTTONDOWN)
	{
		if(obj->state != OBJ_STATE_CLICK)
		{
			obj->state = OBJ_STATE_CLICK;
			guiPostMessage(obj,WM_COMMAND,obj->cmd,0x8000);
			return 1;
		}
	}
	else if(msg == WM_LBUTTONUP)
	{
		if(obj->state != OBJ_STATE_NORMAL)
		{
			obj->state = OBJ_STATE_NORMAL;
			guiPostMessage(obj,WM_COMMAND,obj->cmd,0);
			return 1;
		}
	}

	return 0;
}

static int radiobox_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	int i;
	int changed = 0;
	gui_obj_t* item;
	window_t* win = obj->parent;
	if(!inclient)
	{
		return 0;
	}
	if(msg == WM_LBUTTONDOWN)
	{
		obj->state = OBJ_STATE_CLICK;
		for(i = 0; i < GUI_MAX_OBJS; i ++)
		{
			item = win->objs[i];
			if(!item || (item->style != OBJ_STYLE_RADIOBOX) || (item->id != obj->id) || (item == obj))
			{
				continue;
			}
			item->state = OBJ_STATE_NORMAL;
			changed ++;
		}
		guiPostMessage(obj,WM_COMMAND,obj->cmd,0);
		return changed;
	}
	return 0;	
}

static int checkbox_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	if(inclient == 0)
	{
		return 0;
	}
	if(msg == WM_LBUTTONUP)
	{
		obj->current = !obj->current;
		if(obj->state == OBJ_STATE_NORMAL)
		{
			obj->state = OBJ_STATE_CLICK;
		}
		else
		{
			obj->state = OBJ_STATE_NORMAL;
		}
		guiPostMessage(obj,WM_COMMAND,obj->cmd,obj->state != OBJ_STATE_NORMAL);
		return 1;
	}
	return 0;
}

static int touch_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	window_t* win = hActive;
	if(msg == WM_LBUTTONDOWN)
	{
		char section[256];
		if(obj->url[0] == 0)
		{
			confReadString(win->conf,"gui","home",section);
		}
		else
		{
			strcpy(section,obj->url);
		}
		strcpy(win->nextpage,section);
	}
	return 0;
}

static int gui_in_rect(int x,int y,int ox,int oy,int ow,int oh)
{
	if((ow <= 0) || (oh <= 0) )
	{
		return 0;
	}
	return ((x >= ox) && (y >= oy) && (x <= (ox + ow) ) && (y <= (oy + oh) ) );
}

static int gui_posx_convert(int x,mci_t* mci)
{
	int ret;
	ret = (x - mci->x) * 1920 / mci->w;
	return (ret >= 0)?ret:0;
}

static int gui_posy_convert(int y,mci_t* mci)
{
	int ret;
	ret = (y - mci->y) * 1080 / mci->h;
	return (ret >= 0)?ret:0;
}


static int sdk_x_convert(int x,mci_t* mci)
{
	return mci->x + x * mci->w / 1920;
}

static int sdk_y_convert(int y,mci_t* mci)
{
	return mci->y + y * mci->h / 1080;
}

static int sdk_w_convert(int w,mci_t* mci)
{
	return w * mci->w / 1920;
}

static int sdk_h_convert(int h,mci_t* mci)
{
	return h * mci->h / 1080;
}

static void swap(uint64_t* a,uint64_t* b)
{
        uint64_t tmp = *a;
        *a = *b;
        *b = tmp;
}

static int dropbox_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	gui_obj_t* item;
	window_t* win = (window_t*)obj->parent;
	if(msg == WM_RBUTTONUP)
	{
		win->objs[GUI_LAST_NUM] = NULL;					
		gui_delete_object(obj);	
		return 1;
	}
	else if(msg == WM_LBUTTONUP)
	{
		hDrop = NULL;
	}
	else if(msg == WM_MOUSEMOVE)
	{
		return 1;
	}
	return 0;
}


static int iconitem_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
        int i;
        int changed = 0;
        gui_obj_t* item;
        window_t* win = obj->parent;
        if(!inclient)
        {
                return 0;
        }
        if(msg == WM_LBUTTONDOWN)
        {
                obj->state = OBJ_STATE_CLICK;
                for(i = 0; i < GUI_MAX_OBJS; i ++)
                {
                        item = win->objs[i];
                        if(!item || (item->style != OBJ_STYLE_ICONITEM) || (item == obj))
                        {
                                continue;
                        }
			item->sel = -1;
                        item->state = OBJ_STATE_NORMAL;
                        changed ++;
                }
                guiPostMessage(obj,WM_COMMAND,obj->cmd,0);
                return changed;
        }
        return 0;
}


static int iconlist_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	if(!inclient)
	{
		return 0;
	}
	return 0;
}


static int filelist_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	if(!inclient)
	{
		return 0;
	}
	return 0;
}



static int listbox_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	int i;
	int old;
	gui_obj_t* parent;
	int y = (wparam >> 16) - obj->y;
	if(!inclient)
	{
		return 0;
	}
	switch(msg)
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
			old = obj->current;
			for(i = obj->max - 1;i >= 0; i --)
			{
				if(y >= i * gdiGetTextHeightEx(obj->font,obj->yscale))
				{
					if(msg == WM_LBUTTONDOWN)
					{
						parent = (gui_obj_t*)obj->parent;
						if(parent)
						{
							parent->current = i;
							guiPostMessage(parent,WM_COMMAND,parent->cmd,i);
						}
						return 1;
					}
					if(old != i)
					{
						obj->current = i;
						return 1;
					}
					return 0;
				}
			}
			break;
		default:
			break;		
	}

	return 0;
}

static int combox_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	int x,y,w,h;
	if(!inclient)
	{
		if(msg == WM_LBUTTONDOWN)
		{
			if(obj->state != OBJ_STATE_NORMAL)
			{
				window_t* win = (window_t*)obj->parent;
				if(obj->lb)
				{
					if(win->objs[GUI_LAST_NUM] == obj->lb)
					{
						win->objs[GUI_LAST_NUM] = NULL;
						gui_delete_object(obj->lb);
					}
					obj->lb = NULL;
				}
				obj->state = OBJ_STATE_NORMAL;
				return 1;
			}
		}
		return 0;
	}
	if(msg == WM_LBUTTONDOWN)
	{
		window_t* win = (window_t*)obj->parent;
		if(win->objs[GUI_LAST_NUM])
		{
			return 0;
		}
		if(obj->state == OBJ_STATE_NORMAL)
		{
			if(obj->lb == NULL)
			{
				int j;
				int num = 0;
				char buf[4096];
				char* src = buf;
				char item[32][4096];
				gui_obj_t* t;
				if(!obj->inited)
				{
					strcpy(src,obj->caption);
					for(num = 0; num < 32 && src; num ++)
					{
						item[num][0] = 0;
						src = utlGetStringItem(item[num],src);
					}
					obj->inited = num;
				}
				else
				{
					for(num = 0;num < obj->inited; num ++)
					{
						strncpy(item[num],obj->text[num],256);
					}
					num = obj->inited;
				}
				if(obj->attr & OBJ_ATTR_UDMIRROR)
				{
					w = obj->w;
					h = num * gdiGetTextHeightEx(obj->font,obj->yscale);
					x = obj->x;
					y = obj->y - h;
				}
				else
				{
					x = obj->x;
					y = obj->y + obj->h;
					w = obj->w;
					h = num * gdiGetTextHeightEx(obj->font,obj->yscale);
				}
				
				t = obj->lb = gui_new_object((window_t*)obj,GUI_LAST_NUM,"LISTBOX",x,y,w,h,obj->xscale,obj->yscale,gui_color2string(obj->color),gui_color2string(obj->background),obj->caption,"",NULL,NULL,NULL,NULL,0,16,obj->font);
				if(t)
				{
					int w = t->w;
					int witem = 0;
					t->min = 0;
					t->max = num;
					t->current = obj->current;
					for(j = 0; j < num; j ++)
					{
						strncpy(t->text[j],item[j],256);
						witem = gdiGetTextWidth(item[j],obj->xscale);
						
						if(w < witem)
						{
							w = witem;
						}
						
					}
					if((w + t->x) > (1920 - 8))
					{
						w = 1920 - 8 - t->x;
					}
					t->w = w;
					win->objs[GUI_LAST_NUM] = t;
				}
			}
			obj->state = OBJ_STATE_CLICK;
			return 1;
		}
	}
	return 0;
}

static int text_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	if(!inclient)
	{
		return 0;
	}
	if(obj->attr & OBJ_ATTR_DISABLED)
	{
		return 0;
	}
	if(msg == WM_LBUTTONUP)
	{
		if(obj->attr & OBJ_ATTR_WANTINPUT)
		{
			guiPostMessage(obj,WM_COMMAND,obj->cmd,0);
		}
	}
	return 1;
}

static int bitmap_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	window_t* win = (window_t*)obj->parent;
	int i;
	int delta_x,delta_y;
	gui_bmp_t* bmp = &obj->normal;

	int x = wparam & 0xffff;
	int y = wparam >> 16;


	if(!inclient)
	{
		return 0;
	}
	if(obj->attr & OBJ_ATTR_DISABLED)
	{
		return 0;
	}

	if(msg == WM_LBUTTONUP)
	{
		if(obj->attr & OBJ_ATTR_STEP)
		{
			if(obj->attr & OBJ_ATTR_UDMIRROR)
			{
				int val = (y - obj->y) * 100 / obj->h;
				val = 100 - val;
				if(val < 0)
				{
					val = 0;
				}
				if(obj->current != val)
				{
					obj->current = val;
					guiPostMessage(obj,WM_COMMAND,obj->cmd,val);
					return 1;
				}
			}
			else
			{
				int val = (x - obj->x) * 100 / obj->w;
				if(obj->current != val)
				{
					bmp->x = x;
					obj->current = val;
					guiPostMessage(obj,WM_COMMAND,obj->cmd,val);
					return 1;
				}
			}
		}
		else
		{
			guiPostMessage(obj,WM_COMMAND,obj->cmd,0);
		}
		return 0;
	}
	else if(msg == WM_MOUSEMOVE)
	{
                if(!(lparam & 0x01) || !(obj->attr & OBJ_ATTR_MOVEABLE) )
                {
                        return 0;
                }
		delta_x = mouseGetDeltaX();
		delta_y = mouseGetDeltaY();
		x = obj->x + delta_x;
		y = obj->y + delta_y;
		if((x < 0) || (y < 0) || (x > (CFG_DISPLAY_WIDTH - win->obj.w)) || (y > (CFG_DISPLAY_HEIGHT - win->obj.h)) )
		{
			return 0;
		}
		for(i = 0; i < GUI_MAX_OBJS; i ++)
		{
			obj = win->objs[i];
			if(!obj)
			{
				continue;
			}
			obj->x = obj->x + delta_x;
			obj->y = obj->y + delta_y;
		}
		return 1;
	}

	return 0;

}

static int trackbar_std_process(gui_obj_t* obj,int msg,int wparam,int lparam,int inclient)
{
	int val;
	int x = wparam & 0xffff;
	int y = wparam >> 16;
	int btn = lparam & 0x07;
	gui_bmp_t* bmp = &obj->layer;
	if(!bmp->buf)
	{
		return 0;
	}
	if((x < bmp->x - 16) || (x > (bmp->x + bmp->w + 16)) || ( y < bmp->y - 16) || (y > (bmp->y + bmp->h + 16)) )
	{
		if(inclient && (msg == WM_LBUTTONDOWN) )
		{
			val = (x - obj->x) * 100 / obj->w;
			if(obj->current != val)
			{
				bmp->x = x;
				obj->current = val;
				guiPostMessage(obj,WM_COMMAND,obj->cmd,val);
				return 1;
			}
		}
		return 0;
	}
	switch(msg)
	{
		case WM_MOUSEMOVE:
			if(!(btn & 0x01) )
			{
				break;
			}
			x = bmp->x + mouseGetDeltaX();
			if(x >= obj->x)
			{
				if(x >= (obj->x + obj->w) )
				{
					bmp->x = obj->x + obj->w;
				}
				else
				{
					bmp->x = x;
				}
			}
			else
			{
				bmp->x = obj->x;
			}
			val = (bmp->x - obj->x) * 100 / obj->w;
			if(obj->current != val)
			{
				obj->current = val;
				guiPostMessage(obj,WM_COMMAND,obj->cmd,val);
			}
			return 1;
		default:
		case WM_LBUTTONDOWN:
			break;
	}
	return 0;
}

static gui_obj_t* gui_find_imb(window_t* win)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return 0;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->style != OBJ_STYLE_IMB) )
		{
			continue;
		}
		return obj;
	}
	return NULL;
}


static void gui_adjust_keyboard_position(window_t* hKeyboard,gui_obj_t* obj)
{
	int x;
	int y;
	int w = hKeyboard->obj.w;
	int h = hKeyboard->obj.h;

	x = obj->x;
	y = obj-> y + obj->h;
	if((x + w) > CFG_DISPLAY_WIDTH)
	{
		x = CFG_DISPLAY_WIDTH - w;
	}
	if((y + h) > CFG_DISPLAY_HEIGHT)
	{
		y = obj->y - h;
		if(y < 0)
		{
			y = 0;
		}
	}
	
	guiMoveWindow(hKeyboard,x,y);
}

static int gui_process_message(window_t* win,int is_desktop,int msg,int wparam,int lparam,int* done,int nodrop)
{
	gui_obj_t* obj;
	int i,j;
	int x = wparam & 0xffff;
	int y = wparam >> 16;
	mci_t* mci = &win->mci[0];
	int changed = 0;
	lpfn_std_msg lpfn_std_process[] = 
	{
		bitmap_std_process,
		text_std_process,
		trackbar_std_process,
		checkbox_std_process,
		combox_std_process,
		button_std_process,
		NULL,//edit_std_process,
		radiobox_std_process,
		listbox_std_process,
		touch_std_process,
		NULL,//imb_std_process
		dropbox_std_process,
		filelist_std_process,
		iconlist_std_process,
		iconitem_std_process,
	};
	if(!win || !win->visible)
	{
		return 0;
	}
	for(i = GUI_MAX_OBJS - 1; i >= 0; i --)
	{
		obj = win->objs[i];
		if(!obj || (obj->attr & OBJ_ATTR_DISABLED) || (obj->attr & OBJ_ATTR_HIDE))
		{
			continue;
		}
		if((x >= obj->x) && (x <= (obj->x + obj->w)) && (y >= obj->y) && (y <= (obj->y + obj->h)) )
		{
			*done = 1;
			if(!nodrop && (obj->attr & OBJ_ATTR_DROP))
			{
				if((msg == WM_MOUSEMOVE) && (lparam & 0x01))
				{
					gui_obj_t* item;
					if(obj->lb)
					{
						if(win->objs[GUI_LAST_NUM] == obj->lb)
						{
							win->objs[GUI_LAST_NUM] = NULL;
						}
						gui_delete_object(obj->lb);
						obj->lb = NULL;
						obj->state = OBJ_STATE_NORMAL;
					}
					item = gui_new_object(win,obj->id,"DROPBOX",x,y,obj->w,obj->h,obj->xscale,obj->yscale,"white","none",obj->caption,"","none","click","disable","layer",0,16,-1);
					if(item)
					{
						changed = 1;
						if(item->cmd < 0)
						{
							item->cmd = obj->cmd;
						}
						win->objs[GUI_LAST_NUM] = hDrop = item;
					}
				
					break;
				}
			}

			if(lpfn_std_process[obj->style])
			{
				changed += lpfn_std_process[obj->style](obj,msg,wparam,lparam,1);
			}
			if(is_desktop) 
			{
				if(msg == WM_LBUTTONDOWN)
				{
					if(hFocus)
					{
						hFocus->scroll[hFocus->row] = 0;
					}
					hFocus = obj;
					if(hFocus->style == OBJ_STYLE_EDIT)
					{
						gui_get_edit_scroll(hFocus,hFocus->row);
					}
					if(hKeyboard)
					{
						if(obj->attr & OBJ_ATTR_MODIFY)
						{
							hKeyboard->visible = 1;
							gui_adjust_keyboard_position(hKeyboard,hFocus);
						}
						else
						{
							hKeyboard->visible = 0;
						}
					}
				}
			}
			for(j = 0; j < GUI_MAX_OBJS; j ++)
			{
				obj = win->objs[j];
				if((j == i) || !obj)
				{
					continue;
				}
				if(lpfn_std_process[obj->style])
				{
					changed += lpfn_std_process[obj->style](obj,msg,wparam,lparam,0);
				}
			}
			break;
		}
		else
		{
			if(lpfn_std_process[obj->style])
			{
				changed += lpfn_std_process[obj->style](obj,msg,wparam,lparam,0);
			}
		}
	}

	return changed;
}

static char* gui_find_first_dialog(window_t* win,char** caption)
{
	int i;
	gui_obj_t* obj;
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || !obj->url[0])
		{
			continue;
		}
		if(caption)
		{
			*caption = obj->caption;
		}
		return obj->url;
	}
	return "";
}


static int gui_in_control_area(window_t* win,int x,int y)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return 0;
	}
	for(i = 0; i < 	GUI_MAX_OBJS; i ++)
	{
		if(!win->objs[i])
		{
			continue;
		}
		obj = win->objs[i];
		if(gui_in_rect(x,y,obj->x,obj->y,obj->w,obj->h))
		{
			return 1;
		}
	}
	return 0;
}

int guiSendMessage(void* hwnd,int msg,int wparam,int lparam)
{
	window_t* win = (window_t*)hwnd;
	int x = wparam & 0xffff;
	int y = wparam >> 16;
	int changed = 0;
	int done = 0;
	int timeout;

	if(!win)
	{
		return -1;
	}
	guiLock();

	if(hActive->timeout.seconds)
	{
		hActive->timeout.time = time(NULL) + hActive->timeout.seconds;
	}
	else
	{
		hActive->timeout.time = 0;
	}

	if(hDrop)
	{
		changed += dropbox_std_process(hDrop,msg,wparam,lparam,1);
	}
	else
	{
		if(hMsgbox)
		{
			changed += gui_process_message(hMsgbox,0,msg,wparam,lparam,&done,1);
			done = 1;
		}
		else if(hKeyboard && hKeyboard->visible)
		{
			changed += gui_process_message(hKeyboard,0,msg,wparam,lparam,&done,1);
		}
		if(!done)
		{
			changed += gui_process_message(win,1,msg,wparam,lparam,&done,(hMsgbox || (hKeyboard && hKeyboard->visible))?1:0);
		}
	}
	if(strcmp(win->nextpage,""))
	{
		guiGoto(win,win->nextpage);
		strcpy(win->nextpage,"");
	}
	guiUnlock();
	return 0;
}

window_t* guiGetActiveWindow()
{
	return hActive;
}

window_t* guiGetDesktopWindow()
{
	return hDesktop;
}

window_t* guiGetKeyboardWindow()
{
	return hKeyboard;
}



int guiInit(char* dev)
{
	gExit = 0;
	mutex = osCreateSemaphore("MTX-GUI",1);
	return 0;
}

int guiUninit()
{	
	gExit = 1;
	osDestroySemaphore(mutex);

	return 0;
}

int guiResetContent(window_t* win,int cmd)
{
	gui_obj_t* obj = gui_find_object(win,cmd);
	if(!obj)
	{
		return -1;
	}
	obj->inited = 0;
	obj->current = 0;
	obj->max = 0;
	return 0;
}


int guiShowWindow(window_t* win,int show)
{
	if(!win)
	{
		return 0;
	}
	win->visible = show;
	return 0;
}

int guiMoveWindow(window_t* win,int x,int y)
{
	int i;
	int delta_x;
	int delta_y;
	gui_obj_t* obj = win->objs[0];
	if(!obj)
	{
		return -1;
	}
	delta_x = x - obj->x;
	delta_y = y - obj->y;
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj)
		{
			continue;
		}
		obj->x += delta_x;
		obj->y += delta_y;
	}
	return 0;
}

window_t* guiNewWindow(window_t* parent,char* section,int ox,int oy)
{
	int i;
	int n;
	char buf[256];
	gui_obj_t* obj;
	keyword_t* kw;
	void* conf;
	window_t* win;
	int cmdbase = 0;
	kw = confGetKeyword(parent->conf,section);
	if(!kw)
	{
		return NULL;
	}
	win = (window_t*)osMalloc(sizeof(window_t));
	if(!win)
	{
		return NULL;
	}
	memset(win,0,sizeof(window_t));
	strncpy(win->section,section,256);
	conf = win->conf = parent->conf;
	win->parent = parent;
	win->cbcmd = 0;

	if(confReadString(conf,win->section,"font",buf) > 0)
	{
		win->font = atoi(buf);
	}
	if(confReadString(conf,win->section,"offset",buf) > 0)
	{
		sscanf(buf,"%d,%d",&win->offset_x,&win->offset_y);
	}
	
	if(confReadString(conf,win->section,"icon",buf) > 0)
	{
		sscanf(buf,"%d,%d,%d,%d,%d,%d",&win->iconx,&win->icony,&win->iconw,&win->iconh,&win->textw,&win->texth);
	}
	
	if(confReadString(conf,win->section,"sepw",buf) > 0)
	{
		sscanf(buf,"%d,%d,%d,%d,%d,%d,%d,%d",&win->sepw[0],&win->sepw[1],&win->sepw[2],&win->sepw[3],&win->sepw[4],&win->sepw[5],&win->sepw[6],&win->sepw[7]);
	}
	if(confReadString(conf,win->section,"seph",buf) > 0)
	{
		sscanf(buf,"%d,%d,%d,%d,%d,%d,%d,%d",&win->seph[0],&win->seph[1],&win->seph[2],&win->seph[3],&win->seph[4],&win->seph[5],&win->seph[6],&win->seph[7]);
	}
	if(confReadString(conf,win->section,"cmdbase",buf) > 0)
	{
		cmdbase = atoi(buf);
	}
	win->cmdbase = cmdbase;
	if(confReadString(conf,win->section,"page",buf) > 0)
	{
		sscanf(buf,"%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]",win->page[0],win->page[1],win->page[2],win->page[3],win->page[4],win->page[5],win->page[6],win->page[7],win->page[8],win->page[9],win->page[10],win->page[11],win->page[12],win->page[13],win->page[14],win->page[15]);
	}

	kw = confGetKeyword(conf,win->section);
	for(i = 0; kw ; )
	{
		char type[64] = {0};
		char url[64] = {0};
		int font = -1;
		int maxitem = 16;
		int offsetx = 0,offsety = 0;
		int x,y,w,h,xscale,yscale;
		char color[64] = {0},background[64] = {0},caption[64] = {0};
		char normalbmp[256] = {0},clickbmp[256] = {0},disablebmp[256] = {0},layerbmp[256] = {0};
		if(kw->key_name[0] != '#')
		{
			int attr = 0;
			n = sscanf(kw->key_value,"%[^,]%*c%d,%d,%d,%d,%d,%d,%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%d,%d,%[^,]%*c%d,%d,%d",type,&x,&y,&w,&h,&xscale,&yscale,color,background,caption,normalbmp,clickbmp,disablebmp,layerbmp,&attr,&maxitem,url,&font,&offsetx,&offsety);
			if(n >= 10)
			{
				obj = win->objs[i] = gui_new_object(win,atoi(kw->key_name) + cmdbase,type,ox + x,oy + y,w,h,xscale,yscale,color,background,caption,url,normalbmp,clickbmp,disablebmp,layerbmp,attr,maxitem,font);
				if(obj)
				{
					obj->offsetx = offsetx;
					obj->offsety = offsety;
					if(i == 0)
					{
						win->obj.x = obj->x;
						win->obj.y = obj->y;
						win->obj.w = obj->w;
						win->obj.h = obj->h;
					}
				}
				i ++;
			}
		}
		kw = kw->next;
	}
	guiPostMessage(win,WM_INIT,0,1);
	return win;	
}


window_t* guiMessageBox(window_t* parent,char* caption,char* msg,int style,int id)
{
	int i;
	int n;
	char buf[256];
	gui_obj_t* obj;
	keyword_t* kw;
	void* conf;
	window_t* win;
	if(!parent || !caption || !msg)
	{
		return NULL;
	}
	win = (window_t*)osMalloc(sizeof(window_t));
	if(!win)
	{
		return NULL;
	}
	conf = parent->conf;
	memset(win,0,sizeof(window_t));
	win->id = id;
	win->visible = 1;
	win->parent = parent;

	switch(style & 0x0f)
	{
		case MB_STYLE_YESNO:
			strcpy(win->section,"mb-yn");
			break;
		case MB_STYLE_OKCANCEL:
			strcpy(win->section,"mb-oc");
			break;
		default:
			strcpy(win->section,"mb-null");
			break;
	}
	
	if(confReadString(conf,win->section,"font",buf) > 0)
	{
		win->font = atoi(buf);
		gdiSetFont(win->font);
	}
	kw = confGetKeyword(conf,win->section);
	for(i = 0; kw ; )
	{
		char type[64];
		char url[256];
		int maxitem = 16;
		int font = -1;
		int x,y,w,h,xscale,yscale;
		char color[64] = {0},background[64] = {0},tcaption[64] = {0};
		char normalbmp[256] = {0},clickbmp[256] = {0},disablebmp[256] = {0},layerbmp[256] = {0};
		if(kw->key_name[0] != '#')
		{
			int attr = 0;
			n = sscanf(kw->key_value,"%[^,]%*c%d,%d,%d,%d,%d,%d,%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%d,%d,%[^,]%*c%d",type,&x,&y,&w,&h,&xscale,&yscale,color,background,tcaption,normalbmp,clickbmp,disablebmp,layerbmp,&attr,&maxitem,url,&font);
			if(n >= 10)
			{
				int id = atoi(kw->key_name);
				obj = win->objs[i] = gui_new_object(win,id,type,x,y,w,h,xscale,yscale,color,background,caption,"",(id != 5)?normalbmp:NULL,(id != 5)?clickbmp:NULL,(id != 5)?disablebmp:NULL,layerbmp,attr,maxitem,font);
				if(obj)
				{
					obj->cmd = obj->id + 0xffff0000;
					switch(obj->id)
					{
						case 0:
							win->obj.w = obj->w;
							win->obj.h = obj->h;
							break;
						case 1:/*caption*/
							strncpy(obj->caption,caption,256);
							break;
						case 2:/*msg*/
							strncpy(obj->caption,msg,256);
							break;
					}
					obj->x += (CFG_DISPLAY_WIDTH - win->obj.w) / 2;
					obj->y += (CFG_DISPLAY_HEIGHT - win->obj.h ) / 2;
				}
				i ++;
			}
		}
		kw = kw->next;
	}
	mouseShow(1);
	return win;	
}

int guiHideObject(gui_obj_t* obj)
{
	if(!obj)
	{
		return -1;
	}
	obj->attr |= OBJ_ATTR_HIDE;
	return 0;
}

int guiShowObject(gui_obj_t* obj)
{
	if(!obj)
	{
		return -1;
	}
	obj->attr &= ~OBJ_ATTR_HIDE;
	return 0;
}


int guiSetObjectState(window_t* win,int cmd,int state)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		obj->state = state;
		break;
	}
	return -1;
}

int guiSetMessageCallback(GUICALLBACK callback)
{
	osWaitSemaphore(mutex);
	guiCustomerCallback = callback;
	osPostSemaphore(mutex);
	return 0;
}


int guiPostMessage(void* hwnd,int msg,int wparam,int lparam)
{
	if(guiCustomerCallback)
	{
		return guiCustomerCallback(hwnd,msg,wparam,lparam);
	}
	return -1;
}

int guiGetObjectSel(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return 0;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		return obj->current;
	}
	return 0;
}

char* guiGetObjectText(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return "";
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		if(obj->style == OBJ_STYLE_COMBOX)
		{
			return obj->text[obj->current];
		}
		else
		{
			return obj->text[0];
		}
	}
	return "";
}

int guiGetObjectState(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return OBJ_STATE_NORMAL;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		return obj->state;
	}
	return OBJ_STATE_NORMAL;
}

int guiEnableObject(window_t* win,int cmd,int en)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		if(en)
		{
			obj->attr &= ~OBJ_ATTR_DISABLED;
		}
		else
		{
			obj->attr |= OBJ_ATTR_DISABLED;
		}
		return 0;
	}
	return -1;

}

int guiShowTips(int x,int y,int duration,char* text,int color)
{
	if(color)
	{
		sprite.color = color;
	}
	return gui_set_sprite_text(x,y,duration,text);
}

gui_obj_t* guiFindObject(window_t* win,int cmd)
{
	return gui_find_object(win,cmd);
}

int guiGoto(window_t* win,char* page)
{
	int i;
	int n;
	int cmdbase = 0;
	char buf[4096];
	char tmp[4096];
	keyword_t* kw;
	void* conf;

	if(!win)
	{
		win = hActive;
	}

	conf = win->conf;
	sprite.visible = 0;
	guiShowWindow(hKeyboard,0);

	if(page)
	{
		strncpy(win->section,page,256);
	}

	gui_destroy_objects(win,0);

	win->offset_x = win->offset_y = 0;
	memset(win->mci,0,sizeof(win->mci));
	memset(win->xmci,0,sizeof(win->xmci));

	if(confReadString(conf,win->section,"offset",buf) > 0)
	{
		sscanf(buf,"%d,%d",&win->offset_x,&win->offset_y);
	}

	if(confReadString(conf,win->section,"font",buf) > 0)
	{
		win->font = atoi(buf);
	}

	if(confReadString(conf,win->section,"cursor",buf) > 0)
	{
		sscanf(buf,"%[^,]%*c%d,%d,%d,%d,%d",tmp,&win->cursor.minx,&win->cursor.miny,&win->cursor.maxx,&win->cursor.maxy,&win->cursor.norb);
		if(!strcmp(tmp,"on"))
		{
			win->cursor.visible = 1;
		}
	}
        
	if(confReadString(conf,page,"icon",buf) > 0)
        {
                sscanf(buf,"%d,%d,%d,%d,%d,%d",&win->iconx,&win->icony,&win->iconw,&win->iconh,&win->textw,&win->texth);
        }
	if(confReadString(conf,win->section,"cmdbase",buf) > 0)
	{
		cmdbase = atoi(buf);
	}
	win->cmdbase = cmdbase;
	if(confReadString(conf,win->section,"mci",buf) > 0)
	{
                int sel;
                int idx = 0;
                char vpss[256];
                i = 0;
                char* str;
		memset(win->mci,0,sizeof(mci_t) * 8);
		if(strcmp(buf,"disable"))
		{
                	str = strstr(buf,"{");
                	while((str != NULL) && (i < 8) )
                	{
                        	n = sscanf(str,"%*c%d,%d,%d,%d,%[^,]%*c%d,%*c",&win->mci[i].x,&win->mci[i].y,&win->mci[i].w,&win->mci[i].h,vpss,&win->mci[i].chn);
                        	if(n >= 6)
                        	{
					win->mci[i].attr = win->mci[i].chn >> 8;
					win->mci[i].chn &= 0x7f;
					if(vpss[0] == '%')
                                	{
                                        	win->mci[i].vpss = atoi(vpss + 1) % 8;
                                	}
                                	else
                                	{
                                       		win->mci[i].vpss = atoi(vpss);
                                	}
                                	str = strstr(str + 1,"{");
                                	i ++;
                        	}
                        	else
                        	{
                                	break;
                        	}
                	}
		}

	}
	if(confReadString(conf,win->section,"timeout",buf) > 0)
	{
		n = sscanf(buf,"%d,%s",&win->timeout.seconds,win->timeout.section);
		win->timeout.time = time(NULL) + win->timeout.seconds;
	}
	else
	{
		win->timeout.time = 0;
		win->timeout.seconds = 0;
	}
	kw = confGetKeyword(conf,win->section);
	for(i = 0; kw ; )
	{
		char type[64];
		int x,y,w,h,xscale,yscale;
		int attr;
		int maxitem = 16;
		int font = -1;
		int offsetx = 0,offsety = 0;
		char color[64] = {0},background[64] = {0},caption[64] = {0};
		char url[64] = {0};
		char normalbmp[256] = {0},clickbmp[256] = {0},disablebmp[256] = {0},layerbmp[256] = {0};
		if(kw->key_name[0] != '#')
		{
			attr = 0;
			n = sscanf(kw->key_value,"%[^,]%*c%d,%d,%d,%d,%d,%d,%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%d,%d,%[^,]%*c%d,%d,%d",type,&x,&y,&w,&h,&xscale,&yscale,color,background,caption,normalbmp,clickbmp,disablebmp,layerbmp,&attr,&maxitem,url,&font,&offsetx,&offsety);
			if(n >= 10)
			{
				gui_obj_t* obj;
				obj = win->objs[i] = gui_new_object(win,atoi(kw->key_name) + cmdbase,type,x,y,w,h,xscale,yscale,color,background,caption,url,normalbmp,clickbmp,disablebmp,layerbmp,attr,maxitem,(font < 0)?win->font:font);
				if(obj)
				{
					obj->offsetx = offsetx;
					obj->offsety = offsety;
					i ++;
				}
			}
		}
		kw = kw->next;
	}
	guiPostMessage(win,WM_INIT,0,1);
	mouseShow(win->cursor.visible);
	return 0;
}

int guiLock()
{
	int ret;
	ret = osWaitSemaphoreTimeout(mutex,1000);
	if(ret != 0)
	{
		while(1)
		{
			osSleep(1000);
			printf("guiLock():mutex is dead lock,please check it!\r\n");
		}
	}
	return ret;
}

void guiUnlock()
{
	osPostSemaphore(mutex);
}

int guiLoadDialog(window_t* win,char* page,int left,int top)
{
	int i;
	int n;
	char buf[256];
	keyword_t* kw;
	void* conf = win->conf;
	int cmdbase = 0;

	if(!page || !page[0])
	{
		return -1;
	}

        if(confReadString(conf,page,"icon",buf) > 0)
        {
                sscanf(buf,"%d,%d,%d,%d,%d,%d",&win->iconx,&win->icony,&win->iconw,&win->iconh,&win->textw,&win->texth);
        }
	
	if(confReadString(conf,win->section,"timeout",buf) > 0)
	{
		n = sscanf(buf,"%d,%s",&win->timeout.seconds,win->timeout.section);
		win->timeout.time = time(NULL) + win->timeout.seconds;
	}
	else
	{
		win->timeout.time = 0;
		win->timeout.seconds = 0;
	}
	if(confReadString(conf,win->section,"cmdbase",buf) > 0)
	{
		cmdbase = atoi(buf);
	}
	win->cmdbase = cmdbase;

        confReadString(conf,page,"caption",win->caption);

	if((left < 0) || (top < 0))
	{
		gui_obj_t* obj = win->objs[0];
		if(obj)
		{
			left = obj->x;
			top = obj->y;
		}
	}
	kw = confGetKeyword(conf,page);
	if(kw)
	{
		gui_destroy_objects(win,32);
		for(i = 0; kw ; )
		{
			gui_obj_t* obj;
			char type[64];
			int x,y,w,h,xscale,yscale;
			int attr;
			int maxitem = 16;
			int font = -1;
			int offsetx = 0,offsety = 0;
			char color[64] = {0},background[64] = {0},caption[64] = {0};
			char url[64] = {0};
			char normalbmp[256] = {0},clickbmp[256] = {0},disablebmp[256] = {0},layerbmp[256] = {0};
			if(kw->key_name[0] != '#')
			{
				attr = 0;
				n = sscanf(kw->key_value,"%[^,]%*c%d,%d,%d,%d,%d,%d,%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%[^,]%*c%d,%d,%[^,]%*c%d,%d,%d",type,&x,&y,&w,&h,&xscale,&yscale,color,background,caption,normalbmp,clickbmp,disablebmp,layerbmp,&attr,&maxitem,url,&font,&offsetx,&offsety);
				if(n >= 10)
				{
					obj = win->objs[32 + i] = gui_new_object(win,atoi(kw->key_name) + cmdbase,type,left + x,top + y,w,h,xscale,yscale,color,background,caption,url,normalbmp,clickbmp,disablebmp,layerbmp,attr,maxitem,font);
					if(obj)
					{
						obj->offsetx = offsetx;
						obj->offsety = offsety;
					}
					i ++;
				}
			}
			kw = kw->next;
		}
	}
	return 0;
}

int guiRun(char* confile)
{
	char buf[1024];
	uint64_t now = 0;
	gui_obj_t* obj;
	hActive = hDesktop = gui_load_from_file(confile);
	if(!hActive)
	{
		return -1;
	}
	hKeyboard = guiNewWindow(hActive,hActive->keyboard,0,0);
	if(hKeyboard)
	{
		guiMoveWindow(hKeyboard,CFG_DISPLAY_WIDTH - hKeyboard->obj.w,CFG_DISPLAY_HEIGHT - hKeyboard->obj.h );
	}
	while(!gExit)
	{
		osSleep(100);

		gui_refresh();
		gui_show();

		guiLock();
		if(hActive->timeout.time)
		{
			if(hActive->timeout.time < time(NULL))
			{
				if(strcmp(hActive->timeout.section,"none"))
				{
					guiGoto(hActive,hActive->timeout.section);
					guiUnlock();
					continue;
				}
				else
				{
					hActive->timeout.time = time(NULL) + hActive->timeout.seconds;
				}
			}
		}

		now = osGetTickCount();

		if(now - tick500ms > 500)
		{
			tick500ms = now;
			guiPostMessage(hActive,WM_TIMER,500,0);

		}
		else if(now - tick1s > 1000)
		{
			tick1s = now;
			guiPostMessage(hActive,WM_TIMER,1000,0);
		}
		guiPostMessage(hActive,WM_TIMER,100,0);
		guiUnlock();
	}
	return 0;
}

int guiDestroy(window_t* win)
{
	if(win)
	{
		gui_destroy_objects(win,0);
		osFree(win);
	}
	return 0;
}


int guiGetRadioBoxChecked(window_t* win,int min,int max)
{
	int i;
	gui_obj_t* obj;
	for(i = min;i <= max; i ++)
	{
		obj = gui_find_object(win,i);
		if(obj && obj->state == OBJ_STATE_CLICK)
		{
			return i;
		}
	}
	return -1;
}

int guiCheckRadioBox(window_t* win,int min,int max,int sel)
{
	int i;
	if((sel < min) || (sel > max) )
	{
		return -1;
	}
	for(i = min;i <= max; i ++)
	{
		guiSetObjectState(win,i,OBJ_STATE_NORMAL);
	}
	guiSetObjectState(win,sel,OBJ_STATE_CLICK);
	return 0;

}

int guiSetObjectText(window_t* win,int cmd,char* text)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		strncpy(obj->text[0],text,256);
		return 0;
	}
	return -1;
}


int guiResetObjectContent(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		obj->inited = 0;
		return 0;
	}
	return -1;
}


int guiSetObjectSel(window_t* win,int cmd,int sel)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		obj->sel = sel;
		return 0;
	}
	return -1;
}


int guiSetObjectValue(window_t* win,int cmd,int value)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return -1;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		obj->current = value;
		return 0;
	}
	return -1;
}


int guiGetObjectInt(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return 0;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		return atoi(obj->text[0]);
	}
	return 0;
}

float guiGetObjectFloat(window_t* win,int cmd)
{
	int i;
	gui_obj_t* obj;
	if(!win)
	{
		return 0;
	}
	for(i = 0; i < GUI_MAX_OBJS; i ++)
	{
		obj = win->objs[i];
		if(!obj || (obj->cmd != cmd) )
		{
			continue;
		}
		return atof(obj->text[0]);
	}
	return 0;
}







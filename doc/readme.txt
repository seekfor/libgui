					gui.conf配置文件格式

<1>配置文件基本格式如下
	#gui段是入口段
	[gui]
	#版本，暂时不使用
	version=1.0.0.1
	#libguiRun()执行后显示的第一个界面，指定段名,某些界面可以直接返回这个界面
	home=main
	#libgui中的默认键盘界面，指定段名
	keyboard=kbd
	[main]
	#这是第一个界面，相当于主界面
	#本界面的命令ID起始数字是1024,每一个OBJ都是在cmdbase上递增
	cmdbase=1024
	#本界面默认的字库,0-12x12,1-16x16,2-24x24,3-32x32,如果只预编译配置了一种字体，那么这个数字不管是什么都只会使用预编译的字体
	font=2
	#本界面超时选项,格式为"秒,超时界面段名"
	timeout=30,fullscreen
	#图像显示区域，最多可以有8个窗口，格式为"mci={x,y,w,h,dev,chn},{x,y,w,h,dev,chn},...",这些窗口是预留透明显示视频的(GUI颜色0x00000000就是透明色)
	mci={0,0,1920,1080,0,0},{100,100,640,480,1,1}
	#鼠标选项,限制鼠标位置，隐藏/显示鼠标,格式为"cursor=on/off,x,y,w,h"
	cursor=on,0,0,1920,1080
	#第一个OBJ,类型为BITMAP,对应的命令ID=cmdbase + 0
	0=BITMAP,0,0,1920,1080,1,1,white,none,none,/mnt/app/guisvr/gui/guide/background.bmp,none,none,none,8
	#第二个OBJ,类型为EDIT,对应的命令ID=cmdbase + 1
	1=EDIT,0,0,600,32,1,3,white,none, ,none.bmp,none,none,none

<2>OBJ格式
	id=type,x,y,w,h,xscale,yscale,color,background,caption,normalbmp,clickbmp,focusbmp,layerbmp,attr,maxsize,url,font,offsetx,offsety
	其中,
	id:		
		id号，0 ~ n,可以不连续,但一般使用连续的id号
	type:		
		OBJ类型，如下:
			"BUTTON":	普通按钮
			"TEXT":		文本
			"TRACKBAR":	进度条
			"COMBOX":	下拉列表框
			"EDIT":		编辑框
			"BITMAP":	图片
			"CHECKBOX":	多选框
			"RADIOBOX":	单选框
			"LISTBOX":	列表框,暂未实现
			"TOUCH":	普通按钮，任意点击返回主界面
			"IMB":		输入法按钮,暂未实现
			"FILELIST":	文件列表，暂未实现
			"ICONLIST":	图标列表，暂未实现
	x,y:		
			OBJ坐标位置
	w,h:		
			OBJ大小，可以比图片大
	xscale,yscale:	
			如果该OBJ是文字类型(TEXT,EDIT,COMBOX等),这两个值决定字体的放大倍数，一般配置为1。例如窗体字体font=3,那么显示效果为32x32,如果配置为2,则显示效果为64x64,xscale,yscale可以独立配置，例如配置为1和2,那么显示字体为32x64
	color/background:		
			OBJ前景颜色/背景颜色，有效值有"black/red/green/blue/yellow/white/gray/canon/none",none表示透明
	caption:
			OBJ的标题文字
	normalbmp:
			OBJ正常状态下的图片路径，支持bmp/jpg/png(编译选项控制)
	clickbmp:
			OBJ被点击的图片路径
	focusbmp:
			暂未使用该值
	layerbmp:
			某些特定控件(TRACKBAR)显示的进度条背景
	attr:
			OBJ的属性,以下值的组合（参考gui.h中的OBJ_ATTR_**）:
			0x000001:	是否允许修改,如果允许则点击后自动弹出输入法键盘
			0x000002:	是否只读
			0x000004:	是否被禁止(Disabled),禁止的OBJ无法接受任何输入
			0x000008:	是否允许移动,如果允许则可以用鼠标拖动位置
			0x000010:	是否隐藏
			0x000020:	是否只允许数字
			0x000040:	是否允许拖放
			0x000080:	是否双消息，已废除
			0x000100:	是否显示文本，某些控件可能需要同时显示图片和文本
			0x000200:	是否步进模式
			0x000400:	是否密码模式显示文本，显示的文字为*****
			0x000800:	是否是日期控件
			0x001000:	是否是时间控件
			0x002000:	是否允许滚动
			0x004000:	是否左右颠倒
			0x008000:	是否上下颠倒
			0x010000:	是否透明
			0x020000:	是否非中文模式
			0x040000:	是否多行模式
			0x080000:	是否允许输入
			0x100000:	是否清除MCI区域，允许后自动清除"mci={x,y,w,h,dev,chn}"设置的区域颜色为透明色
			0x200000:	是否被选中
			0x400000:	是否刷新为caption文字
	maxsize:
			OBJ最大长度
	url:
			OBJ跳转的段名字
	font:
			OBJ使用的字体，不会覆盖窗体font设置
	offsetx/offsety:
			OBJ文字或图片的相对于x/y的偏移地址

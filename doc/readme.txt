					gui.conf�����ļ���ʽ

<1>�����ļ�������ʽ����
	#gui������ڶ�
	[gui]
	#�汾����ʱ��ʹ��
	version=1.0.0.1
	#libguiRun()ִ�к���ʾ�ĵ�һ�����棬ָ������,ĳЩ�������ֱ�ӷ����������
	home=main
	#libgui�е�Ĭ�ϼ��̽��棬ָ������
	keyboard=kbd
	[main]
	#���ǵ�һ�����棬�൱��������
	#�����������ID��ʼ������1024,ÿһ��OBJ������cmdbase�ϵ���
	cmdbase=1024
	#������Ĭ�ϵ��ֿ�,0-12x12,1-16x16,2-24x24,3-32x32,���ֻԤ����������һ�����壬��ô������ֲ�����ʲô��ֻ��ʹ��Ԥ���������
	font=2
	#�����泬ʱѡ��,��ʽΪ"��,��ʱ�������"
	timeout=30,fullscreen
	#ͼ����ʾ������������8�����ڣ���ʽΪ"mci={x,y,w,h,dev,chn},{x,y,w,h,dev,chn},...",��Щ������Ԥ��͸����ʾ��Ƶ��(GUI��ɫ0x00000000����͸��ɫ)
	mci={0,0,1920,1080,0,0},{100,100,640,480,1,1}
	#���ѡ��,�������λ�ã�����/��ʾ���,��ʽΪ"cursor=on/off,x,y,w,h"
	cursor=on,0,0,1920,1080
	#��һ��OBJ,����ΪBITMAP,��Ӧ������ID=cmdbase + 0
	0=BITMAP,0,0,1920,1080,1,1,white,none,none,/mnt/app/guisvr/gui/guide/background.bmp,none,none,none,8
	#�ڶ���OBJ,����ΪEDIT,��Ӧ������ID=cmdbase + 1
	1=EDIT,0,0,600,32,1,3,white,none, ,none.bmp,none,none,none

<2>OBJ��ʽ
	id=type,x,y,w,h,xscale,yscale,color,background,caption,normalbmp,clickbmp,focusbmp,layerbmp,attr,maxsize,url,font,offsetx,offsety
	����,
	id:		
		id�ţ�0 ~ n,���Բ�����,��һ��ʹ��������id��
	type:		
		OBJ���ͣ�����:
			"BUTTON":	��ͨ��ť
			"TEXT":		�ı�
			"TRACKBAR":	������
			"COMBOX":	�����б��
			"EDIT":		�༭��
			"BITMAP":	ͼƬ
			"CHECKBOX":	��ѡ��
			"RADIOBOX":	��ѡ��
			"LISTBOX":	�б��,��δʵ��
			"TOUCH":	��ͨ��ť������������������
			"IMB":		���뷨��ť,��δʵ��
			"FILELIST":	�ļ��б���δʵ��
			"ICONLIST":	ͼ���б���δʵ��
	x,y:		
			OBJ����λ��
	w,h:		
			OBJ��С�����Ա�ͼƬ��
	xscale,yscale:	
			�����OBJ����������(TEXT,EDIT,COMBOX��),������ֵ��������ķŴ�����һ������Ϊ1�����細������font=3,��ô��ʾЧ��Ϊ32x32,�������Ϊ2,����ʾЧ��Ϊ64x64,xscale,yscale���Զ������ã���������Ϊ1��2,��ô��ʾ����Ϊ32x64
	color/background:		
			OBJǰ����ɫ/������ɫ����Чֵ��"black/red/green/blue/yellow/white/gray/canon/none",none��ʾ͸��
	caption:
			OBJ�ı�������
	normalbmp:
			OBJ����״̬�µ�ͼƬ·����֧��bmp/jpg/png(����ѡ�����)
	clickbmp:
			OBJ�������ͼƬ·��
	focusbmp:
			��δʹ�ø�ֵ
	layerbmp:
			ĳЩ�ض��ؼ�(TRACKBAR)��ʾ�Ľ���������
	attr:
			OBJ������,����ֵ����ϣ��ο�gui.h�е�OBJ_ATTR_**��:
			0x000001:	�Ƿ������޸�,��������������Զ��������뷨����
			0x000002:	�Ƿ�ֻ��
			0x000004:	�Ƿ񱻽�ֹ(Disabled),��ֹ��OBJ�޷������κ�����
			0x000008:	�Ƿ������ƶ�,������������������϶�λ��
			0x000010:	�Ƿ�����
			0x000020:	�Ƿ�ֻ��������
			0x000040:	�Ƿ������Ϸ�
			0x000080:	�Ƿ�˫��Ϣ���ѷϳ�
			0x000100:	�Ƿ���ʾ�ı���ĳЩ�ؼ�������Ҫͬʱ��ʾͼƬ���ı�
			0x000200:	�Ƿ񲽽�ģʽ
			0x000400:	�Ƿ�����ģʽ��ʾ�ı�����ʾ������Ϊ*****
			0x000800:	�Ƿ������ڿؼ�
			0x001000:	�Ƿ���ʱ��ؼ�
			0x002000:	�Ƿ��������
			0x004000:	�Ƿ����ҵߵ�
			0x008000:	�Ƿ����µߵ�
			0x010000:	�Ƿ�͸��
			0x020000:	�Ƿ������ģʽ
			0x040000:	�Ƿ����ģʽ
			0x080000:	�Ƿ���������
			0x100000:	�Ƿ����MCI����������Զ����"mci={x,y,w,h,dev,chn}"���õ�������ɫΪ͸��ɫ
			0x200000:	�Ƿ�ѡ��
			0x400000:	�Ƿ�ˢ��Ϊcaption����
	maxsize:
			OBJ��󳤶�
	url:
			OBJ��ת�Ķ�����
	font:
			OBJʹ�õ����壬���Ḳ�Ǵ���font����
	offsetx/offsety:
			OBJ���ֻ�ͼƬ�������x/y��ƫ�Ƶ�ַ

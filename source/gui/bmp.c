
#include <common.h>

#ifdef CFG_BMP_SUPPORT

typedef unsigned char 		BYTE;
typedef unsigned short		WORD;
typedef unsigned int		DWORD; 
typedef unsigned int		LONG;

typedef struct tagBITMAPFILEHEDAER
{
	DWORD	bf_size;
	WORD	bf_reserved1;
	WORD	bf_reserved2;
	DWORD	bf_offbits;
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER32
{
	DWORD	bi_size;
	LONG	bi_width;
	LONG	bi_height;
	WORD	bi_planes;
	WORD 	bi_bitcount;
	DWORD	bi_compression;
	DWORD	bi_sizeimage;
	LONG	bi_xpelspermeter;
	LONG	bi_ypelspermeter;
	DWORD	bi_clrused;
	DWORD	bi_clrimportant;
}BITMAPINFOHEADER32;

typedef struct tagBITMAPINFOHEADER16
{
	DWORD bi_size;
	WORD bi_width;
	WORD bi_height;
	WORD bi_planes;
	WORD bi_bitcount;
}BITMAPINFOHEADER16;


typedef struct tagRGB
{
	BYTE	rgb_blue;
	BYTE	rgb_green;
	BYTE	rgb_red;
}RGB;

#define max(a,b)		(((a) > (b))?(a):(b))
#define min(a,b)		(((a) < (b))?(a):(b))


int read_BMP_file(char* filename,int* w,int* h,int* img)
{
	FILE*				fp;
	char buf[sizeof(BITMAPINFOHEADER32)];
	BITMAPINFOHEADER16* info16 = (BITMAPINFOHEADER16*)buf;
	BITMAPINFOHEADER32* info32 = (BITMAPINFOHEADER32*)buf;
	BITMAPFILEHEADER	bitfilehead;
	WORD	bf_type;
	int 	i,j;
	unsigned char	bgr[1920 * 4];
	int* pRGB;
	int W,H;
	int bpl;/*bytes per line*/
	int pad = 0;
	if((fp = fopen(filename,"rb")) != NULL)
	{
		fread(&bf_type,1,sizeof(WORD),fp);
		if(bf_type != 0x4d42)
		{
			fclose(fp);
			return 0;
		}
		fread(&bitfilehead,1,sizeof(BITMAPFILEHEADER),fp);
		fread(buf,1,sizeof(BITMAPINFOHEADER16),fp);
		if(info16->bi_size == sizeof(BITMAPINFOHEADER32))
		{
			fread(buf + sizeof(BITMAPINFOHEADER16),1,sizeof(BITMAPINFOHEADER32) - sizeof(BITMAPINFOHEADER16),fp);
			W = info32->bi_width;
			H = (info32->bi_height > 0)?info32->bi_height:-info32->bi_height;
		}
		else
		{
			W = info16->bi_width;
			H = (info16->bi_height > 0)?info16->bi_height:-info16->bi_height;
		}
	}
	else
	{
		return 0;
	}
	if(w)
	{
		*w = W;
	}
	if(h)
	{
		*h = H;
	}
	if(!img)
	{
		fclose(fp);
		return 0;
	}


	if((W <= 0) || (H <= 0) )
	{
		fclose(fp);
		return 0;
	}
	if((W * H) > (1920 * 1080))
	{
		fclose(fp);
		return 0;
	}

	if((W * 3) & 0x03)
	{
		pad = 4 - ((W * 3) & 0x03);
	}
	
	bpl = W * 3 + pad;
	pRGB = img;
	fseek(fp,bitfilehead.bf_offbits + bpl * (H - 1),SEEK_SET);
	for(i = 0;i < H;i++)
	{
		unsigned char* src = bgr;
		fread(bgr,bpl,1,fp);
		for(j = 0; j < W; j ++,pRGB ++,src += 3)
		{
			unsigned int val;
			if(src[0] || src[1] || src[2])
			{
				val = (0xff << 24) | (src[2] << 16) | (src[1] << 8) | src[0]; 
			}
			else
			{
				val = 0x000000;
			}
			*pRGB = val;
		}
		fseek(fp,-bpl * 2,SEEK_CUR);
  	}
	fclose(fp);
	return W * H * 4;
}

#endif


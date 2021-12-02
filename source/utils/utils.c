#include <common.h>

char* utlGetStringItem(char* msg,char* dst)
{
        while(*msg)
        {
                if(*msg == '\r')
                {
                        *dst = 0;
                        return msg + 2;
                }
                *dst ++= *msg ++;
        }
        *dst = 0;
        return NULL;
}

int utlFormatCopy(char* dst,char* msg)
{
	*dst = 0;
	while(*msg)
	{
		switch(msg[0])
		{
			case '\\':
				if(msg[1] == 'r')
				{
					*dst = '\r';
					dst ++;
					msg += 2;
					break;
				}
			default:
				*dst = *msg;
				dst ++;
				msg ++;
				break;
		}

	}
	*dst = 0;
	return strlen(dst);
}

char* utlSectionCopy(char* src,char* dst,unsigned char mask1,unsigned char mask2)
{
	uint8_t ch;
	while(src)
	{
		ch = *src++;
		if(ch == 0x00)
		{
			src = NULL;
			break;
		}
		if(ch == mask2)
		{
			break;
		}
		if(ch != mask1)
		{
			*dst ++= ch;
		}
		else
		{
			break;
		}
	}
	*dst = 0;
	return src;
}





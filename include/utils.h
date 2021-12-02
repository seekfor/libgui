#ifndef __UTILS_H__
#define __UTILS_H__


#ifdef __cplusplus
extern "C"
{
#endif

	char* utlGetStringItem(char* dst,char* msg);
	int utlFormatCopy(char* dst,char* msg);
	char* utlSectionCopy(char* src,char* dst,unsigned char mask1,unsigned char mask2);


#ifdef __cplusplus
}
#endif


#endif


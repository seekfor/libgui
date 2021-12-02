#ifndef __CONFIG_H__
#define __CONFIG_H__


#define CFG_DISPLAY_BPP			32
#define CFG_DISPLAY_WIDTH		1920
#define CFG_DISPLAY_HEIGHT		1080

#define CFG_MAX_HIDS			1	

#define CFG_BMP_SUPPORT
//#define CFG_JPG_SUPPORT
//#define CFG_PNG_SUPPORT

#if !defined(CFG_BMP_SUPPORT) && !defined(CFG_JPG_SUPPORT) && !defined(CFG_PNG_SUPPORT)
	#warning You must enable one of CFG_BMP_SUPPORT/CFG_JPG_SUPPORT/CFG_PNG_SUPPORT
	#define CFG_BMP_SUPPORT
#endif


//#define CFG_HZK12_SUPPORT
//#define CFG_HZK16_SUPPORT
#define CFG_HZK24_SUPPORT
//#define CFG_HZK32_SUPPORT


#if !defined(CFG_HZK12_SUPPORT) && !defined(CFG_HZK16_SUPPORT) && !defined(CFG_HZK24_SUPPORT) && !defined(CFG_HZK32_SUPPORT)
	#warning You must enable one of CFG_HZK12_SUPPORT/CFG_HZK16_SUPPORT/CFG_HZK24_SUPPORT/CFG_HZK32_SUPPORT
	#define CFG_HZK24_SUPPORT
#endif



#endif


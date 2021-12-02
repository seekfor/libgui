#ifndef __CONFILE_H__
#define __CONFILE_H__

typedef struct _keyword_t_
{
	void* conf;
	char key_label[256];
        char key_name[1024];
        char key_value[1024];
        struct _keyword_t_* next;
}keyword_t;

typedef struct _section_t_
{
	void* conf;
        char section_name[1024];
        keyword_t* key;
        struct _section_t_* next;
}section_t;

typedef struct
{
        char filename[256];
        section_t* sections;
}conf_t;

#ifdef __cplusplus
extern "C"
{
#endif

	void* confCreate(char* conf);
	void confDisplay(void* hdl);

	keyword_t* confGetKeyword(void* hdl,char* section);
	keyword_t* confCopyKeyword(keyword_t* kw);
	keyword_t* confFindKeyword(keyword_t* kw,char* label);
	int confReadInt(void* hdl,char* section,char* key,int def_value);
	int confReadString(void* hdl,char* section,char* key,char* value);
	int confWriteInt(void* hdl, char* section,char* key,int value);
	int confWriteString(void* hdl,char* section,char* key,char* value);


	int confSave(void* hdl,char* filename);
	int confDestroy(void* hdl);





#ifdef __cplusplus
}
#endif


#endif

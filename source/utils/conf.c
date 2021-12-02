
#include <common.h>
#include <ctype.h>


static int conf_gets(int fd,char* dst,int maxlen)
{
	int ret = 0;
	unsigned char ch;
	do
	{
		if(read(fd,(char*)&ch,1) != 1)
		{
			break;
		}
		if((ch == '\n') || (ch == '\r'))
		{
			if(ret)
			{
				break;
			}
		}
		else/* if(isprint(ch))*/
		{
			*dst++ = ch;
			ret ++;
		}
	}while(ret < maxlen);
	*dst = 0;
	return ret;
}

static section_t* conf_new_section(char* buf)
{
	section_t* sec = (section_t*)osMalloc(sizeof(section_t));
	if(sec)
	{
		memset(sec,0,sizeof(section_t));
		sec->next = NULL;
		if(buf)
		{
			utlSectionCopy(buf + 1,sec->section_name,']','\r');
		}
	}
	return sec;
}

static keyword_t* conf_new_key(char* buf,char* label)
{
	char* equ;
	keyword_t* key;
	if(buf)
	{
		equ = (char*)strstr(buf,"=");
		if(!equ)
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
	key = (keyword_t*)osMalloc(sizeof(keyword_t));
	if(key)
	{
		strcpy(key->key_label,label?label:"");
		key->next = NULL;
		if(buf)
		{
			utlSectionCopy(buf,key->key_name,'=','\r');
			utlSectionCopy(equ + 1,key->key_value,'\r',0);
		}
	}
	return key;
}

static section_t* conf_find_section(conf_t* conf,char* name)
{
	section_t* head;
	head = conf->sections;
	while(head)
	{
		if(!strcmp(head->section_name,name))
		{
			return head;
		}
		head = head->next;
	}
	return NULL;
}

static keyword_t* conf_find_key(section_t* sec,char* name)
{
	keyword_t* head = sec->key;
	while(head)
	{
		if(!strcmp(head->key_name,name))
		{
			return head;
		}
		head = (keyword_t*)head->next;
	}
	return NULL;
}


static section_t* conf_rebuild_sections(char* filename,conf_t* conf)
{
	int fd;
	char buf[1024];
	char label[256] = {0};
	section_t* head = NULL;
	section_t* item = NULL,*next;
	
	keyword_t* key,*last;

	fd = open(filename,O_RDONLY);
	if(fd < 0)
	{
		return (section_t*)NULL;
	}
	while(conf_gets(fd,buf,1024) > 0)
	{
		switch(buf[0])
		{
			case ':':
				strcpy(label,buf + 1);
				break;
			case ';':
				break;
			case '[':
				/*new section asserts,reset local label!*/
				label[0] = 0;
				item = conf_new_section(buf);
				if(!item)
				{
					break;
				}
				item->conf = conf;
				next = head;
				if(!next)
				{
					head = item;
				}
				else
				{
					while(next->next)
					{
						next = next->next;
					}
					next->next = item;
				}
				break;
			default:
				if(item == NULL)
				{
					break;
				}
				key = conf_new_key(buf,label);
				if(!key)
				{
					break;
				}	
				key->conf = conf;
				last = item->key;
				if(!last)
				{
					item->key = key;
				}
				else
				{
					while(last->next)
					{
						last = (keyword_t*)last->next;
					}		
					last->next = key;
				}
				/*reset label now!*/
				label[0] = 0;
				break;
		}		
		
	}
	close(fd);
	return head;
	
}

void* confCreate(char* conf)
{
	conf_t* ret = (conf_t*)osMalloc(sizeof(conf_t));
	if(ret)
	{
		memset(ret,0,sizeof(conf_t));
		if(conf)
		{
			strcpy(ret->filename,conf);
			ret->sections = conf_rebuild_sections(conf,ret);
			if(!ret->sections)
			{
				osFree(ret);
				return NULL;
			}
		}		
	}
	return ret;	
}

int confReadInt(void* hdl,char* section,char* key,int def_value)
{
	conf_t* ret = (conf_t*)hdl;
	section_t* sec;
	keyword_t* kw;
	if(ret)
	{
		sec = conf_find_section(ret,section);
		if(sec)
		{
			kw = conf_find_key(sec,key);
			if(kw)
			{
				return atoi(kw->key_value);
			}
		}
	}
	return def_value;
}
	
int confReadString(void* hdl,char* section,char* key,char* value)
{
	conf_t* ret = (conf_t*)hdl;
	section_t* sec;
	keyword_t* kw;
	*value = 0;
	if(ret)
	{
		sec = conf_find_section(ret,section);
		if(sec)
		{
			kw = conf_find_key(sec,key);
			if(kw)
			{
				strcpy(value,kw->key_value);
				return strlen(value);
			}
		}
	}
	return 0;
}

int confWriteInt(void* hdl, char* section,char* key,int value)
{
	conf_t* conf = (conf_t*)hdl;
	section_t* sec,*next_sec;
	keyword_t* kw,*next_kw;
	if(conf)
	{
		sec = conf_find_section(conf,section);
		if(sec)
		{
			kw = conf_find_key(sec,key);
			if(kw)
			{
				sprintf((char*)kw->key_value,"%d",value);
			}
			else
			{
				kw = conf_new_key(NULL,NULL);
				if(!kw)
				{
					return -1;
				}
				strcpy((char*)kw->key_name,key);
				sprintf((char*)kw->key_value,"%d",value);
				next_kw = sec->key;
				while(next_kw->next)
				{
					next_kw = next_kw->next;
				}
				next_kw->next = kw;				
			}
		}
		else
		{
			sec = conf_new_section(NULL);
			if(!sec)
			{
				return -1;
			}
			kw = conf_new_key(NULL,NULL);
			if(!kw)
			{
				osFree(sec);
				return -1;
			}
			strcpy((char*)kw->key_name,key);
			sprintf((char*)kw->key_value,"%d",value);
			sec->key = kw;
			next_sec = conf->sections;
			if(!next_sec)
			{
				conf->sections = sec;
			}
			else
			{
				while(next_sec->next)
				{
					next_sec = next_sec->next;
				}
				next_sec->next = sec;
			}
			
		}
		return 0;
	}
	return -1;
}

int confWriteString(void* hdl,char* section,char* key,char* value)
{
	conf_t* conf = (conf_t*)hdl;
	section_t* sec,*next_sec;
	keyword_t* kw,*next_kw;
	if(conf)
	{
		sec = conf_find_section(conf,section);
		if(sec)
		{
			kw = conf_find_key(sec,key);
			if(kw)
			{
				strcpy((char*)kw->key_value,value);
			}
			else
			{
				kw = conf_new_key(NULL,NULL);
				if(!kw)
				{
					return -1;
				}
				strcpy((char*)kw->key_name,key);
				strcpy((char*)kw->key_value,value);
				next_kw = sec->key;
				while(next_kw->next)
				{
					next_kw = next_kw->next;
				}
				next_kw->next = kw;				
			}
		}
		else
		{
			sec = conf_new_section(NULL);
			if(!sec)
			{
				return -1;
			}
			kw = conf_new_key(NULL,NULL);
			if(!kw)
			{
				osFree(sec);
				return -1;
			}
			strcpy((char*)kw->key_name,key);
			strcpy((char*)kw->key_value,value);
			sec->key = kw;
			next_sec = conf->sections;
			if(!next_sec)
			{
				conf->sections = sec;
			}
			else
			{
				while(next_sec->next)
				{
					next_sec = next_sec->next;
				}
				next_sec->next = sec;
			}
			
		}
		return 0;
	}
	return -1;
}

keyword_t* confGetKeyword(void* hdl,char* section)
{
	conf_t* ret = (conf_t*)hdl;
	section_t* sec;
	if(!ret)
	{
		return NULL;
	}
	sec = conf_find_section(ret,section);
	if(!sec)
	{
		return NULL;
	}
	return sec->key;
}

keyword_t* confFindKeyword(keyword_t* kw,char* label)
{
	int i;
	int pt;
	keyword_t* ret = kw;
	while(ret)
	{
		if(!strcmp(ret->key_label,label))
		{
			return ret;
		}
		ret = ret->next;
	}
	pt = atoi(label);
	ret = kw;
	for(i = 0; (i < pt) && ret;i ++)
	{
		ret = ret->next;
	}
	printf("head = %p %p:kw = %s,%s\n",kw,ret,kw->key_name,kw->key_value);
	return ret;

}

keyword_t* confCopyKeyword(keyword_t* kw)
{
	keyword_t* hdr = NULL,* item;
	keyword_t* next;
	if(!kw)
	{
		return NULL;
	}
	while(kw)
	{
		item = (keyword_t*)osMalloc(sizeof(keyword_t));
		if(!item)
		{
			return hdr;
		}
		*item = *kw;
		item->next = NULL;
		if(!hdr)
		{
			hdr = item;
		}
		else if(next)
		{
			next->next = item;
		}
		next = item;

		kw = kw->next;
	}
	return hdr;
}


int confSave(void* hdl,char* filename)
{
	conf_t* conf = (conf_t*)hdl;
	int fd;
	char buf[256];
	section_t* sfirst;
	keyword_t* kfirst;
	if(!conf || !filename || !filename[0])
	{
		return -1;
	}
	fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,0777);
	if(fd < 0)
	{
		return -1;
	}
	sfirst = conf->sections;
	while(sfirst)
	{
		sprintf(buf,"[%s]\r\n",sfirst->section_name);
		write(fd,buf,strlen(buf));
		kfirst = sfirst->key;
		while(kfirst)
		{
			sprintf(buf,"%s=%s\r\n",kfirst->key_name,kfirst->key_value);
			printf("%s\n",buf);
			write(fd,buf,strlen(buf));
			kfirst = kfirst->next;
		}

		sfirst = sfirst->next;
		
	}
	close(fd);
	return 0;
}


int confDestroy(void* hdl)
{
	conf_t* conf = (conf_t*)hdl;
	section_t* sec_head,*sec_next;;
	keyword_t* key_head,*key_next;
	if(conf)
	{
		sec_head = conf->sections;
		while(sec_head)
		{
			key_head = sec_head->key;
			while(key_head)
			{
				key_next = key_head->next;
				osFree(key_head);
				key_head = key_next;
			}
			sec_next = sec_head->next;
			osFree(sec_head);
			sec_head = sec_next;
		}
		osFree(conf);
	}
	return 0;
}


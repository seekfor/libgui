#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef __cplusplus
extern "C"
{
#endif

	int netRTPReceive(int ip, int svr_port,int clt_port);
	int netRTSPConnect(int srcip,uint32_t ip,uint32_t port);
	int netRTCPConnect(int client_port,int server_port,unsigned char* hostname);
	int netCreateUDPMulticastReceive(uint32_t src_ip,uint16_t src_port,uint32_t dst_ip,uint16_t dst_port);
	int netRTPConnect(char *hostname, int port);
	int netTCPConnectTimeout(int ip, unsigned short int port, unsigned int timeout );
	int netTCPConnectTimeout2(int srcip,int ip, unsigned short int port, unsigned int timeout );
	int netSetTRXSize(int fd,int bufsize);
	int netSetTCPNoDelay(int fd,int nodelay);

	int netCreateTCPConnect(unsigned short port);
	int netCreateUDPConnect(unsigned int local_ip,unsigned short local_port,unsigned int dst_ip,unsigned short dst_port);
	int netReceive(int fd, unsigned char *buf, int len,int timeout);
	int netSend(int fd, unsigned char *buff, int size, int timeout);
	int netReceive(int fd, unsigned char *buf, int len,int timeout);
	int netWait(int fd,int timeout);
	int netWaitWR(int fd,int timeout);
	int netGetIPFrom(char* hostname);
	int netGetETHInfo(char *eth,int* ip,int* netmask,char* mac,int* link);
	int netGetLocalIP();
	int netGetMAC(unsigned char* mac);
	int netGetGateway(char* gw);
	int netParseURL(unsigned char* url,int defport,unsigned char* username,unsigned char* password,unsigned char* hostname,unsigned char* port,unsigned char* path,unsigned char* params);
	int netAccept(int fd,int timeout,struct sockaddr_in* clientaddr);


#ifdef __cplusplus
}
#endif


#endif

#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>


typedef struct stream   //bytelogic
{
	char ioBuf[1024];
	int  ioSize;
	
	char inbuf[1024*5];
	int  inSize;
	int  availableSize;
	int  instartPos;

	char sendBuf[1024*5];
	int  sendSize;

	int needread;
	int socketId;
	void (*read_cb)(int fd, short flag, void *arg);
	void (*write)(int fd, char* buf,int len);

	void (*callback)(void *arg);
	void* connect_cb;
	void* asyncName;
	struct loop_base *loop;  
	void* lock;
	struct event *ev;
	int istcpconnected;
}stream_t;

typedef struct ClientStream {  //data logic
	stream_t *stream;
	char packetBuf[1024];
	int packetLen;
	void *client;
}ClientStream_t;


stream_t * startServer(char *ipstr, int PORT,void (*callback)(void *arg));
stream_t * clientConnect(char *hostname,int *port,void (*callback)(void *arg));
void  clientDisconnect(stream_t *streamPtr);

#endif


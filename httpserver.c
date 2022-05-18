#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "event.h"

#if 0

struct evhttp_connection;
extern struct loop_base evinterfaceBase;

LIST_HEAD(evconq, evhttp_connection);

struct evhttp_connection {
	int max_headers_size;
	int max_body_size;

	char *headJson;
	char *body;
};


typedef struct httpStream {  //data logic
	struct evconq clientLists;//
	struct stream *stream;
}httpStream_t;

httpStream_t httpServer;
struct loop_base *eventBase;
static void ioRecv(void *arg);
static void ioSend(void* buf);
static void pipeout(void *name);
static void pipein(void *value);

void initInterfaceNode(struct loop_base *loop);
void InterfaceDispatch(struct loop_base *loop);


void ioRecv(void *arg)
{	//if value change
	int nfd;
	char *str=NULL;
	struct sockaddr *peer_sa;
	int peer_socklen;
	iot_msgx("%s %s\n",__FUNCTION__,str);
	event_emit(&evinterfaceBase,"input",NULL);
	event_emit(&evinterfaceBase,"input","{pioneName:name1,value:2}");

}

static void ioSend(void* buf)
{
	
//	httpStream.stream->send(buf,0);

}

static void pipein(void *value)
{
	
}

static void pipeout(void *name)
{
	
}


void initInterfaceNode(struct loop_base *loop)
{
	struct stream *stream;
	stream = startServer("127.0.0.1","112233",ioRecv);
	httpServer.stream = stream;
	//httpStream.stream->read_cb = interfaceRecv; //for socket callback
	
	event_on((&evinterfaceBase,"input",pipein);//for interface shareData	
	event_on((&evinterfaceBase,"input",pipein);//for interface shareData
	event_on((&evinterfaceBase,"output",pipeout);//for interface shareData	
}


void InterfaceDispatch(struct loop_base *loop)
{
	loopRunForEver(loop);
}



#endif

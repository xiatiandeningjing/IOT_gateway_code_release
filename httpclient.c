#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "list.h"
#include "cjson.h"
#include "httpclient.h"
#include <pthread.h>

#if 0
	char httpconfig[] = "{\"host\":\"192.168.1.4\",\"port\":\"80\"}";
	httpinitInterfaceNode(httpconfig);
	makehttpRequest(onhttpResponse);
	httpinterfaceDispatch();
#endif

#if 1

struct evhttp_connection;
extern struct loop_base *evinterfaceBase;
struct loop_base evhttpBase;
pthread_t g_tid;

LIST_HEAD(evconq, evhttp_connection);



static ClientStream_t g_httpClient;
static char g_hostname[100];
static char g_port[100];


struct loop_base *eventBase;
static void httpioRecv(void *arg);
static void httpioSend(void* buf);
static void httppipeout(void *name);
static void httppipein(void *value);


void httpinitInterfaceNode(void *name);
void httpInterfaceDispatch(struct loop_base *loop);



static void httpioRecv(void *arg)
{	//if value change
	int nfd;
	char *str=NULL;
	struct sockaddr *peer_sa;
	int peer_socklen;
	
	event_emit(g_httpClient.stream->loop,"input","{\"pointName\":\"name1\",\"value\":2}");
	event_emit(g_httpClient.stream->loop,"httpRquest",arg);

}

static void httpioSend(void* buf)
{

}

static void httppipein(void *str)
{
	char *key =NULL;
	char *value = NULL;
	int len = 0;
	/*
	cJSON *jsonStr,*str_name,*str_value;
	jsonStr = cJSON_Parse(str);
	
	if (!jsonStr)
	{		
		iot_msgx("JSON Parse error:%s\n\n", cJSON_GetErrorPtr());   
	}    
	else    
	{		
		iot_msgx("JSON Parse OK:\n%s\n\n",  cJSON_Print(jsonStr) );
		str_name = cJSON_GetObjectItem(jsonStr, "pointName"); 
		
		if (str_name->type == cJSON_String)		  
		{ 		   
			iot_msgx("point name:%s\r\n", str_name->valuestring);		 
		}		  
		str_value = cJSON_GetObjectItem(jsonStr, "value");      
		if(str_value->type==cJSON_Number) 	   
		{			
			iot_msgx("point value:%d\r\n", str_value->valueint);		  
		} 	   

		cJSON_Delete(jsonStr);
	}*/
	key = json_get_value_by_name(str,strlen(str),"pointName",&len);
	value = json_get_value_by_name(str,strlen(str),"value",&len);

	iot_msgx("key :%s value:%s\r\n", key,value);		
}

static void httppipeout(void *name)
{
	
}


void httpinitInterfaceNode(void *configStr)
{
	int len = 0;
	iot_msgx("%s %d configStr %s\n",__FUNCTION__,__LINE__,configStr);
	memset(g_hostname,0,sizeof( g_hostname));
	memset(g_port ,0,sizeof( g_port));
	memset(&g_httpClient ,0,sizeof( g_httpClient));
	
	if(!evinterfaceBase)
		loop_base_init(&evinterfaceBase);	

	json_get_value_by_name(configStr,strlen(configStr),"host",g_hostname,&len);

	iot_msgx("%s %d host value %s\n",__FUNCTION__,__LINE__,g_hostname);
	
	json_get_value_by_name(configStr,strlen(configStr),"port",g_port,&len);
	iot_msgx("%s %d port %s\n",__FUNCTION__,__LINE__,g_port);
	
	event_on(evinterfaceBase,"input",httppipein);//for interface shareData	

}

int makehttpRequest(void (*onResponse)(void *arg))
{
	stream_t *streamptr=NULL;
	int ret = -1;
	char urlstr[1024];
	sprintf(urlstr,\
	"GET / HTTP/1.1\r\n\
	Host: %s\r\n\
	Accept: */*\r\n\
	\r\n",g_hostname);
	iot_errx("%s %d urlstr  %s\n",__FUNCTION__,__LINE__,urlstr); 

	streamptr = clientConnect(g_hostname,g_port , httpioRecv);

	if (!streamptr)
		return ;
	
	g_httpClient.stream = streamptr;
	//strcpy(g_httpClient.name,"httpClient");

	event_on(g_httpClient.stream->loop,"httpRquest",onResponse);//for interface shareData
	

	ret = send(streamptr->socketId, urlstr, strlen(urlstr), 0);
	
	return streamptr->socketId;

	
}

static void run(void*arg)
{
	iot_msgx("http %s %d \n",__FUNCTION__,__LINE__);
	loopRunForEver(g_httpClient.stream);
	clientDisconnect(g_httpClient.stream);
}

void httpinterfaceDispatch()
{
	pthread_create(&g_tid,NULL,run,NULL);
}




#endif


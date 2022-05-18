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
#include "event.h"
#include "stream.h"
#include "cjson.h"
#include "log.h"
#include "iot_mqttclient.h"
#include "json_parser.h"
#include "MQTTClient.h"
#include <pthread.h>

static ClientStream_t g_mqttClient;
static char  g_clientID[100];
static char  g_username[100];
static char  g_password[100];
static char  g_topic[100];
static char g_hostname[100];
static char g_port[100];
static pthread_t g_tid;
static pthread_t g_tid2;

extern struct loop_base *evinterfaceBase;
MQTTClient g_client;

int mqttstreamConnect(void (*onResponse)(void *arg));


void messageArrived(MessageData* md)
{
	char value[100];
	int len = 0;
	char *valueStr = md->message->payload;
	
	iot_debugx_("%s \n",__FUNCTION__);
	
	iot_debugx_("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
	iot_debugx_("recv %s\t", valueStr);


	json_get_value_by_name(valueStr,strlen(valueStr),"temperature",value,&len);

	iot_debugx_("recv value %s\t", value);

	event_emit(evinterfaceBase,"mqttRquest",value);	
}

#ifdef xnndebug
int readDo(int fd,char *buf,int len)
{
	static int readlen = 0;//xnndebug
	char data[]={0x30,0x12,0x00,0x0A,0x2F,0x78,0x6E,0x6E,0x2F,0x74,0x6F,0x70,0x69,0x63,0x31,0x31,0x31,0x32,
0x32,0x32};
	memcpy(buf,data + readlen,1);
	readlen += 1;
	
	return 1;
}
//xnndebug
#endif

#ifdef xnndebug
void read2(stream_t *arg)
{
	
	int n=0;
	int totaln = 0;
	int readn = 0;
	int leftn = 0;
	char *ptr=NULL;
	char buffer[1024];
	stream_t *context = arg;
	int clientfd = 9;
	//char *str= g_mqttClient.stream->ioBuf;
	//int size = g_mqttClient.stream->ioSize;
	char *toPtr = context->inbuf;
	int toSize = context->inSize;
	iot_msgx(" %s %d context->needread %d context->inSize %d toSize %d\n",\
			__FUNCTION__,__LINE__,context->needread,context->inSize,toSize);
	memset(buffer,0,sizeof(buffer));

	if (context->needread > 0)
	{
		totaln = context->needread;
		leftn = totaln;
		
		while(readn != totaln)
		{
			n= readDo(clientfd,buffer + readn,leftn);
		
			printf("%s %d readDo buffer str:\n",__FUNCTION__,__LINE__);
			print_hex(buffer + readn ,totaln);
			readn += n;
			leftn -= n;	
		}
		
		//buffer[readn] = '\0';
		context->needread = 0 ;
	}
	else
	{
		totaln = sizeof(buffer) - 1;
		n = readDo(clientfd,buffer,totaln);
		totaln = n;
		//buffer[n] = '\0';	
	}
	
	iot_msgx(" %s %d read more bytes length %d\n",__FUNCTION__,__LINE__,totaln);

	if ((toSize + totaln + 1) > sizeof(context->inbuf))
	{
		iot_errx("%s %d buffer overflow\n",__FUNCTION__,__LINE__);
		return;
	}
		
	printf("%s %d read2 buffer str:\n",__FUNCTION__,__LINE__);
	print_hex(buffer ,totaln);
	
	memcpy(toPtr + toSize,buffer,totaln);
	totaln += toSize;
	context->inSize = totaln;
	//toPtr[totaln] = '\0';
	
	iot_msgx(" %s %d have read  bytes length %d\n",__FUNCTION__,__LINE__,totaln);

	
	printf("%s %d read2 inbuf str:\n",__FUNCTION__,__LINE__);
	print_hex(g_mqttClient.stream->inbuf ,g_mqttClient.stream->inSize);
	
	if (context->callback)
		context->callback(buffer);

}
//xnndebug
#endif

void mqttioRecvdebug(void *arg)
{
}

void mqttioRecv(void *arg)
{	//if value change
	int nfd;
	int ret = 0;
	int readn=0;

	struct sockaddr *peer_sa;
	int peer_socklen;
	Timer timer;
	
    TimerInit(&timer);
    TimerCountdownMS(&timer, 1000);

	ret = mqttdoNextPacket(&g_mqttClient,&timer);
	
	if (BUFFER_OVERFLOW == ret)
	{
		iot_errx("%s %d buffer overflow\n",__FUNCTION__,__LINE__);
	}
}


int mqttinitInterfaceNode(char* configStr,void (*onResponse)(void *arg))
{
	
	int rc = 0;
	int len = 0;
	unsigned char buf[100];
	unsigned char readbuf[100];
	cJSON *jsonStr,*str_name,*str_value;
	char *value;
	
	memset((void*)&g_mqttClient,0,sizeof(g_mqttClient));
	
	memset(&g_topic,0,sizeof(g_topic));
	value = json_get_value_by_name(configStr,strlen(configStr),"topic",g_topic,&len);
	iot_debugx_("[%s %d] topic value %s\n",__FUNCTION__,__LINE__,g_topic);
	
	memset(g_password,0,sizeof(g_password));
	value = json_get_value_by_name(configStr,strlen(configStr),"password",g_password,&len);
	iot_debugx_("[%s %d] password  %s\n",__FUNCTION__,__LINE__,g_password);
	

	memset(g_username,0,sizeof(g_username));
	value = json_get_value_by_name(configStr,strlen(configStr),"username",g_username,&len);
	iot_debugx_("[%s %d] username  %s\n",__FUNCTION__,__LINE__,g_username);
	
	memset(g_clientID,0,sizeof(g_clientID));
	value = json_get_value_by_name(configStr,strlen(configStr),"clientId",g_clientID,&len);
	iot_debugx_("[%s %d] clientID  %s\n",__FUNCTION__,__LINE__,g_clientID);
	
	
	memset(g_hostname,0,sizeof( g_hostname));
	value = json_get_value_by_name(configStr,strlen(configStr),"host",g_hostname,&len);
	iot_debugx_("[%s %d] host  %s\n",__FUNCTION__,__LINE__,g_hostname);
	
	
	memset(g_port ,0,sizeof( g_port));
	value = json_get_value_by_name(configStr,strlen(configStr),"port",g_port,&len);
	iot_debugx_("[%s %d] port  %s\n",__FUNCTION__,__LINE__,g_port);
	
	if(!evinterfaceBase)
		loop_base_init(&evinterfaceBase);
	
	memset((char*)&g_client ,0,sizeof( g_port));
	g_mqttClient.client = &g_client;
	
	if (-1 == mqttstreamConnect(onResponse))
		return -1;

	return 0;
}

int mqttstreamConnect(void (*onResponse)(void *arg))
{
#ifndef xnndebug
	Network n;
	int rc = -1;
	int ret = -1;
	int clientId_len = 0;
	struct stream *streamptr=NULL;
	MQTTClient *client = (MQTTClient *)g_mqttClient.client;

	MQTTClientInit(client,3000);
	
	streamptr = clientConnect(g_hostname,g_port , mqttioRecv);////xnndebug
	
	if (!streamptr)
	{
		iot_errx("[%s %d] clientConnect failed\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	streamptr->istcpconnected = 1;
	g_mqttClient.stream = streamptr;

	event_on(evinterfaceBase,"mqttRquest",onResponse);//for interface shareData
#endif
}

int mqttmakeRequest()
{
#ifndef xnndebug
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;		
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = g_clientID;
	data.username.cstring = g_username;
	data.password.cstring = g_password;
	data.keepAliveInterval = 10;
	data.cleansession = 1;
	iot_debugx_("Connecting to %s %s\n", g_hostname, g_port);
	
	int rc = 0;
	rc = MQTTConnect(&(g_mqttClient), &data);
	iot_debugx_("Connected %d\n", rc);

	iot_debugx_("Subscribing to %s\n", g_topic);
	rc = MQTTSubscribe(&(g_mqttClient), g_topic, QOS0, messageArrived);
	iot_debugx_("Subscribed %d\n", rc);
	
#else
	mqttioRecv(NULL);
#endif
	return rc;

}


static void runio(void*arg)
{
	loopRunForEver(g_mqttClient.stream);
	clientDisconnect(g_mqttClient.stream);
}

void mqttinterfaceDispatch()
{
	pthread_create(&g_tid,NULL,runio,NULL);
}


void mqttClose(void)
{
	iot_msgx("Stopping\n");
}

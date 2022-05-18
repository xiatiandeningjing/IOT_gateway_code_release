/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *   Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *   Ian Craggs - fix for #96 - check rem_len in readPacket
 *   Ian Craggs - add ability to set message handler separately #6
 *******************************************************************************/
#include "MQTTClient.h"
#include "iot_mqttclient.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
//xnndebug
void print_hex(char *buffer, int len){
	int i;
#ifdef debugOn
	printf("******************start code**********************************\n");
	for(i = 1; i <= len; i++){
		printf("0x%02X ",buffer[i-1]);					
		if(i % 16 == 0){
			printf("\n");
		}
	}
	printf("\n");
	printf("********************end code************************************\n");
#endif
}

static void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage) {
    md->topicName = aTopicName;
    md->message = aMessage;
}


static int getNextPacketId(MQTTClient *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}


static int sendPacket(ClientStream_t* c, int length, Timer* timer)
{
    int rc = FAILURE,sent = 0;

    while (sent < length && !TimerIsExpired(timer))
    {
    	//iot_msgx("%s %d c->stream->socketId %d\n",__FUNCTION__,__LINE__,c->stream->socketId);
		print_hex(c->stream->sendBuf,length);

		struct timeval tv;
		
		tv.tv_sec = 0;	/* 30 Secs Timeout */
		tv.tv_usec = 200 * 1000;  // Not init'ing this can cause strange errors
		
		setsockopt(c->stream->socketId, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
		

		//iot_msgx("%s %d \n",__FUNCTION__,__LINE__);
		rc = write(c->stream->socketId, c->stream->sendBuf + sent, length);//send(c->stream->socketId, c->stream->sendBuf + sent,length, 0);
		//iot_msgx("%s %d \n",__FUNCTION__,__LINE__);

		TimerLeftMS(timer);

        if (rc < 0)  // there was an error writing the data
        {
        	iot_err("%s %d send error\n",__FUNCTION__,__LINE__);
            break;
        }
        sent += rc;
    }
    if (sent == length)
    {
 //     TimerCountdown(&c->client->last_sent,c->client->keepAliveInterval); // record the fact that we have successfully sent the packet
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    return rc;
}

#if 0
void MQTTClientInit(MQTTClient* c, Network* network, unsigned int command_timeout_ms,
		unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;
    c->ipstack = network;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    c->buf = sendbuf;
    c->buf_size = sendbuf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->cleansession = 0;
    c->ping_outstanding = 0;
    c->defaultMessageHandler = NULL;
	  c->next_packetid = 1;
    TimerInit(&c->last_sent);
    TimerInit(&c->last_received);
#if defined(MQTT_TASK)
	  MutexInit(&c->mutex);
#endif
}
#endif

#ifdef XNN
void MQTTClientInit(MQTTClient* c,unsigned int command_timeout_ms)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    
    c->isconnected = 0;
    c->cleansession = 0;
    c->ping_outstanding = 0;
    c->defaultMessageHandler = NULL;
	c->next_packetid = 1;
	c->runState = MQTT_STATE_INVALID;
    TimerInit(&c->last_sent);
    TimerInit(&c->last_received);
#if defined(MQTT_TASK)
	  MutexInit(&c->mutex);
#endif
}
#endif

static int decodePacket(ClientStream_t* c, int* value,int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;
	char byte = 0;
	char *inBuf = c->stream->inbuf;
	
    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        //c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        i = inBuf[c->stream->instartPos];
		(c->stream->instartPos)++;
		//(c->stream->size)--;
		rc = 1;
		//xnndebug
		iot_msgx("%s %d remaining length:\n",__FUNCTION__,__LINE__);
		print_hex(&i ,1);

		
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}

#if 0
static int readPacket(MQTTClient* c, Timer* timer,char *str,int len)
{
    MQTTHeader header = {0};
    int headerLen = 1,len= 0;
    int rem_len = 0;
	int rc = -1;
    /* 1. read the header byte.  This has the packet type in it */
    rc = c->ipstack->mqttread(c->ipstack, c->readbuf, 1, TimerLeftMS(timer));
    if (rc != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, TimerLeftMS(timer));
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (rem_len > (c->readbuf_size - len))
    {
        rc = BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (rc = c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, TimerLeftMS(timer)) != rem_len)) {
        rc = 0;
        goto exit;
    }

    header.byte = c->readbuf[0];
    rc = header.bits.type;
    if (c->keepAliveInterval > 0)
        TimerCountdown(&c->last_received, c->keepAliveInterval); // record the fact that we have successfully received a packet
exit:
    return rc;
}
#endif

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}


int deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }

    if (rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }

    return rc;
}


int keepalive(ClientStream_t* clientStream)
{
    int rc = SUCCESS;
	MQTTClient *c = (MQTTClient *)clientStream->client;
	iot_msgx("%s %d \n",__FUNCTION__,__LINE__);

    if (c->keepAliveInterval == 0)
        goto exit;

    if (TimerIsExpired(&c->last_sent) || TimerIsExpired(&c->last_received))
    {
    	iot_msgx("keepalive %s %d  ping_outstanding %d\n",__FUNCTION__,__LINE__,c->ping_outstanding);
	
        if (c->ping_outstanding)
            rc = FAILURE; /* PINGRESP not received in keepalive interval */
        else
        {
        	iot_msgx("keepalive SEND\n");
	
            Timer timer;
            TimerInit(&timer);
            TimerCountdownMS(&timer, 1000);
            int len = MQTTSerialize_pingreq(clientStream->stream->sendBuf,sizeof(clientStream->stream->sendBuf));
            if (len > 0 && (rc = sendPacket(clientStream, len, &timer)) == SUCCESS) // send the ping packet
                c->ping_outstanding = 1;
        }
    }
	iot_msgx("%s %d \n",__FUNCTION__,__LINE__);

exit:
    return rc;
}


void MQTTCleanSession(MQTTClient* c)
{
    int i = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = NULL;
}


void MQTTCloseSession(MQTTClient* c)
{
    c->ping_outstanding = 0;
    c->isconnected = 0;
    if (c->cleansession)
        MQTTCleanSession(c);
}

#if 0
int cycle(MQTTClient* c, Timer* timer,char *str,int len)
{
    int len = 0,
    rc = SUCCESS;

    int packet_type = readPacket(c, timer,str,len);     /* read the socket, see what work is due */

	iot_msgx("%s %d packet_type %d isconnected %d\n",__FUNCTION__,__LINE__,packet_type,c->isconnected);
    switch (packet_type)
    {
        default:
            /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
            rc = packet_type;
            goto exit;
        case 0: /* timed out reading packet */
            break;
        case CONNACK:
        case PUBACK:
        case SUBACK:
        case UNSUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            int intQoS;
            msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
            if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            msg.qos = (enum QoS)intQoS;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                else
                    rc = sendPacket(c, len, timer);
                if (rc == FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        case PUBREL:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size,
                (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket(c, len, timer)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE; // there was a problem
            if (rc == FAILURE)
                goto exit; // there was a problem
            break;
        }

        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }

    if (keepalive(c) != SUCCESS) {
		iot_msgx("keepalive failed %s %d \n",__FUNCTION__,__LINE__);
	
        //check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
        rc = FAILURE;
    }

exit:
    if (rc == SUCCESS)
        rc = packet_type;
    else if (c->isconnected)
        MQTTCloseSession(c);
    return rc;
}


int MQTTYield(MQTTClient* c, int timeout_ms)
{
    int rc = SUCCESS;
    Timer timer;

    TimerInit(&timer);
    TimerCountdownMS(&timer, timeout_ms);
	
	iot_msgx("%s %d  c->keepAliveInterval %d\n",__FUNCTION__,__LINE__,c->keepAliveInterval);
			

	  do
    {
        if (cycle(c, &timer) < 0)
        {
            rc = FAILURE;
            break;
        }
  	} while (!TimerIsExpired(&timer));

    return rc;
}



void MQTTRun(void* parm)
{
	Timer timer;
	MQTTClient* c = (MQTTClient*)parm;

	TimerInit(&timer);

	while (1)
	{
#if defined(MQTT_TASK)
		MutexLock(&c->mutex);
#endif
		TimerCountdownMS(&timer, 500); /* Don't wait too long if no traffic is incoming */
		cycle(c, &timer);
#if defined(MQTT_TASK)
		MutexUnlock(&c->mutex);
#endif
	}
}
#endif


int MQTTIsConnected(MQTTClient* client)
{
  return client->isconnected;
}

#if defined(MQTT_TASK)
int MQTTStartTask(MQTTClient* client)
{
	return ThreadStart(&client->thread, &MQTTRun, client);
}
#endif

#ifdef XNN


int waitfor(MQTTClient* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;
	c->waiting = 1;
	iot_msgx("%s %d client->waiting %x \n",__FUNCTION__,__LINE__,&(c->waiting));

	do
    {
        if (TimerIsExpired(timer))
            break; // we timed out
        //rc = cycle(c, timer);

	
		if (c->ack == packet_type)
		{
			if (SUBACK == packet_type)
				c->runState= MQTT_STATE_CONNECTED;
			
			rc = packet_type;
			break;
		}
    }
    while (1);
	

	c->waiting = 0;

	iot_msgx("%s %d c->waiting %d rc %d\n",__FUNCTION__,__LINE__,c->waiting,rc);
	if (FAILURE == rc)
		c->runState= MQTT_STATE_DISCONNECTED;
		
    return rc;
}

#endif
#if 0
int waitfor(MQTTClient* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;

    do
    {
        if (TimerIsExpired(timer))
            break; // we timed out
        rc = cycle(c, timer);
    }
    while (rc != packet_type && rc >= 0);

    return rc;
}
#endif



int MQTTConnectWithResults(ClientStream_t* mqttStream, MQTTPacket_connectData* options, MQTTConnackData* data)
{
    Timer connect_timer;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;
	stream_t *handle = mqttStream->stream;;
	char packetBuf[1024];
	MQTTClient *c = (MQTTClient *)mqttStream->client;
	
#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
	if (c->isconnected) /* don't send connect packet again if we are already connected */
		goto exit;
	iot_msgx("%s %d \n",__FUNCTION__,__LINE__);

    TimerInit(&connect_timer);
    TimerCountdownMS(&connect_timer, c->command_timeout_ms);

    if (options == 0)
        options = &default_options; /* set default options if none were supplied */

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession = options->cleansession;
    TimerCountdown(&c->last_received, c->keepAliveInterval);
	
    if ((len = MQTTSerialize_connect(handle->sendBuf, sizeof(handle->sendBuf), options)) <= 0)
        goto exit;

//	iot_msgx("%s %d sendBuf:\n",__FUNCTION__,__LINE__);
//	print_hex(mqttStream->stream->sendBuf,len);
	
    if ((rc = sendPacket(mqttStream, len, &connect_timer)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem
	iot_msgx("%s %d \n",__FUNCTION__,__LINE__);

    // this will be a blocking call, wait for the connack
    #if 1
    if (waitfor(c, CONNACK, &connect_timer) == CONNACK)
    {
        data->rc = 0;
        data->sessionPresent = 0;
        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, mqttStream->packetBuf, sizeof(mqttStream->packetBuf)) == 1)
            rc = data->rc;
        else
            rc = FAILURE;
    }
    else
    {
    	iot_msgx("%s %d timeOut\n",__FUNCTION__,__LINE__);
        rc = FAILURE;
    }
	#endif

exit:
    if (rc == SUCCESS)
    {
        c->isconnected = 1;
        c->ping_outstanding = 0;
    }
	else
	{
		iot_errx("%s %d Connect failed\n",__FUNCTION__,__LINE__);
	}

#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif

    return rc;
}


int MQTTConnect(ClientStream_t* c, MQTTPacket_connectData* options)
{
    MQTTConnackData data;
    return MQTTConnectWithResults(c, options, &data);
}


int mqttdoNextPacket(ClientStream_t* mqttStream, Timer* timer)
{
	int len = 0,ret = 0;
	int rc = SUCCESS;
	MQTTClient *client = (MQTTClient *)mqttStream->client;

	if (mqttStream->stream->inSize <= 0)
	{
		rc = FAILURE;
		MQTTCleanSession(client);//socket disconnect
		mqttStream->stream->istcpconnected = 0;;
		goto exit;
	}
	
	int packet_type = readOnePacket(mqttStream);	   /* read the socket, see what work is due */
	
	iot_msgx("%s %d packet_type %d isconnected %d ,mqttStream->stream->inSize %d\n",__FUNCTION__,__LINE__,packet_type,client->isconnected,\
		mqttStream->stream->inSize);
	
	switch (packet_type)
	{
		default:
			/* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
			rc = packet_type;
			goto exit;
		case NEEDREAD:
			//return rc;	//xnndebug
			rc = packet_type;
			
			#ifdef xnndebug
				if (mqttStream->stream->inSize >0)
					mqttioRecv(NULL);
			#else
			//	if ((mqttStream->stream->needread ==0) && (mqttStream->stream->inSize >0))
			//		mqttdoNextPacket(mqttStream,timer);		
			#endif

			
			goto exit;
			//mqttdoNextPacket(mqttStream,timer);
		case 0: /* timed out reading packet */
			break;
		case CONNACK:
		case PUBACK:
		case SUBACK:
		case UNSUBACK:
			goto exit;
			break;
		case PUBLISH:
		{	
			MQTTString topicName;
			MQTTMessage msg;
			int intQoS;

			msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
			if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
			   (unsigned char**)&msg.payload, (int*)&msg.payloadlen, mqttStream->packetBuf, mqttStream->packetLen) != 1)
				goto exit;

			//xnndebug
			iot_msgx("%s %d msg.payload %s\n",__FUNCTION__,__LINE__,msg.payload);
			//print_hex(msg.payload ,len);
				
			msg.qos = (enum QoS)intQoS;
			deliverMessage(client, &topicName, &msg);
			if (msg.qos != QOS0)
			{
				if (msg.qos == QOS1)
					len = MQTTSerialize_ack(mqttStream->stream->sendBuf, sizeof(mqttStream->stream->sendBuf), PUBACK, 0, msg.id);
				else if (msg.qos == QOS2)
					len = MQTTSerialize_ack(mqttStream->stream->sendBuf, sizeof(mqttStream->stream->sendBuf), PUBREC, 0, msg.id);

				//xnndebug
				//printf("%s %d\n",__FUNCTION__,__LINE__);
				//print_hex(mqttStream->stream->sendBuf ,len);

				if (len <= 0)
					rc = FAILURE;
				else
					rc = sendPacket(mqttStream, len, timer);
				if (rc == FAILURE)
					goto exit; // there was a problem
			}
			break;
		}
		case PUBREC:
		case PUBREL:
		{
			unsigned short mypacketid;
			unsigned char dup, type;
			if (MQTTDeserialize_ack(&type, &dup, &mypacketid, mqttStream->packetBuf,  mqttStream->packetLen) != 1)
				rc = FAILURE;
			else if ((len = MQTTSerialize_ack(mqttStream->stream->sendBuf, sizeof(mqttStream->stream->sendBuf),
				(packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0)
				rc = FAILURE;
			else if ((rc = sendPacket(mqttStream, len, timer)) != SUCCESS) // send the PUBREL packet
				rc = FAILURE; // there was a problem
			if (rc == FAILURE)
				goto exit; // there was a problem
			break;
		}

		case PUBCOMP:
			break;
		case PINGRESP:
			client->ping_outstanding = 0;
			iot_msgx("%s %d PINGRESP ok  %d \n",__FUNCTION__,__LINE__,client->ping_outstanding);
			
			break;
	}

exit:
	//xnndebug
	iot_msgx("%s %d rc %d client->waiting %d client %x\n",__FUNCTION__,__LINE__,rc,client->waiting,client);
					
	if (rc == SUCCESS)
	{
		client->ack = packet_type;
		rc = packet_type;
		ret = client->waiting;

		//printf("%s %d client->waiting %x \n",__FUNCTION__,__LINE__,&(client->waiting));
		//while(ret)
		{
			ret = client->waiting;
			iot_msgx("%s %d ret %d\n",__FUNCTION__,__LINE__,ret);
			usleep(200*1000);
		}
	
		int runState = client->runState;
		//printf("%s %d \n",__FUNCTION__,__LINE__);
		if ((MQTT_STATE_CONNECTED == runState) &&\
			((rc =keepalive(mqttStream))!= SUCCESS)) {
			//iot_errx("keepalive failed %s %d \n",__FUNCTION__,__LINE__);
		
			//check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
			rc = FAILURE;
		}
		//printf("%s %d ret %d over\n",__FUNCTION__,__LINE__,ret);


		if ((mqttStream->stream->needread ==0) && (mqttStream->stream->inSize >0))
		{
			iot_msgx("#########%s %d ret %d######### stream->inSize %d\n",__FUNCTION__,__LINE__,ret,mqttStream->stream->inSize);
			mqttdoNextPacket(mqttStream,timer); 
		}
	}
	else if ((FAILURE == rc) && client->isconnected)
	{
		client->ack = 0;
		MQTTCloseSession(client);
	}	
	else 
	{
		client->ack = 0;
	}
	return rc;
}



int readOnePacket(ClientStream_t* c)
{
	MQTTHeader header = {0};
	int len = 0;
	int rem_len = 0;
	char *inBuf = c->stream->inbuf;
	int readLen ;
	int rc = -1;
	int leftSize = 0;
	int validSize = 0;
	memset(c->packetBuf,0,sizeof(c->packetBuf));
	MQTTClient *client = (MQTTClient *)c->client; 
	/* 1. read the header byte.  This has the packet type in it */
	//int rc = c->ipstack->mqttread(c->ipstack, c->readbuf, 1, TimerLeftMS(timer));


	c->packetBuf[0] = inBuf[0];
	c->packetLen = 1;
	len=1;
	rc = 1;
	
	//xnndebug
	iot_msgx("%s %d header byte:\n",__FUNCTION__,__LINE__);
	print_hex(c->packetBuf ,1);

	
	c->stream->instartPos = 1;

	//if ((c->stream->size <= 0)
	//	len = 1;
	/* 2. read the remaining length.  This is variable in itself */
	decodePacket(c, &rem_len, 0);
	

	len += MQTTPacket_encode(c->packetBuf + 1, rem_len); /* put the original remaining length back into the buffer */

	if ((rem_len > (sizeof(c->packetBuf) - len)))
	{
		rc = BUFFER_OVERFLOW;
		goto exit;
	}

	validSize = c->stream->inSize - c->stream->instartPos;

	if (validSize < rem_len)
	{
		c->stream->needread = rem_len - validSize;
		rc = NEEDREAD;
		goto exit;
	}

	iot_msgx(" %s %d rem_len %d\n",__FUNCTION__,__LINE__,rem_len);
	/* 3. read the rest of the buffer using a callback to supply the rest of the data */
	//if (rem_len > 0 && (rc = c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, TimerLeftMS(timer)) != rem_len)) {
	if (rem_len < 0){
		rc = 0;
		goto exit;
	}
	
	//c->stream->inSize = c->stream->inSize - rem_len;
	c->packetLen = len + rem_len;
	memcpy(c->packetBuf + len,c->stream->inbuf + c->stream->instartPos,rem_len);
	
	iot_msgx(" %s %d packetLen %d\n",__FUNCTION__,__LINE__,c->packetLen);
	c->stream->instartPos = c->stream->instartPos + rem_len;
	
	leftSize = (c->stream->inSize - c->stream->instartPos);
	char *swapPtr = (char*)calloc(1,leftSize);
	if (!swapPtr)
	{
		iot_errx(" %s %d alloc falied\n",__FUNCTION__,__LINE__);
		goto exit;
	}
		
	memcpy(swapPtr,inBuf,leftSize);
	memcpy(inBuf,swapPtr,leftSize);
	free(swapPtr);
	
	c->stream->inSize = leftSize;
	
	//xnndebug
	iot_msgx("%s %d rest of the data rem_len %d rc %d leftSize %d\n",__FUNCTION__,__LINE__,rem_len,rc,leftSize);
	print_hex(c->packetBuf + len ,rem_len);
	
	header.byte = c->packetBuf[0];
	rc = header.bits.type;
	if (client->keepAliveInterval > 0)
		TimerCountdown(&client->last_received, client->keepAliveInterval); // record the fact that we have successfully received a packet

exit:
	return rc;
}



int MQTTSetMessageHandler(MQTTClient* c, const char* topicFilter, messageHandler messageHandler)
{
    int rc = FAILURE;
    int i = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != NULL && strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0)
        {
            if (messageHandler == NULL) /* remove existing */
            {
                c->messageHandlers[i].topicFilter = NULL;
                c->messageHandlers[i].fp = NULL;
            }
            rc = SUCCESS; /* return i when adding new subscription */
            break;
        }
    }
    /* if no existing, look for empty slot (unless we are removing) */
    if (messageHandler != NULL) {
        if (rc == FAILURE)
        {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == NULL)
                {
                    rc = SUCCESS;
                    break;
                }
            }
        }
        if (i < MAX_MESSAGE_HANDLERS)
        {
            c->messageHandlers[i].topicFilter = topicFilter;
            c->messageHandlers[i].fp = messageHandler;
        }
    }
    return rc;
}


int MQTTSubscribeWithResults(ClientStream_t* mqttStream, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler, MQTTSubackData* data)
{
    int rc = FAILURE;
    Timer timer;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
	stream_t *handle = mqttStream->stream;;
	MQTTClient *c = (MQTTClient *)mqttStream->client;


#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		    goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    len = MQTTSerialize_subscribe(handle->sendBuf, sizeof(handle->sendBuf), 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(mqttStream, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem


    if (waitfor(c, SUBACK, &timer) == SUBACK)      // wait for suback
    {
        int count = 0;
        unsigned short mypacketid;
        data->grantedQoS = QOS0;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int*)&data->grantedQoS, mqttStream->packetBuf, sizeof(mqttStream->packetBuf)) == 1)
        {
            if (data->grantedQoS != 0x80)
                rc = MQTTSetMessageHandler(c, topicFilter, messageHandler);
        }
    }
    else
        rc = FAILURE;

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTSubscribe(ClientStream_t* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler)
{
    MQTTSubackData data;
    return MQTTSubscribeWithResults(c, topicFilter, qos, messageHandler, &data);
}


int MQTTUnsubscribe(MQTTClient* c, const char* topicFilter)
{
    int rc = FAILURE;
    Timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		  goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (waitfor(c, UNSUBACK, &timer) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
        {
            /* remove the subscription message handler associated with this topic, if there is one */
            MQTTSetMessageHandler(c, topicFilter, NULL);
        }
    }
    else
        rc = FAILURE;

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTPublish(MQTTClient* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    Timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		    goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (message->qos == QOS1)
    {
        if (waitfor(c, PUBACK, &timer) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitfor(c, PUBCOMP, &timer) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTDisconnect(MQTTClient* c)
{
    int rc = FAILURE;
    Timer timer;     // we might wait for incomplete incoming publishes to complete
    int len = 0;

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

	  len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket(c, len, &timer);            // send the disconnect packet
    MQTTCloseSession(c);

#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}

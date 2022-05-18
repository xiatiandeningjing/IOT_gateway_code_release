#ifndef IOT_MQTT_H_
#define IOT_MQTT_H_

#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

int mqttinitInterfaceNode(char* configStr,void (*onResponse)(void *arg));
void mqttinterfaceDispatch();
void mqttioRecv(void *arg);
int mqttmakeRequest();



#endif

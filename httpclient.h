#ifndef __HTTP_CLIENT__
#define __HTTP_CLIENT__

#include "event.h"
#include "stream.h"

struct evhttp_connection {
	int max_headers_size;
	int max_body_size;

	char *headJson;
	char *body;
};


typedef struct httpClientStream {  //data logic
	stream_t *stream;
	char *buff;
	char name[100];
}httpClientStream_t;

void httpinitInterfaceNode(void *name);
void httpinterfaceDispatch();
int makehttpRequest(void (*onResponse)(void *arg));
#endif
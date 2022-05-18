
#ifndef EVENT_T
#define EVENT_T
#include "stream.h"
#include "list.h"
#include <pthread.h>
#define EV_TIMEOUT	0x01
/** readable event*/
#define EV_READABLE		0x02
/** writeable event*/
#define EV_WRITEABLE	0x04
/** signal event*/
#define EV_SIGNAL	0x08
#define EV_CLOSED	0x80

/** event active*/
#define EVLIST_ACTIVE	    0x08
#define EVLIST_INTERNAL	    0x10
#define EVLIST_ACTIVE_LATER 0x20
#define EVLIST_FINALIZING   0x40
#define EVLIST_INIT	    0x80

#define EVLIST_ALL          0xff
#define EV_PERSIST	0x10

#if 1

/* Fix so that people don't have to run with <sys/queue.h> */
#ifndef TAILQ_ENTRY
#define EVENT_DEFINED_TQENTRY_
#define TAILQ_ENTRY(type)\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}
#endif /* !TAILQ_ENTRY */
#endif

typedef int (*io_callback_t)(int, short, void *);
typedef void (*signal_callback_t)(void *);


#define NAME_LENGTH 512


struct socketIoOpt {
	/** The name of this backend. */
	const char *name;

	void *(*_init)(struct loop_base *);
	int (*add)(struct loop_base *, int fd, short old, short events, void *fdinfo);
	int (*del)(struct loop_base *, int fd, short old, short events, void *fdinfo);
	int (*dispatch)(struct loop_base *, struct timeval *);
	void (*dealloc)(struct loop_base *);

};

LIST_HEAD(evLists_head_t, event) ;  //list head


struct loop_base
{
	void  *evbase;
	const struct socketIoOpt *evsel;

	struct evLists_head_t sigQueues;  //list head
	struct evLists_head_t ioQueues;  //list head
	struct evLists_head_t activeQueues;  //list head

	
	int sigNumqueues;
	int ioNumqueues;
	int activeNumqueues;


    pthread_mutex_t sigLock;
    pthread_mutex_t ioLock;
	pthread_mutex_t activeLock;
	pthread_mutex_t selectLock;

};

struct event {
	char name[NAME_LENGTH];
	io_callback_t ev_cb;
	signal_callback_t ev_sig_cb;
	
	char *ev_arg;
	LIST_ENTRY (event) ev_next;  //list elem type

	char *data;
	int ev_fd;

	short ev_events;
	short ev_res;		/* result passed to event callback */
	short ev_flags;

	struct loop_base *ev_base;

};

int event_item_add(struct loop_base *ev_base,struct event *ev);
struct event * event_item_alloc(struct loop_base *base, int  fd, short events, void (*callback)(int, short, void *), void *arg);
struct loop_base *loop_base_new();
void loopRunForEver(stream_t *clientStream);
int event_emit(struct loop_base *ev_base,char *eventName,void *arg);
int event_on(struct loop_base *ev_base,char *eventName,signal_callback_t func);
struct loop_base *loop_base_init(struct loop_base **base);
int event_io_active(struct loop_base *ev_base,int fd,int flag);

#endif

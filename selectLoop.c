#include "log.h"
#include "event.h"

#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>

#include <sys/types.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned long fd_mask2;

#define NFDBITS (sizeof(fd_mask2)*8)
/* Divide positive x by y, rounding up. */
#define DIV_ROUNDUP(x, y)   (((x)+((y)-1))/(y))

/* How many bytes to allocate for N fds? */
#define SELECT_ALLOC_SIZE(n) \
	(DIV_ROUNDUP(n, NFDBITS) * sizeof(fd_mask2))

struct selectop {
	int event_fds;		/* Highest fd in fd set */
	int event_fdsz;
	int resize_out_sets;
	fd_set *event_readset_in;
};

static void *select_init(struct loop_base *);
static int select_add(struct loop_base *, int, short old, short events, void*);
static int select_del(struct loop_base *, int, short old, short events, void*);
static int select_dispatch(struct loop_base *, struct timeval *);
static void select_dealloc(struct loop_base *);
#if 0
const struct socketIoOpt selectops = {
	"select",
	select_init,
	select_add,
	select_del,
	select_dispatch,
	select_dealloc,
	1, /* need_reinit. */
};
#endif

struct socketIoOpt *select_opt_init()
{
	struct socketIoOpt *ptr = NULL;
	if ((ptr = calloc(1, sizeof(struct socketIoOpt))) == NULL) {
		iot_err("%s: calloc failed", __func__);
		return NULL;
	}

	ptr->_init = select_init;
	ptr->add	  = select_add;
	ptr->del   = select_del;
	ptr->dispatch = select_dispatch;
	ptr->dealloc = select_dealloc;
	return ptr;

}

static int select_resize(struct selectop *sop, int fdsz);
static void select_free_selectop(struct selectop *sop);


static int
select_resize(struct selectop *sop, int fdsz)
{
	fd_set *readset_in = NULL;

	if ((readset_in = realloc(sop->event_readset_in, fdsz)) == NULL)
		goto error;
	sop->event_readset_in = readset_in;

	memset((char *)sop->event_readset_in + sop->event_fdsz, 0,
	fdsz - sop->event_fdsz);
	sop->event_fdsz = fdsz;

	return (0);

 error:
	iot_warn("malloc");
	return (-1);
}

static void *
select_init(struct loop_base *base)
{
	struct selectop *sop;

	if (base->evbase)
		return base->evbase;
	//init=1;
	
	////iot_msgx("%s %d\n",__FUNCTION__,__LINE__);
	
	if (!(sop = calloc(1,sizeof(struct selectop))))
		return (NULL);
	
	if (select_resize(sop, SELECT_ALLOC_SIZE(32 + 1))) {
		select_free_selectop(sop);
		return (NULL);
	}
	//iot_msgx("%s %d OK\n",__FUNCTION__,__LINE__);
	return sop;
}


static int
select_add(struct loop_base *base, int fd, short old, short events, void *p)
{
	struct selectop *sop = base->evbase;

	////iot_msgx("%s %d\n",__FUNCTION__,__LINE__);

	if (sop && (sop->event_fds < fd)) {
	
		int fdsz = sop->event_fdsz;
		

		if (fdsz < (int)sizeof(fd_mask2))
			fdsz = (int)sizeof(fd_mask2);

		if (fdsz < (int) SELECT_ALLOC_SIZE(fd + 1))
				fdsz *= 2;
	
			if (fdsz != sop->event_fdsz) {
				if (select_resize(sop, fdsz)) {
					return (-1);
				}
			}
			
		sop->event_fds = fd;
	}
	
	if (events & EV_READABLE)
	{
		//iot_msgx("%s %d insert fd %d EV_READABLE OK\n",__FUNCTION__,__LINE__,fd);
		FD_SET(fd, sop->event_readset_in);
	}
	//iot_msgx("%s %d OK\n",__FUNCTION__,__LINE__);

	return (0);
}

/*
 * Nothing to be done here.
 */

static int
select_del(struct loop_base *base, int fd, short old, short events, void *p)
{
	struct selectop *sop = base->evbase;
	(void)p;

	assert((events & EV_SIGNAL) == 0);

	if (sop->event_fds < fd) {
		return (0);
	}

	if (events & EV_READABLE)
		FD_CLR(fd, sop->event_readset_in);

	return (0);
}

static void
select_free_selectop(struct selectop *sop)
{
#if 1
	if (sop->event_readset_in)
		free(sop->event_readset_in);
	memset(sop, 0, sizeof(struct selectop));
	free(sop);
#endif
}

static void
select_dealloc(struct loop_base *base)
{

}



select_dispatch(struct loop_base *base,struct timeval *tv)
{
	int res=0, i=0, j=0, nfds=0;
	
	struct selectop *sop = base->evbase;

//	//iot_msgx("%s %d sop->event_fds %d\n",__FUNCTION__,__LINE__,sop->event_fds);

	nfds = sop->event_fds+1;

	res = select(nfds, sop->event_readset_in,
		NULL, NULL, NULL);
	
	//iot_msgx("%s %d res %d\n",__FUNCTION__,__LINE__,res);
	//sleep(30000);
	
	if (res == -1) {
		if (errno != EINTR) {
			iot_warn("select");
			return (-1);
		}

		return (0);
	}

	for (j = 0; j < nfds; ++j) {
		if (++i >= nfds)
			i = 0;
		res = 0;
		if (FD_ISSET(i,sop->event_readset_in))
		{
			//iot_msgx("%s %d event_readset active\n",__FUNCTION__,__LINE__);
			res |= EV_READABLE;
			
		}
		
		if (res == 0)
			continue;
		
		event_io_active(base, i, res);
		
	}
	
	return (0);

}


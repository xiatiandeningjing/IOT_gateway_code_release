#ifndef LOG_INTERNAL_H_INCLUDED_
#define LOG_INTERNAL_H_INCLUDED_

#include <stdint.h>
#include"stdarg.h" 

#define EVENT_LOG_DEBUG 0
#define EVENT_LOG_MSG   1
#define EVENT_LOG_WARN  2
#define EVENT_LOG_ERR   3


#define _EVENT_LOG_DEBUG EVENT_LOG_DEBUG
#define _EVENT_LOG_MSG EVENT_LOG_MSG
#define _EVENT_LOG_WARN EVENT_LOG_WARN
#define _EVENT_LOG_ERR EVENT_LOG_ERR



#ifdef __cplusplus
extern "C" {
#endif

void iot_err(const char *fmt, ...);
void iot_warn(const char *fmt, ...);
void iot_sock_err(int eval, int sock, const char *fmt, ...);
void iot_sock_warn(int sock, const char *fmt, ...);
void iot_errx(const char *fmt, ...) ;
void iot_warnx(const char *fmt, ...);
void iot_msgx(const char *fmt, ...) ;
void iot_debugx_(const char *fmt, ...);
void iot_logv_(int severity, const char *errstr, const char *fmt, va_list ap);


#define event_debug(x) do {			\
		event_debugx_ x;		\
	}					\
	} while (0)

#endif

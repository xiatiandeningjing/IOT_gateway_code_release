/* Define to 1 if you have the `select' function. */
#include "list.h"
#include "event.h"
#define EVENT__HAVE_SELECT 1

struct point {
	int value;
	char *name;
} ;

struct interface {
	int value;
	char *name;
} ;

typedef struct deviceInterfaceConfig{
char *name;
void (*init)(struct loop_base * ev_base);
void (*dispatch)(struct loop_base * ev_base);

}interfaceConfig_t;

LIST_HEAD(pointsLists_head_t, point) pointsListsHead;  //list head
LIST_HEAD(interfacesLists_head_t, interface) interfacesListsHead;  //list head


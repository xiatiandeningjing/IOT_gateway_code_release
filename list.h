#ifndef __LIST__INCLUDE__
#define __LIST__INCLUDE__
/*
 * Singly-linked List definitions.
 */
#define SLIST_HEAD(name, type)						\
struct name {								\
	struct type *slh_first;	/* first element */			\
}

#define	SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define SLIST_ENTRY(type)						\
struct {								\
	struct type *sle_next;	/* next element */			\
}


/*
 * Singly-linked List access methods.
 */
#define	SLIST_FIRST(head)	((head)->slh_first)
#define	SLIST_END(head)		NULL
#define	SLIST_EMPTY(head)	(SLIST_FIRST(head) == SLIST_END(head))
#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)

#define	SLIST_FOREACH(var, head, field)					\
	for((var) = SLIST_FIRST(head);					\
	    (var) != SLIST_END(head);					\
	    (var) = SLIST_NEXT(var, field))

/*
 * Singly-linked List functions.
 */
#define	SLIST_INIT(head) {						\
	SLIST_FIRST(head) = SLIST_END(head);				\
}

#define	SLIST_INSERT_AFTER(slistelm, elm, field) do {			\
	(elm)->field.sle_next = (slistelm)->field.sle_next;		\
	(slistelm)->field.sle_next = (elm);				\
} while (0)

#define	SLIST_INSERT_HEAD(head, elm, field) do {			\
	(elm)->field.sle_next = (head)->slh_first;			\
	(head)->slh_first = (elm);					\
} while (0)

#define	SLIST_REMOVE_HEAD(head, field) do {				\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} while (0)


#ifndef TAILQ_HEAD
#define EVENT_DEFINED_TQHEAD_
#define TAILQ_HEAD(name, type)			\
struct name {					\
	struct type *tqh_first;			\
	struct type **tqh_last;			\
}
#endif

/* Fix so that people don't have to run with <sys/queue.h> */
#ifndef LIST_ENTRY
#define EVENT_DEFINED_LISTENTRY_
#define LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}
#endif /* !LIST_ENTRY */

#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}





/*
 * List access methods
 */
#define	LIST_FIRST(head)		((head)->lh_first)
#define	LIST_END(head)			NULL
#define	LIST_EMPTY(head)		(LIST_FIRST(head) == LIST_END(head))
#define	LIST_NEXT(elm, field)		((elm)->field.le_next)

#define LIST_FOREACH(var, head, field)					\
	for((var) = LIST_FIRST(head);					\
	    (var)!= LIST_END(head);					\
	    (var) = LIST_NEXT(var, field))

/*
 * List functions.
 */
#define	dLIST_INIT(head) do {						\
	LIST_FIRST(head) = LIST_END(head);				\
} while (0)

#define dLIST_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define	dLIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)


#define dLIST_INSERT_HEAD(head, elm, field) do {				\
		if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
			(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
		(head)->lh_first = (elm);					\
		(elm)->field.le_prev = &(head)->lh_first;			\
	} while (0)
	

#define dLIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)

#define dLIST_REPLACE(elm, elm2, field) do {				\
	if (((elm2)->field.le_next = (elm)->field.le_next) != NULL)	\
		(elm2)->field.le_next->field.le_prev =			\
		    &(elm2)->field.le_next;				\
	(elm2)->field.le_prev = (elm)->field.le_prev;			\
	*(elm2)->field.le_prev = (elm2);				\
} while (0)

//for json list
/*
 * Get offset of a member variable.
 *
 * @param[in]	type	 the type of the struct this is embedded in.
 * @param[in]	member	 the name of the variable within the struct.
 */
#define aos_offsetof(type, member)   ((size_t)&(((type *)0)->member))

/*
 * Get the struct for this entry.
 *
 * @param[in]	ptr 	the list head to take the element from.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the variable within the struct.
 */
#define aos_container_of(ptr, type, member) \
	((type *) ((char *) (ptr) - aos_offsetof(type, member)))

/* for double link list */
typedef struct dlist_s {
	struct dlist_s *prev;
	struct dlist_s *next;
} dlist_t;

static inline void __dlist_add(dlist_t *node, dlist_t *prev, dlist_t *next)
{
	node->next = next;
	node->prev = prev;

	prev->next = node;
	next->prev = node;
}

/*
 * Get the struct for this entry.
 *
 * @param[in]	addr	the list head to take the element from.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the dlist_t within the struct.
 */
#define dlist_entry(addr, type, member) \
	((type *)((long)addr - aos_offsetof(type, member)))


static inline void dlist_add(dlist_t *node, dlist_t *queue)
{
	__dlist_add(node, queue, queue->next);
}

static inline void dlist_add_tail(dlist_t *node, dlist_t *queue)
{
	__dlist_add(node, queue->prev, queue);
}

static inline void dlist_del(dlist_t *node)
{
	dlist_t *prev = node->prev;
	dlist_t *next = node->next;

	prev->next = next;
	next->prev = prev;
}

static inline void dlist_init(dlist_t *node)
{
	node->next = node->prev = node;
}

static inline void INIT_AOS_DLIST_HEAD(dlist_t *list)
{
	list->next = list;
	list->prev = list;
}

static inline int dlist_empty(const dlist_t *head)
{
	return head->next == head;
}

/*
 * Initialise the list.
 *
 * @param[in]	list	the list to be inited.
 */
#define AOS_DLIST_INIT(list)  {&(list), &(list)}

/*
 * Get the first element from a list
 *
 * @param[in]	ptr 	the list head to take the element from.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the dlist_t within the struct.
 */
#define dlist_first_entry(ptr, type, member) \
	dlist_entry((ptr)->next, type, member)

/*
 * Iterate over a list.
 *
 * @param[in]	pos 	the &struct dlist_t to use as a loop cursor.
 * @param[in]	head	he head for your list.
 */
#define dlist_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/*
 * Iterate over a list safe against removal of list entry.
 *
 * @param[in]	pos 	the &struct dlist_t to use as a loop cursor.
 * @param[in]	n		another &struct dlist_t to use as temporary storage.
 * @param[in]	head	he head for your list.
 */
#define dlist_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		 pos = n, n = pos->next)

/*
 * Iterate over list of given type.
 *
 * @param[in]	queue	he head for your list.
 * @param[in]	node	the &struct dlist_t to use as a loop cursor.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the dlist_t within the struct.
 */
#define dlist_for_each_entry(queue, node, type, member) \
	for (node = aos_container_of((queue)->next, type, member); \
		 &node->member != (queue); \
		 node = aos_container_of(node->member.next, type, member))

/*
 * Iterate over list of given type safe against removal of list entry.
 *
 * @param[in]	queue	the head for your list.
 * @param[in]	n		the type * to use as a temp.
 * @param[in]	node	the type * to use as a loop cursor.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the dlist_t within the struct.
 */
#define dlist_for_each_entry_safe(queue, n, node, type, member) \
	for (node = aos_container_of((queue)->next, type, member),	\
		 n = (queue)->next ? (queue)->next->next : NULL;		\
		 &node->member != (queue);								\
		 node = aos_container_of(n, type, member), n = n ? n->next : NULL)

/*
 * Get the struct for this entry.
 * @param[in]	ptr 	the list head to take the element from.
 * @param[in]	type	the type of the struct this is embedded in.
 * @param[in]	member	the name of the variable within the struct.
 */
#define list_entry(ptr, type, member) \
	aos_container_of(ptr, type, member)


/*
 * Iterate backwards over list of given type.
 *
 * @param[in]	pos 	the type * to use as a loop cursor.
 * @param[in]	head	he head for your list.
 * @param[in]	member	the name of the dlist_t within the struct.
 * @param[in]	type	the type of the struct this is embedded in.
 */
#define dlist_for_each_entry_reverse(pos, head, member, type) \
	for (pos = list_entry((head)->prev, type, member);		  \
		 &pos->member != (head);							  \
		 pos = list_entry(pos->member.prev, type, member))


/*
 * Get the list length.
 *
 * @param[in]  queue  the head for your list.
 */
static inline int __dlist_entry_number(dlist_t *queue)
{
	int num;
	dlist_t *cur = queue;
	for (num = 0; cur->next != queue; cur = cur->next, num++)
		;

	return num;
}

/*
 * Get the list length.
 *
 * @param[in]  queue  the head for your list.
 */
#define dlist_entry_number(head) \
	__dlist_entry_number(head)

/*
 * Initialise the list.
 *
 * @param[in]	name	the list to be initialized.
 */
#define AOS_DLIST_HEAD_INIT(name) { &(name), &(name) }

/*
 * Initialise the list.
 *
 * @param[in]	name	the list to be initialized.
 */
#define AOS_DLIST_HEAD(name) \
	dlist_t name = AOS_DLIST_HEAD_INIT(name)
	
#endif

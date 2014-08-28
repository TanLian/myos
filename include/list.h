#ifndef _list_h_
#define _list_h_

struct list_head
{
    struct list_head *next, *prev;
};

#define INIT_LIST_HEAD(ptr)     \
do { \
    (ptr)->next = (ptr); \
    (ptr)->prev = (ptr); \
} while (0)


#define list_add(new_node, prev_node, next_node) do {\
    (next_node)->prev = (new_node);      \
    (new_node)->next = (next_node);      \
    (new_node)->prev = (prev_node);      \
    (prev_node)->next = (new_node);      \
} while (0)


#define list_del(entry) do {  \
    (entry)->next->prev = (entry)->prev;    \
    (entry)->prev->next = (entry)->next;    \
} while (0)

#define LIST_EACH(type,task_head, tmp)   \
    for ((tmp) = (type)(task_head)->list.next; (task_head) != (tmp); (tmp) = (type)(tmp)->list.next)

    
#define list_entry(ptr, type, member)   \
    ((type *)((unsigned long)(ptr) - (unsigned long)(&((type *)0)->member)))

/*

//#define list_entry(ptr, type, member) \
//    container_of(ptr, type, member)

//#define container_of(ptr, type, member) ( { \
//    const typeof( ((type *)0->member) *__mptr = (ptr); \
//    (type *)( (char *)__mptr - offsetof(type, member) ); })
*/
#endif




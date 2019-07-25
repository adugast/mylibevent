#ifndef __LIST_H__
#define __LIST_H__


typedef __SIZE_TYPE__ size_t;


struct list_head {
    struct list_head *next, *prev;
};


#define LIST_HEAD_INIT(name) { &(name), &(name) }

/**
 * INIT_LIST - Define and initialize a head of list
 *
 * @name: name of the structure
 */
#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void init_list(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

/**
 * list_entry - retrieve the struct for this entry
 *
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct member within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each - iterate over a list
 *
 * @pos:    the struct list_head ptr to use as a loop counter.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_entry - iterate over list of given type
 *
 * @pos:    the ptr type of the struct to retrieve and used as a loop counter.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)                          \
    for (pos = list_entry((head)->next, typeof(*pos), member);          \
            &pos->member != (head);                                     \
            pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 *
 * @pos:    the &struct list_head to use as a loop counter.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_safe(pos, n, head)                    \
    for (pos = (head)->next, n = pos->next; pos != (head);  \
            pos = n, n = pos->next)

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 *
 * @pos:    the type * to use as a loop counter.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)                  \
    for (pos = list_entry((head)->next, typeof(*pos), member),          \
            n = list_entry(pos->member.next, typeof(*pos), member);     \
            &pos->member != (head);                                     \
            pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_prev - iterate over a list backwards
 *
 * @pos:    the &struct list_head to use as a loop counter.
 * @head:   the head for your list.
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * __list_insert - ONLY FOR INTERNAL USE
 *
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 *
 * Use as base for list_add_head and list_add_tail
 */
static inline void __list_insert(struct list_head *entry,
        struct list_head *prev,
        struct list_head *next)
{
    next->prev = entry;
    entry->next = next;
    entry->prev = prev;
    prev->next = entry;
}

/**
 * list_add_head - add a new entry
 *
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add_head(struct list_head *entry, struct list_head *head)
{
    __list_insert(entry, head, head->next);
}

/**
 * list_add_tail - add a new entry
 *
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *entry, struct list_head *head)
{
    __list_insert(entry, head->prev, head);
}

/**
 * ONLY FOR INTERNAL USE - __list_del
 *
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 *
 * Use as base for list_del and list_del_init
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - deletes entry from list.
 *
 * @entry: the element to delete from the list.
 *
 * list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (void *) 0;
    entry->prev = (void *) 0;
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 *
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * list_empty - tests whether a list is empty
 *
 * @head: the list to test.
 */
static inline int list_empty(struct list_head *head)
{
    return head->next == head;
}

/**
 * list_length - return the number of structure that compose the list.
 *
 * @head: the head of your list.
 */
static inline size_t list_length(struct list_head *head)
{
    size_t len = 0;
    struct list_head *pos;

    list_for_each(pos, head)
        len++;

    return len;
}

/**
 * list_move - delete from one list and add as another's head
 *
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_head(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 *
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
        struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}


/**
 * ONLY FOR INTERNAL USE - __list_splice
 *
 * Use as base for list_splice_init and list_splice
 */
static inline void __list_splice(struct list_head *list,
        struct list_head *head)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;
    struct list_head *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 *
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * list_splice - join two lists
 *
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head);
}

#endif /* __LIST_H__ */


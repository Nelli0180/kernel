#ifndef LIST_H
#define LIST_H

#include <lib/stddef.h>

typedef struct list_head {
	struct list_head *next;
	struct list_head *prev;
} list_head_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	list_head_t name = LIST_HEAD_INIT(name)

static inline void list_init(list_head_t *list) {
	list->next = list;
	list->prev = list;
}

static inline void list_add(list_head_t *new, list_head_t *head) {
	new->next = head->next;
	new->prev = head;
	head->next->prev = new;
	head->next = new;
}

static inline void list_add_tail(list_head_t *new, list_head_t *head) {
	new->next = head;
	new->prev = head->prev;
	head->prev->next = new;
	head->prev = new;
}

static inline void list_del(list_head_t *entry) {
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
	entry->next = NULL;
	entry->prev = NULL;
}

static inline int list_empty(const list_head_t *head) {
	return head->next == head;
}

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#endif /* LIST_H */
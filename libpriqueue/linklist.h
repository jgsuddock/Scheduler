#ifndef LINKLIST_H
#define LINKLIST_H

typedef struct _listnode_t {
	struct _listnode_t *next;
} listnode_t;

typedef struct _linklist_t {
	int count;
	_listnode_t *head;
} linklist_t;

void linklist_init    (linklist_t *list);

int  linklist_push    (linklist_t *list);
int  linklist_pop     (linklist_t *list);

int  linklist_size

void linklist_destroy

#endif

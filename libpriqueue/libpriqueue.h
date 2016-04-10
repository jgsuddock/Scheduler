/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

// struct listnode_t;

/**
  Node Which Holds Values for Priqueue
 */
typedef struct _listnode_t
{
	void *value;
	struct _listnode_t *next;
} listnode_t;

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
	//<0 The element pointed by p1 goes before the element pointed by p2
	//0  The element pointed by p1 is equivalent to the element pointed by p2
	//>0 The element pointed by p1 goes after the element pointed by p2
	int(*comparer)(const void *, const void *);
	int size;
	struct _listnode_t *front;
} priqueue_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */

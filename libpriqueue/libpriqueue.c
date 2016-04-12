/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "libpriqueue.h"

/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->size = 0;
	q->front = NULL;
	q->comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	listnode_t *node = (listnode_t *)malloc(sizeof(listnode_t));
	node->value = ptr;
	node->next = NULL;
	q->size++;

	int saveindex = -1;
	if(q->front == NULL) { //Array is empty
		q->front = node;
		saveindex = 0;
	}
	else { // Array is not empty
		if(q->comparer(ptr, q->front->value) < 0) {
			node->next = q->front;
			q->front = node;
			saveindex = 0;
		}
		else {
			listnode_t *tempptr = q->front;
			saveindex = 1;
			while(tempptr->next != NULL && q->comparer(ptr, tempptr->next->value) >= 0) {
				tempptr = tempptr->next;
				saveindex++;
			}
			node->next = tempptr->next;
			tempptr->next = node;
		}
	}
	return (saveindex);
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->size == 0) {
		return NULL;
	}
	else {
		return q->front->value;
	}
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->front == NULL) {
		return NULL;
	}
	else {
		void* value = q->front->value;
		listnode_t* temp = q->front;
		q->front = q->front->next;
		free(temp);
		q->size--;
		return value;
	}
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	listnode_t *temp;
	temp = q->front;
	int i = 0;
	while(i < index && temp != NULL) {
		temp = temp->next;
		i++;
	}
	return temp->value;
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	if(q->size == 0) {
		return 0;
	}
	else {
		int entriesRemoved = 0;
		while(q->front->value == ptr) {
			listnode_t *remove = q->front;
			q->front = q->front->next;
			free(remove);
			entriesRemoved++;
			q->size--;
		}
		listnode_t *temp;
		for(temp = q->front; temp->next != NULL; temp = temp->next) {
			if(temp->next->value == ptr) {
				listnode_t *remove = temp->next;
				temp->next = temp->next->next;
				free(remove);
				entriesRemoved++;
				q->size--;
			}
		}
		return entriesRemoved;
	}
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(q->size == 0) {
		return NULL;
	}
	else {
		if(index == 0){
			void *value = q->front->value;
			listnode_t *remove = q->front;
			q->front = q->front->next;
			free(remove);
			q->size--;
			return value;
		}
		else {
			int i;
			listnode_t *temp = q->front;
			for(i = 0; i < index - 1 && temp->next != NULL; i++) {
				temp = temp->next;
			}
			if(temp->next != NULL) {
				void *value = temp->next->value;
				listnode_t *remove = temp->next;
				temp->next = temp->next->next;
				free(remove);
				q->size--;
				return value;
			}
			else {
				return NULL;
			}
		}
	}
	return NULL;
}

/**
  Returns the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	int i;
	for(i = 0; i < q->size; i++) {
		listnode_t *remove = q->front;
		q->front = q->front->next;
		free(remove);
	}
	q->size = 0;
}

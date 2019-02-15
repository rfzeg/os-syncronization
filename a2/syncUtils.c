#include "syncUtils.h"

void lock(pthread_mutex_t *mutex) {
    int returnValue = pthread_mutex_lock(mutex);
    if (returnValue != 0) {
        perror("Mutex lock failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}

void destroyMutex(pthread_mutex_t* mutex) {
    int returnValue = pthread_mutex_destroy(mutex);
    if (returnValue != 0) {
        perror("Mutex destruction failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}

void destroyConditionVariable(pthread_cond_t* cond) {
    int returnValue = pthread_cond_destroy(cond);
    if (returnValue != 0) {
        perror("Condition variable destruction failed."
               "@ " __FILE__ " : " LINE_STRING "\n");
    }
}

IntQueue_t *initIntQueue(){
	IntQueue_t *int_queue = malloc(sizeof(IntQueue_t));
	int_queue->head = NULL;
	int_queue->tail = NULL;
	int_queue->size = 0;
	return int_queue;
}

void enqueue(IntQueue_t *q, int i){
	IntQueueNode_t *newElement = malloc(sizeof(IntQueueNode_t));
	newElement->val = i;
	newElement->next = NULL;

	if (q->head == NULL) {
		q->head = newElement;
		q->tail = newElement;
	} else {
		q->tail->next = newElement;
	}
	q->size += 1;
}

int dequeue(IntQueue_t *q){
	int headVal = - 1;
	if (q->size > 0) {
		IntQueueNode_t *head = q->head;
		q->head = q->head->next;
		q->size -= 1;

		headVal = head->val;
		free(head);
	}
	return headVal;
}

void freeQueue(IntQueue_t *q){
	if (q != NULL){
		while (q->size != 0) {
			dequeue(q);
		}
	}
	free(q);
}

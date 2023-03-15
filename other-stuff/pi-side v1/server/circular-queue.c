#include "rpi.h"

typedef struct {
    void *data;
    int elementSize;
    int front;
    int rear;
    int count;
    int capacity;
} CircularQueue;

void initializeQueue(CircularQueue *queue, int elementSize, int capacity) {
    queue->elementSize = elementSize;
    queue->capacity = capacity;
    queue->data = kmalloc(elementSize * capacity);
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
}

unsigned isFull(CircularQueue *queue) {
    return queue->count == queue->capacity;
}

unsigned isEmpty(CircularQueue *queue) {
    return queue->count == 0;
}

int mod(int dividend, int divisor) {
    int quotient = 0;
    while (dividend >= divisor) {
        dividend -= divisor;
        quotient++;
    }
    return dividend;
}

unsigned enqueue(CircularQueue *queue, void *value) {
    if (isFull(queue)) {
        printk("Queue is full, cannot enqueue.\n");
        return 0;
    }
    memcpy((char *)queue->data + queue->rear * queue->elementSize, value, queue->elementSize);
    queue->rear = mod((queue->rear + 1), queue->capacity);
    queue->count++;
    return 1;
}

unsigned dequeue(CircularQueue *queue, void *value) {
    if (isEmpty(queue)) {
        printk("Queue is empty, cannot dequeue.\n");
        return 0;
    }
    memcpy(value, (char *)queue->data + queue->front * queue->elementSize, queue->elementSize);
    queue->front = mod((queue->front + 1), queue->capacity);
    queue->count--;
    return 1;
}

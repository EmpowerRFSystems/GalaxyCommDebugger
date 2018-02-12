#include "app.h"
#include "fifo.h"

void FifoInitialize(buffer16* buffer) {
    buffer->read = 0;
    buffer->write = 0;
    buffer->capacity = sizeof(buffer->buffer) / sizeof(buffer->buffer[0]);
    buffer->currentCount = 0;
}

unsigned char IsFifoFull(buffer16* buffer) {
    if (buffer->currentCount == buffer->capacity) {
        return TRUE;
    }
    return FALSE;
}

unsigned char IsFifoEmpty(buffer16* buffer) {
    if (buffer->currentCount == 0) {
        return TRUE;
    }
    return FALSE;
}

unsigned char FifoEnqueue(buffer16* buffer, unsigned int data) {
    if (IsFifoFull(buffer)) {
        return FALSE;
    }
    buffer->buffer[buffer->write] = data;
    buffer->write++;
    if (buffer->write == FIFO_SIZE)
        buffer->write = 0;
    buffer->currentCount++;
    return TRUE;
}

unsigned int FifoDequeue(buffer16* buffer) {
    unsigned int data;
    if (IsFifoEmpty(buffer)) {
        return FALSE;
    }
    data = buffer->buffer[buffer->read];
    buffer->read++;
    if (buffer->read == FIFO_SIZE)
        buffer->read = 0;
    buffer->currentCount--;
    return data;
}



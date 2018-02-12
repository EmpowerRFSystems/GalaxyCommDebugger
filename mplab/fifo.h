/* 
 * File:   circular_buffer.h
 * Author: dmiles813
 *
 * Created on December 16, 2017, 12:31 AM
 */

#ifndef CIRCULAR_BUFFER_H
#define	CIRCULAR_BUFFER_H

#ifdef	__cplusplus
extern "C" {
#endif

    
    
typedef struct {
    unsigned int buffer[FIFO_SIZE];
    unsigned char read;
    unsigned char write;
    unsigned char currentCount;
    unsigned char capacity;
} buffer16;

void FifoInitialize(buffer16 * buffer);
unsigned char IsFifoFull(buffer16 * buffer);
unsigned char IsFifoEmpty(buffer16 * buffer);
unsigned char FifoEnqueue(buffer16* buffer, unsigned int data);
unsigned int FifoDequeue(buffer16* buffer);

#ifdef	__cplusplus
}
#endif

#endif	/* CIRCULAR_BUFFER_H */


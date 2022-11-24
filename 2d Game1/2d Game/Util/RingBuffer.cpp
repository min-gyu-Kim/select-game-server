#include <stdlib.h>
#include <memory.h>
#include "RingBuffer.h"

RingBuffer::RingBuffer()
{
    _bufferSize = DEFAULT_SIZE;
    _buffer = (char*)malloc(_bufferSize);
    _frontOffset = 0;
    _rearOffset = 0;
}

RingBuffer::RingBuffer(int bufferSize)
{
    _bufferSize = bufferSize;
    _buffer = (char*)malloc(_bufferSize);
    _frontOffset = 0;
    _rearOffset = 0;
}

RingBuffer::~RingBuffer()
{
    free(_buffer);
}

int RingBuffer::GetUseSize() const
{
    if (this->_rearOffset >= this->_frontOffset)
    {
        return this->_rearOffset - this->_frontOffset;
    }
    else
    {
        return this->_bufferSize - this->_frontOffset + this->_rearOffset;
    }
}

int RingBuffer::GetFreeSize() const
{
    return this->_bufferSize - GetUseSize() - 1;
}

int RingBuffer::DirectEnqueueSize() const
{
    if (this->_rearOffset >= this->_frontOffset)
    {
        if (this->_frontOffset == 0)
        {
            return this->_bufferSize - this->_rearOffset - 1;
        }
        else
        {
            return this->_bufferSize - this->_rearOffset;
        }
    }
    else
    {
        return this->_frontOffset - this->_rearOffset - 1;
    }
}

int RingBuffer::DIrectDequeueSize() const
{
    if (this->_frontOffset >= this->_rearOffset)
    {
        return this->_bufferSize - this->_frontOffset;
    }
    else
    {
        return this->_rearOffset - this->_frontOffset;
    }
}

int RingBuffer::Enqueue(const char* buffer, int size)
{
    int freeSize = this->GetFreeSize();
    if (size >= freeSize)
    {
        size = freeSize;
    }

    int directSize = this->DirectEnqueueSize();
    if (size <= directSize)
    {
        memcpy(this->_buffer + this->_rearOffset, buffer, size);
        this->_rearOffset += size;
    }
    else
    {
        memcpy(this->_buffer + this->_rearOffset, buffer, directSize);
        memcpy(this->_buffer, buffer + directSize, size - directSize);
        this->_rearOffset = size - directSize;
    }

    return size;
}

int RingBuffer::Dequeue(char* outBuffer, int size)
{
    int readSize = this->GetUseSize();

    if (readSize > size)
    {
        readSize = size;
    }

    int directSize = this->DIrectDequeueSize();
    if (readSize <= directSize)
    {
        memcpy(outBuffer, this->_buffer + this->_frontOffset, readSize);
        this->_frontOffset += readSize;
    }
    else
    {
        memcpy(outBuffer, this->_buffer + this->_frontOffset, directSize);
        memcpy(outBuffer + directSize, this->_buffer, readSize - directSize);
        this->_frontOffset = readSize - directSize;
    }

    return readSize;
}

int RingBuffer::Peek(char* outBuffer, int size)
{
    int readSize = this->GetUseSize();

    if (readSize > size)
    {
        readSize = size;
    }

    int directSize = this->DIrectDequeueSize();
    if (readSize <= directSize)
    {
        memcpy(outBuffer, this->_buffer + this->_frontOffset, readSize);
    }
    else
    {
        memcpy(outBuffer, this->_buffer + this->_frontOffset, directSize);
        memcpy(outBuffer + directSize, this->_buffer, readSize - directSize);
    }

    return readSize;
}

void RingBuffer::MoveRear(int size)
{
    int moveOffset = this->_rearOffset + size;
    if (this->_bufferSize <= moveOffset)
    {
        moveOffset -= this->_bufferSize;
    }

    this->_rearOffset = moveOffset;
}

int RingBuffer::MoveFront(int size)
{
    int moveOffset = this->_frontOffset + size;
    if (this->_bufferSize <= moveOffset)
    {
        moveOffset -= this->_bufferSize;
    }

    this->_frontOffset = moveOffset;

    return 0;
}

void RingBuffer::ClearBuffer()
{
    this->_frontOffset = 0;
    this->_rearOffset = 0;
}

char* RingBuffer::GetFrontBufferPtr() const
{
    return this->_buffer + this->_frontOffset;
}

char* RingBuffer::GetRearBufferPtr() const
{
    return this->_buffer + this->_rearOffset;
}

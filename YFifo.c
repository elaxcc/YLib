#include "YFIFO.h"

uint32_t YFifo8Push(struct YFifo *fifo, uint8_t value)
{
	uint32_t next_position_for_tail_elm = fifo->tail_ptr_ + 1;
	if (next_position_for_tail_elm == fifo->size_)
	{
		next_position_for_tail_elm = 0;
	}
	if (next_position_for_tail_elm != fifo->head_ptr_)
	{
		fifo->buf_ptr_[fifo->tail_ptr_] = value;
		fifo->tail_ptr_ = next_position_for_tail_elm;
		return Y_FIFO8_NO_ERROR;
	}
	return Y_FIFO8_FULL_ERROR;
}

uint32_t YFifo8Pop(struct YFifo *fifo, uint8_t *value)
{
	uint32_t next_position_for_head_elm = fifo->head_ptr_ + 1;
	if (next_position_for_head_elm == fifo->size_)
	{
		next_position_for_head_elm = 0;
	}
	if (next_position_for_head_elm != fifo->tail_ptr_)
	{
		fifo->head_ptr_ = next_position_for_head_elm;
		*value = fifo->buf_ptr_[next_position_for_head_elm];
		return Y_FIFO8_NO_ERROR;
	}
	return Y_FIFO8_EMPTY_ERROR;
}

void YFifo8Flush(struct YFifo *fifo)
{
	fifo->head_ptr_ = 0;
	fifo->tail_ptr_ = 1;
}

uint32_t YFifo8IsFull(struct YFifo *fifo)
{
	uint32_t next_position_for_tail = fifo->tail_ptr_ + 1;
	if (next_position_for_tail == fifo->size_)
	{
		next_position_for_tail = 0;
	}
	if (next_position_for_tail == fifo->head_ptr_)
	{
		return 1;
	}
	return 0;
}

uint32_t YFifo8IsEmpty(struct YFifo *fifo)
{
	uint32_t next_position_for_head_elm = fifo->head_ptr_ + 1;
	if (next_position_for_head_elm == fifo->size_)
	{
		next_position_for_head_elm = 0;
	}
	if (next_position_for_head_elm == fifo->tail_ptr_)
	{
		return 1;
	}
	return 0;
}

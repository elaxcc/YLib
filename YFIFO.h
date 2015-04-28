#ifndef __YFIFO_H_
#define __YFIFO_H_

#include <stdint.h>

/*! 
 * \brief At firt use YFifo8Flush(YFifo *fifo) function for init FIFO pointers
 */

//! some errors
#define Y_FIFO8_NO_ERROR 0
#define Y_FIFO8_FULL_ERROR 1
#define Y_FIFO8_EMPTY_ERROR 2

/*!
 * \brief struct for store FIFO information
 * \member buf_ptr_ pointer to buffer that stores FIFO
 * \member head_ptr_ pointer to head of FIFO
 * \member tail_ptr_ pointer to tail of FIFO
 * \member size_ size of buffer that stores FIFO, real size of FIFO is (size_ - 2)
 */
struct YFifo
{
	uint8_t *buf_ptr_;
	uint32_t head_ptr_;
	uint32_t tail_ptr_;
	uint32_t size_;
};

/*!
 * \brief push element into FIFO
 * \param[in] pointer to fifo
 * \param[in] value of pushed element into FIFO
 * \return error if something going wrong
 */
uint32_t YFifo8Push(struct YFifo *fifo, uint8_t value);

/*!
 * \brief extract element from FIFO
 * \param[in] pointer to fifo
 * \param[out] value of extracted element from FIFO
 * \return error if something going wrong
 */
uint32_t YFifo8Pop(struct YFifo *fifo, uint8_t *value);

/*!
 * \brief flush FIFO
 * \param[in] pointer to fifo
 */
void YFifo8Flush(struct YFifo *fifo);

/*!
 * \brief allow to know that FIFO is full
 * \param[in] pointer to fifo
 * \return 1 if FIFO full else 0
 */
uint32_t YFifo8IsFull(struct YFifo *fifo);

/*!
 * \brief allow to know that FIFO is empty
 * \param[in] pointer to fifo
 * \return 1 if FIFO enpty else 0
 */
uint32_t YFifo8IsEmpty(struct YFifo *fifo);

#endif // __YFIFO_H_

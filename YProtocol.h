#ifndef __YPROTOCOL_H_
#define __YPROTOCOL_H_

#include "stm32f4xx_conf.h"

#include "YBool.h"

#include <stdint.h>

/*!
 * \brief Some definitions of status of parsing
 * \definition Y_PARSE_IS_OK - all is well
 * \definition Y_PARSE_ERROR_BC - wrong byte code in incoming packet
 * \definition Y_PARSE_ERROR_FC - wrong function code in incoming packet
 * \definition Y_PARSE_ERROR_CRC - wrong CRC in incoming packet
 * \definition Y_PARSE_ERROR_PP - error occurred then packet has been processed
 * \definition Y_PARSE_FIFO_FULL - parsing bytes FIFO is full
 * \definition Y_PARSE_FIFO_EMPTY - parsing bytes FIFO is empty
 * \definition Y_PARSE_OUT_FIFO_FULL - outcoming bytes FIFO is full
 * \definition Y_PARSE_OUT_FIFO_EMPTY - outcoming bytes FIFO is empty
 */
#define Y_PARSE_IS_OK 0
#define Y_PARSE_ERROR_BC -1
#define Y_PARSE_ERROR_FC -2
#define Y_PARSE_ERROR_CRC -3
#define Y_PARSE_ERROR_PP -4
#define Y_PARSE_FIFO_FULL -5
#define Y_PARSE_FIFO_EMPTY -6
#define Y_PARSE_OUT_FIFO_FULL -7
#define Y_PARSE_OUT_FIFO_EMPTY -8

/*!
 * \brief This function initializes protocol
 * \param[in] buffers_size - FIFOs buffers size
 * \param[in] read_byte_func_ptr - read bytes functor
 * \param[in] send_byte_func_ptr - send bytes functor
 * \param[in] process_func_ptr - proceess incoming packet functor
 * \param[in] enable_disable_transmit_interrupt_func_ptr - functor that can enable\disable interrupt for transmiting data,
 * \param[in] ticks - maximum ticks between receiving bytes
 * for example, when you use USART and insert data for transmition using YProtocolSendByte() or YProtocolSendPacket(), for begining
 * transmition TC interrupt must been enabled and after outcoming FIFO have been erased TC interrupt must been disabled
 */
void YProtocolInit(uint32_t buffers_size, uint8_t (*read_byte_func_ptr)(void), void (*send_byte_func_ptr)(uint8_t),
	int32_t (*process_func_ptr)(void), void (*enable_disable_transmit_interrupt_func_ptr)(YBOOL enabled));

/*!
 * \brief Enable timer for receiving packet
 * \param[in] ticks - maximum ticks between receiving bytes
 * \param[in] start_timer_func_ptr - start timer external function
 * \param[in] stop_timer_func_ptr - stop timer external function
 */
void YProtocolEnableTimer(unsigned ticks, void (*start_timer_func_ptr)(void), void (*stop_timer_func_ptr)(void));

/*!
 * \brief Disable timer for receiving packet
 */
void YProtocolDisableTimer(void);

/*!
 * \brief This function must be used in interrupt of timer
 */
void YProtocolTimerInterrupt(void);
	
/*!
 * \brief Function reinitializes parse variables and flush buffer for incoming data
 */
void YProtocolReinit(void);

/*!
 * \brief It is main function of protocol, use it in a thread or infinite loop
 * \retval status of parsing
 */
int32_t YProtocolThread(void);

/*!
 * \brief It is interrupt function of protocol, call it in interrupt of reciever/transmitter,
 * for example, in USART interrupt
 * \param[in] is_recieved - if interrupt uccured when byte have been recieved this param must be YTRUE
 * else YFALSE (for transmite)
 * \retval status of parsing
 */
int32_t YProtocolInterrupt(YBOOL is_recieved);

/*!
 * \brief This function inserts byte into FIFO that will have been transmitted
 * \paran[in] byte - byte for transmition
 */
void YProtocolSendByte(uint8_t byte);

/*!
 * \brief This function inserts packet into FIFO that will have been transmitted
 * \param[in] func_code - function code of packet
 * \param[in] data - data for transmition
 * \param[in] data_size - sze of data that will have been transmitted
 * \retval status of parsing
 */
int32_t YProtocolSendPacket(unsigned char func_code, unsigned char *data, uint32_t data_size);

/*!
 * \brief Function helps to know function code of recieved packet
 * \retval function code of recieved packet
 */
uint8_t YProtocolFunctionCode(void);

/*!
 * \brief Function helps to know data of recieved packet
 * \retval data of recieved packet
 */
uint8_t* YProtocolParsedData(void);

/*!
 * \brief Function helps to know size of data of recieved packet
 * \retval size of data of recieved packet
 */
uint16_t YProtocolParsedDataSize(void);

#endif /*__YPROTOCOL_H_*/

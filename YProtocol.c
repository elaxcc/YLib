#include "YProtocol.h"

#include "YFIFO.h"

#include <stdlib.h>
#include <string.h>

/*!
 * \brief Bits definitions of status variable of parsing
 * \definition byte counter flag
 * \definition byte counter flag, low part
 * \definition byte counter flag, high part
 * \definition function code flag
 * \definition got data flag
 * \definition got CRC16 low byte
 * \definition got CRC16 high byte
 * \definition packet is parsed
 */
#define PARSE_FLAG_BC 1
#define PARSE_FLAG_BC_L 2
#define PARSE_FLAG_BC_H 4
#define PARSE_FLAG_FC 8
#define PARSE_FLAG_GD 16
#define PARSE_FLAG_CRCL 32
#define PARSE_FALG_CRCH 64
#define PARSE_FLAG_IS_PARSED 128

/*!
 * \global out_fifo_ FIFO for transmitted data
 * \global in_fifo_ FIFO for recieved data
 */
struct YFifo out_fifo_;
struct YFifo in_fifo_;

/*!
 * \brief Parse variables
 * \global parse_flag_ - parse flag
 * \global parse_error_ - error byte
 * \global parse_bc_low_ - byte counter, low part
 * \global parse_bc_high_ - byte counter, high part
 * \global parse_bc_ - byte counter
 * \global parse_fc_ - function code
 * \global parse_crc_calc_ - calculated CRC16
 * \global parse_crc_income_ - incoming CRC16
 * \global parse_incoming_data_size_ - copy of the Byte Counter. Used for indicate end of data field
 * \global parse_incoming_data_ - buffer for incoming data
 * \global parse_ptr_ - pointer on the current byte in Data Buffer (parse_incoming_data_)
 */
uint8_t parse_flag_;
uint8_t parse_error_;
uint8_t parse_bc_low_;
uint8_t parse_bc_high_;
uint16_t parse_bc_;
uint8_t parse_fc_;
uint16_t parse_crc_calc_;
uint16_t parse_crc_income_;
uint16_t parse_incoming_data_size_;
uint8_t *parse_incoming_data_ = NULL;
uint16_t parse_ptr_;

/*!
 * \brief pointer for storing functors
 * \global packet_process_func_ptr_ - process packet functor
 * \global read_byte_func_ptr_ - recieve packet functor
 * \global send_byte_func_ptr_ - transmit packet functor
 * \global enable_disable_transmit_interrupt_func_ptr_ - enable/disable interrupt functor
 */
int32_t (*packet_process_func_ptr_)(void);
uint8_t (*read_byte_func_ptr_)(void);
void (*send_byte_func_ptr_)(uint8_t);
void (*enable_disable_transmit_interrupt_func_ptr_)(YBOOL enabled);

/*!
 * \brief Timer data
 * \global timer_state - state of the timer, 0 - disabled, 1 - enabled
 * \global use_timer_ - use timer for receiving packet
 * \global ticks_ - maximum ticks between receiving bytes
 * \global current_tick_ - current tick
 * \global start_timer_func_ptr_ - start timer external function
 * \global stop_timer_func_ptr_ - stop timer external function
 */
uint8_t timer_state_ = 0;
YBOOL use_timer_ = YFALSE;
unsigned ticks_ = 0;
unsigned current_tick_ = 0;
void (*start_timer_func_ptr_)(void) = 0;
void (*stop_timer_func_ptr_)(void) = 0;

void YProtocolStartTimer(void)
{
	timer_state_ = 1;
	current_tick_ = 0;
	
	start_timer_func_ptr_();
}

void YProtocolStopTimer(void)
{
	timer_state_ = 0;
	
	stop_timer_func_ptr_();
}

void YProtocolResetTimer(void)
{
	current_tick_ = 0;
}

void YProtocolTimerInterrupt(void)
{
	if (use_timer_ == YTRUE)
	{
		current_tick_++;
		
		if (current_tick_ >= ticks_)
		{
			YProtocolStopTimer();
			YProtocolReinit();
		}
	}
}

void YProtocolEnableTimer(unsigned ticks, void (*start_timer_func_ptr)(void), void (*stop_timer_func_ptr)(void))
{
	use_timer_ = YTRUE;
	ticks_ = ticks;
	
	start_timer_func_ptr_ = start_timer_func_ptr;
	stop_timer_func_ptr_ = stop_timer_func_ptr;
}

void YProtocolDisableTimer(void)
{
	if (use_timer_ == YTRUE)
	{
		stop_timer_func_ptr_();
		use_timer_ = YFALSE;
	}
}

void YProtocolReinit(void)
{
	// Reinitialization of the Parse variables
	//__disable_irq();

	parse_flag_ = 0;
	parse_error_ = 0;
	parse_bc_high_ = 0;
	parse_bc_low_ = 0;
	parse_bc_ = 0;
	parse_fc_ = 0;
	parse_crc_calc_ = 0xFFFF;
	parse_crc_income_ = 0;
	parse_incoming_data_size_ = 0;
	parse_ptr_ = 0;
	// If parse_incoming_data_ isn't NULL, free memory
	if(parse_incoming_data_ != NULL)
	{
		free(parse_incoming_data_);
		parse_incoming_data_ = NULL;
	}

	//__enable_irq();
}

void YProtocolInit(uint32_t buffers_size, uint8_t (*read_byte_func_ptr)(void), void (*send_byte_func_ptr)(uint8_t),
	int32_t (*process_func_ptr)(void), void (*enable_disable_transmit_interrupt_func_ptr)(YBOOL enabled))
{
	packet_process_func_ptr_ = process_func_ptr;
	read_byte_func_ptr_ = read_byte_func_ptr;
	send_byte_func_ptr_ = send_byte_func_ptr;
	enable_disable_transmit_interrupt_func_ptr_ = enable_disable_transmit_interrupt_func_ptr;
	
	YProtocolReinit();
	
	// FIFOs init
	in_fifo_.buf_ptr_ = (uint8_t*) malloc (buffers_size);
	in_fifo_.size_ = buffers_size;
	out_fifo_.buf_ptr_ = (uint8_t*) malloc (buffers_size);
	out_fifo_.size_ = buffers_size;
	YFifo8Flush(&out_fifo_);
	YFifo8Flush(&in_fifo_);
}

uint16_t YProtocolCalcCRC16(uint8_t* Arr, uint16_t Size, uint16_t CRC16)
{
	int i, j;
	for(i = 0; i < Size; i++)
	{
		CRC16 = CRC16 ^ Arr[i];
		
		for(j = 0; j < 8; j++)
		{
			if(CRC16 & 0x0001) CRC16 = (CRC16 >> 1) ^ 0xA001;
			else CRC16 = CRC16 >> 1;
		}
	}
	
	return CRC16;
}

//! \fixme create timeout
int32_t YProtocolParse(uint8_t byte)
{
	// Did we get low part of Byte Counter?
	if (!(parse_flag_ & PARSE_FLAG_BC_L))
	{
		// Didn't get low part Byte Counter, it is beginning of the packet
		
		// Save Byte Counter
		parse_bc_low_ = byte;
		// Set PARSE_FLAG_BC flag
		parse_flag_ = parse_flag_ | PARSE_FLAG_BC_L;
	}
	else 
	{
		// Got low part of Byte counter
		
		// Did we get high part of byte counter?
		if (!(parse_flag_ & PARSE_FLAG_BC_H))
		{	
			// Didn't get high part Byte Counter, it is beginning of the packet
		
			// Save Byte Counter
			parse_bc_high_ = byte;
			parse_bc_ = (uint16_t) parse_bc_high_;
			parse_bc_ = (parse_bc_ << 8) | ((uint16_t) parse_bc_low_);
			
			if (parse_bc_ == 0x00)
			{
				YProtocolReinit();
				if (use_timer_ == YTRUE)
				{
					YProtocolStopTimer();
				}
				return Y_PARSE_ERROR_BC;
			}				
			
			// Set PARSE_FLAG_BC flag
			parse_flag_ = parse_flag_ | PARSE_FLAG_BC_H;
			parse_flag_ = parse_flag_ | PARSE_FLAG_BC;
			// Save Copy Byte Counter
			parse_incoming_data_size_ = parse_bc_ - 3;
		}
		else
		{
			// Got high part of Byte Counter
			
			// Got Byte Counter
		
			// For next bytes except CRCL and CRCH we must calculate CRC16
			if ((parse_flag_ & PARSE_FLAG_BC) & (!(parse_flag_ & PARSE_FLAG_GD)))
			{
				parse_crc_calc_ = YProtocolCalcCRC16(&byte, 1, parse_crc_calc_);
			}
		
			// Did we get Function Code?
			if (!(parse_flag_ & PARSE_FLAG_FC))
			{
				// Didn't get Function Code
			
				// Save Function Code
				parse_fc_ = byte;
				// Set PARSE_FLAG_FC flag
				parse_flag_ = parse_flag_ | PARSE_FLAG_FC;
			
				if (parse_bc_ > 3)
				{
					// Allocate memory for data
					parse_incoming_data_ = (uint8_t*) malloc((parse_bc_ - 3));
				}
				else
				{
					// We don't have data
					parse_flag_ = parse_flag_ | PARSE_FLAG_GD;
				}
			}
			else
			{
				// Got Function Code
			
				// Did we get all data?
				if (!(parse_flag_ & PARSE_FLAG_GD))
				{
					// Didn't get all data
				
					// Save data
					parse_incoming_data_[parse_ptr_] = byte;
					++parse_ptr_;
				
					// Did we get last byte? If we get last byte then parse_ptr_ equal to parse_incoming_data_size___
					if (parse_ptr_ == parse_incoming_data_size_)
					{
						// We got last byte
					
						// Set PARSE_FLAG_GD flag
						parse_flag_ = parse_flag_ | PARSE_FLAG_GD;
					}
				}
				else
				{
					// Got all data
				
					// Did we get low part of the CRC16?
					if (!(parse_flag_ & PARSE_FLAG_CRCL))
					{
						// Didn't get low part of the CRC16
					
						// Save low part of the CRC16
						parse_crc_income_ = (uint16_t) byte;
						// Set PARSE_FLAG_CRCL flag
						parse_flag_ = parse_flag_ | PARSE_FLAG_CRCL;
					}
					else
					{
						// Got low part of the CRC16
					
						// Did we get high part of the CRC16
						if (!(parse_flag_ & PARSE_FALG_CRCH))
						{
							// Save high part of the CRC16
							parse_crc_income_ = (parse_crc_income_) | (((uint16_t) byte)<<8);

							// Compare incoming CRC16 with calculated CRC16
							if (parse_crc_income_ == parse_crc_calc_)
							{
								// CRC16 is right, packet is parsed
								
								int32_t err;
								
								// Set flag PARSE_FLAG_IS_PARSED
								parse_flag_ = parse_flag_ | PARSE_FLAG_IS_PARSED;
								
								// Packet was parsed
								err = packet_process_func_ptr_();
								YProtocolReinit();
								if (use_timer_ == YTRUE)
								{
									YProtocolStopTimer();
								}
								return err;
							}
							else 
							{
								// Wrong CRC6, reinitialization of the Parse variables
							
								YProtocolReinit();
								if (use_timer_ == YTRUE)
								{
									YProtocolStopTimer();
								}
								return Y_PARSE_ERROR_CRC;
							}
						}
					}
				}
			}
		}
	}
	
	return Y_PARSE_IS_OK;
}

void YProtocolSendByte(uint8_t byte)
{
	YFifo8Push(&out_fifo_, byte);
	enable_disable_transmit_interrupt_func_ptr_(YTRUE);
}

int32_t YProtocolSendPacket(uint8_t func_code, uint8_t *data, uint32_t data_size)
{
	int32_t i, err;
	uint8_t byte;
	uint16_t crc = 0xFFFF;

	//__disable_irq();
	
	// Byte counter
	byte = (uint8_t) (data_size + 3); // low part of Byte counter
	err = YFifo8Push(&out_fifo_, byte);
	byte = (uint8_t) ((data_size + 3) >> 8); // High part of the Byte counter
	err = YFifo8Push(&out_fifo_, byte);
	
	// Function code
	YFifo8Push(&out_fifo_, func_code);
	crc = YProtocolCalcCRC16(&func_code, 1, crc);
	
	// Data
	for (i = 0; i < data_size; ++i)
	{
		err = YFifo8Push(&out_fifo_, data[i]);
		crc = YProtocolCalcCRC16(&data[i], 1, crc);
	}
	
	// CRC
	byte = (uint8_t) crc; // Low part of the CRC
	err = YFifo8Push(&out_fifo_, byte);
	byte = (uint8_t) (crc >> 8); // High part of the CRC
	err = YFifo8Push(&out_fifo_, byte);
	
	enable_disable_transmit_interrupt_func_ptr_(YTRUE);
	
	//__enable_irq();
	
	return err;
}

int32_t YProtocolThread(void)
{
	uint8_t buf;
	int err;
	
	// Get byte from InBuffer
	__disable_irq();
	err = YFifo8Pop(&in_fifo_, &buf);
	__enable_irq();
	
	if (err == Y_FIFO8_NO_ERROR)
	{
		return YProtocolParse(buf);
	}
	return Y_PARSE_FIFO_EMPTY;
}

int32_t YProtocolInterrupt(YBOOL is_recieved)
{
	int32_t err;
	uint8_t byte;
	
	if (is_recieved)
	{
		if (use_timer_ == YTRUE)
		{
			// if timer started we are receving packet - reset timer
			// else if timer not started we got new packet
			if (timer_state_ == 0) // new packet
			{
				YProtocolStartTimer();
			}
			else
			{
				YProtocolResetTimer();
			}
		}
		
		// process incoming byte
		byte = read_byte_func_ptr_();
		err = YFifo8Push(&in_fifo_, byte);
		if(err == Y_FIFO8_FULL_ERROR)
		{
			return Y_PARSE_FIFO_FULL;
		}
	}
	else
	{
		// process outcoming byte
		err = YFifo8Pop(&out_fifo_, &byte);
		if(err == Y_FIFO8_EMPTY_ERROR)
		{
			enable_disable_transmit_interrupt_func_ptr_(YFALSE);
			return Y_PARSE_OUT_FIFO_EMPTY;
		}
		send_byte_func_ptr_(byte);
	}
	return Y_PARSE_IS_OK;
}

uint8_t YProtocolFunctionCode(void)
{
	return parse_fc_;
}

uint8_t* YProtocolParsedData(void)
{
	return parse_incoming_data_;
}

uint16_t YProtocolParsedDataSize(void)
{
	return parse_incoming_data_size_;
}

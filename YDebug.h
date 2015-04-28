#ifndef __YFIFO_H_
#define __YFIFO_H_

#include "YBool.h"

#include <stdint.h>

//#define YDEBUG

/*!
 * \brief it is private members for debug system? don't use it manualy
 * \member YDebugSendFunctionPtr_ - pointer on transmitted data function
 * - \param[in] uint8_t value byte for transmition
 * \member YDebugCompleteFunctionPtr_ - pointer on checking transmission complete function
  - \retval must return non zero value if transmition is completed
 * \member YDebugIterator_ - private iterator used in cycles
 * \member YDebugByte_ - private temporary buffer
 */
#ifdef YDEBUG
	void (*YDebugSendFunctionPtr_)(uint8_t) = 0;
	YBOOL (*YDebugCompleteFunctionPtr_)(void) = 0;
	unsigned YDebugIterator_ = 0;
	unsigned char YDebugByte_ = 0;
#endif // YDEBUG

/*!
 * \brief Function of initialization of debug system. Use this function
 * before using debug system
 * \param[in] _send_func - address of transmitted byte function
 * \param[in] _complete_func - address of checking transmission completed function
 */
#ifdef YDEBUG
	#define YDebugInit(_send_func, _complete_func) \
		YDebugSendFunctionPtr_ = _send_func; \
		YDebugCompleteFunctionPtr_ = _complete_func;
#else
	#define YDebugInit(_send_func, _complete_func)
#endif // YDEBUG

/*!
 * \brief Transmitted message function
 * before using debug system
 * \param[in] _message - transmitted message
 * \param[in] _message_size - transmitted message size
 */
#ifdef YDEBUG
	#define YDebugSendMessage(_message, _message_size) \
		YDebugIterator_ = 0; \
		while (_message_size != YDebugIterator_) \
		{ \
			YDebugSendFunctionPtr_(_message[YDebugIterator_]); \
			while (YDebugCompleteFunctionPtr_() == YFALSE) {} \
			YDebugIterator_++; \
		}
#else
	#define YDebugSendMessage(_message, _message_size)
#endif // YDEBUG

/*!
 * \brief Function transmit new line character
 */
#ifdef YDEBUG
	#define YDebugSendEndl() \
		YDebugSendFunctionPtr_('\r'); \
		while (YDebugCompleteFunctionPtr_() == YFALSE) {} \
		YDebugSendFunctionPtr_('\n'); \
		while (YDebugCompleteFunctionPtr_() == YFALSE) {}
#else
	#define YDebugSendEndl(_byte)
#endif // YDEBUG

/*!
 * \brief Function transmit byte
 * \param[in] _byte - transmited byte
 */
#ifdef YDEBUG
	#define YDebugSendByte(_byte) \
		YDebugSendFunctionPtr_(_byte); \
		while (YDebugCompleteFunctionPtr_() == YFALSE) {}
#else
	#define YDebugSendByte(_byte)
#endif // YDEBUG

/*!
 * \brief Function transmit word
 * \param[in] _word - transmited word
 */
#ifdef YDEBUG
	#define YDebugSendWord(_word) \
		for(YDebugIterator_ = 0; YDebugIterator_ < 2; ++YDebugIterator_) \
		{ \
			YDebugByte_ = (unsigned char) (_word >> (8 * YDebugIterator_)); \
			YDebugSendFunctionPtr_(YDebugByte_); \
			while (YDebugCompleteFunctionPtr_() == YFALSE) {} \
		}
#else
	#define YDebugSendWord(_word)
#endif // YDEBUG

/*!
 * \brief Function transmit double word
 * \param[in] _sword - transmited double word
 */
#ifdef YDEBUG
	#define YDebugSendDWord(_dword) \
		for(YDebugIterator_ = 0; YDebugIterator_ < 4; ++YDebugIterator_) \
		{ \
			YDebugByte_ = (unsigned char) (_dword >> (8 * YDebugIterator_)); \
			YDebugSendFunctionPtr_(YDebugByte_); \
			while (YDebugCompleteFunctionPtr_() == YFALSE) {} \
		}
#else
	#define YDebugSendDWord(_word)
#endif // YDEBUG

/*!
 * \brief Function transmit message if condition is false and
 * then continue program
 * \param[in] _condition - condition
 * \param[in] _message - transmitted message
 * \param[in] _message_size - transmitted message size
 */
#ifdef YDEBUG
	#define YASSERT_CONTINUE(_condition, _message, _message_size) \
		if (_condition == YFALSE) \
		{ \
			YDebugSendMessage(_message, _message_size); \
		}
#else
	#define YASSERT_CONTINUE(_value, _message, _message_size)
#endif // YDEBUG

/*!
 * \brief Function transmit message if condition is false and
 * then stop program execution
 * \param[in] _condition - condition
 * \param[in] _message - transmitted message
 * \param[in] _message_size - transmitted message size
 */
#ifdef YDEBUG
	#define YASSERT_STOP(_condition, _message, _message_size) \
		if (_condition == YFALSE) \
		{ \
			YDebugSendMessage(_message, _message_size); \
			while(YTRUE) {} \
		}
#else
	#define YASSERT_CONTINUE(_value, _message, _message_size)
#endif // YDEBUG

#endif // __YFIFO_H_

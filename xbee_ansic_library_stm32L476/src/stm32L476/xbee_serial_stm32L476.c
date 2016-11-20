/*
 * Copyright (c) 2010-2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
/**
	@addtogroup hal_hcs08
	@{
	@file xbee_serial_hcs08.c
	Serial Interface for Programmable XBee Module (Freescale HCS08 Platform).
*/

// NOTE: Documentation for these functions can be found in xbee/serial.h.

#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include "xbee/platform.h"
#include "xbee/serial.h"
#include "xbee/cbuf.h"

#include "stm32l4xx_hal.h"

bool_t xbee_ser_invalid( xbee_serial_t *serial)
{
	if(serial == huart5)
	{
	    return 0;
	}
	
	return 1;
}


const char *xbee_ser_portname( xbee_serial_t *serial)
{
	if (serial == huart5)
	{
			return "UART5";
	}

	return "(invalid)";
}


int xbee_ser_write( xbee_serial_t *serial, const void FAR *buffer,
	int length)
{
	uint8_t ret_val = 1;
	
	XBEE_SER_CHECK( serial);

	if (length < 0)
	{
		return -EINVAL;
	}

	if(!(HAL_UART_Transmit(serial, buffer, length, 1000)));
	{
	    return length;
	}
	
	return  -EINVAL;
}

// macro checks for flow-control enabled and RTS deasserted
#define XBEE_SER_CHECK_RTS(s)	\
	(((_flags >> (s->port - 1)) & (_FLAG_FLOW(1) | _FLAG_RTS(1))) \
																			== _FLAG_FLOW(1))
int xbee_ser_read( xbee_serial_t *serial, void FAR *buffer, int bufsize)
{
	int ret_val;

	XBEE_SER_CHECK( serial);


	if(!(READ_BIT(huart2.Instance->CR1, USART_CR1_RXNEIE)))
	{
	    return 0;
	}
	
	HAL_UART_Receive(serial, buffer, bufsize, 200)
	
	if (bufsize < 0)
	{
		return -EINVAL;
	}

	retval = xbee_cbuf_get( serial->*pRxBuffPtr, buffer,
		(bufsize > 255) ? 255 : (uint8_t) bufsize);

	// if there's at least x characters in the buffer (because we read x+
	// or a buffer check shows x+) then reassert RTS
	if (XBEE_SER_CHECK_RTS( serial)
		&& (retval >= RTS_ASSERT_BYTES
			|| xbee_cbuf_free( serial->rxbuf) >= RTS_ASSERT_BYTES))
	{
		xbee_ser_set_rts( serial, TRUE);
	}

	return retval;
}


int xbee_ser_putchar( xbee_serial_t *serial, uint8_t ch)
{
	int retval;

	// the call to xbee_ser_write() does XBEE_SER_CHECK() for us
	retval = xbee_ser_write( serial, &ch, 1);
	if (retval == 1)
	{
		return 0;
	}
	else if (retval == 0)
	{
		return -ENOSPC;
	}
	else
	{
		return retval;
	}
}


int xbee_ser_getchar( xbee_serial_t *serial)
{
	int retval;

	XBEE_SER_CHECK( serial);
	retval = xbee_cbuf_getch( serial->*pRxBuffPtr);

	if (retval < 0)
	{
		return -ENODATA;
	}

	// reassert RTS if not asserted and RTS_ASSERT_BYTES or more free in cbuf
	if (XBEE_SER_CHECK_RTS( serial)
		&& xbee_cbuf_free( serial->rxbuf) >= RTS_ASSERT_BYTES)
	{
		xbee_ser_set_rts( serial, TRUE);
	}

	return retval;
}


// Since we're using blocking transmit, there isn't a transmit buffer.
// Therefore, have xbee_ser_tx_free() and xbee_ser_tx_used() imply that
// we have an empty buffer that can hold an unlimited amount of data.

int xbee_ser_tx_free( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	return INT_MAX;
}
int xbee_ser_tx_used( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	return 0;
}


int xbee_ser_tx_flush( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	return 0;
}


int xbee_ser_rx_free( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	return xbee_cbuf_free( serial->rxbuf);
}


int xbee_ser_rx_used( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	return xbee_cbuf_used( serial->rxbuf);
}


int xbee_ser_rx_flush( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);
	xbee_cbuf_flush( serial->rxbuf);
	return 0;
}


int xbee_ser_baudrate( xbee_serial_t *serial, uint32_t baudrate)
{
	word baud_value;

	XBEE_SER_CHECK( serial);

	// Avoid floating point calculation for baud rate divider, use compile-time
	// calculations.
	// HCS08 32-bit compares require lots of code.  Cheat and just use
	// lower 16 bits of baud rate.  Saves 33 code bytes.
	switch ((uint16_t) baudrate)
	{
		case 9600:
			baud_value = UART_BAUD_9600;
			break;
		case 19200:
			baud_value = UART_BAUD_19200;
			break;
		case 38400:
			baud_value = UART_BAUD_38400;
			break;
		case (115200 & 0xFFFF):
			baud_value = UART_BAUD_115200;
			break;
		default:
			return -EIO;
	}

	switch (serial->port)
	{
		case SERIAL_PORT_SCI1:
			SCI1BD = baud_value;
			break;

		case SERIAL_PORT_SCI2:
			SCI2BD = baud_value;
			break;

		default:
			return -EINVAL;
	}

	serial->baudrate = baudrate;

	return 0;
}

// Use same configuration for both serial ports
#define SCIxC1_DEFAULT LOOPBACK_OFF | LOOPBACK_TWO_WIRE | UART_OFF_DURING_WAIT \
				| CHARACTER_8BIT | WAKEUP_IDLE_LINE | IDLE_COUNT_AFTER_START_BIT \
				| PARITY_OFF | PARITY_EVEN
#define SCIxC3_DEFAULT TX_DATA_NORMAL | RX_PARITY_ERROR_INTERRUPT_DISABLE \
				| RX_FRAMING_ERROR_INTERRUPT_DISABLE \
				| RX_NOISE_ERROR_INTERRUPT_DISABLE \
				| RX_OVERRUN_INTERRUPT_DISABLE | ONE_WIRE_RX_MODE
#define SCIxS2_DEFAULT RX_BREAK_DETECT_10_BITS | TX_BREAK_10_BITS \
				| RX_IDLE_DETECT_OFF | RX_DATA_NORMAL
#define SCIxC2_DEFAULT RX_WAKEUP_CONTROL_OFF | RX_ENABLE | TX_ENABLE \
				| RX_INTERRUPT_ENABLE | IDLE_INTERRUPT_DISABLE \
				| TX_COMPLETE_INTERRUPT_DISABLE \
				| TX_BUFFER_READY_INTERRUPT_DISABLE

int xbee_ser_open( xbee_serial_t *serial, uint32_t baudrate)
{
	XBEE_SER_CHECK( serial);

	switch (serial->port)
	{
		case SERIAL_PORT_SCI1:
			SCI1C1 = SCIxC1_DEFAULT;
			SCI1C3 = SCIxC3_DEFAULT;
			SCI1S2 = SCIxS2_DEFAULT;
			SCI1C2 = SCIxC2_DEFAULT;
			break;

		case SERIAL_PORT_SCI2:
			SCI2C1 = SCIxC1_DEFAULT;
			SCI2C3 = SCIxC3_DEFAULT;
			SCI2S2 = SCIxS2_DEFAULT;
			SCI2C2 = SCIxC2_DEFAULT;
			break;

		default:
			return -EINVAL;
	}

	return xbee_ser_baudrate( serial, baudrate);
}


int xbee_ser_close( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);

	return 0;
}


int xbee_ser_break( xbee_serial_t *serial, bool_t enabled)
{
	XBEE_SER_CHECK( serial);

	return 0;
}


int xbee_ser_flowcontrol( xbee_serial_t *serial, bool_t enabled)
{
	XBEE_SER_CHECK( serial);

	if (enabled)
	{
		_flags |= _FLAG_FLOW( serial->port);
		// update RTS based on free space in receive buffer
		xbee_ser_set_rts( serial,
			xbee_cbuf_free( serial->rxbuf) >= RTS_ASSERT_BYTES);
	}
	else
	{
		_flags &= ~_FLAG_FLOW( serial->port);
		xbee_ser_set_rts( serial, TRUE);						// leave RTS asserted
	}
	return 0;
}

int xbee_ser_set_rts( xbee_serial_t *serial, bool_t asserted)
{
	XBEE_SER_CHECK( serial);

	switch (serial->port)
	{
		case SERIAL_PORT_SCI1:			// Host (debug)
			PTCDD_PTCDD0 = 1;								// set pin as output
			PTCD_PTCD0 = ! asserted;					// IO_DIO7_CTS_HOST
			break;

		case SERIAL_PORT_SCI2:			// EM250 (radio)
			PTCDD_PTCDD1 = 1;								// set pin as output
			PTCD_PTCD1 = ! asserted;					// IO_RTS_RADIO
			break;

		default:
			return -EINVAL;
	}

	if (asserted)
	{
		_flags |= _FLAG_RTS( serial->port);
	}
	else
	{
		_flags &= ~_FLAG_RTS( serial->port);
	}

	return 0;
}


int xbee_ser_get_cts( xbee_serial_t *serial)
{
	XBEE_SER_CHECK( serial);

	switch (serial->port)
	{
		case SERIAL_PORT_SCI1:			// Host (debug)
			return ! PTDD_PTDD7;			// IO_DIO6_RTS_HOST
			break;

		case SERIAL_PORT_SCI2:			// EM250 (radio)
			return ! PTDD_PTDD6;			// IO_CTS_RADIO
			break;

		default:
			return -EINVAL;
	}
}

// deassert RTS if buffer is too full (and not already deasserted)
#define _ISR_RTS(port, pin)		\
	if ((_flags & (_FLAG_FLOW_ ## port | _FLAG_RTS_ ## port)) == 				\
						(_FLAG_FLOW_ ## port | _FLAG_RTS_ ## port)					\
		&& xbee_cbuf_free( port ## _SERIAL_PORT.rxbuf) < RTS_ASSERT_BYTES)	\
	{ _flags &= ~_FLAG_RTS_ ## port; pin = 1; }

#ifdef XBEE_CW6
/**
	ISR for receiving data on the SCI1 serial port, which is connected to the
	"host" on the Programmable XBee.  It pushes characters into the circular
	buffer tied to HOST_SERIAL_PORT.

	Make sure the .prm file for the project contains the following line if
	you want to have interrupt driven serial receive:

	@code VECTOR ADDRESS 0x0000F1E0  vSci1Rx
	@endcode
*/
#pragma TRAP_PROC
void vSci1Rx( void)
{
	(void) SCI1S1;								// Clear ISR source
	xbee_cbuf_putch( HOST_SERIAL_PORT.rxbuf, SCI1D);
	// deassert RTS if necessary
	_ISR_RTS( HOST, PTCD_PTCD0);
}
#endif
/**
	@brief	This is the ISR for receiving data on the SCI2 serial
    port, which is connected to the Ember radio on the Programmable XBee.

	Make sure the .prm file for the project contains the following line:
	@code VECTOR ADDRESS 0x0000F1D2  vSci2Rx
	@endcode
*/
#pragma TRAP_PROC
#ifdef XBEE_CW6
void vSci2Rx(void)
#elif defined(XBEE_CW10)
void radio_uart_isr( void)
#else
#error "platform must define XBEE_CW6 or XBEE_CW10"
#endif
{
	(void) SCI2S1;								// Clear ISR source
	xbee_cbuf_putch( EMBER_SERIAL_PORT.rxbuf, SCI2D);
	// deassert RTS if necessary
	_ISR_RTS( EMBER, PTCD_PTCD1);
}

//@}

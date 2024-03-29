/****************************************************************************
*
*   Copyright (c) 2006 Dave Hylands     <dhylands@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
*   Alternatively, this software may be distributed under the terms of BSD
*   license.
*
*   See README and COPYING for more details.
*
****************************************************************************/
/**
*
*   @file   bioloid.c
*
*   @brief  Program which interfaces between the gumstix and the bioloid
*           AX-12 servos.
*
*           The AX-12 servos use a half-duplex protocol. We use UART-0
*           to connect to the gumstix, and UART-1 to connect to the servos.
*
*           To support the half duplex, we assume that the UART-1 Rx and Tx
*           are wired together. 
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "CBUF.h"

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

#define UART0_BAUD_RATE     115200
#define UART1_BAUD_RATE     1000000

#define	START_OF_PACKET		0xFF	// Two in a row is the start of a packet

#define UART_DATA_BIT_8  (( 1 << UCSZ1 ) | ( 1 << UCSZ0 ))
#define UART_PARITY_NONE (( 0 << UPM1 )  | ( 0 << UPM0 ))
#define UART_STOP_BIT_1  ( 0 << USBS )

#define UBRR0_INIT   (( CFG_CPU_CLOCK / 8 / UART0_BAUD_RATE ) - 1 )
#define UBRR1_INIT   (( CFG_CPU_CLOCK / 8 / UART1_BAUD_RATE ) - 1 )

#define UCSR0A_INIT  ( 1 << U2X0 )
#define UCSR0B_INIT  (( 1 << RXCIE ) | ( 1 << RXEN ) | ( 1 << TXEN ))
#define UCSR0C_INIT  ( UART_DATA_BIT_8 | UART_PARITY_NONE | UART_STOP_BIT_1 )

#define UCSR1A_INIT  ( 1 << U2X1 )
#define UCSR1B_INIT  (( 1 << RXCIE ) | ( 1 << RXEN ))
#define UCSR1C_INIT  ( UART_DATA_BIT_8 | UART_PARITY_NONE | UART_STOP_BIT_1 )

//--------------------------------------------------------------------------
// LED Constants

#if !defined( ROBOSTIX )
#  error ROBOSTIX should be defined to be 0 or 1
#endif

#if ROBOSTIX

#define RED_LED_PIN     4
#define RED_LED_MASK    ( 1 << RED_LED_PIN )
#define RED_LED_DDR     DDRG
#define RED_LED_PORT    PORTG

#define BLUE_LED_PIN    3
#define BLUE_LED_MASK   ( 1 << BLUE_LED_PIN )
#define BLUE_LED_DDR    DDRG
#define BLUE_LED_PORT   PORTG

#define YELLOW_LED_PIN  4
#define YELLOW_LED_MASK ( 1 << YELLOW_LED_PIN )
#define YELLOW_LED_DDR  DDRB
#define YELLOW_LED_PORT PORTB

#else

#define RED_LED_PIN     2
#define RED_LED_MASK    ( 1 << RED_LED_PIN )
#define RED_LED_DDR     DDRC
#define RED_LED_PORT    PORTC

#define BLUE_LED_PIN    1
#define BLUE_LED_MASK   ( 1 << BLUE_LED_PIN )
#define BLUE_LED_DDR    DDRC
#define BLUE_LED_PORT   PORTC

#define YELLOW_LED_PIN  0
#define YELLOW_LED_MASK ( 1 << YELLOW_LED_PIN )
#define YELLOW_LED_DDR  DDRC
#define YELLOW_LED_PORT PORTC

#endif

//--------------------------------------------------------------------------
// Some convenience macros to turn the LEDs on/off.
//
//  Usage:  LED_ON( BLUE );
//
// to turn on the blue LED.
//
// Note: Setting the pin to 0 turns the LED on.

#define LED_ON( color )     do { color ## _LED_PORT &= ~color ## _LED_MASK; } while (0)
#define LED_OFF( color )    do { color ## _LED_PORT |= color ## _LED_MASK; } while (0)
#define LED_TOGGLE( color ) do { color ## _LED_PORT ^= color ## _LED_MASK; } while (0)

/**
 *  Buffer_t is a buffer structure which is compatible with the CBUF.h
 *  circular buffer macros.
 */

#define BUFFER_SIZE 128 // Must be a power of 2 which is <= 128                         

typedef struct
{
    uint8_t     m_getIdx;
    uint8_t     m_putIdx;
    uint8_t     m_entry[ BUFFER_SIZE ];

} Buffer_t;

/* ---- Private Variables ------------------------------------------------ */

static  volatile    uint8_t     gTxActivity;
static  volatile    uint8_t     gRxActivity;

#define	STATE_OFF		0
#define	STATE_FIRST_FF	1
#define	STATE_ON		2

static	uint8_t		gState = STATE_OFF;

static  Buffer_t    gToServoBuf;
static  Buffer_t    gFromServoBuf;

#define gToServoBuf_SIZE    sizeof( gToServoBuf.m_entry )
#define gFromServoBuf_SIZE  sizeof( gFromServoBuf.m_entry )

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

//***************************************************************************
/**
*   Interrupt handler for characters from the gumstix.
*/

ISR( USART0_RX_vect )
{
    uint8_t ch = UDR0;   // Read the character from the UART

	switch ( gState )
	{
		case STATE_OFF:
		{
			if ( ch == START_OF_PACKET )
			{
				gState = STATE_FIRST_FF;
			}
			break;
		}

		case STATE_FIRST_FF:
		{
			if ( ch != START_OF_PACKET )
			{
				gState = STATE_OFF;
				break;
			}
			gState = STATE_ON;

			// Push the first 0xFF that we dropped earlier onto the queue
			// so that it gets sent out.

			CBUF_Push( gToServoBuf, ch );

			// Fall through into STATE_ON (causes the second 0xFF to get sent)
		}

		case STATE_ON:
		{
			if ( !CBUF_IsFull( gToServoBuf ))
			{
				CBUF_Push( gToServoBuf, ch );

				// We want to write a character, so we need to enable the transmitter
				// and disable the receiver.

				UCSR1B &= ~( 1 << RXEN );
				UCSR1B |= (( 1 << TXEN ) | ( 1 << UDRIE ));
			}
			break;
		}
	}

} // USART0_RX_vect

//***************************************************************************
/**
*   Interrupt handler for characters from the servo.
*/

ISR( USART1_RX_vect )
{
    uint8_t ch = UDR1;   // Read the character from the UART

	if ( !CBUF_IsFull( gFromServoBuf ))
	{
		CBUF_Push( gFromServoBuf, ch );

		// Enable the transmit interrupt for the gumstix side now that 
		// there's a character in the buffer.

		UCSR0B |= ( 1 << UDRIE );
	}

	gRxActivity = 1;

} // USART1_RX_vect

//***************************************************************************
/**
*   Interrupt handler for Uart Data Register Empty (Space available in
*   the uart to the gumstix)
*/

ISR( USART0_UDRE_vect )
{
    if ( CBUF_IsEmpty( gFromServoBuf ))
    {
        // Nothing left to transmit, disable the transmit interrupt

        UCSR0B &= ~( 1 << UDRIE );
    }
    else
    {
        // Otherwise, write the next character from the TX Buffer

        UDR0 = CBUF_Pop( gFromServoBuf );
    }

}  // USART0_UDRE_vect

//***************************************************************************
/**
*   Interrupt handler for Uart Data Register Empty (Space available in
*   the uart to the servo).
*/

ISR( USART1_UDRE_vect )
{
    if ( CBUF_IsEmpty( gToServoBuf ))
    {
        // Nothing left to transmit, disable the transmit interrupt.
		// The transmitter will be disabled when the Tx Complete interrupt 
		// occurs.

        UCSR1B &= ~( 1 << UDRIE );
    }
    else
    {
        // Write the next character from the TX Buffer

        UDR1 = CBUF_Pop( gToServoBuf );

		// Enable the Tx Complete interrupt. This is used to turn off the
		// transmitter and enable the receiver.

		UCSR1B |= ( 1 << TXCIE );

		gTxActivity = 1;
    }

}  // USART1_UDRE_vect

//***************************************************************************
/**
*   Interrupt handler for when the data has completely left the Tx shift
*	register.
*
*	We use this to disable the transmitter and re-enable the receiver.
*/

ISR( USART1_TX_vect )
{
	// Since we were called, the data register is also empty. This means
	// that we can go ahead and disable the transmitter and re-enable the
	// receiver.

	UCSR1B &= ~(( 1 << TXEN ) | ( 1 << TXCIE ));
	UCSR1B |=   ( 1 << RXEN );

} // USART1_TX_vect

//***************************************************************************
/**
*   Spin for ms milliseconds
*/

#define LOOPS_PER_MS ( CFG_CPU_CLOCK / 1000 / 4)

static void ms_spin( unsigned short ms )
{
   if (!ms)
           return;

   /* the inner loop takes 4 cycles per iteration */
   __asm__ __volatile__ (
           "1:                     \n"
           "       ldi r26, %3     \n"
           "       ldi r27, %2     \n"
           "2:     sbiw r26, 1     \n"
           "       brne 2b         \n"
           "       sbiw %1, 1      \n"
           "       brne 1b         \n"
           : "+w" (ms)
           : "w" (ms), "i" (LOOPS_PER_MS >> 8), "i" (0xff & LOOPS_PER_MS)
           : "r26", "r27"
           );

} // ms_spin

//***************************************************************************
/**
*   Main loop, which really just manages the activity LEDs
*/

int main( void )
{
    int     hearbeat;

    PORTA   = 0xFF;   // enable pullups for inputs
    PORTB   = 0xFF;
    PORTC   = 0xFF;
    PORTD   = 0xFF;
    PORTE   = 0xFF;
    PORTF   = 0xFF;
    PORTG   = 0xFF;

    // Set LEDs outputs

	YELLOW_LED_DDR |= YELLOW_LED_MASK;
	RED_LED_DDR    |= RED_LED_MASK;
	BLUE_LED_DDR   |= BLUE_LED_MASK;

    // Set both UARTs GPIO lines as inputs

    DDRD &= ~(( 1 << 2 ) | ( 1 << 3 )); // UART-1 D.2 & D.3
    DDRE &= ~(( 1 << 0 ) | ( 1 << 1 )); // UART-0 E.0 & E.1

    // TOSC1 & TOSC2 are connected to the Red/Blue LEDs so we need to set
    // AS0 to 0. AS0 is the only writable bit.

    ASSR = 0;

    // Setup the UARTs

    UBRR0H = UBRR0_INIT >> 8;
    UBRR0L = UBRR0_INIT & 0xFF;

    UCSR0A = UCSR0A_INIT;
    UCSR0B = UCSR0B_INIT;
    UCSR0C = UCSR0C_INIT;

    UBRR1H = UBRR1_INIT >> 8;
    UBRR1L = UBRR1_INIT & 0xFF;

    UCSR1A = UCSR1A_INIT;
    UCSR1B = UCSR1B_INIT;
    UCSR1C = UCSR1C_INIT;

    sei();  // Enable interrupts.

    // The foreground loops sole responsibility to do the LED flashing.

    hearbeat = 0;
    while ( 1 )
    {
        ms_spin( 50 );

        if ( gRxActivity )
        {
            LED_TOGGLE( RED );

            gRxActivity = 0;
        }
        else
        {
            LED_OFF( RED );
        }

        if ( gTxActivity )
        {
            LED_TOGGLE( BLUE );

            gTxActivity = 0;
        }
        else
        {
            LED_OFF( BLUE );
        }

        if ( ++hearbeat >= 4 )
        {
            LED_TOGGLE(  YELLOW );
            hearbeat = 0;
        }
    }

    return 0;

} // main


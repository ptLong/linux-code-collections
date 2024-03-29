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

/* ---- Include Files ---------------------------------------------------- */

#include <avr/io.h>
#include <stdio.h>

#include "Log.h"
#include "Timer.h"
#include "Timer-UART.h"
#include "UART.h"

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

#define LED_DDR         DDRD
#define LED_PORT        PORTD
#define LED_MASK        ( 1 << 3  )

/* ---- Private Variables ------------------------------------------------ */

/* ---- Functions -------------------------------------------------------- */

//***************************************************************************
/**
*   Uart Test
*/

int main(void)
{
    int     i;

    InitTimer();
    InitUART();
    InitTimerUART();

    LED_DDR |= LED_MASK;

    // The first handle opened for read goes to stdin, and the first handle
    // opened for write goes to stdout.

    Log( "*****\n" );
    Log( "***** TimerUart Test program\n" );
    Log( "*****\n" );

    UART0_PutStr( "The Logging output is being directed to the TimerUART\n" );

    while( 1 )
    {
        LED_PORT ^= LED_MASK;

        //printf( "." );

        Log( "." );

        // Tick rate is 100/sec so waiting for 100 waits for 1 sec

        for ( i = 0; i < 100; i++ ) 
        {
            tick_t prevCount = gTickCount;
            while ( gTickCount == prevCount )
            {
                LogBufDump();
            }

            if ( UART0_IsCharAvailable() )
            {
                char    ch = UART0_GetChar();

                Log( "Read: '%c'\n", ch );

                if ( ch == ' ' )
                {
                    Log( "*** Press a key to continue\n" );
                    ch = UART0_GetChar();
                    Log( "*** Continuing...\n" );
                }
            }
        }
    }

    return 0;
}


###########################################################################
#
# tiny45-flasher Makefile
#
###########################################################################

MK_OS				= avr
MK_AVR_MCU			= m8
MK_AVR_FREQ			= 16MHz
MK_AVR_PROGRAMMER  ?= BootHost
					  
MK_HEX_TARGET	= adc-test
MK_SRC_FILES	= adc-test.c adc.c Log.c StrPrintf.o UART.c Timer.c

include ../../rules/mkRules.mk


###########################################################################
#
#	Makefile to build SysTick
#
###########################################################################

MK_OS			= cortex-m3
MK_ELF_TARGET 	= SysTick

MK_SRC_FILES 	= SysTick.c StrPrintf.c lcd-printf.c lcd-osram96x16. startup-gcc.c osram96x16.c

LM3S811_DIR	= $(MK_ROOT)/WinARM/lm3s811_evalboard

CPPFLAGS	+= -I $(LM3S811_DIR) -I $(LM3S811_DIR)/src -I$(LM3S811_DIR)/ev-lm3s811
LDFLAGS		+= -L $(LM3S811_DIR)
LOADLIBES	+= -lluminary

include ../../rules/mkRules.mk


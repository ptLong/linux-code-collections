MCU_TARGET      = atmega8
MCU_PROG_TARGET = m8
OPTIMIZE        = -O2

DEFS            =
LIBS            =

# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
PROGRAMMER     = avrdude

PROGRAMMER_OPTS = -p $(MCU_PROG_TARGET) -c avrisp -P COM1

.PHONY: MiniSumo FlashLed

all: MiniSumo

MiniSumo: MiniSumo.hex
FlashLed: FlashLed.hex

MiniSumo.elf : MiniSumo.o a2d.o Avoid.o DebugKey.o Delay.o Motor.o Sensor.o Puts.o Uart.o

FlashLed.elf : FlashLed.o a2d.o DebugKey.o Delay.o Motor.o Sensor.o Puts.o Uart.o arclib_10.o        

FORCE:

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
        
d download: MiniSumo.dld
    
%.elf :
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

BOOTHOST = /MyDocuments/Robotics/MyBots/Orion/BootHost/exe/BootHost.exe
BOOTHOST_OPTS = -m

%.dld : %.hex
#    $(PROGRAMMER) $(PROGRAMMER_OPTS) -e -U flash:w:$<
	@echo "Downloading $< ..."
	$(BOOTHOST) $(BOOTHOST_OPTS) $<


%.cod: %.c FORCE
   $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -g -Wa,-ahdl=$@ -o $(@:.cod=.o) $<
        
%.pp: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -E -Wp,-dN,-C -o $@ $<
    

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@

# Every thing below here is used by avr-libc's build system and can be ignored
# by the casual user.

FIG2DEV                 = fig2dev
EXTRA_CLEAN_FILES       = *.hex *.bin *.srec

dox: eps png pdf

eps: $(PRG).eps
png: $(PRG).png
pdf: $(PRG).pdf

%.eps: %.fig
	$(FIG2DEV) -L eps $< $@

%.pdf: %.fig
	$(FIG2DEV) -L pdf $< $@

%.png: %.fig
	$(FIG2DEV) -L png $< $@


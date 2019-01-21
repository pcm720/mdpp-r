
PRJ = main
MCU = atmega88p
CLK = 8000000UL

# Libraries
LIBSRC = $(wildcard *.c)
LIBOBJ = $(LIBSRC:.c=.o)
LIBHDR = $(LIBSRC:.c=.h)

# Compiler and linker settings
CPPFLAGS = -mmcu=$(MCU) -DF_CPU=$(CLK) -I. -I$(LIBDIR) -Wall
CFLAGS = -Os -g -Wall --std=gnu11
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections 
LDFLAGS = -Wl,-Map,$(PRJ).map -mmcu=$(MCU) 
LDFLAGS += -Wl,--gc-sections

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size --format=avr --mcu=$(MCU)
CC      = avr-gcc

# Programmer settings
PRG = apu2 -P ft0
AVRDUDE = taskset -c 1 avrdude -c $(PRG) -p $(MCU) -C ~/.avrduderc -C /etc/avrdude.conf
LFU = 0xE2
HFU = 0xDF
EFU = 0xFF

all: $(PRJ).hex

clean:
	rm -f *.hex *.elf *.o *.map $(LIBOBJ)

test:
	$(AVRDUDE) -v

flash: all
	$(AVRDUDE) -U flash:w:$(PRJ).hex:i

fuse:
	$(AVRDUDE) -U lfuse:w:$(LFU):m -B 1200 -U hfuse:w:$(HFU):m -U efuse:w:$(EFU):m

disasm: $(PRJ).elf
	$(OBJDUMP) -d $(PRJ).elf

%.o: %.c $(LIBHDR) Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<;

$(PRJ).elf: $(LIBOBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PRJ).hex: $(PRJ).elf
	rm -f $(PRJ).hex
	$(OBJCOPY) -j .text -j .data -O ihex $(PRJ).elf $(PRJ).hex
	$(SIZE) $(PRJ).elf

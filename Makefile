EE_BIN = Chip8-Emulator-PS2.ELF
EE_OBJS = src/main.o src/graphics.o src/explorer.o src/pad.o \
			resources/background_buffer.o
			
EE_LIBS = -lkernel -lpatches -lelf-loader-nocolour -lfileXio -lpad -L$(PS2SDK)/ports/lib -lps2_drivers -lpngps2 -L$(GSKIT)/lib -lgskit -ldmakit
EE_INCS = -Iinclude -I$(GSKIT)/include -I$(PS2SDK)/ports/include
EE_CFLAGS = -O3 -DNEWLIB_PORT_AWARE

all: background_buffer $(EE_BIN)
	$(MAKE) -C core

clean:
	rm -rf $(EE_BIN) $(EE_OBJS) resources/background_buffer.c
	$(MAKE) -C core clean

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

background_buffer:
	bin2c resources/CHIP8-PS2-BACKGROUND.png resources/background_buffer.c BACKGROUND

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

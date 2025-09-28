EE_BIN = Chip8-CORE.ELF
EE_OBJS = main.o chip8.o pad.o modules/poweroff_irx.o

EE_LIBS = -lkernel -lpad -lpoweroff -lpatches -lelf-loader-nocolour -L$(GSKIT)/lib -lgskit -ldmakit -L$(PS2SDK)/ports/lib -lps2_drivers
EE_CFLAGS = -Wall -O3 -flto
EE_INCS = -I$(GSKIT)/include -I$(PS2SDK)/ports/include

all: poweroff_irx $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) modules/poweroff_irx.c 

run:
	ps2client execee host:$(EE_BIN)

poweroff_irx:
	bin2c $(PS2SDK)/iop/irx/poweroff.irx modules/poweroff_irx.c PWROFF

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal_cpp

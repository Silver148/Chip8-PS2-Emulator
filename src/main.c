#include "main.h"
#include "graphics.h"
#include "explorer.h"
#include "pad.h"

extern char file_paths[256][256];

int port=0, slot=0;
static int padBuf[256] __attribute__((aligned(64)));
u32 old_pad = 0;
u32 new_pad, paddata;
struct padButtonStatus buttons;
int index_rom = 0;

char *argv[3] = {0};

void Select_A_Rom()
{
    for(;;)
    {
        int ret = padRead(port, slot, &buttons);

        if(ret != 0)
        {
            paddata = 0xffff ^ buttons.btns; 
            new_pad = paddata & ~old_pad;
            old_pad = paddata;

            if(new_pad & PAD_UP)
            {
                index_rom--;
                if(index_rom <= 0)
                    index_rom = 0;

                RenderMenu(); //Update menu

                printf("ROM: %s\n", file_paths[index_rom]);
            }

            if(new_pad & PAD_DOWN)
            {
                index_rom++;
                if(file_paths[index_rom][0] == '\0')
                    index_rom--;

                RenderMenu();

                printf("ROM: %s\n", file_paths[index_rom]);
            }

            if(new_pad & PAD_CROSS)
            {
                if(file_paths[index_rom][0] != '\0')
                {
                    argv[1] = file_paths[index_rom];
                    LoadELFFromFile("mass0:Chip8-Emulator-PS2/core/Chip8-CORE.ELF", 1, &argv[1]); //Execute core
                    return;
                }
            }
        }
    }
}

int main()
{
    SifInitRpc(0);

    sbv_patch_disable_prefix_check();
    sbv_patch_enable_lmb();
    sbv_patch_fileio();

    int ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if(ret < 0)
    {
        printf("Failed to load SIO2MAN: %d\n", ret);
        return -1;
    }

    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    if(ret < 0)
    {
        printf("Failed to load PADMAN: %d\n", ret);
        return -1;
    }

    padInit(0);

    padPortOpen(port, slot, padBuf);

    initializePad(port, slot);

    init_fileXio_driver();
    fileXioInit();
    init_usb_driver();
    waitUntilDeviceIsReady("mass0:");

    ReadDir("mass0:"); //Read files from mass0:

    InitGraphics(); //Init Graphics

    RenderMenu(); //Render Menu

    Select_A_Rom(); // PAD Input Loop

    SleepThread();

    return 0;
}

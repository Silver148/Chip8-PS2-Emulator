#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <pthread.h>
#include <sched.h>
#include "chip8.hpp"
#include "pad.hpp"
#include "stdint.h"

extern "C"{
    #include "kernel.h"
    #include "sifrpc.h"
    #include "loadfile.h"
    #include "libpad.h"
    #include "gsKit.h"
    #include "dmaKit.h"
    #include "malloc.h"
    #include "tamtypes.h"
    #include "libpwroff.h"
    #include "sbv_patches.h"
    #include "ps2_all_drivers.h"
    #include "elf-loader.h"
}

#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define PS2_WIDTH 640
#define PS2_HEIGHT 448

#define SCALE_FACTOR 8
#define CHIP8_ON_COLOR GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0x80) // White
#define CHIP8_OFF_COLOR GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x80) // Black

extern unsigned char PWROFF[];
extern unsigned int size_PWROFF;

const u32 pad_map[16] = {
    //Keyboard chip8
    PAD_START,      // 0
    PAD_L1,         // 1
    PAD_R1,         // 2
    PAD_L2,         // 3
    PAD_TRIANGLE,   // 4
    PAD_UP,         // 5
    PAD_SQUARE,     // 6
    PAD_LEFT,       // 7
    PAD_DOWN,       // 8
    PAD_RIGHT,      // 9 
    PAD_SELECT,     // A 
    PAD_L3,         // B
    PAD_R2,         // C
    PAD_CIRCLE,     // D
    PAD_CROSS,      // E
    PAD_R3          // F 
};

float screen_x1 = (PS2_WIDTH - (CHIP8_WIDTH * SCALE_FACTOR)) / 2.0f;
float screen_y1 = (PS2_HEIGHT - (CHIP8_HEIGHT * SCALE_FACTOR)) / 2.0f;
float screen_x2 = screen_x1 + (CHIP8_WIDTH * SCALE_FACTOR);
float screen_y2 = screen_y1 + (CHIP8_HEIGHT * SCALE_FACTOR);

void init_texture_chip8(GSGLOBAL *gsGlobal, GSTEXTURE *texture)
{
    texture->Width = CHIP8_WIDTH;
    texture->Height = CHIP8_HEIGHT;
    texture->Filter = GS_FILTER_NEAREST;
    texture->PSM = GS_PSM_CT32;
    texture->Vram = gsKit_vram_alloc(gsGlobal, 
        gsKit_texture_size(texture->Width, texture->Height, texture->PSM), 
        GSKIT_ALLOC_USERBUFFER);
    texture->Mem = (u32*)memalign(128, gsKit_texture_size(texture->Width, texture->Height, texture->PSM));
    texture->ClutPSM = 0;
    texture->Clut = NULL;

    //Clean the memory texture
    for(int i = 0; i < 2048; i++) {
        ((u32*)texture->Mem)[i] = CHIP8_OFF_COLOR;
    }

    gsKit_texture_upload(gsGlobal, texture); //Upload the texture to the VRAM
}

std::mutex Chip8_mutex;

void PadThread(Chip8& c8, struct padButtonStatus& buttons, int& port, int& slot, bool &is_paused, unsigned int &old_pad, unsigned int &new_pad, unsigned int &paddata, int& ret)
{
    printf("PAD thread started\n");
    for(;;){
        ret = padRead(port, slot, &buttons);

        if(ret != 0)
        {   
            Chip8_mutex.lock();
            paddata = 0xffff ^ buttons.btns; 
            new_pad = paddata & old_pad; //new_pad made to detect if a button is still pressed
            old_pad = paddata;

            for (int i = 0; i < 16; i++) {
                if (new_pad & pad_map[i]) {
                    //printf("Key %d pressed\n", pad_map[i]);
                    c8.keypad[i] = 1;
                }else if(!(new_pad & pad_map[i]) && c8.keypad[i] == 1){
                    //printf("Key %d released\n", pad_map[i]);
                    c8.keypad[i] = 0;
                }

            }

            if ((new_pad & PAD_L1) && (new_pad & PAD_R1)) {
                is_paused = !is_paused; 
            }
            
            if((new_pad & PAD_L1) && (new_pad & PAD_R1) && (new_pad & PAD_SQUARE))
            {
                poweroffShutdown(); //poweroff console
            }

            if((new_pad & PAD_L1) && (new_pad & PAD_R1) && (new_pad & PAD_L2) && 
                (new_pad & PAD_R2) && (new_pad & PAD_START) && (new_pad & PAD_SELECT))
            {
                LoadELFFromFile("mass0:Chip8-Emulator-PS2/Chip8-Emulator-PS2.ELF", 0, NULL); //return to main menu
            }

            Chip8_mutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }
}

void RenderThread(Chip8& c8, GSTEXTURE *texture_chip8, GSGLOBAL *gs)
{
    printf("Render thread started\n");
    while(1)
    {
        gsKit_clear(gs, GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x00));
        Chip8_mutex.lock();
        if(c8.drawFlag)
        {
            for (int i = 0; i < 2048; ++i) {

                if (c8.video[i]) {
                    ((u32*)texture_chip8->Mem)[i] = CHIP8_ON_COLOR;
                    }else{
                    ((u32*)texture_chip8->Mem)[i] = CHIP8_OFF_COLOR;
                    }
            }
            gsKit_texture_upload(gs, texture_chip8); //Refresh the graphics buffer
            c8.drawFlag = false;
        }    

        gsKit_prim_sprite_texture(gs, texture_chip8, 
        screen_x1, screen_y1,
        0.0f, 0.0f,
        screen_x2, screen_y2,
        CHIP8_WIDTH, CHIP8_HEIGHT,
        0.0f, GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0xFF));

        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
        Chip8_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }
}

void ThreadEmulation(Chip8& c8, bool &is_paused)
{
    printf("Emulation thread started\n");
    while(1)
    {
        Chip8_mutex.lock();
        if(!is_paused){
            for(unsigned int i = 0; i < 8; i++) //Run 8 cycles per frame
            c8.emulate_cycles(); 
        }
        Chip8_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }
}

int main(int argc, char *argv[])
{
    Chip8 c8 = Chip8();
    GSTEXTURE *texture_chip8 = (GSTEXTURE*)malloc(sizeof(GSTEXTURE));
    GSGLOBAL *gs;
    static int padBuf[256] __attribute__((aligned(64)));
    int ret=0, port=0, slot=0;
    unsigned int old_pad = 0;
    unsigned int new_pad, paddata;
    struct padButtonStatus buttons;
    bool is_paused = false;

    /*
    The emulator will not work if the GUI ELF does not pass the path of a ROM as an argument
    (This is only core of the emulator)
    */

    if(argc != 2)
    {
        printf("Usage: %s <ROM>\n", argv[0]);
        exit(0);
    }

    SifInitRpc(0);

    sbv_patch_disable_prefix_check();
    sbv_patch_enable_lmb();
    sbv_patch_fileio();

    if(SifLoadModule("rom0:SIO2MAN", 0, NULL) < 0){
        printf("Error to loading SIO2MAN\n");
        exit(0);
    }

    if(SifLoadModule("rom0:PADMAN", 0, NULL) < 0){
        printf("Error to loading PADMAN\n");
        exit(0);
    }

    if(SifExecModuleBuffer(PWROFF, size_PWROFF, 0, NULL, NULL) < 0)
    {
        printf("Error to loading module poweroff\n");
        exit(0);
    }

    init_usb_driver();

    waitUntilDeviceIsReady("mass0:");

    poweroffInit();

    PAD pad;

    padInit(0);

    if(padPortOpen(slot, port, padBuf) < 0)
    {
        printf("ERROR TO OPEN PORT\n");
        exit(0);
    }

    pad.initializePad(slot, port);

    gs = gsKit_init_global();

    gs->Mode = GS_MODE_NTSC;
    gs->Field = GS_FIELD;
    gs->ZBuffer = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_mode_switch(gs, GS_ONESHOT);

    init_texture_chip8(gs, texture_chip8);

    gsKit_init_screen(gs);

    printf("LOADING ROM %s\n", argv[1]);
    c8.LoadROM(argv[1]);
    
    std::thread emulation_thread(ThreadEmulation, std::ref(c8), std::ref(is_paused));
    std::thread pad_thread(PadThread, std::ref(c8), std::ref(buttons), std::ref(port), std::ref(slot), std::ref(is_paused), std::ref(old_pad), std::ref(new_pad), std::ref(paddata), std::ref(ret));
    std::thread graphics_thread(RenderThread, std::ref(c8), texture_chip8, gs);

    emulation_thread.join();
    pad_thread.join();
    graphics_thread.join();

    SleepThread();

    return 0;
}

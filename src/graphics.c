#include "graphics.h"
#include "explorer.h"

GSGLOBAL *gsGlobal;
GSTEXTURE background;
GSFONTM *fm;

extern unsigned char BACKGROUND[];
extern unsigned int size_BACKGROUND;

extern char file_paths[256][256];
char selected_rom[256];
extern int index_rom;
float textSize = 0.50f;

void InitGraphics()
{
    gsGlobal = gsKit_init_global();
    fm = gsKit_init_fontm();

    gsGlobal->Mode = GS_MODE_NTSC;
    gsGlobal->Width = 400;
    gsGlobal->Height = 400;
    gsGlobal->PSM = GS_PSM_CT32;
    gsGlobal->PSMZ = GS_PSMZ_32;
    gsGlobal->ZBuffering = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;

    gsKit_init_screen(gsGlobal);

    LoadBackground();

    gsKit_fontm_upload(gsGlobal, fm);

    fm->Spacing = 0.95f;

    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

}

void LoadBackground()
{
    load_texture_from_buffer(gsGlobal, &background, BACKGROUND, size_BACKGROUND, GS_PSM_CT24);
    print_vram_usage(gsGlobal);
}

void RenderMenu()
{
    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x00));

    draw_texture(gsGlobal, &background, 0, 0);

	gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
	gsKit_set_test(gsGlobal, GS_ATEST_OFF);
    
    gsKit_fontm_print_scaled(gsGlobal, fm, 100, 200, 0, textSize, GS_SETREG_RGBA(0xFF, 0xFF, 0x00, 0xFF), (const char*)DeletePrefix(file_paths[index_rom], "mass0:", selected_rom));

	gsKit_set_test(gsGlobal, GS_ATEST_ON);
	gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);

    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

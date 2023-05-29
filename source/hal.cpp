
#include "hal.h"
#define SCREEN_WIDTH (240)
#define SCREEN_HEIGHT (160)

u16 (*vram)[240] = (u16 (*)[240])VRAM;
EWRAM_BSS u16 vramBuf[SCREEN_HEIGHT][SCREEN_WIDTH];
auto VRAM_SIZE = sizeof(u16)*SCREEN_WIDTH*SCREEN_HEIGHT;
bool sync_enable = true;

int halGetScreenWidth() { return SCREEN_WIDTH; }

int halGetScreenHeight() { return SCREEN_HEIGHT; }

void halDrawPixel(int x, int y, u16 color) { vramBuf[y][x] = color; }

const char *halGetFontDataPtr() {
  return my_font;
}

void syncEnable(){
  sync_enable = true;
}
void syncDisable(){
  sync_enable = false;
}
void syncToScreen(){
  if(sync_enable){
    dmaCopy(vramBuf,vram,VRAM_SIZE);
    // memcpy(vram[isTop],vramBuf[isTop],VRAM_SIZE);
  }
}
void halClearPixel(){
  memset(vramBuf,0,VRAM_SIZE);
}
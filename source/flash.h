#pragma once

#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#define GAME_ROM (0x8000000)
#define GAME_ROM_W1 (0x8000000)
#define GAME_ROM_W2 (0xA000000)
#define GAME_ROM_W3 (0xC000000)
//256KB
#define BLOCK_SIZE (0x40000)
//1KB 每次Buffer写入的大小
#define SECTOR_SIZE (0x400)

vu16 readFlash(u32 addr);
volatile void writeFlash(u32 addr, u16 data);
vu8 readFlashFlip(u32 addr);
int unlockBlock(int block);
int eraseBlock(int block);
int flashIntelBuffered(int block,int sector,int sector_num,bool cal=false);
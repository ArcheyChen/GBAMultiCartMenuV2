#pragma once
#include <gba_types.h>
//这个文件里面的所有代码都是关键区域代码

//64KB 等于SRAM的大小
#define BUFFER_SIZE (0x10000)
extern u8 globle_buffer[BUFFER_SIZE];//64KB
void consoleClear();
void pressToContinue(bool show = false);
void gotoChipOffset(int MBoffset,bool bootGame);
void backupSram();

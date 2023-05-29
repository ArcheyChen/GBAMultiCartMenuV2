#pragma once
#include <gba_types.h>
//这个文件里面的所有代码都是关键区域代码

//64KB 等于SRAM的大小
#define BUFFER_SIZE (0x10000)
extern u8 globle_buffer[BUFFER_SIZE];//64KB
extern u8 sramBackup[3];
extern int gameCnt;
void pressToContinue(bool show = false);
void gotoChipOffset(int MBoffset,bool bootGame,bool isAutoBoot=false);
void backupSramLite();
void findGames();

#define GAME_NAME_LEN 18
#define GAME_NAME_LEN_EXPAND 14*3
/*
  0A0h    12    Game Title       (uppercase ascii, max 12 characters)
  0ACh    4     Game Code        (uppercase ascii, 4 characters)
  0B0h    2     Maker Code       (uppercase ascii, 2 characters)
*/

struct RedirectStruct{
    char MAGIC_WORD[12];
    int offset;
};
struct GameEntry{
    char name[GAME_NAME_LEN_EXPAND + 1];
    int MB_offset;
};

extern GameEntry gameEntries[32];

#include "manage.h"
#include <gba_console.h>
#include "misc.h"
#include <stdio.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include "flash.h"
#include <string.h>

const int MAGIC_LEN = 32;
const char MAGIC_CODE_STD[MAGIC_LEN] = "THIS IS A TEST VER";
const int META_BLOCK_IDX = 7 * 1024 / 256;//存放在第7MB的地方
struct LastTimeRun{
    char MAGIC_CODE1[MAGIC_LEN];
    int MBOffset;
    bool auto_start;
    char MAGIC_CODE2[MAGIC_LEN];
    LastTimeRun(const volatile LastTimeRun& that){
        this->MBOffset = that.MBOffset;
        this->auto_start = that.auto_start;
        for(int i=0;i<MAGIC_LEN;i++){
            this->MAGIC_CODE1[i] = that.MAGIC_CODE1[i];
            this->MAGIC_CODE2[i] = that.MAGIC_CODE2[i];
        }
    }
    bool isValid(){
        int cmp1 = strncmp(MAGIC_CODE1,MAGIC_CODE_STD,MAGIC_LEN);
        int cmp2 = strncmp(MAGIC_CODE2,MAGIC_CODE_STD,MAGIC_LEN);
        return (cmp1 == 0 && cmp2 == 0);
    }
    LastTimeRun (int _MBOffset){
        this->MBOffset = _MBOffset;
        this->auto_start = true;
        strncpy(MAGIC_CODE1,MAGIC_CODE_STD,MAGIC_LEN);
        strncpy(MAGIC_CODE2,MAGIC_CODE_STD,MAGIC_LEN);
    }
};


int askMBOffset(){
    int offset = 0;
    while(1){
        consoleClear();
        printf("Offset = %d MB\n",offset);
        
		scanKeys();
        auto keys = keysDown();
        if(keys & KEY_UP){
            offset += 8;
        }else if (keys & KEY_DOWN){
            offset -= 8;
        }
        if(offset<16){
            offset = 16;
        }
        if(offset>256-8){
            offset = 256-8;
        }
        if(keys & KEY_A){
            break;
        }
		VBlankIntrWait();
    }
    LastTimeRun newLastRun(offset);
    *(LastTimeRun*)globle_buffer = newLastRun;
    unlockBlock(META_BLOCK_IDX);
    eraseBlock(META_BLOCK_IDX);
    flashIntelBuffered(META_BLOCK_IDX,0,1);
    return offset;
}
bool autoStartGame(){
    LastTimeRun last_run = *(volatile LastTimeRun*)(GAME_ROM + META_BLOCK_IDX * BLOCK_SIZE); 
    if(last_run.isValid() && last_run.auto_start){
        printf("Auto run!\n offset=%d\n",last_run.MBOffset);
        while(1);
        // gotoChipOffset(last_run.MBOffset,true);
        // return true;//should never return
    }
    return false;
}
bool pressedKeyOnBoot(u16 key){
    for(int i=0;i<1024;i++){
        scanKeys();
        if(keysDownRepeat() & key){
            return true;
        }
    }
    return false;
}
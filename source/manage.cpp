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
void loadFlashSaveToBuffer(int GameMBOffset);
void saveSramSaveToBuffer();
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


int askMBOffset(int lastOffset){

    int offset = 0;
    while(1){
        consoleClear();
        if(lastOffset>0){
            printf("Saved sram at %d MB Game\n",lastOffset);
        }
        printf("Offset = %d MB\n",offset);
        
		scanKeys();
        auto keys = keysDown();
        if(keys & KEY_UP){
            offset += 8;
        }else if (keys & KEY_DOWN){
            offset -= 8;
        }else if (keys & KEY_LEFT){
            offset -= 16;
        }else if (keys & KEY_RIGHT){
            offset += 16;
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
    *(LastTimeRun*)globle_buffer = newLastRun;//新的Meta

    unlockBlock(META_BLOCK_IDX);
    eraseBlock(META_BLOCK_IDX);
    flashIntelBuffered(META_BLOCK_IDX,0,1);//烧写Meta

    loadFlashSaveToBuffer(offset);//加载先前的存档
    gotoChipOffset(offset,true,false);//开始游戏
    return offset;
}

bool autoStartGame(){
    LastTimeRun last_run = *(volatile LastTimeRun*)(GAME_ROM + META_BLOCK_IDX * BLOCK_SIZE); 
    if(last_run.isValid() && last_run.auto_start){
        gotoChipOffset(last_run.MBOffset,true,true);
        return true;//should never return
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
void saveSramToFlash(int GameMBOffset){
    int gameIdx = GameMBOffset / 8;
    int blockIdx = (8 * 1024/256) + gameIdx;//从8MB的地方开始放，最多支持32个游戏，32*256KB=8MB，刚好用前16MB空间
    saveSramSaveToBuffer();
    unlockBlock(blockIdx);
    eraseBlock(blockIdx);
    flashIntelBuffered(blockIdx,0,64);
    int flashAddr = blockIdx * BLOCK_SIZE;
    vu8* flash = (vu8*)(GAME_ROM + flashAddr);
    for(int i=0;i< 64 * 1024;i++){
        if(flash[i] != globle_buffer[i]){
            printf("sram save error at %d\n",i);
            printf("flash=%x buffer=%x\n",flash[i],globle_buffer[i]);
        }
    }
}
void saveSramSaveToBuffer(){
    vu8* sram = (vu8*)SRAM;
    for(int i=0;i<64 * 1024;i++){
        globle_buffer[i] = sram[i];
    }
	globle_buffer[2] = sramBackup[0];
	globle_buffer[3] = sramBackup[1];
	globle_buffer[4] = sramBackup[2];
}
void loadFlashSaveToBuffer(int GameMBOffset){
    int gameIdx = GameMBOffset / 8;
    int blockIdx = (8 * 1024/256) + gameIdx;//从8MB的地方开始放，最多支持32个游戏，32*256KB=8MB，刚好用前16MB空间
    gotoChipOffset(0,false);//回到0 Offset的位置，保证读取到的是正确的东西
    int flashAddr = blockIdx * BLOCK_SIZE;
    vu8* flash = (vu8*)(GAME_ROM + flashAddr);
    for(int i=0;i<64 * 1024;i++){
        globle_buffer[i] = flash[i];
    }
}
int trySaveGame(){
    LastTimeRun last_run = *(volatile LastTimeRun*)(GAME_ROM + META_BLOCK_IDX * BLOCK_SIZE); 
    if(last_run.isValid()){
        saveSramToFlash(last_run.MBOffset);
        return last_run.MBOffset;
    }
    return -1;
}
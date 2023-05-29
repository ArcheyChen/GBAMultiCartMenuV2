#include "manage.h"
#include <gba_console.h>
#include "misc.h"
#include <stdio.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include "flash.h"
#include <string.h>
#include "menu.h"
#include "font.h"

const int MAGIC_LEN = 32;
const char MAGIC_CODE_STD[MAGIC_LEN] = "THIS IS A TEST VER";
const int META_BLOCK_IDX = 8 * 1024 / 256;//存放在第8MB的地方
const int SRAM_NOT_SAVE_BACKUP_BLOCK_IDX = META_BLOCK_IDX + 1;
//如果一开始没选save sram,可能导致sram炸了，因此这个时候要把sram保存到这个位置
void loadFlashSaveToBuffer(int GameMBOffset,bool loadFromAutoSave);
void saveSramSaveToBuffer();
struct LastTimeRun{
    char MAGIC_CODE1[MAGIC_LEN];
    char gameName[GAME_NAME_LEN + 1];
    int MBOffset;
    bool load_from_auto_save;
    char MAGIC_CODE2[MAGIC_LEN];
    LastTimeRun(const volatile LastTimeRun& that){
        this->MBOffset = that.MBOffset;
        this->load_from_auto_save = that.load_from_auto_save;
        for(int i=0;i<MAGIC_LEN;i++){
            this->MAGIC_CODE1[i] = that.MAGIC_CODE1[i];
            this->MAGIC_CODE2[i] = that.MAGIC_CODE2[i];
        }
        for(int i=0;i<GAME_NAME_LEN+1;i++){
            this->gameName[i] = that.gameName[i];
        }
    }
    bool isValid(){
        int cmp1 = strncmp(MAGIC_CODE1,MAGIC_CODE_STD,MAGIC_LEN);
        int cmp2 = strncmp(MAGIC_CODE2,MAGIC_CODE_STD,MAGIC_LEN);
        return (cmp1 == 0 && cmp2 == 0);
    }
    LastTimeRun (int _MBOffset){
        this->MBOffset = _MBOffset;
        this->load_from_auto_save = false;
        strncpy(MAGIC_CODE1,MAGIC_CODE_STD,MAGIC_LEN);
        strncpy(MAGIC_CODE2,MAGIC_CODE_STD,MAGIC_LEN);
    }
};

void saveMetaToFlash(LastTimeRun newLastRun){

    LastTimeRun* lastRunBuffer = (LastTimeRun*)globle_buffer;
    *lastRunBuffer = newLastRun;//新的Meta
    unlockBlock(META_BLOCK_IDX);
    eraseBlock(META_BLOCK_IDX);
    flashIntelBuffered(META_BLOCK_IDX,0,1);//烧写Meta
}

int askMBOffset(int lastOffset){

    findGames();
    Menu gameMenu("=============游戏列表=============");
    for(int i=0;i<gameCnt;i++){
        gameMenu.addOption(std::to_string(gameEntries[i].MB_offset) +std::string("MB  ") +std::string(gameEntries[i].name));
    }
    int option = gameMenu.getDecision();
    while(option == -1){
        option = gameMenu.getDecision();
    }
    int offset = gameEntries[option].MB_offset;    

    LastTimeRun newLastRun(offset);//建立新的meta
    strncpy(newLastRun.gameName, gameEntries[option].name,GAME_NAME_LEN);
    newLastRun.gameName[GAME_NAME_LEN]='\0';

    saveMetaToFlash(newLastRun);

    loadFlashSaveToBuffer(offset,false);//加载先前的存档
    gotoChipOffset(offset,true,false);//开始游戏
    return offset;
}

bool autoStartGame(){
    LastTimeRun last_run = *(volatile LastTimeRun*)(GAME_ROM + META_BLOCK_IDX * BLOCK_SIZE); 
    if(last_run.isValid()){
        if(last_run.load_from_auto_save){//如果上次游戏进过菜单，sram可能会损坏，需要从autosave中load出来
            printf_zh("加载自动保存的存档中...");//等待时间会比较久，让玩家知道这是正常的
            last_run.load_from_auto_save = false;
            saveMetaToFlash(last_run);
            loadFlashSaveToBuffer(0,true);
            gotoChipOffset(last_run.MBOffset,true,false);//这次我们不用sram save lite中的数据恢复
        }
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
void saveSramToFlash(int GameMBOffset,bool isAutoSave){
    int gameIdx = GameMBOffset / 8;
    int blockIdx = (8 * 1024/256) + gameIdx;//从8MB的地方开始放，最多支持32个游戏，32*256KB=8MB，刚好用前16MB空间
    if(isAutoSave){
        blockIdx = SRAM_NOT_SAVE_BACKUP_BLOCK_IDX;//没选保存的情况下，就烧到这个地方。
    }
    saveSramSaveToBuffer();
    unlockBlock(blockIdx);
    eraseBlock(blockIdx);
    flashIntelBuffered(blockIdx,0,64);
    int flashAddr = blockIdx * BLOCK_SIZE;
    vu8* flash = (vu8*)(GAME_ROM + flashAddr);
    for(int i=0;i< 64 * 1024;i++){
        if(flash[i] != globle_buffer[i]){
            printf_zh("sram save error at %d\n",i);
            printf_zh("flash=%x buffer=%x\n",flash[i],globle_buffer[i]);
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
void loadFlashSaveToBuffer(int GameMBOffset,bool loadFromAutoSave){
    int gameIdx = GameMBOffset / 8;
    int blockIdx = (8 * 1024/256) + gameIdx;//从8MB的地方开始放，最多支持32个游戏，32*256KB=8MB，刚好用前16MB空间
    if(loadFromAutoSave){
        blockIdx = SRAM_NOT_SAVE_BACKUP_BLOCK_IDX;
    }
    int flashAddr = blockIdx * BLOCK_SIZE;
    vu8* flash = (vu8*)(GAME_ROM + flashAddr);
    for(int i=0;i<64 * 1024;i++){
        globle_buffer[i] = flash[i];
    }
}
int trySaveGame(){

    LastTimeRun last_run = *(volatile LastTimeRun*)(GAME_ROM + META_BLOCK_IDX * BLOCK_SIZE); 
    if(last_run.isValid()){

        printf_zh("欢迎使用Ausar合卡管理菜单V0.9\n");
        printf_zh("仅供学习交流使用，禁止商业用途\n\n");
        std::string gameInfoStr = "上次运行的游戏:\n游戏名: ";
        gameInfoStr += (const char*)last_run.gameName;
        gameInfoStr += "\n偏移: " +std::to_string(last_run.MBOffset)+std::string("MB");
        std::string menuTitle = "是否保存上次游戏存档到Flash?\n" + gameInfoStr;
        
        Menu menu(menuTitle.c_str());
        menu.addOption("Yes");
        menu.addOption("No");
        int option = menu.getDecision();
        if(option == 0){
            saveSramToFlash(last_run.MBOffset,false);
            printf_zh("Sram 已保存\n");
        }else{
            printf_zh("跳过保存\n");
            printf_zh("处理中,请稍后...\n");
            saveSramToFlash(last_run.MBOffset,true);
            last_run.load_from_auto_save = true;
            saveMetaToFlash(last_run);
        }
        // pressToContinue(true);
        pressToContinue(true);
        return last_run.MBOffset;
    }
    return -1;
}
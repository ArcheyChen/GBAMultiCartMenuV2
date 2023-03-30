#include <gba_base.h>
#include "misc.h"
#include "flash.h"

EWRAM_BSS u8 globle_buffer[BUFFER_SIZE];//64KB
EWRAM_BSS u8 sramBackup[3];



EWRAM_BSS GameEntry gameEntries[32];//最多64个游戏
EWRAM_BSS int gameCnt=0;

void backupSramLite(){
    vu8* sram = (vu8*)SRAM;
    sramBackup[0] = sram[2];
    sramBackup[1] = sram[3];
    sramBackup[2] = sram[4];
    return;
}
inline void restoreSramLite(){
    vu8* sram = (vu8*)SRAM;
	sram[2] = sramBackup[0];
	sram[3] = sramBackup[1];
	sram[4] = sramBackup[2];
    return;
}
IWRAM_CODE void gotoChipOffset(int MBoffset,bool bootGame,bool isAutoBoot){
    u32 chipAddr = (MBoffset/32 * 0x10000000) + (0x4000C0 + (MBoffset & 31) * 0x20202);
	union{
		u32 addr;
		u8 byte[4];
	}addr;
	u16 data = readFlash(0xBD);
	addr.addr = chipAddr;
    vu8* sram = (vu8*)SRAM;
	sram[2] = addr.byte[3];
	sram[3] = addr.byte[2];
	sram[4] = addr.byte[1];
    if(bootGame){
        sram[3]=addr.byte[2] | 0x80;
    }
	int timeout = 0x1000;
	while(timeout && readFlash(0xBD) == data)timeout--;
    if(bootGame){
        REG_IE = 0;
        // restoreSram();
        if(isAutoBoot){
            restoreSramLite();
        }else{
            vu8* sram = (vu8*)SRAM;
            for(int i=0;i<64 * 1024;i++){
                sram[i] = globle_buffer[i];
            }  
        } 
        __asm("SWI 0");
    }
    return;
}
IWRAM_CODE char isGame(){
	volatile unsigned char* nintendo_logo = (volatile unsigned char*)0x8000004;
	unsigned long checksum = 0;
    unsigned int i;
	for(i = 0;i<0x9C;i++){
		checksum += nintendo_logo[i];
	}
	return checksum == 0x4B1B;
}
IWRAM_CODE void findGames(){
    printf("Finding Games,Please Wait...\n");
    gameCnt = 0;
    unsigned int i;
    u16 MB_Offset;
    for(MB_Offset = 16 ;MB_Offset < 256; MB_Offset += 8){\
        gotoChipOffset(MB_Offset,0);
        if(isGame()){
            vu8 *romName = (unsigned char*)0x80000A0;
            for(i=0;i<GAME_NAME_LEN;i++){
                gameEntries[gameCnt].name[i] = romName[i];
            }
            gameEntries[gameCnt].name[GAME_NAME_LEN] = 0;//字符串结尾
            gameEntries[gameCnt].MB_offset = MB_Offset;
            gameCnt++;
        }
    }
    gotoChipOffset(0,0);//返回menu
    return;
}

void consoleClear(){
    printf("\x1b[2J");
}

void pressToContinue(bool show){
    if(show){   
        printf("press to continue\n");
    }
    while(1){
        scanKeys();
        if(keysDown()){
            return;
        }
    }
}
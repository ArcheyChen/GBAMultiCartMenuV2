#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include "flash.h"
#include "misc.h"

#define WAIT_STATUS_READY [](u16 a){return (a&0x80)>0;}
#define WAIT_NON_FFFF [](u16 a){return a!=0xFFFF;}
#define WAIT_FOR_FFFF [](u16 a){return a==0xFFFF;}
#define WAIT_NON_0 [](u16 a){return a!=0;}
#define WAIT_FOR_0 [](u16 a){return a==0;}
const u32 TIMEOUT_5S = 0xA00000;
const u32 TIMEOUT_8S = 0xF00000;


IWRAM_CODE vu16 readFlash(u32 addr){
	return *(vu16*)(addr|GAME_ROM);
}

IWRAM_CODE volatile void writeFlash(u32 addr, u16 data){
	*(vu16 *)(addr|0x8000000) = data;
	asm("nop");
}
IWRAM_CODE void writeFlashFlip(u32 addr, u16 data){
	if(false){
		u16 flipped = data & 0xFFFC;
		flipped |= ( data & 1 ) << 1;
		flipped |= ( data & 2 ) >> 1;
		writeFlash(addr, flipped);
	}else writeFlash(addr, data);
}

IWRAM_CODE void writeArray(u32 addr, u8 len, u16 data[]){
	for(int i = 0; i<len;i++){
		writeFlash(addr,data[i]);
	}
}

IWRAM_CODE bool waitForFlash(u32 address, bool (*isReady)(u16), int timeout){
	while(timeout && !isReady(readFlash(address)) ){
		timeout--;
	}
	if(!timeout){
		return false;
	}
	return true;
}
IWRAM_CODE int eraseBlock(int block){
	u32 addr = block*BLOCK_SIZE;
	u16 cmd[] = {0x50,0xFF,0x20,0xD0};
	writeArray(addr, 4, cmd);
	
	waitForFlash(addr, WAIT_FOR_0, 0x10000);
	
	if(waitForFlash(addr, WAIT_STATUS_READY, TIMEOUT_8S)){
		writeFlash(addr, 0xFF);
		return 0;
	}

	if(readFlash(addr)&0x20){
			printf("Sector %d erase failed\n", block);
			writeFlash(addr, 0xFF);
			return -1;
	}
	
	printf("Intel erase timeout on block %d\n",block);
	writeFlash(addr, 0xFF);
	return -1;
}
IWRAM_CODE int unlockBlock(int block){
	u32 addr = block*BLOCK_SIZE;
	u16 cmd[] = {0x50,0xFF,0x60,0xD0};
	writeArray(addr, 4, cmd);
	
	if(!waitForFlash(addr, WAIT_STATUS_READY, 0x100000)){
		writeFlash(addr, 0xFF);
		return -1;
	}

	writeFlash(addr, 0xFF);
	return 0;
}

IWRAM_CODE int flashIntelBuffered(int block,int sector,int sector_num,bool cal){

	u32 writeAddr =  block* BLOCK_SIZE + sector * SECTOR_SIZE;
	int bufAddr = 0;
    for(int sector_idx = 0;sector_idx < sector_num;sector_idx++){//分成若干个1KB的块来写
        writeFlash(writeAddr, 0x50);
        writeFlash(writeAddr, 0xFF);
        writeFlashFlip(writeAddr, 0xEA);
        writeFlash(writeAddr, 0x1FF);

        for(int i=0;i<0x200;i++){//一次写2KB，所以计数是200，即512KB
			u16 data = (globle_buffer[bufAddr] | (globle_buffer[bufAddr+1] << 8));
			writeFlash(writeAddr+(i*2), data);
			bufAddr+=2;
		}
        writeFlash(writeAddr, 0xD0);
        
        waitForFlash(writeAddr, WAIT_NON_FFFF, 0x1000);
        if(!waitForFlash(writeAddr, WAIT_STATUS_READY, 0x5000)){
            printf("Writing timed out at 0x%06lX\n", writeAddr);
            return -1;
        }
            
        if(readFlash(writeAddr)&3){
            printf("Writing failed at 0x%06lX\n", writeAddr);
        }
        
        writeFlash(writeAddr, 0xFF);//进入读模式
        writeAddr+=0x400;
        
    }
	if(cal){
		u32 readAddr =  block* BLOCK_SIZE + sector * SECTOR_SIZE;
		for(int i=0;i<sector_num;i++){
			for(int j=0;j<1024;j+=2){
				u16 data = (globle_buffer[sector*1024 + j] | (globle_buffer[sector*1024 + j+1] <<8));
				if(readFlash(readAddr) != data ){
					printf("data error at sector%d,%d",i,j);
					printf("want:%x get:%x\n",data,readFlash(readAddr));
					pressToContinue(true);
				}
				readAddr+=2;
			}
		}
		printf("flash cal complete\n");
		pressToContinue(true);
	}
    
	
	return 0;
}
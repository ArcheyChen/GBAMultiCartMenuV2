#include <stdio.h>
#include "sramtest.h"
#include "font.h"
void sram_fill(u8 seed){
    vu8* sram = (vu8*)SRAM;
    for(int i=0;i<64*1024;i++){
        sram[i] = seed;
    }
}
bool sram_test(u8 seed){
    vu8* sram = (vu8*)SRAM;
    for(int i=0;i<64*1024;i++){
        if(sram[i] != seed){
            printf_zh("sram error at [%d]\n",i);
            return false;
        }
    }

    printf_zh("sram OK\n");
    return true;
}
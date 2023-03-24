#include <stdio.h>
#include "sramtest.h"
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
            printf("sram error at [%d]\n",i);
            return false;
        }
    }

    printf("sram OK\n");
    return true;
}
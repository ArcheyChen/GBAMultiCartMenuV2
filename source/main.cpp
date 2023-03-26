
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include "flash.h"
#include "misc.h"
#include "sramtest.h"
#include "manage.h"
#include "menu.h"
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
IWRAM_CODE int main(void) {
//---------------------------------------------------------------------------------


	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();
	backupSramLite();
	//////////////////////

	while(1){
		Menu menu("Menu Test");
		std::string str="Test ";
		for(int i=0;i<50;i++){
			menu.addOption(str+std::to_string(i));
		}
		int option = menu.getDecision();
		consoleClear();
		printf("Option is %d\n",option);
		pressToContinue(true);
	}



	////////////////////
	int offset;
	if(pressedKeyOnBoot(KEY_A | KEY_B)){
		int lastOffset = trySaveGame();
		offset = askMBOffset(lastOffset);
	}

	if(!autoStartGame()){//试图自动开始游戏
		offset = askMBOffset(-1);
	}

	// printf("Manully boot Offset set = %d MB",offset);

	while (1) {
		VBlankIntrWait();
	}
}




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
#include "font.h"
volatile static char Dummy_SaveType[]="SRAM_V113";//让存档管理器识别为SRAM存档，虽然可能认为只有256K.....
	
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
IWRAM_CODE int main(void) {
//---------------------------------------------------------------------------------


	char DONT_OPT_MY_SAVE_TYPE_STR = Dummy_SaveType[0];	//让编译器不优化掉n
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	SetMode (MODE_3 | BG2_ENABLE );
	fbInit();
	REG_IME = 1;
	backupSramLite();
	//////////////////////



	////////////////////
	if(pressedKeyOnBoot(KEY_L | KEY_R)){
		int lastOffset = trySaveGame();
		askMBOffset(lastOffset);
	}

	if(!autoStartGame()){//试图自动开始游戏
		printf_zh("未找到上次运行的游戏\n");
		pressToContinue(true);
		askMBOffset(-1);
	}

	// printf_zh("Manully boot Offset set = %d MB",offset);

	while (1) {
		VBlankIntrWait();
	}
}




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
	backupSram();
	int offset;
	if(!pressedKeyOnBoot(KEY_A | KEY_B)){
		if(!autoStartGame()){
			offset = askMBOffset();
		}
	}
	offset = askMBOffset();

	printf("Manully boot Offset set = %d MB",offset);

	while (1) {
		VBlankIntrWait();
	}
}



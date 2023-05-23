#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gba.h>
#include "my_font.h"


int halGetScreenWidth();

int halGetScreenHeight();

void halDrawPixel(int x, int y, u16 color);

const char *halGetFontDataPtr();
void halClearPixel();
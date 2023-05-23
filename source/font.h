#pragma once
#include "hal.h"
#include <stdarg.h>

#define RGB(r,g,b) ((r)+(g<<5)+(b<<10))

struct Cord{
  u16 x,y;
};
typedef struct _FB_FONT {
  int valid;
  int charWidth;
  int charHeight;
  int charDataSize;
  int pageSize;
  u8 *pData;
  u8 *pIndex;
  u8 *pCharData;
} FB_FONT;

void setPos(u16 x, u16 y);

void fbInit();
void fbDrawUtf8String(const char *utf8Str, u16 color);
void printf_zh(const char *format, ...);
void printf_zh_color(u16 color,const char *format, ...);
void clearConsole();
//******************************************************************************
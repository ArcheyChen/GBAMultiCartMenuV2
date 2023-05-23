#include "font.h"
#include "hal.h"
FB_FONT fbFontCJK16;
FB_FONT *fbCurrentFont;
Cord currentCord;
void setPos(u16 x, u16 y){
  currentCord.x = x;
  currentCord.y = y;
}
static int fbLoadFontData(FB_FONT *pFont, const char *fontData) {
  memset(pFont, 0, sizeof(FB_FONT));
  pFont->valid = 1;//跳过了对魔数的检查，就这样吧
  pFont->charWidth = *(u8 *)(fontData + 13);
  pFont->charHeight = *(u8 *)(fontData + 14);
  pFont->charDataSize = 1 + ((pFont->charWidth + 7) / 8) * pFont->charHeight;
  pFont->pageSize = pFont->charDataSize * 256;
  pFont->pData = (u8 *)fontData;
  pFont->pIndex = pFont->pData + 16;
  pFont->pCharData = pFont->pIndex + 256;
  return !pFont->valid;
}

void fbInit() {
  FB_FONT *pFont = &fbFontCJK16;
  const char *fontDataBuf = halGetFontDataPtr();
  fbLoadFontData(pFont, fontDataBuf);
  setPos(0,0);
  fbCurrentFont = pFont;
}

static int fbDrawUnicodeRune(u32 rune,u16 color) {
  if (!fbCurrentFont) {
    return 0;
  }
  // int fontW = fbCurrentFont->charWidth;
  int fontH = fbCurrentFont->charHeight;
  int fontCharSize = fbCurrentFont->charDataSize;
  int fontPageSize = fbCurrentFont->pageSize;

  int screenW = halGetScreenWidth();
  int screenH = halGetScreenHeight();
  rune = (u16)(rune);
  if (rune == '\n') {
    currentCord.y += fontH + 2;
    currentCord.x = 0;
    return 0;
  }
  u8 pgOffset = fbCurrentFont->pIndex[rune >> 8];
  if (pgOffset == 0xFF) {
    return 0;
  }
  u8 *ptr = fbCurrentFont->pCharData + fontPageSize * pgOffset +
            fontCharSize * (rune & 0xff);
  u8 width = *ptr;
  ptr++;

  if (currentCord.x + width >= screenW) {
    currentCord.y += fontH + 2;
    currentCord.x = 0;
  }
  if (currentCord.y + fontH >= screenH) {
    return 0;
  }
  for (u8 y = 0; y < fontH; y++) {
    for (u8 x = 0; x < width; x++) {
      u8 pix = ptr[y * 2 + x / 8] & (1 << (x % 8));
      if (pix) {
        halDrawPixel(currentCord.x + x, currentCord.y + y, color);
      }
    }
  }
  currentCord.x += width + 2;
  if (currentCord.x >= screenW) {
    currentCord.y += fontH + 2;
    currentCord.x = 0;
  }
  return width;
}

void fbDrawUtf8String(const char *utf8Str, u16 color) {
  u8 *p = (u8 *)utf8Str;
  u16 rune = 0;
  while (*p) {
    rune = 0;
    u8 byte1 = *p;
    p++;
    if ((byte1 & 0x80) == 0) {
      rune = byte1;
    } else {
      u8 byte2 = *p;
      p++;
      if (byte2 == 0) {
        break;
      }
      if ((byte1 & 0xE0) == 0xC0) {
        rune = ((byte1 & 0x1F) << 6) | (byte2 & 0x3F);
      } else {
        u8 byte3 = *p;
        p++;
        if (byte3 == 0) {
          break;
        }
        if ((byte1 & 0xf0) == 0xE0) {
          rune =
              ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
        } else {
          break;
        }
      }
    }
    fbDrawUnicodeRune(rune,color);
  }
}
char printf_str_buffer[1024];//一个字最多3B，1K应该不会爆了吧
void printf_zh(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    vsprintf(printf_str_buffer, format, va);
    va_end(va);
    fbDrawUtf8String(printf_str_buffer,RGB(31,31,31));
}
void printf_zh_color(u16 color,const char *format, ...)
{
    va_list va;
    va_start(va, format);
    vsprintf(printf_str_buffer, format, va);
    va_end(va);
    fbDrawUtf8String(printf_str_buffer,color);
}
void clearConsole(){
  halClearPixel();
  setPos(0,0);  
}
//******************************************************************************
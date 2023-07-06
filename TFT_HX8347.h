/***************************************************
  Arduino TFT graphics library targetted at the UNO
  and Mega boards.

  This library has been derived from the Adafruit_GFX
  library and the associated driver library. See text
  at the end of this file.

  This is a standalone library that contains the
  hardware driver, the graphics funtions and the
  proportional fonts.

  The larger fonts are Run Length Encoded to reduce
  their FLASH footprint.

 ****************************************************/

// Include header file that defines the fonts loaded and the pins to be used
#include "User_Setup.h"

// Include header file from the FastLED library for fast pin toggling using direct port access
#include "TFT_FastPin.h"

// Stop fonts being loaded multiple times
#ifndef _TFT_HX8347H_
#define _TFT_HX8347H_

// Only load the fonts defined in User_Setup.h (to save space)
// Set flag so RLE rendering code is optionally compiled
#ifdef LOAD_GLCD
  #include "Fonts/glcdfont.c"
#endif

#ifdef LOAD_FONT2
  #include "Fonts/Font16.h"
#endif

#ifdef LOAD_FONT4
  #include "Fonts/Font32rle.h"
  #define LOAD_RLE
#endif

#ifdef LOAD_FONT6
  #include "Fonts/Font64rle.h"
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#ifdef LOAD_FONT7
  #include "Fonts/Font7srle.h"
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#ifdef LOAD_FONT8
  #include "Fonts/Font72rle.h"
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#include <Arduino.h>
#include <Print.h>

#include <avr/pgmspace.h>

// Swap any type
template <typename T> static inline void
swap(T& a, T& b) { T t = a; a = b; b = t; }

//These define the ports and port bits used for the chip select (CS) and data/command (DC) lines
#define TFT_CS_L	FastPin<TFT_CS>::lo()
#define TFT_DC_C 	FastPin<TFT_DC>::lo()
#define TFT_CS_H	FastPin<TFT_CS>::hi()
#define TFT_DC_D 	FastPin<TFT_DC>::hi()

//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right


// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#define HX8347_TFTWIDTH  240
#define HX8347_TFTHEIGHT 320

#define HX8347_INIT_DELAY 0x80

// These are the HX8347 control registers
#define HX8347_DISP_MODE   0x01

#define HX8347_START_COL_2 0x02
#define HX8347_START_COL_1 0x03
#define HX8347_END_COL_2   0x04
#define HX8347_END_COL_1   0x05
#define HX8347_START_ROW_2 0x06
#define HX8347_START_ROW_1 0x07
#define HX8347_END_ROW_2   0x08
#define HX8347_END_ROW_1   0x09

#define HX8347_MEM_ACC_CTRL   0x16

#define HX8347_GRAM_WRITE   0x22

//Masks for bit access
#define HX8347_MEM_ACC_CTRL_MY (1<<7)
#define HX8347_MEM_ACC_CTRL_MX (1<<6)
#define HX8347_MEM_ACC_CTRL_MV (1<<5)
#define HX8347_MEM_ACC_CTRL_ML (1<<4)
#define HX8347_MEM_ACC_CTRL_BGR (1<<3)


#define HX8347_DISP_MODE_INV   (1<<1)


// New color definitions use for all my libraries
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
#define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define TFT_PINK        0xF81F

// Color definitions for backwards compatibility
#define HX8347_BLACK       0x0000      /*   0,   0,   0 */
#define HX8347_NAVY        0x000F      /*   0,   0, 128 */
#define HX8347_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define HX8347_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define HX8347_MAROON      0x7800      /* 128,   0,   0 */
#define HX8347_PURPLE      0x780F      /* 128,   0, 128 */
#define HX8347_OLIVE       0x7BE0      /* 128, 128,   0 */
#define HX8347_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define HX8347_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define HX8347_BLUE        0x001F      /*   0,   0, 255 */
#define HX8347_GREEN       0x07E0      /*   0, 255,   0 */
#define HX8347_CYAN        0x07FF      /*   0, 255, 255 */
#define HX8347_RED         0xF800      /* 255,   0,   0 */
#define HX8347_MAGENTA     0xF81F      /* 255,   0, 255 */
#define HX8347_YELLOW      0xFFE0      /* 255, 255,   0 */
#define HX8347_WHITE       0xFFFF      /* 255, 255, 255 */
#define HX8347_ORANGE      0xFD20      /* 255, 165,   0 */
#define HX8347_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define HX8347_PINK        0xF81F

typedef struct {
	const unsigned char *chartbl;
	const unsigned char *widthtbl;
	unsigned       char height;
	} fontinfo;

// This is a structure to conveniently hold infomation on the fonts
// Stores font character image address pointer, width table and height

const PROGMEM fontinfo fontdata [] = {
   { 0, 0, 0 },

   { 0, 0, 8 },

  #ifdef LOAD_FONT2
   { (const unsigned char *)chrtbl_f16, widtbl_f16, chr_hgt_f16},
  #else
   { 0, 0, 0 },
  #endif

   { 0, 0, 0 },

  #ifdef LOAD_FONT4
   { (const unsigned char *)chrtbl_f32, widtbl_f32, chr_hgt_f32},
  #else
   { 0, 0, 0 },
  #endif

   { 0, 0, 0 },

  #ifdef LOAD_FONT6
   { (const unsigned char *)chrtbl_f64, widtbl_f64, chr_hgt_f64},
  #else
   { 0, 0, 0 },
  #endif

  #ifdef LOAD_FONT7
   { (const unsigned char *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s},
  #else
   { 0, 0, 0 },
  #endif

  #ifdef LOAD_FONT8
   { (const unsigned char *)chrtbl_f72, widtbl_f72, chr_hgt_f72}
  #else
   { 0, 0, 0 }
  #endif
};


// Class functions and variables
class TFT_HX8347 : public Print {

 public:

  TFT_HX8347(int16_t _W = HX8347_TFTWIDTH, int16_t _H = HX8347_TFTHEIGHT);

  void     init(void), begin(void), // Same - begin included for backwards compatibility

           drawPixel(uint16_t x, uint16_t y, uint16_t color),
           fastPixel(uint16_t x, uint16_t y, uint16_t color),
           fastSetup(void),

           drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t font),
           setWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1),

           pushColor(uint16_t color),
           pushColor(uint16_t color, uint16_t len),

           pushColors(uint16_t *data, uint8_t len),
           pushColors(uint8_t  *data, uint16_t len),

           fillScreen(uint16_t color),

           writeEnd(void),
           backupSPCR(void),
           restoreSPCR(void),

           //spiWait17(void),
           //spiWait15(void),
           //spiWait14(void),

           drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
           drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
           drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),

           drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
           fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
           drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
           fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),

           setRotation(uint8_t r),
           invertDisplay(boolean i),

           drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
           drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color),
           fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
           fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color),

           drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color),
           fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color),

           drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),
           fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),

           drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color),

           setCursor(int16_t x, int16_t y),
           setCursor(int16_t x, int16_t y, uint8_t font),
           setTextColor(uint16_t color),
           setTextColor(uint16_t fgcolor, uint16_t bgcolor),
           setTextSize(uint8_t size),
           setTextFont(uint8_t font),
           setTextWrap(boolean wrap),
           setTextDatum(uint8_t datum),
           setTextPadding(uint16_t x_width),

           spiwrite(uint8_t),
           writecommand(uint8_t c),
           writedata(uint8_t d),
           commandList(const uint8_t *addr);

  uint8_t  getRotation(void);

  uint16_t fontsLoaded(void),
           color565(uint8_t r, uint8_t g, uint8_t b);

  int16_t  drawChar(unsigned int uniCode, int x, int y, int font),
           drawNumber(long long_num,int poX, int poY, int font),
           drawFloat(float floatNumber,int decimal,int poX, int poY, int font),

           drawString(const char *string, int poX, int poY, int font),
           drawCentreString(const char *string, int dX, int poY, int font),
           drawRightString(const char *string, int dX, int poY, int font),

           height(void),
           width(void),
           textWidth(const char *string, int font),
           fontHeight(int font);

//    void  spiWrite16(uint16_t data, int16_t count);
//    void  spiWrite16s(uint16_t data);
//    void  spiWrite16R(uint16_t data, int16_t count);

 virtual  size_t write(uint8_t);

 private:

    void  setAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

  uint8_t  tabcolor;

  boolean  hwSPI;

  uint8_t  mySPCR, savedSPCR;

 protected:

  int16_t  cursor_x, cursor_y, win_xe, win_ye, padX;

  uint16_t _width, _height, // Display w/h as modified by current rotation
           textcolor, textbgcolor, fontsloaded, addr_row, addr_col;

  uint8_t  textfont,
           textsize,
           textdatum,
           rotation;

  boolean  textwrap; // If set, 'wrap' text at right edge of display

};

#endif

/***************************************************

  ORIGINAL LIBRARY HEADER

  This is our library for the Adafruit  HX8347 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  Updated with new functions by Bodmer 14/4/15
 ****************************************************/

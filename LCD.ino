// ************************************************************************************************************
// LCD & display & monitoring
// ************************************************************************************************************
// in any of the following cases an LCD is required and
// the primitives for exactly one of the available types are setup
#if defined(LCD_CONF) || defined(LCD_TELEMETRY) || defined(HAS_LCD)

static char line1[18],line2[18];
#if defined(OSD_DISPLAY)
  static char osd_line[22];
  #undef DISPLAY_FONT_DSIZE
  #undef LCD_TELEMETRY
#endif
uint8_t font;    // OLED FONT SIZE -> 0=SMALL 5x8, 1=BIG 8x14, OSD_DISPLAY -> line attributes
#ifdef DISPLAY_FONT_DSIZE
  static uint8_t line_is_valid = 0;
#endif
#if ( defined(LOG_PERMANENT) && defined(DISPLAY_MULTILINE) )
  static uint8_t lnr = 0;
#endif

char digit10000(uint16_t v) {return '0' + v / 10000;}
char digit1000(uint16_t v) {return '0' + v / 1000 - (v/10000) * 10;}
char digit100(uint16_t v) {return '0' + v / 100 - (v/1000) * 10;}
char digit10(uint16_t v) {return '0' + v / 10 - (v/100) * 10;}
char digit1(uint16_t v) {return '0' + v - (v/10) * 10;}

#if defined(OLED_I2C_128x64)
// ########################################
// #  i2c OLED display funtion primitives #
// ########################################
#define OLED_address   0x3C     // OLED at address 0x3C in 7bit
char LINE_FILL_STRING[] = "                      "; // Used by clear_OLED() 128 bits / 6 bytes = 21 chars per row  
unsigned char CHAR_FORMAT = 0;      // use to INVERSE characters
uint8_t ocol,orow;

#ifndef SUPPRESS_OLED_I2C_128x64LOGO       // Do we want the Logo displayed ?
const uint8_t PROGMEM LOGO[] = { // logo....
    0x00, 0x00, 0x02, 0xFE, 0xFE, 0x0E, 0xFC, 0xF8, 0xC0, 0x00, 0xC0, 0xF8, 0xFC, 0x0E, 0xFE, 0xFE,
    0xFE, 0x02, 0x00, 0x00, 0x30, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x30, 0xF0, 0xF0, 0x00, 0x00, 0x00,
    0x02, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x30, 0xF8, 0xFE, 0x30, 0x30, 0x30, 0x00, 0x00, 0x30, 0xF6,
    0xF6, 0x00, 0x00, 0x00, 0x02, 0x06, 0x1E, 0xFE, 0xFE, 0xC2, 0x00, 0xC2, 0xFE, 0x7E, 0xFE, 0xC2,
    0x00, 0xC2, 0xFE, 0xFE, 0x3E, 0x06, 0x02, 0x00, 0x30, 0xF6, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x30,
    0xF6, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x1F, 0x1F, 0x10, 0x00, 0x83, 0x9F, 0x9F, 0x9F, 0x83, 0x80, 0x90, 0x9F, 0x9F,
    0x9F, 0x10, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x18, 0x18, 0x18, 0x0C, 0x1F, 0x1F, 0x10, 0x00, 0x00,
    0x10, 0x1F, 0x1F, 0x10, 0x00, 0x80, 0x80, 0x8F, 0x9F, 0x98, 0x9E, 0x8F, 0x80, 0x80, 0x90, 0x1F,
    0x1F, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x1E, 0x1F, 0x03, 0x00, 0x07, 0x1F,
    0x1E, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x1F, 0x1F, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x1F, 0x1F, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xE0, 0xF8, 0x3C, 0x0E, 0x07, 0x03, 0x03, 0x01, 0x81, 0xC1, 0xC1, 0xC1, 0xC1, 0x81, 0x01,
    0x03, 0x03, 0x07, 0x0E, 0x3C, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
    0xF8, 0x3C, 0x0E, 0x07, 0x03, 0x03, 0x01, 0x81, 0xC1, 0xC1, 0xC1, 0xC1, 0x81, 0x01, 0x03, 0x03,
    0x07, 0x0E, 0x3C, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x3E, 0xF8, 0xC0,
    0xFC, 0x0E, 0xFC, 0xC0, 0xF8, 0x3E, 0x06, 0x00, 0xFE, 0xFE, 0x00, 0x06, 0xFF, 0xFF, 0x86, 0x86,
    0x00, 0xFF, 0xFF, 0x0C, 0x06, 0x06, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0x7F, 0xF0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0C, 0x0C, 0x0F, 0x07, 0x00,
    0x00, 0x00, 0x80, 0xC0, 0xF0, 0x7F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
    0x7F, 0xF0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0C, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00,
    0x80, 0xC0, 0xF0, 0x7F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xC1, 0xC1,
    0x80, 0x80, 0x00, 0x01, 0x01, 0x00, 0x00, 0xC0, 0xC1, 0xC1, 0xC0, 0xC0, 0xC0, 0xC1, 0xC1, 0xC1,
    0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x81, 0x81, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x03, 0x03, 0x01, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0xF0, 0xE0, 0xE0, 0xF0, 0x38, 0x1C, 0x0E,
    0x07, 0x03, 0x01, 0x03, 0x03, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x03,
    0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFE, 0xFF, 0x07, 0x03, 0x01, 0x01, 0xE1, 0xE1, 0xE1,
    0xE3, 0xE7, 0xE7, 0xE6, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x71, 0x71, 0x71, 0x71, 0x7B, 0x7F,
    0x3F, 0x1F, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x3F, 0x39, 0x71, 0x71, 0x71, 0xE3, 0xE7, 0xE7, 0x86,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0xC0, 0xE0, 0x70, 0x30, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x38, 0x30, 0x70, 0xE0, 0xF0, 0xB8, 0x1C, 0x0E, 0x07, 0x03, 0x01, 0x01, 0x03, 0x07, 0x0E, 0x1C,
    0xB8, 0xF0, 0xE0, 0x70, 0x30, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x38, 0x30,
    0x70, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x0E, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x0E, 0x0F, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x0F, 0x1E, 0x1C, 0x1C, 0x1C, 0x1C, 0x1E, 0x0F, 0x0F, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFE, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFC, 0xCC, 0xCC, 0xFC, 0x78, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
    0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFC, 0xCC, 0xCC, 0xFC, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x20, 0x40,
    0x80, 0x00, 0x60, 0x80, 0x00, 0x00, 0x00, 0x80, 0x60, 0x00, 0xF0, 0x08, 0x04, 0x04, 0x04, 0x08,
    0xF0, 0x00, 0x00, 0xE0, 0x5C, 0x44, 0x44, 0x44, 0x84, 0x04, 0x00, 0x00, 0x10, 0x08, 0x04, 0x04,
    0x04, 0x8C, 0x70, 0x00, 0x00, 0x30, 0x48, 0x84, 0x84, 0x84, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x07, 0x0F, 0x1C, 0x38, 0x30, 0x70, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
    0x70, 0x30, 0x38, 0x1C, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x07, 0x0F, 0x1C, 0x38, 0x30, 0x70, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x70, 0x30,
    0x38, 0x1C, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x08, 0x07, 0x00, 0x00, 0x0F, 0x12, 0x22, 0x22, 0x22, 0x12,
    0x0B, 0x00, 0x00, 0x01, 0x0E, 0x30, 0x0E, 0x01, 0x00, 0x00, 0x0F, 0x10, 0x20, 0x20, 0x20, 0x10,
    0x0F, 0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00, 0x00, 0x20, 0x30, 0x28, 0x24,
    0x22, 0x21, 0x20, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, 0x00, 0x00, 0x00

};
#endif
const uint8_t PROGMEM Font5x7[][5] = { // Refer to "Times New Roman" Font Database... 5 x 7 font
  { 0x00,0x00,0x00,0x00,0x00},
  { 0x00,0x00,0x4F,0x00,0x00}, //   (  1)  ! - 0x0021 Exclamation Mark
  { 0x00,0x07,0x00,0x07,0x00}, //   (  2)  " - 0x0022 Quotation Mark
  { 0x14,0x7F,0x14,0x7F,0x14}, //   (  3)  # - 0x0023 Number Sign
  { 0x24,0x2A,0x7F,0x2A,0x12}, //   (  4)  $ - 0x0024 Dollar Sign
  { 0x23,0x13,0x08,0x64,0x62}, //   (  5)  % - 0x0025 Percent Sign
  { 0x36,0x49,0x55,0x22,0x50}, //   (  6)  & - 0x0026 Ampersand
  { 0x00,0x05,0x03,0x00,0x00}, //   (  7)  ' - 0x0027 Apostrophe
  { 0x00,0x1C,0x22,0x41,0x00}, //   (  8)  ( - 0x0028 Left Parenthesis
  { 0x00,0x41,0x22,0x1C,0x00}, //   (  9)  ) - 0x0029 Right Parenthesis
  { 0x14,0x08,0x3E,0x08,0x14}, //   ( 10)  * - 0x002A Asterisk
  { 0x08,0x08,0x3E,0x08,0x08}, //   ( 11)  + - 0x002B Plus Sign
  { 0x00,0x50,0x30,0x00,0x00}, //   ( 12)  , - 0x002C Comma
  { 0x08,0x08,0x08,0x08,0x08}, //   ( 13)  - - 0x002D Hyphen-Minus
  { 0x00,0x60,0x60,0x00,0x00}, //   ( 14)  . - 0x002E Full Stop
  { 0x20,0x10,0x08,0x04,0x02}, //   ( 15)  / - 0x002F Solidus
  { 0x3E,0x51,0x49,0x45,0x3E}, //   ( 16)  0 - 0x0030 Digit Zero
  { 0x00,0x42,0x7F,0x40,0x00}, //   ( 17)  1 - 0x0031 Digit One
  { 0x42,0x61,0x51,0x49,0x46}, //   ( 18)  2 - 0x0032 Digit Two
  { 0x21,0x41,0x45,0x4B,0x31}, //   ( 19)  3 - 0x0033 Digit Three
  { 0x18,0x14,0x12,0x7F,0x10}, //   ( 20)  4 - 0x0034 Digit Four
  { 0x27,0x45,0x45,0x45,0x39}, //   ( 21)  5 - 0x0035 Digit Five
  { 0x3C,0x4A,0x49,0x49,0x30}, //   ( 22)  6 - 0x0036 Digit Six
  { 0x01,0x71,0x09,0x05,0x03}, //   ( 23)  7 - 0x0037 Digit Seven
  { 0x36,0x49,0x49,0x49,0x36}, //   ( 24)  8 - 0x0038 Digit Eight
  { 0x06,0x49,0x49,0x29,0x1E}, //   ( 25)  9 - 0x0039 Dight Nine
  { 0x00,0x36,0x36,0x00,0x00}, //   ( 26)  : - 0x003A Colon
  { 0x00,0x56,0x36,0x00,0x00}, //   ( 27)  ; - 0x003B Semicolon
  { 0x08,0x14,0x22,0x41,0x00}, //   ( 28)  < - 0x003C Less-Than Sign
  { 0x14,0x14,0x14,0x14,0x14}, //   ( 29)  = - 0x003D Equals Sign
  { 0x00,0x41,0x22,0x14,0x08}, //   ( 30)  > - 0x003E Greater-Than Sign
  { 0x02,0x01,0x51,0x09,0x06}, //   ( 31)  ? - 0x003F Question Mark
  { 0x32,0x49,0x79,0x41,0x3E}, //   ( 32)  @ - 0x0040 Commercial At
  { 0x7E,0x11,0x11,0x11,0x7E}, //   ( 33)  A - 0x0041 Latin Capital Letter A
  { 0x7F,0x49,0x49,0x49,0x36}, //   ( 34)  B - 0x0042 Latin Capital Letter B
  { 0x3E,0x41,0x41,0x41,0x22}, //   ( 35)  C - 0x0043 Latin Capital Letter C
  { 0x7F,0x41,0x41,0x22,0x1C}, //   ( 36)  D - 0x0044 Latin Capital Letter D
  { 0x7F,0x49,0x49,0x49,0x41}, //   ( 37)  E - 0x0045 Latin Capital Letter E
  { 0x7F,0x09,0x09,0x09,0x01}, //   ( 38)  F - 0x0046 Latin Capital Letter F
  { 0x3E,0x41,0x49,0x49,0x7A}, //   ( 39)  G - 0x0047 Latin Capital Letter G
  { 0x7F,0x08,0x08,0x08,0x7F}, //   ( 40)  H - 0x0048 Latin Capital Letter H
  { 0x00,0x41,0x7F,0x41,0x00}, //   ( 41)  I - 0x0049 Latin Capital Letter I
  { 0x20,0x40,0x41,0x3F,0x01}, //   ( 42)  J - 0x004A Latin Capital Letter J
  { 0x7F,0x08,0x14,0x22,0x41}, //   ( 43)  K - 0x004B Latin Capital Letter K
  { 0x7F,0x40,0x40,0x40,0x40}, //   ( 44)  L - 0x004C Latin Capital Letter L
  { 0x7F,0x02,0x0C,0x02,0x7F}, //   ( 45)  M - 0x004D Latin Capital Letter M
  { 0x7F,0x04,0x08,0x10,0x7F}, //   ( 46)  N - 0x004E Latin Capital Letter N
  { 0x3E,0x41,0x41,0x41,0x3E}, //   ( 47)  O - 0x004F Latin Capital Letter O
  { 0x7F,0x09,0x09,0x09,0x06}, //   ( 48)  P - 0x0050 Latin Capital Letter P
  { 0x3E,0x41,0x51,0x21,0x5E}, //   ( 49)  Q - 0x0051 Latin Capital Letter Q
  { 0x7F,0x09,0x19,0x29,0x46}, //   ( 50)  R - 0x0052 Latin Capital Letter R
  { 0x46,0x49,0x49,0x49,0x31}, //   ( 51)  S - 0x0053 Latin Capital Letter S
  { 0x01,0x01,0x7F,0x01,0x01}, //   ( 52)  T - 0x0054 Latin Capital Letter T
  { 0x3F,0x40,0x40,0x40,0x3F}, //   ( 53)  U - 0x0055 Latin Capital Letter U
  { 0x1F,0x20,0x40,0x20,0x1F}, //   ( 54)  V - 0x0056 Latin Capital Letter V
  { 0x3F,0x40,0x38,0x40,0x3F}, //   ( 55)  W - 0x0057 Latin Capital Letter W
  { 0x63,0x14,0x08,0x14,0x63}, //   ( 56)  X - 0x0058 Latin Capital Letter X
  { 0x07,0x08,0x70,0x08,0x07}, //   ( 57)  Y - 0x0059 Latin Capital Letter Y
  { 0x61,0x51,0x49,0x45,0x43}, //   ( 58)  Z - 0x005A Latin Capital Letter Z
  { 0x00,0x7F,0x41,0x41,0x00}, //   ( 59)  [ - 0x005B Left Square Bracket
  { 0x02,0x04,0x08,0x10,0x20}, //   ( 60)  \ - 0x005C Reverse Solidus
  { 0x00,0x41,0x41,0x7F,0x00}, //   ( 61)  ] - 0x005D Right Square Bracket
  { 0x04,0x02,0x01,0x02,0x04}, //   ( 62)  ^ - 0x005E Circumflex Accent
  { 0x40,0x40,0x40,0x40,0x40}, //   ( 63)  _ - 0x005F Low Line
  { 0x01,0x02,0x04,0x00,0x00}, //   ( 64)  ` - 0x0060 Grave Accent
  { 0x20,0x54,0x54,0x54,0x78}, //   ( 65)  a - 0x0061 Latin Small Letter A
  { 0x7F,0x48,0x44,0x44,0x38}, //   ( 66)  b - 0x0062 Latin Small Letter B
  { 0x38,0x44,0x44,0x44,0x20}, //   ( 67)  c - 0x0063 Latin Small Letter C
  { 0x38,0x44,0x44,0x48,0x7F}, //   ( 68)  d - 0x0064 Latin Small Letter D
  { 0x38,0x54,0x54,0x54,0x18}, //   ( 69)  e - 0x0065 Latin Small Letter E
  { 0x08,0x7E,0x09,0x01,0x02}, //   ( 70)  f - 0x0066 Latin Small Letter F
  { 0x06,0x49,0x49,0x49,0x3F}, //   ( 71)  g - 0x0067 Latin Small Letter G
  { 0x7F,0x08,0x04,0x04,0x78}, //   ( 72)  h - 0x0068 Latin Small Letter H
  { 0x00,0x44,0x7D,0x40,0x00}, //   ( 73)  i - 0x0069 Latin Small Letter I
  { 0x20,0x40,0x44,0x3D,0x00}, //   ( 74)  j - 0x006A Latin Small Letter J
  { 0x7F,0x10,0x28,0x44,0x00}, //   ( 75)  k - 0x006B Latin Small Letter K
  { 0x00,0x41,0x7F,0x40,0x00}, //   ( 76)  l - 0x006C Latin Small Letter L
  { 0x7C,0x04,0x18,0x04,0x7C}, //   ( 77)  m - 0x006D Latin Small Letter M
  { 0x7C,0x08,0x04,0x04,0x78}, //   ( 78)  n - 0x006E Latin Small Letter N
  { 0x38,0x44,0x44,0x44,0x38}, //   ( 79)  o - 0x006F Latin Small Letter O
  { 0x7C,0x14,0x14,0x14,0x08}, //   ( 80)  p - 0x0070 Latin Small Letter P
  { 0x08,0x14,0x14,0x18,0x7C}, //   ( 81)  q - 0x0071 Latin Small Letter Q
  { 0x7C,0x08,0x04,0x04,0x08}, //   ( 82)  r - 0x0072 Latin Small Letter R
  { 0x48,0x54,0x54,0x54,0x20}, //   ( 83)  s - 0x0073 Latin Small Letter S
  { 0x04,0x3F,0x44,0x40,0x20}, //   ( 84)  t - 0x0074 Latin Small Letter T
  { 0x3C,0x40,0x40,0x20,0x7C}, //   ( 85)  u - 0x0075 Latin Small Letter U
  { 0x1C,0x20,0x40,0x20,0x1C}, //   ( 86)  v - 0x0076 Latin Small Letter V
  { 0x3C,0x40,0x30,0x40,0x3C}, //   ( 87)  w - 0x0077 Latin Small Letter W
  { 0x44,0x28,0x10,0x28,0x44}, //   ( 88)  x - 0x0078 Latin Small Letter X
  { 0x0C,0x50,0x50,0x50,0x3C}, //   ( 89)  y - 0x0079 Latin Small Letter Y
  { 0x44,0x64,0x54,0x4C,0x44}, //   ( 90)  z - 0x007A Latin Small Letter Z
  { 0x00,0x08,0x36,0x41,0x00}, //   ( 91)  { - 0x007B Left Curly Bracket
  { 0x00,0x00,0x7F,0x00,0x00}, //   ( 92)  | - 0x007C Vertical Line
  { 0x00,0x41,0x36,0x08,0x00}, //   ( 93)  } - 0x007D Right Curly Bracket
  { 0x02,0x01,0x02,0x04,0x02}, //   ( 94)  ~ - 0x007E Tilde
  { 0x3E,0x55,0x55,0x41,0x22}, //   ( 95)  C - 0x0080 <Control>
  { 0x00,0x00,0x00,0x00,0x00}, //   ( 96)    - 0x00A0 No-Break Space
  { 0x00,0x00,0x79,0x00,0x00}, //   ( 97)  ! - 0x00A1 Inverted Exclamation Mark
  { 0x18,0x24,0x74,0x2E,0x24}, //   ( 98)  c - 0x00A2 Cent Sign
  { 0x48,0x7E,0x49,0x42,0x40}, //   ( 99)  L - 0x00A3 Pound Sign
  { 0x5D,0x22,0x22,0x22,0x5D}, //   (100)  o - 0x00A4 Currency Sign
  { 0x15,0x16,0x7C,0x16,0x15}, //   (101)  Y - 0x00A5 Yen Sign
  { 0x00,0x00,0x77,0x00,0x00}, //   (102)  | - 0x00A6 Broken Bar
  { 0x0A,0x55,0x55,0x55,0x28}, //   (103)    - 0x00A7 Section Sign
  { 0x00,0x01,0x00,0x01,0x00}, //   (104)  " - 0x00A8 Diaeresis
  { 0x00,0x0A,0x0D,0x0A,0x04}, //   (105)    - 0x00AA Feminine Ordinal Indicator
  { 0x08,0x14,0x2A,0x14,0x22}, //   (106) << - 0x00AB Left-Pointing Double Angle Quotation Mark
  { 0x04,0x04,0x04,0x04,0x1C}, //   (107)    - 0x00AC Not Sign
  { 0x00,0x08,0x08,0x08,0x00}, //   (108)  - - 0x00AD Soft Hyphen
  { 0x01,0x01,0x01,0x01,0x01}, //   (109)    - 0x00AF Macron
  { 0x00,0x02,0x05,0x02,0x00}, //   (110)    - 0x00B0 Degree Sign
  { 0x44,0x44,0x5F,0x44,0x44}, //   (111) +- - 0x00B1 Plus-Minus Sign
  { 0x00,0x00,0x04,0x02,0x01}, //   (112)  ` - 0x00B4 Acute Accent
  { 0x7E,0x20,0x20,0x10,0x3E}, //   (113)  u - 0x00B5 Micro Sign
  { 0x06,0x0F,0x7F,0x00,0x7F}, //   (114)    - 0x00B6 Pilcrow Sign
  { 0x00,0x18,0x18,0x00,0x00}, //   (115)  . - 0x00B7 Middle Dot
  { 0x00,0x40,0x50,0x20,0x00}, //   (116)    - 0x00B8 Cedilla
  { 0x00,0x0A,0x0D,0x0A,0x00}, //   (117)    - 0x00BA Masculine Ordinal Indicator
  { 0x22,0x14,0x2A,0x14,0x08}, //   (118) >> - 0x00BB Right-Pointing Double Angle Quotation Mark
  { 0x17,0x08,0x34,0x2A,0x7D}, //   (119) /4 - 0x00BC Vulgar Fraction One Quarter
  { 0x17,0x08,0x04,0x6A,0x59}, //   (120) /2 - 0x00BD Vulgar Fraction One Half
  { 0x30,0x48,0x45,0x40,0x20}, //   (121)  ? - 0x00BF Inverted Question Mark
};

const uint8_t PROGMEM Font8x14[][14] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 
	0x00,0x00,0x1C,0xFE,0xFE,0x1C,0x00,0x00,0x00,0x00,0x06,0x06,0x00,0x00,	//!
	0x00,0x07,0x0F,0x00,0x00,0x0F,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//"
	0x10,0xFC,0xFC,0x10,0xFC,0xFC,0x10,0x01,0x07,0x07,0x01,0x07,0x07,0x01,	//#
	0x9C,0x3E,0x22,0x23,0x23,0xE6,0xCC,0x01,0x03,0x02,0x0E,0x0E,0x03,0x01,	//$
	0x18,0x18,0x80,0xC0,0x60,0x30,0x18,0x06,0x03,0x01,0x00,0x00,0x06,0x06,	//%
	0xC0,0xEC,0x3E,0x72,0xDE,0xEC,0x20,0x03,0x07,0x04,0x04,0x03,0x07,0x04,	//&
	0x00,0x08,0x0F,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//'
	0x00,0x00,0xF8,0xFC,0x06,0x02,0x00,0x00,0x00,0x01,0x03,0x06,0x04,0x00,	//(
	0x00,0x00,0x02,0x06,0xFC,0xF8,0x00,0x00,0x00,0x04,0x06,0x03,0x01,0x00,	//)
	0x40,0x50,0xF0,0xE0,0xF0,0x50,0x00,0x00,0x01,0x01,0x00,0x01,0x01,0x00,	//*
	0x00,0x40,0x40,0xF0,0xF0,0x40,0x40,0x00,0x00,0x00,0x01,0x01,0x00,0x00,	//+
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x0F,0x07,0x00,0x00,	//,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x06,0x00,0x00,	//.
	0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0x06,0x03,0x01,0x00,0x00,0x00,0x00,	///
	0xFC,0xFE,0x82,0x62,0x12,0xFE,0xFC,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//0
	0x00,0x08,0x0C,0xFE,0xFE,0x00,0x00,0x00,0x04,0x04,0x07,0x07,0x04,0x04,	//1
	0x04,0x86,0xC2,0x62,0x32,0x1E,0x0C,0x07,0x07,0x04,0x04,0x04,0x06,0x06,	//2
	0x04,0x06,0x22,0x22,0x22,0xFE,0xDC,0x02,0x06,0x04,0x04,0x04,0x07,0x03,	//3
	0x60,0x70,0x58,0x4C,0xFE,0xFE,0x40,0x00,0x00,0x00,0x04,0x07,0x07,0x04,	//4
	0x3E,0x3E,0x22,0x22,0x62,0xE2,0xC2,0x02,0x06,0x04,0x04,0x04,0x07,0x03,	//5
	0xF8,0xFC,0x26,0x22,0x22,0xE0,0xC0,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//6
	0x06,0x06,0x82,0xC2,0x62,0x3E,0x1E,0x00,0x00,0x07,0x07,0x00,0x00,0x00,	//7
	0xDC,0xFE,0x22,0x22,0x22,0xFE,0xDC,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//8
	0x1C,0x3E,0x22,0x22,0x22,0xFE,0xFC,0x00,0x04,0x04,0x04,0x06,0x03,0x01,	//9
	0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x00,	//:
	0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x04,0x07,0x03,0x00,0x00,	//;
	0x00,0x40,0xE0,0xB0,0x18,0x0C,0x04,0x00,0x00,0x00,0x01,0x03,0x06,0x04,	//<
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	//=
	0x00,0x04,0x0C,0x18,0xB0,0xE0,0x40,0x00,0x04,0x06,0x03,0x01,0x00,0x00,	//>
	0x0C,0x0E,0x02,0xE2,0xF2,0x1E,0x0C,0x00,0x00,0x00,0x06,0x06,0x00,0x00,	//?
	0xF8,0xFC,0x04,0xE4,0xE4,0xFC,0xF8,0x03,0x07,0x04,0x05,0x05,0x05,0x00,	//@
	0xF0,0xF8,0x4C,0x46,0x4C,0xF8,0xF0,0x07,0x07,0x00,0x00,0x00,0x07,0x07,	//A
	0x02,0xFE,0xFE,0x22,0x22,0xFE,0xDC,0x04,0x07,0x07,0x04,0x04,0x07,0x03,	//B
	0xF8,0xFC,0x06,0x02,0x02,0x06,0x0C,0x01,0x03,0x06,0x04,0x04,0x06,0x03,	//C
	0x02,0xFE,0xFE,0x02,0x06,0xFC,0xF8,0x04,0x07,0x07,0x04,0x06,0x03,0x01,	//D
	0x02,0xFE,0xFE,0x22,0x72,0x06,0x0E,0x04,0x07,0x07,0x04,0x04,0x06,0x07,	//E
	0x02,0xFE,0xFE,0x22,0x72,0x06,0x0E,0x04,0x07,0x07,0x04,0x00,0x00,0x00,	//F
	0xF8,0xFC,0x06,0x42,0x42,0xC6,0xCC,0x01,0x03,0x06,0x04,0x04,0x03,0x07,	//G
	0xFE,0xFE,0x20,0x20,0x20,0xFE,0xFE,0x07,0x07,0x00,0x00,0x00,0x07,0x07,	//H
	0x00,0x00,0x02,0xFE,0xFE,0x02,0x00,0x00,0x00,0x04,0x07,0x07,0x04,0x00,	//I
	0x80,0x80,0x00,0x02,0xFE,0xFE,0x02,0x03,0x07,0x04,0x04,0x07,0x03,0x00,	//J
	0x02,0xFE,0xFE,0x60,0xF8,0x9E,0x06,0x04,0x07,0x07,0x00,0x00,0x07,0x07,	//K
	0x02,0xFE,0xFE,0x02,0x00,0x00,0x00,0x04,0x07,0x07,0x04,0x04,0x06,0x07,	//L
	0xFE,0xFE,0x1C,0x38,0x1C,0xFE,0xFE,0x07,0x07,0x00,0x00,0x00,0x07,0x07,	//M
	0xFE,0xFE,0x1C,0x38,0x70,0xFE,0xFE,0x07,0x07,0x00,0x00,0x00,0x07,0x07,	//N
	0xF8,0xFC,0x06,0x02,0x06,0xFC,0xF8,0x01,0x03,0x06,0x04,0x06,0x03,0x01,	//O
	0x02,0xFE,0xFE,0x22,0x22,0x3E,0x1C,0x04,0x07,0x07,0x04,0x00,0x00,0x00,	//P
	0xFC,0xFE,0x02,0x02,0x02,0xFE,0xFC,0x03,0x07,0x04,0x07,0x1E,0x1F,0x13,	//Q
	0x02,0xFE,0xFE,0x22,0x62,0xFE,0x9C,0x04,0x07,0x07,0x00,0x00,0x07,0x07,	//R
	0x0C,0x1E,0x32,0x22,0x62,0xCE,0x8C,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//S
	0x00,0x0E,0x06,0xFE,0xFE,0x06,0x0E,0x00,0x00,0x04,0x07,0x07,0x04,0x00,	//T
	0xFE,0xFE,0x00,0x00,0x00,0xFE,0xFE,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//U
	0xFE,0xFE,0x00,0x00,0x00,0xFE,0xFE,0x00,0x01,0x03,0x06,0x03,0x01,0x00,	//V
	0xFE,0xFE,0x00,0xC0,0x00,0xFE,0xFE,0x01,0x07,0x07,0x01,0x07,0x07,0x01,	//W
	0x06,0x9E,0xF8,0x60,0xF8,0x9E,0x06,0x06,0x07,0x01,0x00,0x01,0x07,0x06,	//X
	0x00,0x1E,0x3E,0xE0,0xE0,0x3E,0x1E,0x00,0x00,0x04,0x07,0x07,0x04,0x00,	//Y
	0x0E,0x86,0xC2,0x62,0x32,0x1E,0x0E,0x07,0x07,0x04,0x04,0x04,0x06,0x07,	//Z
	0x00,0x00,0xFE,0xFE,0x02,0x02,0x00,0x00,0x00,0x07,0x07,0x04,0x04,0x00,	//[
	0x1C,0x38,0x70,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,	//
	0x00,0x00,0x02,0x02,0xFE,0xFE,0x00,0x00,0x00,0x04,0x04,0x07,0x07,0x00,	//]
	0x04,0x06,0x03,0x01,0x03,0x06,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//^
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,	//_
	0x00,0x00,0x01,0x03,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//`
	0x80,0xD0,0x50,0x50,0xF0,0xE0,0x00,0x03,0x07,0x04,0x04,0x03,0x07,0x04,	//a
	0x02,0xFE,0xFE,0x10,0x30,0xE0,0xC0,0x04,0x07,0x03,0x04,0x04,0x07,0x03,	//b
	0xE0,0xF0,0x10,0x10,0x10,0x30,0x20,0x03,0x07,0x04,0x04,0x04,0x06,0x02,	//c
	0xC0,0xE0,0x30,0x12,0xFE,0xFE,0x00,0x03,0x07,0x04,0x04,0x03,0x07,0x04,	//d
	0xE0,0xF0,0x50,0x50,0x50,0x70,0x60,0x03,0x07,0x04,0x04,0x04,0x06,0x02,	//e
	0x20,0xFC,0xFE,0x22,0x06,0x0C,0x00,0x04,0x07,0x07,0x04,0x00,0x00,0x00,	//f
	0xE0,0xF0,0x10,0x10,0xE0,0xF0,0x10,0x13,0x37,0x24,0x24,0x3F,0x1F,0x00,	//g
	0x02,0xFE,0xFE,0x20,0x10,0xF0,0xE0,0x04,0x07,0x07,0x00,0x00,0x07,0x07,	//h
	0x00,0x00,0x10,0xF6,0xF6,0x00,0x00,0x00,0x00,0x04,0x07,0x07,0x04,0x00,	//i
	0x00,0x00,0x00,0x00,0x10,0xF6,0xF6,0x00,0x18,0x38,0x20,0x20,0x3F,0x1F,	//j
	0x02,0xFE,0xFE,0xC0,0xE0,0x30,0x10,0x04,0x07,0x07,0x00,0x01,0x07,0x06,	//k
	0x00,0x00,0x02,0xFE,0xFE,0x00,0x00,0x00,0x00,0x04,0x07,0x07,0x04,0x00,	//l
	0xF0,0xF0,0x30,0xE0,0x30,0xF0,0xE0,0x07,0x07,0x00,0x07,0x00,0x07,0x07,	//m
	0x10,0xF0,0xE0,0x10,0x10,0xF0,0xE0,0x00,0x07,0x07,0x00,0x00,0x07,0x07,	//n
	0xE0,0xF0,0x10,0x10,0x10,0xF0,0xE0,0x03,0x07,0x04,0x04,0x04,0x07,0x03,	//o
	0x10,0xF0,0xE0,0x10,0x10,0xF0,0xE0,0x20,0x3F,0x3F,0x24,0x04,0x07,0x03,	//p
	0xE0,0xF0,0x10,0x10,0xE0,0xF0,0x10,0x03,0x07,0x04,0x24,0x3F,0x3F,0x20,	//q
	0x10,0xF0,0xE0,0x10,0x10,0x70,0x60,0x00,0x07,0x07,0x00,0x00,0x00,0x00,	//r
	0x20,0x70,0xD0,0x90,0x90,0x30,0x20,0x02,0x06,0x04,0x04,0x05,0x07,0x02,	//s
	0x10,0x10,0xFC,0xFE,0x10,0x10,0x00,0x00,0x00,0x03,0x07,0x04,0x06,0x02,	//t
	0xF0,0xF0,0x00,0x00,0xF0,0xF0,0x00,0x03,0x07,0x04,0x04,0x03,0x07,0x04,	//u
	0x00,0xF0,0xF0,0x00,0x00,0xF0,0xF0,0x00,0x01,0x03,0x06,0x06,0x03,0x01,	//v
	0xF0,0xF0,0x00,0x80,0x00,0xF0,0xF0,0x03,0x07,0x06,0x03,0x06,0x07,0x03,	//w
	0x10,0x30,0xE0,0xC0,0xE0,0x30,0x10,0x04,0x06,0x03,0x01,0x03,0x06,0x04,	//x
	0xF0,0xF0,0x00,0x00,0x00,0xF0,0xF0,0x23,0x27,0x24,0x24,0x34,0x1F,0x0F,	//y
	0x30,0x30,0x90,0xD0,0x70,0x30,0x10,0x06,0x07,0x05,0x04,0x04,0x06,0x06	//z
};

void i2c_OLED_send_cmd(uint8_t command) {
  //TWBR = ((F_CPU / 400000L) - 16) / 2; // change the I2C clock rate
  i2c_writeReg(OLED_address, 0x80, (uint8_t)command);
}

void i2c_OLED_data_start() {
  //TWBR = ((F_CPU / 400000L) - 16) / 2; // change the I2C clock rate
  i2c_rep_start(OLED_address<<1);
  i2c_write(0x40);
}

void  i2c_OLED_init(void){
  i2c_OLED_send_cmd(0xae);        //display off
  i2c_OLED_send_cmd(0xa4);        //SET All pixels OFF
  i2c_OLED_send_cmd(0x20);        //Set Memory Addressing Mode
  i2c_OLED_send_cmd(0x02);        //Set Memory Addressing Mode to Page addressing mode(RESET)
  i2c_OLED_send_cmd(0xa1);        //colum address 127 mapped to SEG0 (POR) ** wires at top of board
  i2c_OLED_send_cmd(0xC8);        // Scan from Left to Right               ** wires at top
  i2c_OLED_send_cmd(0xa6);        // Set WHITE chars on BLACK backround
  i2c_OLED_send_cmd(0x81);        // Setup CONTRAST CONTROL, following byte is the contrast Value
  i2c_OLED_send_cmd(0xaf);        // contrast value between 1 ( == dull) to 256 ( == bright)
  delay(20);
  i2c_OLED_send_cmd(0xaf);            //display on
}

void i2c_OLED_send_char(unsigned char ascii){
  unsigned char i,buffer,cf=0;
  if(ocol>120 || orow>7)  return;
  i2c_OLED_data_start();
  if(font) {
    if(CHAR_FORMAT) cf=0xff;
    for(i=0;i<7;i++){
      buffer = pgm_read_byte(&(Font8x14[ascii - 32][i])); // call the macro to read ROM byte and put it in buffer
      i2c_write(buffer ^ cf);
    }
    i2c_write(cf);                              //the gap bettwen characters
    i2c_stop();
    i2c_OLED_send_cmd(0xb0+orow+1);             //set page address
    i2c_OLED_send_cmd(0x00+(ocol&0x0f));        //set low col address
    i2c_OLED_send_cmd(0x10+((ocol>>4)&0x0f));   //set high col address
    cf = CHAR_FORMAT>>2;
    i2c_OLED_data_start();
    for(i=7;i<14;i++){
      buffer = pgm_read_byte(&(Font8x14[ascii - 32][i])); // call the macro to read ROM byte and put it in buffer
      i2c_write(buffer ^ cf);
    }
    i2c_write(cf);                              //the gap bettwen characters
    i2c_stop();
    ocol += 8;
    i2c_OLED_send_cmd(0xb0+orow);               //set page address
    i2c_OLED_send_cmd(0x00+(ocol&0x0f));        //set low col address
    i2c_OLED_send_cmd(0x10+((ocol>>4)&0x0f));   //set high col address
  } else {
    for(i=0;i<5;i++){
      buffer = pgm_read_byte(&(Font5x7[ascii - 32][i])); // call the macro to read ROM byte and put it in buffer
      i2c_write(buffer ^ CHAR_FORMAT);
    }
    i2c_write(CHAR_FORMAT);                     //the gap bettwen characters
    i2c_stop();
    ocol+=6;
  }
}

#ifndef SUPPRESS_OLED_I2C_128x64LOGO       // Do we want the Logo displayed ?
void i2c_OLED_Put_Logo(void){
  unsigned char i,j,buffer;
  i2c_OLED_send_cmd(0xa6);              //Set Normal Display
  i2c_OLED_send_cmd(0xae);              // Display OFF
  i2c_OLED_send_cmd(0x20);              // Set Memory Addressing Mode
  i2c_OLED_send_cmd(0x00);              // Set Memory Addressing Mode to Horizontal addressing mode
  i2c_OLED_send_cmd(0xb0);              // set page address to 0
  i2c_OLED_send_cmd(0X40);              // Display start line register to 0
  i2c_OLED_send_cmd(0);                 // Set low col address to 0
  i2c_OLED_send_cmd(0x10);              // Set high col address to 0
  i2c_OLED_data_start();
  for(int i=0; i<1024; i++) {           // fill the display's RAM with graphic... 128*64 pixel picture
    buffer = pgm_read_byte(&(LOGO[i]));
    i2c_write(buffer);
  }
  i2c_stop();
  i2c_OLED_send_cmd(0x81);              // Setup CONTRAST CONTROL, following byte is the contrast Value... always a 2 byte instruction
  i2c_OLED_send_cmd(250);               // Here you can set the brightness 1 = dull, 255 is very bright
  i2c_OLED_send_cmd(0xaf);              // display on
  ocol=0;
  orow=0;
}
#endif  // SUPPRESS_OLED_I2C_128x64LOGO

void i2c_OLED_set_XY(byte col, byte row) {        //  Not used in MW V2.0 but its here anyway!
  i2c_OLED_send_cmd(0xb0+row);                //set page address
  i2c_OLED_send_cmd(0x00+(col&0x0f));       //set low col address
  i2c_OLED_send_cmd(0x10+((col>>4)&0x0f));  //set high col address
  ocol=col;
  orow=row;
}

void i2c_OLED_set_line(byte row) {   // goto the beginning of a single row, compattible with LCD_CONFIG
  if(row>7) row=7;
  i2c_OLED_send_cmd(0xb0+row);   //set page address
  i2c_OLED_send_cmd(0);          //set low col address
  i2c_OLED_send_cmd(0x10);       //set high col address
  ocol=0;
  orow=row;
}

void i2c_clear_OLED(void){
  i2c_OLED_send_cmd(0xa6);              //Set Normal Display
  i2c_OLED_send_cmd(0xae);              // Display OFF
  i2c_OLED_send_cmd(0x20);              // Set Memory Addressing Mode
  i2c_OLED_send_cmd(0x00);              // Set Memory Addressing Mode to Horizontal addressing mode
  i2c_OLED_send_cmd(0xb0);              // set page address to 0
  i2c_OLED_send_cmd(0X40);              // Display start line register to 0
  i2c_OLED_send_cmd(0);                 // Set low col address to 0
  i2c_OLED_send_cmd(0x10);              // Set high col address to 0
  i2c_OLED_data_start();
  for(uint16_t i=0; i<1024; i++) {           // fill the display's RAM with graphic... 128*64 pixel picture
     i2c_write(0);  // clear
  }
  i2c_stop();
  i2c_OLED_send_cmd(0x81);              // Setup CONTRAST CONTROL, following byte is the contrast Value... always a 2 byte instruction
  i2c_OLED_send_cmd(200);               // Here you can set the brightness 1 = dull, 255 is very bright
  i2c_OLED_send_cmd(0xaf);              // display on
  ocol=0;
  orow=0;
}

#endif // OLED_I2C_128x64

#if defined(LCD_ETPP)
#define LCD_ETPP_ADDRESS 0x3B

// *********************
// i2c Eagle Tree Power Panel primitives
// *********************
void i2c_ETPP_init () {
  i2c_rep_start(LCD_ETPP_ADDRESS<<1); // LCD_ETPP i2c address: 0x3B in 7 bit form. Shift left one bit and concatenate i2c write command bit of zero
  i2c_write(0x00);// LCD_ETPP command register
  i2c_write(0x24);// Function Set 001D0MSL D : data length for parallel interface only; M: 0 = 1x32 , 1 = 2x16; S: 0 = 1:18 multiplex drive mode, 1x32 or 2x16 character display, 1 = 1:9 multiplex drive mode, 1x16 character display; H: 0 = basic instruction set plus standard instruction set, 1 = basic instruction set plus extended instruction set
  i2c_write(0x0C);// Display on   00001DCB D : 0 = Display Off, 1 = Display On; C : 0 = Underline Cursor Off, 1 = Underline Cursor On; B : 0 = Blinking Cursor Off, 1 = Blinking Cursor On
  i2c_write(0x06);// Cursor Move  000001IS I : 0 = DDRAM or CGRAM address decrements by 1, cursor moves to the left, 1 = DDRAM or CGRAM address increments by 1, cursor moves to the right; S : 0 = display does not shift,  1 = display does shifts
  LCDclear();
}
void i2c_ETPP_send_cmd (byte c) {
  i2c_rep_start(LCD_ETPP_ADDRESS<<1); // I2C write direction
  i2c_write(0x00);// LCD_ETPP command register
  i2c_write(c);
}
void i2c_ETPP_send_char (char c) {
  if (c > 0x0f) c |= 0x80; // LCD_ETPP uses character set "R", which has A->z mapped same as ascii + high bit; don't mess with custom chars.
  i2c_rep_start(LCD_ETPP_ADDRESS<<1);// I2C write direction
  i2c_write(0x40);// LCD_ETPP data register
  i2c_write(c);
}

void i2c_ETPP_set_cursor (byte addr) {
  i2c_ETPP_send_cmd(0x80 | addr); // High bit is "Set DDRAM" command, remaing bits are addr.
}
void i2c_ETPP_set_cursor (byte col, byte row) {
  row = min(row,1);
  col = min(col,15);
  byte addr = col + row * 0x40; // Why 0x40? RAM in this controller has many more bytes than are displayed.  In particular, the start of the second line (line 1 char 0) is 0x40 in DDRAM. The bytes between 0x0F (last char of line 1) and 0x40 are not displayable (unless the display is placed in marquee scroll mode)
  i2c_ETPP_set_cursor(addr);
}
void i2c_ETPP_create_char (byte idx, uint8_t* array) {
  i2c_ETPP_send_cmd(0x80); // CGRAM and DDRAM share an address register, but you can't set certain bits with the CGRAM address command.   Use DDRAM address command to be sure high order address bits are zero.
  i2c_ETPP_send_cmd(0x40 | byte(idx * 8));// Set CGRAM address
  i2c_rep_start(LCD_ETPP_ADDRESS<<1);// I2C write direction
  i2c_write(0x40);// LCD_ETPP data register
  for (byte i = 0; i<8; i++) {i2c_write(*array); array++;}
}

static boolean charsInitialized; // chars for servo signals are initialized
void ETPP_barGraph(byte num, int val) { // num chars in graph; percent as 1 to 100
  if (!charsInitialized) {
    charsInitialized = true;

    static byte bars[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15,};
    static byte less[8] = {0x00, 0x04, 0x0C, 0x1C, 0x0C, 0x04, 0x00, 0x15,};
    static byte grt [8] = {0x00, 0x04, 0x06, 0x07, 0x06, 0x04, 0x00, 0x15,};

    byte pattern = 0x10;
    for (int8_t i = 0; i <= 5; i++) {
      for (int8_t j = 0; j < 7; j++) {
        bars[j] = pattern;
      }
      i2c_ETPP_create_char(i, bars);
      pattern >>= 1;
    }
    i2c_ETPP_create_char(6, less);
    i2c_ETPP_create_char(7, grt);
  }

  static char bar[16];
  for (int8_t i = 0; i < num; i++) {bar[i] = 5;}

  if (val < -100 || val > 100) {bar[0] = 6; bar[num] = 7;} // invalid
  else if (val < 0) {bar[0] = 6;} // <...
  else if (val >= 100) {bar[3] = 7;} // ...>
  else {bar[val/(100/num)] = (val%(100/num))/5;} // ..|.

  for (int8_t i = 0; i < num; i++) {
    i2c_ETPP_send_char(bar[i]);
  }
}
#endif //LCD_ETPP

#if defined(LCD_LCD03) // LCD_LCD03
#define LCD_LCD03_ADDRESS 0x63
// *********************
// I2C LCD03 primitives
// *********************
void i2c_LCD03_init () {
  i2c_rep_start(LCD_LCD03_ADDRESS<<1); // The LCD03 is located on the I2C bus at address 0xC6
  i2c_write(0x00);// Command register
  i2c_write(04);// Hide cursor
  i2c_write(12);// Clear screen
  i2c_write(19);// Backlight on
}
void i2c_LCD03_send_cmd (byte c) {
  i2c_rep_start(LCD_LCD03_ADDRESS<<1);
  i2c_write(0x00);
  i2c_write(c);
}
void i2c_LCD03_send_char (char c) {
  i2c_rep_start(LCD_LCD03_ADDRESS<<1);
  i2c_write(0x00);
  i2c_write(c);
}
void i2c_LCD03_set_cursor (byte col, byte row) {
  row = min(row,1);
  col = min(col,15);
  i2c_LCD03_send_cmd(03); // set cursor (row, column)
  i2c_LCD03_send_cmd(row+1);
  i2c_LCD03_send_cmd(col+1);
}
#endif // LCD_LCD03

#if defined(OLED_DIGOLE) // OLED_DIGOLE
#define OLED_DIGOLE_ADDRESS 0x27 // 7bit address
// *********************
// I2C DIGOLE primitives
// *********************
void i2c_OLED_DIGOLE_init () {
  i2c_OLED_DIGOLE_send_string("CL");    // clear screen
  i2c_OLED_DIGOLE_send_string("CT");  // contrast _
  i2c_write(100); // contrast [0;100]
}
void i2c_OLED_DIGOLE_send_byte (byte c) {
  i2c_rep_start(OLED_DIGOLE_ADDRESS<<1);
  i2c_write(0x00);
  i2c_write(c);
}
void i2c_OLED_DIGOLE_send_string(const char *string){  // Sends a string of chars but not the null terminator
  i2c_rep_start(OLED_DIGOLE_ADDRESS<<1);
  i2c_write(0x00);
  while(*string){
    i2c_write(*string);
    *string++;
  }
}
void i2c_OLED_DIGOLE_printString(const char *string){  // prints a string of chars
  i2c_OLED_DIGOLE_send_string("TT");  // type text _
  while(*string){
    i2c_write(*string);
    *string++;
  }
  i2c_write(0x00);
}
void i2c_OLED_DIGOLE_printChar(char c){  // prints a single char - should be printable
  i2c_rep_start(OLED_DIGOLE_ADDRESS<<1);
  i2c_write(0x00);
  i2c_write('T');
  i2c_write('T');
  i2c_write(c);
  i2c_write(0x00);
}
void i2c_OLED_DIGOLE_clear(void) {
  i2c_OLED_DIGOLE_send_string("CLSF");    // set font _
  #ifdef DISPLAY_FONT_DSIZE
    i2c_write(0);                         // _ one of 6,10,18,51,120,123,0 - font 0 is 5x16 linesxrows; font 10 is 7x21
  #else
    i2c_write(10);
  #endif
}
#endif // OLED_DIGOLE

/* ------------------------------------------------------------------ */

#if !defined(OSD_DISPLAY)
void LCDprint(uint8_t i) {
  #if defined(LCD_SERIAL3W)
    // 1000000 / 9600  = 104 microseconds at 9600 baud.
    // we set it below to take some margin with the running interrupts
  #define BITDELAY 102
    LCDPIN_OFF;
    delayMicroseconds(BITDELAY);
    for (uint8_t mask = 0x01; mask; mask <<= 1) {
      if (i & mask) {LCDPIN_ON;} else {LCDPIN_OFF;} // choose bit
      delayMicroseconds(BITDELAY);
    }
    LCDPIN_ON //switch ON digital PIN 0
    delayMicroseconds(BITDELAY);
  #elif defined(LCD_TEXTSTAR) || defined(LCD_VT100) || defined(LCD_TTY)
    SerialWrite(LCD_SERIAL_PORT, i );
  #elif defined(LCD_ETPP)
    i2c_ETPP_send_char(i);
  #elif defined(LCD_LCD03)
    i2c_LCD03_send_char(i);
  #elif defined(OLED_I2C_128x64)
    i2c_OLED_send_char(i);
  #elif defined(OLED_DIGOLE)
    i2c_OLED_DIGOLE_printChar(i);
  #endif
}
#endif

void LCDprintChar(const char *s) {
  #if defined(OLED_DIGOLE)
    i2c_OLED_DIGOLE_printString(s);
  #elif defined(OSD_DISPLAY)
    serialSendOsdString(font, s);
  #else
    while (*s) {LCDprint(*s++);}
  #endif
}

void LCDcrlf() {
  #if ( defined(OLED_I2C_128x64)|| defined(LCD_VT100) || defined(OLED_DIGOLE) || defined(OSD_DISPLAY) )
    // do nothing - these displays use line positioning
  #else
    LCDprintChar("\r\n");
  #endif
}
void LCDclear() {
  #if defined(LCD_SERIAL3W)
    LCDprint(0xFE);LCDprint(0x01);delay(10);LCDprint(0xFE);LCDprint(0x02);delay(10); // clear screen, cursor line 1, pos 0 for serial LCD Sparkfun - contrib by flyman777
  #elif defined(LCD_TEXTSTAR)
    LCDprint(0x0c);
  #elif defined(LCD_VT100)
    LCDcrlf();
    LCDprint(0x1B); LCDprint(0x5B); LCDprintChar("2J"); //ED2
    LCDcrlf();
    LCDprint(0x1B); LCDprint(0x5B); LCDprintChar("1;1H");//cursor top left
  #elif defined(LCD_TTY)
    LCDcrlf();
  #elif defined(LCD_ETPP)
    i2c_ETPP_send_cmd(0x01); // Clear display command, which does NOT clear an Eagle Tree because character set "R" has a '>' at 0x20
    for (byte i = 0; i<80; i++) i2c_ETPP_send_char(' ');// Blanks for all 80 bytes of RAM in the controller, not just the 2x16 display
  #elif defined(LCD_LCD03)
    i2c_LCD03_send_cmd(12); // clear screen
  #elif defined(OLED_I2C_128x64)
    i2c_clear_OLED();
  #elif defined(OLED_DIGOLE)
    i2c_OLED_DIGOLE_clear();
  #elif defined(OSD_DISPLAY)
    serialSendOsdString(0x10, line1);
    delay(60);
  #endif
  #if ( defined(LOG_PERMANENT) && defined(DISPLAY_MULTILINE) )
    lnr = 0;
  #endif
}

void LCDsetLine(byte line) { // Line = 1 or 2 - vt100 has lines 1-99
  #ifdef DISPLAY_FONT_DSIZE
    if (line >=1 && line <= (MULTILINE_PRE+MULTILINE_POST)) {
      line_is_valid = 1;
    } else {
      line_is_valid = 0;
      return;
    }
  #endif
  #if defined(LCD_SERIAL3W)
    if (line==1) {LCDprint(0xFE);LCDprint(128);} else {LCDprint(0xFE);LCDprint(192);}
  #elif defined(LCD_TEXTSTAR)
    LCDcrlf(); LCDprint(0xfe);LCDprint('L');LCDprint(line);
  #elif defined(LCD_VT100)
    #ifndef DEBUG // sanity check for production only. Debug runs with all possible side effects
      if (line<1 || line>(MULTILINE_PRE+MULTILINE_POST)) line = 1;
    #endif
    LCDcrlf();
    LCDprint(0x1b); LCDprint(0x5b);
    LCDprint( digit10(line) );
    LCDprint( digit1(line) );
    LCDprintChar(";1H"); //pos line 1
    LCDprint(0x1b); LCDprint(0x5b); LCDprintChar("2K");//EL2
  #elif defined(LCD_TTY)
    LCDcrlf();
  #elif defined(LCD_ETPP)
    i2c_ETPP_set_cursor(0,line-1);
  #elif defined(LCD_LCD03)
    i2c_LCD03_set_cursor(0,line-1);
  #elif defined(OLED_I2C_128x64)
   i2c_OLED_set_line((line-1)<<font);
  #elif defined(OLED_DIGOLE)
    i2c_OLED_DIGOLE_send_string("TP"); i2c_write(0); i2c_write(line-1); // first column is 0, line numbers start with 0
  #elif defined(OSD_DISPLAY)
    font &= 0xF0;
    font |= line+2;
  #endif
}

#if defined(LCD_VT100)
  void LCDattributesBold() {LCDprint(0x1b); LCDprint(0x5b); LCDprintChar("1m");}
  void LCDattributesReverse() {LCDprint(0x1b); LCDprint(0x5b); LCDprintChar("7m");}
  void LCDattributesOff() {LCDprint(0x1b); LCDprint(0x5b); LCDprintChar("0m");}
  void LCDalarmAndReverse() {LCDattributesReverse(); if (f.ARMED) { LCDprint(0x07); }; } // audio for errors only while armed
#elif defined(OLED_I2C_128x64)
  void LCDattributesBold() {/*CHAR_FORMAT = 0b01111111; */}
  void LCDattributesReverse() {CHAR_FORMAT = 0b01111111; }
  void LCDattributesOff() {CHAR_FORMAT = 0; }
  void LCDalarmAndReverse() {LCDattributesReverse(); }
#elif defined(OSD_DISPLAY)
  void LCDattributesBold() {}
  void LCDattributesReverse()  {font |= 0x40;}
  void LCDattributesOff()      {font &= 0x0F;}
  void LCDalarmAndReverse() {LCDattributesReverse(); }
#else
  void LCDattributesBold() {}
  void LCDattributesReverse() {}
  void LCDattributesOff() {}
  void LCDalarmAndReverse() {}
#endif

#define LCD_FLUSH {delay(20);}

void initLCD() {
  #if defined(LCD_SERIAL3W)
    SerialEnd(0);
    //init LCD
    PINMODE_LCD;//TX PIN for LCD = Arduino RX PIN (more convenient to connect a servo plug on arduino pro mini)
  #elif defined(LCD_TEXTSTAR)
    // Cat's Whisker Technologies 'TextStar' Module CW-LCD-02
    // http://cats-whisker.com/resources/documents/cw-lcd-02_datasheet.pdf
    //LCDprint(0xFE);LCDprint(0x43);LCDprint(0x02); //cursor blink mode
    LCDprint(0xFE);LCDprint('R');//reset
  #elif defined(LCD_VT100)
    //LCDprint(0x1b); LCDprint('c'); //RIS
  #elif defined(LCD_TTY)
    ; // do nothing special to init the serial tty device
  #elif defined(LCD_ETPP)
    // Eagle Tree Power Panel - I2C & Daylight Readable LCD
    i2c_ETPP_init();
  #elif defined(LCD_LCD03)
    // LCD03 - I2C LCD
    // http://www.robot-electronics.co.uk/htm/Lcd03tech.htm
    i2c_LCD03_init();
  #elif defined(OLED_I2C_128x64)
    i2c_OLED_init();
    #ifndef SUPPRESS_OLED_I2C_128x64LOGO
      i2c_OLED_Put_Logo();
      #ifndef OLED_I2C_128x64LOGO_PERMANENT
        delay(1000);
      #endif
    #endif
  #elif defined(OLED_DIGOLE)
    i2c_OLED_DIGOLE_init();
  #endif
  #if defined(OSD_DISPLAY)
    delay(100);
  #endif  
  #if !defined(OLED_I2C_128x64LOGO_PERMANENT) && !defined(OSD_DISPLAY)
    LCDclear();
    font = 1;
    strcpy_P(line1,PSTR("MultiWii V-.--"));
    line1[10] = digit100(VERSION);
    line1[12] = digit10(VERSION);
    line1[13] = digit1(VERSION);
    LCDattributesBold();
    LCDsetLine(1); LCDprintChar(line1);
    strcpy_P(line2,PSTR("  Unknown Modell"));
    #if defined(TRI)
      strcpy_P(line2,PSTR("  TRICopter"));
    #elif defined(QUADP)
      strcpy_P(line2,PSTR("  QUAD-P"));
    #elif defined(QUADX)
      strcpy_P(line2,PSTR("  QUAD-X"));
    #elif defined(BI)
      strcpy_P(line2,PSTR("  BICopter"));
    #elif defined(Y6)
      strcpy_P(line2,PSTR("  Y6"));
    #elif defined(HEX6)
      strcpy_P(line2,PSTR("  HEX6"));
    #elif defined(FLYING_WING)
      strcpy_P(line2,PSTR("  FLYING_WING"));
    #elif defined(Y4)
      strcpy_P(line2,PSTR("  Y4"));
    #elif defined(HEX6X)
      strcpy_P(line2,PSTR("  HEX6-X"));
    #elif defined(HEX6H)
      strcpy_P(line2,PSTR("  HEX6-H"));
    #elif defined(HEX6U)
      strcpy_P(line2,PSTR("  HEX6-U"));
    #elif defined(OCTOX8)
      strcpy_P(line2,PSTR("  OCTOX8"));
    #elif defined(OCTOFLATP)
      strcpy_P(line2,PSTR("  OCTOFLAT-P"));
    #elif defined(OCTOFLATX)
      strcpy_P(line2,PSTR("  OCTOFLAT-X"));
    #elif defined (AIRPLANE)
      strcpy_P(line2,PSTR("  AIRPLANE"));
    #elif defined (HELI_120_CCPM)
      strcpy_P(line2,PSTR("  HELI_120_CCPM"));
    #elif defined (HELI_90_DEG)
      strcpy_P(line2,PSTR("  HELI_90_DEG"));
    #elif defined(VTAIL4)
      strcpy_P(line2,PSTR("  VTAIL Quad"));
    #endif
    //LCDattributesBold();
    LCDsetLine(2); LCDprintChar(line2);
    LCDattributesOff();
    font = 0;
  #endif // OLED_I2C_128x64LOGO_PERMANENT
  #if defined(LCD_TEXTSTAR) || defined(LCD_VT100)
    //delay(2500);
    LCDclear();
  #endif
/*  
  #if defined(OLED_I2C_128x64) && !(defined(OLED_I2C_128x64LOGO_PERMANENT)) && defined(NEW_OLED_FONT) && !(defined(LCD_TELEMETRY))
    // no need to diplay this, if LCD telemetry is enabled
    //   optional instruction on the display......
    LCDsetLine(4); LCDprintChar("To ENTER CONFIG      ");// 21 characters on each line
    LCDsetLine(5); LCDprintChar("YAW RIGHT & PITCH FWD");
    LCDsetLine(7); LCDprintChar("To SAVE CONFIG       ");
    LCDsetLine(8); LCDprintChar("YAW LEFT & PITCH FWD ");
  #endif
*/  
  #ifdef LCD_TELEMETRY_STEP
    // load the first page of the step sequence
    LCDclear();
    telemetry = telemetryStepSequence[telemetryStepIndex]; //[++telemetryStepIndex % strlen(telemetryStepSequence)];
  #endif
}
#endif //Support functions for LCD_CONF and LCD_TELEMETRY

// -------------------- configuration menu to LCD over serial/i2c ----------------------------------

#ifdef LCD_CONF

typedef void (*formatter_func_ptr)(void *, uint8_t, uint8_t);
typedef void (*inc_func_ptr)(void *, int16_t);

/*typedef*/struct lcd_type_desc_t {
  formatter_func_ptr fmt;
  inc_func_ptr inc;
};

static lcd_type_desc_t LTU8 = {&__u8Fmt, &__u8Inc};
static lcd_type_desc_t LTU16 = {&__u16Fmt, &__u16Inc};
static lcd_type_desc_t LTS16 = {&__s16Fmt, &__s16Inc};
static lcd_type_desc_t LTS32 = {&__s32Fmt, &__s32Inc};
static lcd_type_desc_t LPMM = {&__upMFmt, &__nullInc};
static lcd_type_desc_t LPMS = {&__upSFmt, &__nullInc};
static lcd_type_desc_t LAUX1 = {&__uAuxFmt1, &__u16Inc};
static lcd_type_desc_t LAUX2 = {&__uAuxFmt2, &__u16Inc};
static lcd_type_desc_t LAUX3 = {&__uAuxFmt3, &__u16Inc};
static lcd_type_desc_t LAUX4 = {&__uAuxFmt4, &__u16Inc};

struct lcd_param_def_t {
  lcd_type_desc_t * type;
  uint8_t decimal;
  uint8_t multiplier;
  uint16_t increment;
};

// ************************************************************************************************************
// LCD Layout definition
// ************************************************************************************************************
// Descriptors
static lcd_param_def_t __RT = {&LTS32, 0, 1, 100};
static lcd_param_def_t __P = {&LTU8, 1, 1, 1};
static lcd_param_def_t __I = {&LTU8, 3, 1, 1};
static lcd_param_def_t __D = {&LTU8, 0, 1, 1};
static lcd_param_def_t __RC = {&LTU8, 2, 1, 1};
static lcd_param_def_t __PM = {&LPMM, 1, 1, 0};
static lcd_param_def_t __PS = {&LPMS, 1, 1, 0};
static lcd_param_def_t __PT = {&LTU8, 0, 1, 1};
static lcd_param_def_t __VB = {&LTU8, 1, 1, 0};
static lcd_param_def_t __L = {&LTU8, 0, 1, 0};
static lcd_param_def_t __FS = {&LTU8, 1, 1, 0};
static lcd_param_def_t __SE = {&LTU16, 0, 1, 5};
static lcd_param_def_t __SE1 = {&LTU16, 0, 1, 1};
static lcd_param_def_t __ST = {&LTS16, 0, 1, 5};
static lcd_param_def_t __AUX1 = {&LAUX1, 0, 1, 1};
static lcd_param_def_t __AUX2 = {&LAUX2, 0, 1, 8};
static lcd_param_def_t __AUX3 = {&LAUX3, 0, 1, 64};
static lcd_param_def_t __AUX4 = {&LAUX4, 0, 1, 512};

// Program Space Strings - These sit in program flash, not SRAM.
//                                        0123456789
const char PROGMEM lcd_param_text01 [] = "Pit/Roll P";
const char PROGMEM lcd_param_text02 [] = "Roll     P";
const char PROGMEM lcd_param_text03 [] = "Roll     I";
const char PROGMEM lcd_param_text04 [] = "Roll     D";
const char PROGMEM lcd_param_text05 [] = "Pitch    P";
const char PROGMEM lcd_param_text06 [] = "Pitch    I";
const char PROGMEM lcd_param_text07 [] = "Pitch    D";
const char PROGMEM lcd_param_text08 [] = "Yaw      P";
const char PROGMEM lcd_param_text09 [] = "Yaw      I";
const char PROGMEM lcd_param_text10 [] = "Yaw      D";
#if  BARO && (!defined(SUPPRESS_BARO_ALTHOLD))
const char PROGMEM lcd_param_text11 [] = "Alt      P";
const char PROGMEM lcd_param_text12 [] = "Alt      I";
const char PROGMEM lcd_param_text13 [] = "Alt      D";
#endif
const char PROGMEM lcd_param_text17 [] = "Ang/Hor  P";
const char PROGMEM lcd_param_text18 [] = "Ang/Hor  I";
const char PROGMEM lcd_param_text188[] = "Ang/Hor  D";
#if MAG
const char PROGMEM lcd_param_text19 [] = "Mag      P";
#endif
const char PROGMEM lcd_param_text20 [] = "RC Rate   ";
const char PROGMEM lcd_param_text21 [] = "RC Expo   ";
const char PROGMEM lcd_param_text20t []= "Thro Mid  ";
const char PROGMEM lcd_param_text21t []= "Thro Expo ";
const char PROGMEM lcd_param_text22 [] = "P/R Rate  ";
const char PROGMEM lcd_param_text23 [] = "Yaw Rate  ";
const char PROGMEM lcd_param_text24 [] = "Thro PID  ";
#ifdef LOG_VALUES
#if (LOG_VALUES >= 3)
const char PROGMEM lcd_param_text25 [] = "pmeter  m0";
const char PROGMEM lcd_param_text26 [] = "pmeter  m1";
const char PROGMEM lcd_param_text27 [] = "pmeter  m2";
const char PROGMEM lcd_param_text28 [] = "pmeter  m3";
const char PROGMEM lcd_param_text29 [] = "pmeter  m4";
const char PROGMEM lcd_param_text30 [] = "pmeter  m5";
const char PROGMEM lcd_param_text31 [] = "pmeter  m6";
const char PROGMEM lcd_param_text32 [] = "pmeter  m7";
#endif //                                 0123456789
#endif
#ifdef FLYING_WING
const char PROGMEM lcd_param_text36 [] = "ServoMid 1";
const char PROGMEM lcd_param_text37 [] = "ServoMid 2";
#endif
#ifdef TRI //                             0123456789
const char PROGMEM lcd_param_text38 [] = "ServoMid Y";
#endif
#if defined(LCD_CONF_AUX)
const char PROGMEM lcd_param_text41 [] = "AUX angle ";
const char PROGMEM lcd_param_text42 [] = "AUX horizn";
const char PROGMEM lcd_param_text43 [] = "AUX baro  ";
const char PROGMEM lcd_param_text44 [] = "AUX mag   ";
const char PROGMEM lcd_param_text45 [] = "AUX camstb";
const char PROGMEM lcd_param_text46 [] = "AUX camtrg";
const char PROGMEM lcd_param_text47 [] = "AUX arm   ";
const char PROGMEM lcd_param_text48 [] = "AUX gpshom";
const char PROGMEM lcd_param_text49 [] = "AUX gpshld";
const char PROGMEM lcd_param_text50 [] = "AUX passth";
const char PROGMEM lcd_param_text51 [] = "AUX headfr";
const char PROGMEM lcd_param_text52 [] = "AUX buzzer";
const char PROGMEM lcd_param_text53 [] = "AUX vario ";
const char PROGMEM lcd_param_text54 [] = "AUX calib ";
const char PROGMEM lcd_param_text55 [] = "AUX govern";
const char PROGMEM lcd_param_text56 [] = "AUX osd   ";
const char PROGMEM lcd_param_text57 [] = "AUX f.safe";
// 53 to 61 reserved
#endif
#ifdef HELI_120_CCPM //                   0123456789
const char PROGMEM lcd_param_text73 [] = "ServoMid N";
const char PROGMEM lcd_param_text74 [] = "ServoMid L";
const char PROGMEM lcd_param_text75 [] = "ServoMid Y";
const char PROGMEM lcd_param_text76 [] = "ServoMid R";
#endif
#ifdef GYRO_SMOOTHING //                  0123456789
const char PROGMEM lcd_param_text80 [] = "GySmooth R";
const char PROGMEM lcd_param_text81 [] = "GySmooth P";
const char PROGMEM lcd_param_text82 [] = "GySmooth Y";
#endif
#if defined(AIRPLANE) || defined(VTAIL_AIRPLANE) //                        0123456789
const char PROGMEM lcd_param_text83 [] = "ServoMid 3";
const char PROGMEM lcd_param_text84 [] = "ServoMid 4";
const char PROGMEM lcd_param_text85 [] = "ServoMid 5";
const char PROGMEM lcd_param_text86 [] = "ServoMid 6";
const char PROGMEM lcd_param_text87 [] = "ServoMid 7";
#endif
#if GPS
const char PROGMEM lcd_param_text91 [] = "GPS Pos. P";
const char PROGMEM lcd_param_text92 [] = "GPS Pos. I";
const char PROGMEM lcd_param_text93 [] = "Pos Rate P";
const char PROGMEM lcd_param_text94 [] = "Pos Rate I";
const char PROGMEM lcd_param_text95 [] = "Pos Rate D";
const char PROGMEM lcd_param_text96 [] = "NAV Rate P";
const char PROGMEM lcd_param_text97 [] = "NAV Rate I";
const char PROGMEM lcd_param_text98 [] = "NAV Rate D";
#endif
#if defined (FAILSAFE)
const char PROGMEM lcd_param_text101 [] = "FailThrott";
const char PROGMEM lcd_param_text100 [] = "RTH Altit.";
#endif
#ifdef VBAT
const char PROGMEM lcd_param_text35 [] =  "CurrVolt  ";
const char PROGMEM lcd_param_text102 [] = "VbatScale ";
const char PROGMEM lcd_param_text103 [] = "Batt Warn ";
const char PROGMEM lcd_param_text104 [] = "Batt Crit ";
#endif
#ifdef POWERMETER
const char PROGMEM lcd_param_text33 [] = "pmeter sum";
const char PROGMEM lcd_param_text34 [] = "pAlarm /50"; // change text to represent PLEVELSCALE value
#ifdef POWERMETER_HARD
  const char PROGMEM lcd_param_text111 [] = "Pm Sensor ";
  const char PROGMEM lcd_param_text114 [] = "Pm INT2MA ";
#endif
#ifdef POWERMETER_SOFT
  const char PROGMEM lcd_param_text113 [] = "Pm Divider";
#endif
#endif
#ifdef CYCLETIME_FIXATED
const char PROGMEM lcd_param_text120 [] = "Cycle Time";
#endif
#ifdef MMGYRO
const char PROGMEM lcd_param_text121 [] = "MM Gyro   ";
#endif
const char PROGMEM lcd_param_text131 [] = "MinThtottl";
#if defined(ARMEDTIMEWARNING)
const char PROGMEM lcd_param_text132 [] = "ArmedTWarn";
#endif
#if defined(GOVERNOR_P)
const char PROGMEM lcd_param_text133 [] = "Govern   P";
const char PROGMEM lcd_param_text134 [] = "Govern   D";
const char PROGMEM lcd_param_text135 [] = "Govern Rpm";
#endif
//                                         0123456789

PROGMEM const void * const lcd_param_ptr_table [] = {
  &lcd_param_text01, &conf.pid[ROLL].P8, &__P,
  &lcd_param_text02, &conf.pid[ROLL].P8, &__P,
  &lcd_param_text03, &conf.pid[ROLL].I8, &__I,
  &lcd_param_text04, &conf.pid[ROLL].D8, &__D,
  &lcd_param_text05, &conf.pid[PITCH].P8, &__P,
  &lcd_param_text06, &conf.pid[PITCH].I8, &__I,
  &lcd_param_text07, &conf.pid[PITCH].D8, &__D,
  &lcd_param_text08, &conf.pid[YAW].P8, &__P,
  &lcd_param_text09, &conf.pid[YAW].I8, &__I,
  &lcd_param_text10, &conf.pid[YAW].D8, &__D,
#if BARO && (!defined(SUPPRESS_BARO_ALTHOLD))
  &lcd_param_text11, &conf.pid[PIDALT].P8, &__P,
  &lcd_param_text12, &conf.pid[PIDALT].I8, &__I,
  &lcd_param_text13, &conf.pid[PIDALT].D8, &__D,
#endif
  &lcd_param_text17, &conf.pid[PIDLEVEL].P8, &__P,
  &lcd_param_text18, &conf.pid[PIDLEVEL].I8, &__I,
  &lcd_param_text188, &conf.pid[PIDLEVEL].D8, &__D,
#if MAG
  &lcd_param_text19, &conf.pid[PIDMAG].P8, &__P,
#endif
  &lcd_param_text20t, &conf.thrMid8, &__RC,
  &lcd_param_text21t, &conf.thrExpo8, &__RC,
  &lcd_param_text20, &conf.rcRate8, &__RC,
  &lcd_param_text21, &conf.rcExpo8, &__RC,
  &lcd_param_text22, &conf.rollPitchRate, &__RC,
  &lcd_param_text23, &conf.yawRate, &__RC,
  &lcd_param_text24, &conf.dynThrPID, &__RC,
#if GPS
 &lcd_param_text91, &conf.pid[PIDPOS].P8 , &__RC,
 &lcd_param_text92, &conf.pid[PIDPOS].I8 , &__I,
 &lcd_param_text93, &conf.pid[PIDPOSR].P8, &__P,
 &lcd_param_text94, &conf.pid[PIDPOSR].I8, &__I,
 &lcd_param_text95, &conf.pid[PIDPOSR].D8, &__I,
 &lcd_param_text96, &conf.pid[PIDNAVR].P8, &__P,
 &lcd_param_text97, &conf.pid[PIDNAVR].I8, &__RC,
 &lcd_param_text98, &conf.pid[PIDNAVR].D8, &__I,
#endif
#ifdef LCD_CONF_AUX
  #if ACC
    &lcd_param_text41, &conf.activate[BOXANGLE], &__AUX1,
    &lcd_param_text41, &conf.activate[BOXANGLE], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text41, &conf.activate[BOXANGLE], &__AUX3,
      &lcd_param_text41, &conf.activate[BOXANGLE], &__AUX4,
    #endif
    &lcd_param_text42, &conf.activate[BOXHORIZON], &__AUX1,
    &lcd_param_text42, &conf.activate[BOXHORIZON], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text42, &conf.activate[BOXHORIZON], &__AUX3,
      &lcd_param_text42, &conf.activate[BOXHORIZON], &__AUX4,
    #endif
  #endif
  #if BARO && (!defined(SUPPRESS_BARO_ALTHOLD))
    &lcd_param_text43, &conf.activate[BOXBARO], &__AUX1,
    &lcd_param_text43, &conf.activate[BOXBARO], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text43, &conf.activate[BOXBARO], &__AUX3,
      &lcd_param_text43, &conf.activate[BOXBARO], &__AUX4,
    #endif
  #endif
  #if MAG
    &lcd_param_text44, &conf.activate[BOXMAG], &__AUX1,
    &lcd_param_text44, &conf.activate[BOXMAG], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text44, &conf.activate[BOXMAG], &__AUX3,
      &lcd_param_text44, &conf.activate[BOXMAG], &__AUX4,
    #endif
  #endif
  #ifdef GIMBAL
    &lcd_param_text45, &conf.activate[BOXCAMSTAB], &__AUX1,
    &lcd_param_text45, &conf.activate[BOXCAMSTAB], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text45, &conf.activate[BOXCAMSTAB], &__AUX3,
      &lcd_param_text45, &conf.activate[BOXCAMSTAB], &__AUX4,
    #endif
    &lcd_param_text46, &conf.activate[BOXCAMTRIG], &__AUX1,
    &lcd_param_text46, &conf.activate[BOXCAMTRIG], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text46, &conf.activate[BOXCAMTRIG], &__AUX3,
      &lcd_param_text46, &conf.activate[BOXCAMTRIG], &__AUX4,
    #endif
  #endif
  &lcd_param_text47, &conf.activate[BOXARM], &__AUX1,
  &lcd_param_text47, &conf.activate[BOXARM], &__AUX2,
  #ifndef SUPPRESS_LCD_CONF_AUX34
    &lcd_param_text47, &conf.activate[BOXARM], &__AUX3,
    &lcd_param_text47, &conf.activate[BOXARM], &__AUX4,
  #endif
  #if GPS
    &lcd_param_text48, &conf.activate[BOXGPSHOME], &__AUX1,
    &lcd_param_text48, &conf.activate[BOXGPSHOME], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text48, &conf.activate[BOXGPSHOME], &__AUX3,
      &lcd_param_text48, &conf.activate[BOXGPSHOME], &__AUX4,
    #endif
    &lcd_param_text49, &conf.activate[BOXGPSHOLD], &__AUX1,
    &lcd_param_text49, &conf.activate[BOXGPSHOLD], &__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text49, &conf.activate[BOXGPSHOLD], &__AUX3,
      &lcd_param_text49, &conf.activate[BOXGPSHOLD], &__AUX4,
    #endif
  #endif
  #if defined(FIXEDWING) || defined(HELICOPTER)
    &lcd_param_text50, &conf.activate[BOXPASSTHRU],&__AUX1,
    &lcd_param_text50, &conf.activate[BOXPASSTHRU],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text50, &conf.activate[BOXPASSTHRU],&__AUX3,
      &lcd_param_text50, &conf.activate[BOXPASSTHRU],&__AUX4,
    #endif
  #endif
  #if MAG
    &lcd_param_text51, &conf.activate[BOXHEADFREE],&__AUX1,
    &lcd_param_text51, &conf.activate[BOXHEADFREE],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text51, &conf.activate[BOXHEADFREE],&__AUX3,
      &lcd_param_text51, &conf.activate[BOXHEADFREE],&__AUX4,
    #endif
  #endif
  #if defined(BUZZER)
    &lcd_param_text52, &conf.activate[BOXBEEPERON],&__AUX1,
    &lcd_param_text52, &conf.activate[BOXBEEPERON],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text52, &conf.activate[BOXBEEPERON],&__AUX3,
      &lcd_param_text52, &conf.activate[BOXBEEPERON],&__AUX4,
    #endif
  #endif
  #ifdef VARIOMETER
    &lcd_param_text53, &conf.activate[BOXVARIO],&__AUX1,
    &lcd_param_text53, &conf.activate[BOXVARIO],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text53, &conf.activate[BOXVARIO],&__AUX3,
      &lcd_param_text53, &conf.activate[BOXVARIO],&__AUX4,
    #endif
  #endif
  #ifdef INFLIGHT_ACC_CALIBRATION
    &lcd_param_text54, &conf.activate[BOXCALIB],&__AUX1,
    &lcd_param_text54, &conf.activate[BOXCALIB],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text54, &conf.activate[BOXCALIB],&__AUX3,
      &lcd_param_text54, &conf.activate[BOXCALIB],&__AUX4,
    #endif
  #endif
#ifdef GOVERNOR_P
  &lcd_param_text55, &conf.activate[BOXGOV],&__AUX1,
  &lcd_param_text55, &conf.activate[BOXGOV],&__AUX2,
  #ifndef SUPPRESS_LCD_CONF_AUX34
    &lcd_param_text55, &conf.activate[BOXGOV],&__AUX3,
    &lcd_param_text55, &conf.activate[BOXGOV],&__AUX4,
  #endif
#endif
#ifdef OSD_SWITCH
  &lcd_param_text56, &conf.activate[BOXOSD],&__AUX1,
  &lcd_param_text56, &conf.activate[BOXOSD],&__AUX2,
  #ifndef SUPPRESS_LCD_CONF_AUX34
    &lcd_param_text56, &conf.activate[BOXOSD],&__AUX3,
    &lcd_param_text56, &conf.activate[BOXOSD],&__AUX4,
  #endif
#endif
  #if defined(FAILSAFE)
    &lcd_param_text57, &conf.activate[BOXFAILSAFE],&__AUX1,
    &lcd_param_text57, &conf.activate[BOXFAILSAFE],&__AUX2,
    #ifndef SUPPRESS_LCD_CONF_AUX34
      &lcd_param_text57, &conf.activate[BOXFAILSAFE],&__AUX3,
      &lcd_param_text57, &conf.activate[BOXFAILSAFE],&__AUX4,
    #endif
  #endif
#endif //lcd.conf.aux

#ifdef LOG_VALUES
#if (LOG_VALUES >= 3)
#if (NUMBER_MOTOR > 0)
  &lcd_param_text25, &pMeter[0], &__PM,
#endif
#if (NUMBER_MOTOR > 1)
  &lcd_param_text26, &pMeter[1], &__PM,
#endif
#if (NUMBER_MOTOR > 2)
  &lcd_param_text27, &pMeter[2], &__PM,
#endif
#if (NUMBER_MOTOR > 3)
  &lcd_param_text28, &pMeter[3], &__PM,
#endif
#if (NUMBER_MOTOR > 4)
  &lcd_param_text29, &pMeter[4], &__PM,
#endif
#if (NUMBER_MOTOR > 5)
  &lcd_param_text30, &pMeter[5], &__PM,
#endif
#if (NUMBER_MOTOR > 6)
  &lcd_param_text31, &pMeter[6], &__PM,
#endif
#if (NUMBER_MOTOR > 7)
  &lcd_param_text32, &pMeter[7], &__PM,
#endif
#endif
#endif
#ifdef POWERMETER
  &lcd_param_text33, &pMeter[PMOTOR_SUM], &__PM,
  &lcd_param_text34, &conf.powerTrigger1, &__PT,
  #ifdef POWERMETER_HARD
    &lcd_param_text114, &conf.pint2ma, &__PT,
    &lcd_param_text111, &conf.psensornull, &__SE1,
  #endif
#endif
#if defined(ARMEDTIMEWARNING)
  &lcd_param_text132, &conf.armedtimewarning, &__SE,
#endif
  &lcd_param_text131, &conf.minthrottle, &__ST,
#if defined (FAILSAFE)
  &lcd_param_text101, &conf.failsafe_throttle, &__ST,
  &lcd_param_text100, &conf.rth_altitude, &__RT,
#endif
#ifdef VBAT
  &lcd_param_text35, &analog.vbat, &__VB,
  &lcd_param_text102, &conf.vbatscale, &__PT,
  &lcd_param_text103, &conf.vbatlevel_warn, &__P,
  &lcd_param_text104, &conf.vbatlevel_crit, &__P,
#endif
#ifdef FLYING_WING
  &lcd_param_text36, &conf.servoConf[3].middle, &__SE,
  &lcd_param_text37, &conf.servoConf[4].middle, &__SE,
#endif
#ifdef TRI
  &lcd_param_text38, &conf.servoConf[TRI_SERVO-1].middle, &__SE, &__SE,
#endif
#ifdef HELI_120_CCPM
  &lcd_param_text73, &conf.servoConf[3].middle, &__SE,
  &lcd_param_text74, &conf.servoConf[4].middle, &__SE,
  &lcd_param_text76, &conf.servoConf[6].middle, &__SE,
  &lcd_param_text75, &conf.servoConf[5].middle, &__SE,
#endif
#ifdef GOVERNOR_P
  &lcd_param_text133, &conf.governorP, &__D,
  &lcd_param_text134, &conf.governorD, &__D,
  &lcd_param_text135, &conf.governorR, &__P,
#endif

#ifdef GYRO_SMOOTHING
  &lcd_param_text80, &conf.Smoothing[0], &__D,
  &lcd_param_text81, &conf.Smoothing[1], &__D,
  &lcd_param_text82, &conf.Smoothing[2], &__D,
#endif
#if defined(AIRPLANE) || defined(VTAIL_AIRPLANE)  
  &lcd_param_text83, &conf.servoConf[3].middle, &__SE,
  &lcd_param_text84, &conf.servoConf[4].middle, &__SE,
  &lcd_param_text85, &conf.servoConf[5].middle, &__SE,
  &lcd_param_text86, &conf.servoConf[6].middle, &__SE,
#endif
#ifdef MMGYRO
  &lcd_param_text121, &conf.mmgyro, &__D,
#endif
};
#define PARAMMAX (sizeof(lcd_param_ptr_table)/6 - 1)
// ************************************************************************************************************

void __u8Inc(void * var, int16_t inc) {*(uint8_t*)var += (uint8_t)inc;};
void __u16Inc(void * var, int16_t inc) {*(uint16_t*)var += inc;};
void __s16Inc(void * var, int16_t inc) {*(int16_t*)var += inc;};
void __s32Inc(void * var, int16_t inc) {*(int32_t*)var += inc;};
void __nullInc(void * var, int16_t inc) {};

void __u8Fmt(void * var, uint8_t mul, uint8_t dec) {
  uint16_t unit = *(uint8_t*)var;
  unit *= mul;
  char c1 = '0'+unit/100; char c2 = '0'+unit/10-(unit/100)*10; char c3 = '0'+unit-(unit/10)*10;
  switch (dec) {
    case 0: line2[6] = c1; line2[7] = c2; line2[8] = c3; break;
    case 1: line2[5] = c1; line2[6] = c2; line2[7] = '.'; line2[8] = c3; break;
    case 2: line2[5] = c1; line2[6] = '.'; line2[7] = c2; line2[8] = c3; break;
    case 3: line2[4] = '0'; line2[5] = '.'; line2[6] = c1; line2[7] = c2; line2[8] = c3; break;
  }
}

void __u16Fmt(void * var, uint8_t mul, uint8_t dec) {
  uint16_t unit = *(uint16_t*)var;
  unit *= mul;
  line2[4] = digit10000(unit);
  line2[5] = digit1000(unit);
  line2[6] = digit100(unit);
  line2[7] = digit10(unit);
  line2[8] = digit1(unit);
}
void __s16Fmt(void * var, uint8_t mul, uint8_t dec) {
  int16_t unit = *(int16_t*)var;
  if (unit >= 0) {
    line2[3] = ' ';
  } else {
    line2[3] = '-';
    unit = -unit;
  }
  __u16Fmt(&unit, mul, dec);
}
void __s32Fmt(void * var, uint8_t mul, uint8_t dec) {
  int16_t tmp = (*(int32_t*)var) / 100L;  // special version for RTH altitude - display in meters
  __s16Fmt(&tmp, mul, dec);
}

void __uAuxFmt1(void * var, uint8_t mul, uint8_t dec) {  __uAuxFmt(var, mul, dec, 1); }
void __uAuxFmt2(void * var, uint8_t mul, uint8_t dec) {  __uAuxFmt(var, mul, dec, 2); }
void __uAuxFmt3(void * var, uint8_t mul, uint8_t dec) {  __uAuxFmt(var, mul, dec, 3); }
void __uAuxFmt4(void * var, uint8_t mul, uint8_t dec) {  __uAuxFmt(var, mul, dec, 4); }


void __uAuxFmt(void * var, uint8_t mul, uint8_t dec, uint8_t aux) {
  uint16_t unit = *(uint16_t*)var;
#if defined(OLED_I2C_128x64) || defined(OSD_DISPLAY) // not enough space for 16 chars, sorry
  uint16_t sh = 1<<(3*(aux-1));
  line2[4] =  aux+'0';
  line2[6] =  ( unit & sh ? 'L' : '.' );
  sh <<=1;
  line2[7] =  ( unit & sh ? 'M' : '.' );
  sh <<=1;
  line2[8] =  ( unit & sh ? 'H' : '.' );
#else
  line2[0] =  (aux == 1 ? '1' : ' ');
  line2[1] =  ( unit & 1<<0 ? 'L' : '.' );
  line2[2] =  ( unit & 1<<1 ? 'M' : '.' );
  line2[3] =  ( unit & 1<<2 ? 'H' : '.' );
  line2[4] =  (aux == 2 ? '2' : ' ');
  line2[5] =  ( unit & 1<<3 ? 'L' : '.' );
  line2[6] =  ( unit & 1<<4 ? 'M' : '.' );
  line2[7] =  ( unit & 1<<5 ? 'H' : '.' );

  line2[8] =  (aux == 3 ? '3' : ' ');
  line2[9] =  ( unit & 1<<6 ? 'L' : '.' );
  line2[10] = ( unit & 1<<7 ? 'M' : '.' );
  line2[11] = ( unit & 1<<8 ? 'H' : '.' );
  line2[12] = (aux == 4 ? '4' : ' ');
  line2[13] = ( unit & 1<<9 ? 'L' : '.' );
  line2[14] = ( unit & 1<<10 ? 'M' : '.' );
  line2[15] = ( unit & 1<<11 ? 'H' : '.' );
#endif
}

#ifdef POWERMETER
  void __upMFmt(void * var, uint8_t mul, uint8_t dec) {
    uint32_t unit = *(uint32_t*)var;
    // pmeter values need special treatment, too many digits to fit standard 8 bit scheme
    unit = unit / PLEVELDIVSOFT;// [0:1000] * 1000/3 samples per second(loop time) * 60 seconds *5 minutes -> [0:10000 e4] per motor
                                // (that is full throttle for 5 minutes sampling with high sampling rate for wmp only)
                                // times 6 for a maximum of 6 motors equals [0:60000 e4] for the sum
                                // we are only interested in the big picture, so divide by 10.000
    __u16Fmt(&unit, mul, dec);
  }

void __upSFmt(void * var, uint8_t mul, uint8_t dec) {
  uint32_t unit = *(uint32_t*)var;
#if defined(POWERMETER_SOFT)
  unit = unit / PLEVELDIVSOFT;
#elif defined(POWERMETER_HARD)
  unit = unit / PLEVELDIV;
#endif
  __u16Fmt(&unit, mul, dec);
}
#endif

static uint8_t lcdStickState[4];
#define IsLow(x)  (lcdStickState[x] & 0x1)
#define IsHigh(x) (lcdStickState[x] & 0x2)
#define IsMid(x)  (!lcdStickState[x])


/* ------------ DISPLAY_2LINES ------------------------------------*/
#ifdef DISPLAY_2LINES
void ConfigRefresh(uint8_t p) {
  blinkLED(10,20,1);
  #if defined(BUZZER)
    alarmArray[0] = 1;
  #endif
  strcpy_P(line1,PSTR("                "));
  strcpy(line2,line1);
  strcpy_P(line1, (char*)pgm_read_word(&(lcd_param_ptr_table[p * 3])));
  lcd_param_def_t* deft = (lcd_param_def_t*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 2]));
  deft->type->fmt((void*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 1])), deft->multiplier, deft->decimal);
  LCDclear();
  LCDsetLine(1);LCDprintChar(line1); //refresh line 1 of LCD
  LCDsetLine(2);LCDprintChar(line2);//refresh line 2 of LCD
}
#endif // DISPLAY_2LINES
/* ------------ DISPLAY_MULTILINE ---------------------------------*/
#ifdef DISPLAY_MULTILINE
// display slice of config items prior and after current item (index p)
void ConfigRefresh(uint8_t p) {
  uint8_t j, l = 1;
  int8_t pp = (int8_t)p;
  #if !defined(OLED_I2C_128x64) && !defined(OSD_DISPLAY)
   blinkLED(2,4,1);
   #if defined(BUZZER)
     alarmArray[0] = 1;
   #endif
   LCDclear();
  #else
   #ifdef DISPLAY_FONT_DSIZE
     font = 1;
   #else  
     font = 0;
   #endif
  #endif
  for (int8_t i=pp - MULTILINE_PRE; i<pp + MULTILINE_POST; i++) {
    //j = i % (1+PARAMMAX); // why does modulo not work here?
    j = (i<0 ? i + 1 + PARAMMAX : i);
    if (j > PARAMMAX) j -= (1 + PARAMMAX);
    strcpy_P(line1,PSTR("          "));
    strcpy(line2,line1);
    strcpy_P(line1, (char*)pgm_read_word(&(lcd_param_ptr_table[j * 3])));
    lcd_param_def_t* deft = (lcd_param_def_t*)pgm_read_word(&(lcd_param_ptr_table[(j * 3) + 2]));
    deft->type->fmt((void*)pgm_read_word(&(lcd_param_ptr_table[(j * 3) + 1])), deft->multiplier, deft->decimal);

    LCDsetLine(l++);
    if (j == p) {
      #if !defined(OLED_I2C_128x64) && !defined(OLED_DIGOLE) && !defined(OSD_DISPLAY)
        LCDprint('>');
      #endif
      #if ( defined(OLED_DIGOLE) )
        line2[0] = '*';
      #endif
      LCDattributesReverse();
    }
    #if defined(OSD_DISPLAY)
     if (j == p) osd_line[0]='>'; else osd_line[0]=' ';
     osd_line[1]=' ';
     strcpy(osd_line+2,line1);
     strcat(osd_line,line2+2);
     if (j == p) osd_line[20]='<'; else osd_line[20]=' ';
     osd_line[19]=' ';
     LCDprintChar(osd_line);
     delay(8);
     if (j == p) {LCDattributesOff();}
    #else
      LCDprintChar(line1); // the label
      if (j == p) {LCDattributesOff();}
      #ifdef OLED_I2C_128x64
        LCDprintChar(line2+3); // the value
      #else
        LCDprintChar(line2);   // the value
        #if !defined(OLED_DIGOLE) && !defined(OSD_DISPLAY)
          if (j == p) {LCDprint('<');}
        #endif  
      #endif
    #endif
  }
  LCDcrlf();
}
#endif // DISPLAY_MULTILINE
void configurationLoop() {
  uint8_t i, p;
  uint8_t LCD=1;
  uint8_t refreshLCD = 1;
  uint8_t key = 0;
  uint8_t allow_exit = 0;
  initLCD();
  LCDclear();
  STABLEPIN_OFF;
  LEDPIN_OFF;
  p = 0;
  while (LCD == 1) {
    if (refreshLCD) {
      ConfigRefresh(p);
      if(refreshLCD>6) refreshLCD=6;
      delay(400 - 64*(uint16_t)refreshLCD);    // dynamic repeat speed
    }
    #if defined(SPEKTRUM)
      readRawRC(1); delay(44); // For digital receivers like Spektrum, SBUS, and Serial, to ensure that an "old" frame does not cause immediate exit at startup. 
    #endif
    #if defined(ELERES_RX)
      eLeReS_control();
      computeRC();
    #endif
    #if defined(LCD_TEXTSTAR) || defined(LCD_VT100) || defined(LCD_TTY) // textstar, vt100 and tty can send keys
      key = ( SerialAvailable(LCD_SERIAL_PORT) ? SerialRead(LCD_SERIAL_PORT) : 0 );
    #endif
    #ifdef LCD_CONF_DEBUG
      delay(1000);
      if (key == LCD_MENU_NEXT) key=LCD_VALUE_UP; else key = LCD_MENU_NEXT;
    #endif
    #ifndef LCD_TELEMETRY
      annexCode();    // for V_BATT processing, collide with LCD telemetry
    #endif  
    for (i = ROLL; i <= THROTTLE; i++) {uint16_t Tmp = readRawRC(i); lcdStickState[i] = (Tmp < MINCHECK) | ((Tmp > MAXCHECK) << 1);};
    if (IsMid(YAW) && IsMid(PITCH) && IsMid(ROLL)) allow_exit = 1;
    if (key == LCD_MENU_SAVE_EXIT || (IsLow(YAW) && IsHigh(PITCH) && allow_exit)) LCD = 0; // save and exit
    else if (key == LCD_MENU_ABORT || (IsHigh(YAW) && IsHigh(PITCH) && allow_exit)) LCD = 2;// exit without save: eeprom has only 100.000 write cycles
    else if (key == LCD_MENU_NEXT || (IsLow(PITCH) && IsMid(YAW))) { //switch config param with pitch
      refreshLCD++; p++; if (p>PARAMMAX) p = 0;
    } else if (key == LCD_MENU_PREV || (IsHigh(PITCH) && IsMid(YAW))) {
      refreshLCD++; p--; if (p == 0xFF) p = PARAMMAX;
    } else if (key == LCD_VALUE_DOWN || (IsLow(ROLL))) { //+ or - param with low and high roll
      refreshLCD++;
      lcd_param_def_t* deft = (lcd_param_def_t*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 2]));
      deft->type->inc((void*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 1])), -(IsHigh(THROTTLE) ? 10: 1) * deft->increment);
      if (p == 0) conf.pid[PITCH].P8 = conf.pid[ROLL].P8;
    } else if (key == LCD_VALUE_UP || (IsHigh(ROLL))) {
      refreshLCD++;
      lcd_param_def_t* deft = (lcd_param_def_t*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 2]));
      deft->type->inc((void*)pgm_read_word(&(lcd_param_ptr_table[(p * 3) + 1])), +(IsHigh(THROTTLE) ? 10 : 1) * deft->increment);
      if (p == 0) conf.pid[PITCH].P8 = conf.pid[ROLL].P8;
    } else refreshLCD=0;
    #if defined(PRI_SERVO_TO)
      #define MAX_SERV 7
      #if(PRI_SERVO_TO < MAX_SERV)
       #undef  MAX_SERV
       #define MAX_SERV PRI_SERVO_TO
      #endif
      for(i=PRI_SERVO_FROM-1; i<MAX_SERV; i++) servo[i] = conf.servoConf[i].middle;
      #if defined(HELICOPTER) && YAWMOTOR
        servo[5] =  MINCOMMAND;
      #endif  
      #if defined(TRI) && defined(MEGA_HW_PWM_SERVOS) && defined(MEGA)
        servo[3] = servo[5];
      #endif
      writeServos();    
    #endif
  } // while (LCD == 1)
  LCDclear();
  LCDsetLine(1);
  if (LCD == 0) {
    strcpy_P(line1,PSTR(" Saving..."));
    LCDprintChar(line1);
    writeParams(1);
  } else {
    strcpy_P(line1,PSTR(" Aborting"));
    LCDprintChar(line1);
    readEEPROM(); // roll back all values to last saved state
  }
  blinkLED(20,30,1);
  #if defined(BUZZER)
    alarmArray[7] = 1;
  #endif
  LCDsetLine(2);
  strcpy_P(line1,PSTR(" Exit"));
  LCDprintChar(line1);
  #if defined(LCD_LCD03)
    delay(2000); // wait for two seconds then clear screen and show initial message
    initLCD();
  #endif
  #if defined(LCD_SERIAL3W)
    SerialOpen(0,115200);
  #endif
  #if defined(LCD_TELEMETRY) || defined(OLED_I2C_128x64)
    delay(1500); // keep exit message visible for one and one half seconds even if (auto)telemetry continues writing in main loop
  #endif
  #if defined(OSD_DISPLAY)
    delay(1500);
    serialSendOsdString(0x20, line1);    // exit command
  #endif
  cycleTime = 0;
  #if defined(OLED_I2C_128x64)
    #if defined(OLED_I2C_128x64LOGO_PERMANENT)
      i2c_OLED_Put_Logo();
    #elif !defined(LOG_PERMANENT_SHOW_AFTER_CONFIG)
      LCDclear();
    #endif
  #endif
  #ifdef LOG_PERMANENT_SHOW_AFTER_CONFIG
    if (!f.ARMED) dumpPLog(0);
  #endif
}

void LCDinfo() {
  #if defined(OLED_I2C_128x64) && !defined(LCD_TELEMETRY) && !defined(OLED_I2C_128x64LOGO_PERMANENT)
    static uint32_t dispFrq = 4000000;
    static uint8_t line = 0;
    if(currentTime < dispFrq)  return;
    dispFrq = currentTime+100000;
    font=1;
    i2c_OLED_set_line(2*line);
    switch(line) {
      case 0:
          strcpy_P(line2,PSTR("MultiWii V-.--  "));
          line2[10] = digit100(VERSION);
          line2[12] = digit10(VERSION);
          line2[13] = digit1(VERSION);
          LCDprintChar(line2);
          break;
      case 1:
          if(calibratingG)
            strcpy_P(line2,PSTR("GYRO Calibration"));
          else if(calibratingA)
            strcpy_P(line2,PSTR("ACC Calibration "));
          else if(f.CALIBRATE_MAG)
            strcpy_P(line2,PSTR("MAG Calibration "));
          #if defined(FAILSAFE)
            else if(f.FAILSAFE_ON && !rcOptions[BOXFAILSAFE])
              strcpy_P(line2,PSTR("No TX Signal !  "));
          #endif
          else {
            strcpy_P(line2,PSTR("Profile:        "));
            line2[9] = digit1(global_conf.currentSet+1);
          }
          LCDprintChar(line2);
          break;
      case 2:
          #ifdef VBAT
            strcpy_P(line2,PSTR("BAT:     V      "));
            __u8Fmt(&analog.vbat, 1, 1);
            LCDprintChar(line2);
            break;
          #else
            line++;
          #endif
      case 3:
          #if(GPS)
            if(GPS_Present) {
              strcpy_P(line2,PSTR("GPS: OK, SAT:   "));
              if(GPS_numSat>9) line2[13] = digit10(GPS_numSat);
              line2[14] = digit1(GPS_numSat);
            } else strcpy_P(line2,PSTR("GPS: Not Ready  "));
            LCDprintChar(line2);
            break;
          #else
            line++;
          #endif
    }
    if(++line>3) line=0;
  #endif
}

#endif // LCD_CONF
// -------------------- telemetry output to LCD over serial/i2c ----------------------------------

#ifdef LCD_TELEMETRY

void lcdprint_int16(int16_t v) {
  uint16_t unit;
  char line[7] = "      ";
  if (v < 0 ) {
    unit = -v;
    line[0] = '-';
  } else {
    unit = v;
    line[0] = ' ';
  }
  line[1] = digit10000(unit);
  line[2] = digit1000(unit);
  line[3] = digit100(unit);
  line[4] = digit10(unit);
  line[5] = digit1(unit);
  LCDprintChar(line);
}
void lcdprint_uint32(uint32_t v) {
  char line[14] = "-.---.---.---";
  //               0 2 4 6 8   12
  line[0]  = '0' + v  / 1000000000;
  line[2]  = '0' + v  / 100000000 - (v/1000000000) * 10;
  line[3]  = '0' + v  / 10000000  - (v/100000000)  * 10;
  line[4]  = '0' + v  / 1000000   - (v/10000000)   * 10;
  line[6]  = '0' + v  / 100000    - (v/1000000)    * 10;
  line[7]  = '0' + v  / 10000     - (v/100000)     * 10;
  line[8]  = '0' + v  / 1000      - (v/10000)      * 10;
  line[10]  = '0' + v  / 100       - (v/1000)       * 10;
  line[11] = '0' + v  / 10        - (v/100)        * 10;
  line[12] = '0' + v              - (v/10)         * 10;
  LCDprintChar(line);
}

// LCDbar(n,v) : draw a bar graph - n number of chars for width, v value in % to display
void LCDbar(uint8_t n,uint8_t v) {
  if (v > 200) v = 0;
  else if (v > 100) v = 100;
#if defined(LCD_SERIAL3W)
  for (uint8_t i=0; i< n; i++) LCDprint((i<n*v/100 ? '=' : '.'));
#elif defined(LCD_TEXTSTAR)
  LCDprint(0xFE);LCDprint('b');LCDprint(n);LCDprint(v);
#elif defined(LCD_VT100) || defined(LCD_TTY)
  uint8_t i, j = (n*v)/100;
  for (i=0; i< j; i++) LCDprint( '=' );
  for (i=j; i< n; i++) LCDprint( '.' );
#elif defined(LCD_ETPP)
  ETPP_barGraph(n,v);
#elif defined(LCD_LCD03)
  for (uint8_t i=0; i< n; i++) LCDprint((i<n*v/100 ? '=' : '.'));
#elif defined(OLED_I2C_128x64)
  uint8_t i, j = (n*v)/100;
  for (i=0; i< j; i++) LCDprint( '=' );
  for (i=j; i< n; i++) LCDprint( '.' );
#endif
}

void fill_line1_deg() {
  uint16_t unit;
  strcpy_P(line1,PSTR("Deg ---.-  ---.-"));
  // 0123456789.12345
  if (att.angle[0] < 0 ) {
    unit = -att.angle[0];
    line1[3] = '-';
  } else
  unit = att.angle[0];
  line1[4] = digit1000(unit);
  line1[5] = digit100(unit);
  line1[6] = digit10(unit);
  line1[8] = digit1(unit);
  if (att.angle[1] < 0 ) {
    unit = -att.angle[1];
    line1[10] = '-';
  } else
  unit = att.angle[1];
  line1[11] = digit1000(unit);
  line1[12] = digit100(unit);
  line1[13] = digit10(unit);
  line1[15] = digit1(unit);
}
#ifdef POWERMETER_HARD
void fill_line2_AmaxA() {
  uint16_t unit;
  strcpy_P(line2,PSTR("---,-A max---,-A"));
  unit = ((uint32_t)powerValue * conf.pint2ma) / 100;
  line2[0] = digit1000(unit);
  line2[1] = digit100(unit);
  line2[2] = digit10(unit);
  line2[4] = digit1(unit);
  unit = (powerMax * conf.pint2ma) / 100;
  line2[10] = digit1000(unit);
  line2[11] = digit100(unit);
  line2[12] = digit10(unit);
  line2[14] = digit1(unit);
}
#endif

void output_V() {
  #ifdef VBAT
    strcpy_P(line1,PSTR(" --.-V"));
    //                   0123456789.12345
    line1[1] = digit100(analog.vbat);
    line1[2] = digit10(analog.vbat);
    line1[4] = digit1(analog.vbat);
    if (analog.vbat < conf.vbatlevel_warn) { LCDattributesReverse(); }
    LCDbar(7, (((analog.vbat - conf.vbatlevel_warn)*100)/(VBATNOMINAL-conf.vbatlevel_warn)) );
    LCDattributesOff(); // turn Reverse off for rest of display
    LCDprintChar(line1);
  #endif
}

void output_Vmin() {
  #ifdef VBAT
    strcpy_P(line1,PSTR(" --.-Vmin"));
    //                   0123456789.12345
    line1[1] = digit100(vbatMin);
    line1[2] = digit10(vbatMin);
    line1[4] = digit1(vbatMin);
    if (vbatMin < conf.vbatlevel_crit) { LCDattributesReverse(); }
    LCDbar(7, (vbatMin > conf.vbatlevel_crit ? (((vbatMin - conf.vbatlevel_crit)*100)/(VBATNOMINAL-conf.vbatlevel_crit)) : 0 ));
    LCDattributesOff();
    LCDprintChar(line1);
  #endif
}
void output_mAh() {
  #ifdef POWERMETER
    strcpy_P(line1,PSTR(" -----mAh"));
    line1[1] = digit10000(analog.intPowerMeterSum);
    line1[2] = digit1000(analog.intPowerMeterSum);
    line1[3] = digit100(analog.intPowerMeterSum);
    line1[4] = digit10(analog.intPowerMeterSum);
    line1[5] = digit1(analog.intPowerMeterSum);
    if (conf.powerTrigger1) {
      int8_t v = 100 - ( analog.intPowerMeterSum/(uint16_t)conf.powerTrigger1) *2; // bar graph powermeter (scale intPowerMeterSum/powerTrigger1 with *100/PLEVELSCALE)
      if (v <= 0) { LCDattributesReverse(); } // buzzer on? then add some blink for attention
      LCDbar(7, v);
      LCDattributesOff();
    }
    LCDprintChar(line1);
  #endif
}
void fill_line1_cycle() {
  strcpy_P(line1,PSTR("Cycle    -----us")); //uin16_t cycleTime
  // 0123456789.12345*/
  //strcpy_P(line2,PSTR("(-----, -----)us")); //uin16_t cycleTimeMax
  line1[9] = digit10000(cycleTime);
  line1[10] = digit1000(cycleTime);
  line1[11] = digit100(cycleTime);
  line1[12] = digit10(cycleTime);
  line1[13] = digit1(cycleTime);
}
void fill_line2_cycleMinMax() {
  strcpy_P(line2,PSTR("(-----, -----)us")); //uin16_t cycleTimeMax
#if (LOG_VALUES >= 2)
  line2[1] = digit10000(cycleTimeMin );
  line2[2] = digit1000(cycleTimeMin );
  line2[3] = digit100(cycleTimeMin );
  line2[4] = digit10(cycleTimeMin );
  line2[5] = digit1(cycleTimeMin );
  line2[8] = digit10000(cycleTimeMax);
  line2[9] = digit1000(cycleTimeMax);
  line2[10] = digit100(cycleTimeMax);
  line2[11] = digit10(cycleTimeMax);
  line2[12] = digit1(cycleTimeMax);
#endif
}
void output_fails() {
  uint16_t unit;
  //                   0123456789012345
  strcpy_P(line2,PSTR("-- Fails  -- i2c"));
  unit = failsafeEvents;
  //line2[0] = '0' + unit / 1000 - (unit/10000) * 10;
  //line2[1] = '0' + unit / 100  - (unit/1000)  * 10;
  line2[0] = digit10(unit);
  line2[1] = digit1(unit);
  unit = i2c_errors_count;
  //line2[5] = '0' + unit / 1000 - (unit/10000) * 10;
  //line2[6] = '0' + unit / 100  - (unit/1000)  * 10;
  line2[10] = digit10(unit);
  line2[11] = digit1(unit);
  LCDprintChar(line2);
}
void output_annex() {
  //                   0123456789
  strcpy_P(line2,PSTR("annex --"));
  line2[6] = digit10(annex650_overrun_count);
  line2[7] = digit1(annex650_overrun_count);
  LCDprintChar(line2);
}
static char checkboxitemNames[][4] = {
    "Arm",
    #if ACC
      "Ang","Hor",
    #endif
    #if BARO && (!defined(SUPPRESS_BARO_ALTHOLD))
      "Bar",
    #endif
    #ifdef VARIOMETER
      "Var",
    #endif
    #if MAG
      "Mag",
      "HFr",
      "HAd",
    #endif
    #if defined(SERVO_TILT) || defined(GIMBAL)|| defined(SERVO_MIX_TILT)
      "CSt",
    #endif
    #if defined(CAMTRIG)
      "CTr",
    #endif
    #if GPS
      "GHm",
      "GHd",
    #endif
    #if defined(FIXEDWING) || defined(HELICOPTER)
      "Pas",
    #endif
    #if defined(BUZZER)
      "Buz",
    #endif
    #if defined(LED_FLASHER)
      "LEM",
      "LEL",
    #endif
    #if defined(LANDING_LIGHTS_DDR)
      "LLs",
    #endif
    #ifdef INFLIGHT_ACC_CALIBRATION
      "Cal",
    #endif
    #ifdef GOVERNOR_P
      "Gov",
    #endif
    #ifdef OSD_SWITCH
      "OSD",
    #endif
  ""};
void output_checkboxitems() {
  for (uint8_t i=0; i<CHECKBOXITEMS; i++ ) {
    if (rcOptions[i] || ((i==BOXARM)&&(f.ARMED)) ) {
      LCDprintChar(checkboxitemNames[i]);
      LCDprint(' ');
    }
  }
}

#define GYROLIMIT 60 // threshold: for larger values replace bar with dots
#define ACCLIMIT 60 // threshold: for larger values replace bar with dots
void outputSensor(uint8_t num, int16_t data, int16_t limit) {
  if (data < -limit)     {LCDprintChar("<<<<");}
  else if (data > limit) {LCDprintChar(">>>>");}
  else LCDbar(num, limit + data *50/limit);
}
void print_uptime(uint16_t sec) {
  uint16_t m, s;
  char line[6] = "--:--";
  m = sec / 60;
  s = sec - (60 * m);
  line[0] = digit10(m);
  line[1] = digit1(m);
  line[3] = digit10(s);
  line[4] = digit1(s);
  LCDprintChar(line);
}
#if GPS
void fill_line1_gps_lat(uint8_t sat) {
  int32_t aGPS_latitude = abs(GPS_coord[LAT]);
  strcpy_P(line1,PSTR(".---.-------    "));
  //                   0123456789012345
  line1[0] = GPS_coord[LAT]<0?'S':'N';
  if (sat) {
    line1[13] = '#';
    line1[14] = digit10(GPS_numSat);
    line1[15] = digit1(GPS_numSat);
  } //                                987654321
  line1[1]  = '0' + aGPS_latitude  / 1000000000;
  line1[2]  = '0' + aGPS_latitude  / 100000000 - (aGPS_latitude/1000000000) * 10;
  line1[3]  = '0' + aGPS_latitude  / 10000000  - (aGPS_latitude/100000000)  * 10;
  line1[5]  = '0' + aGPS_latitude  / 1000000   - (aGPS_latitude/10000000)   * 10;
  line1[6]  = '0' + aGPS_latitude  / 100000    - (aGPS_latitude/1000000)    * 10;
  line1[7]  = '0' + aGPS_latitude  / 10000     - (aGPS_latitude/100000)     * 10;
  line1[8]  = '0' + aGPS_latitude  / 1000      - (aGPS_latitude/10000)      * 10;
  line1[9]  = '0' + aGPS_latitude  / 100       - (aGPS_latitude/1000)       * 10;
  line1[10] = '0' + aGPS_latitude  / 10        - (aGPS_latitude/100)        * 10;
  line1[11] = '0' + aGPS_latitude              - (aGPS_latitude/10)         * 10;
}
void fill_line2_gps_lon(uint8_t status) {
  int32_t aGPS_longitude = abs(GPS_coord[LON]);
  strcpy_P(line2,PSTR(".---.-------    "));
  //                   0123456789012345
  line2[0] = GPS_coord[LON]<0?'W':'E';
  if (status) {
    line2[13] = (GPS_update ? 'U' : '.');
    line2[15] = (GPS_Present ? 'P' : '.');
  }
  line2[1]  = '0' + aGPS_longitude / 1000000000;
  line2[2]  = '0' + aGPS_longitude / 100000000 - (aGPS_longitude/1000000000) * 10;
  line2[3]  = '0' + aGPS_longitude / 10000000  - (aGPS_longitude/100000000)  * 10;
  line2[5]  = '0' + aGPS_longitude / 1000000   - (aGPS_longitude/10000000)   * 10;
  line2[6]  = '0' + aGPS_longitude / 100000    - (aGPS_longitude/1000000)    * 10;
  line2[7]  = '0' + aGPS_longitude / 10000     - (aGPS_longitude/100000)     * 10;
  line2[8]  = '0' + aGPS_longitude / 1000      - (aGPS_longitude/10000)      * 10;
  line2[9]  = '0' + aGPS_longitude / 100       - (aGPS_longitude/1000)       * 10;
  line2[10] = '0' + aGPS_longitude / 10        - (aGPS_longitude/100)        * 10;
  line2[11] = '0' + aGPS_longitude             - (aGPS_longitude/10)         * 10;
}
#endif

/* ------------ DISPLAY_2LINES ------------------------------------*/
#ifdef DISPLAY_2LINES
void lcd_telemetry() {
  static uint8_t linenr = 0;
  switch (telemetry) { // output telemetry data
    uint16_t unit;
    uint8_t i;
#ifndef SUPPRESS_TELEMETRY_PAGE_1
    case 1: // button A on Textstar LCD -> angles
    case '1':
    if (linenr++ % 2) {
      fill_line1_deg();
      LCDsetLine(1);
      LCDprintChar(line1);
    } else {
      #ifdef POWERMETER_HARD
        fill_line2_AmaxA();
        LCDsetLine(2);LCDprintChar(line2);
      #endif
    }
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_2
    case 2: // button B on Textstar LCD -> Voltage, PowerSum and power alarm trigger value
    case '2':
    if (linenr++ % 2) {
      LCDsetLine(1);
      output_V();
    } else {
      LCDsetLine(2);
      output_mAh();
    }
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_3
    case 3: // button C on Textstar LCD -> cycle time
    case '3':
    if (linenr++ % 2) {
      fill_line1_cycle();
      LCDsetLine(1);
      LCDprintChar(line1);
    } else {
#if (LOG_VALUES >= 2)
      fill_line2_cycleMinMax();
      LCDsetLine(2);
      LCDprintChar(line2);
#endif
    }
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_4
    case 4: // button D on Textstar LCD -> sensors
    case '4':
    if (linenr++ % 2) {
      LCDsetLine(1);LCDprintChar("G "); //refresh line 1 of LCD
      outputSensor(4, imu.gyroData[0], GYROLIMIT); LCDprint(' ');
      outputSensor(4, imu.gyroData[1], GYROLIMIT); LCDprint(' ');
      outputSensor(4, imu.gyroData[2], GYROLIMIT);
    } else {
      LCDsetLine(2);LCDprintChar("A "); //refresh line 2 of LCD
      outputSensor(4, imu.accSmooth[0], ACCLIMIT); LCDprint(' ');
      outputSensor(4, imu.accSmooth[1], ACCLIMIT); LCDprint(' ');
      outputSensor(4, imu.accSmooth[2] - acc_1G, ACCLIMIT);
    }
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_5
    case 5:
    case '5':
    if (linenr++ % 2) {
      LCDsetLine(1);
      output_fails();
    } else {
      LCDsetLine(2);
      output_annex();
    }
      break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_6
    case 6: // RX inputs
    case '6':
    if (linenr++ % 2) {
      strcpy_P(line1,PSTR("Roll Pitch Throt"));
      if (f.ARMED) line2[14] = 'A'; else line2[14] = 'a';
      if (failsafeCnt > 5) line2[15] = 'F'; else line2[15] = 'f';
      LCDsetLine(1);LCDprintChar(line1);
    } else {
      // 0123456789012345
      strcpy_P(line2,PSTR("---- ---- ----xx"));
      line2[0] = digit1000( rcData[ROLL] );
      line2[1] = digit100( rcData[ROLL] );
      line2[2] = digit10( rcData[ROLL] );
      line2[3] = digit1( rcData[ROLL] );
      line2[5] = digit1000( rcData[PITCH] );
      line2[6] = digit100( rcData[PITCH] );
      line2[7] = digit10( rcData[PITCH] );
      line2[8] = digit1( rcData[PITCH] );
      line2[10] = digit1000( rcData[THROTTLE] );
      line2[11] = digit100( rcData[THROTTLE] );
      line2[12] = digit10( rcData[THROTTLE] );
      line2[13] = digit1( rcData[THROTTLE] );
      LCDsetLine(2);LCDprintChar(line2);
    }
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_7
    case 7:
    case '7':
      #if GPS
      if (linenr++ % 2) {
        fill_line1_gps_lat(1); // including #sat
        LCDsetLine(1);LCDprintChar(line1);
       
      } else {
        fill_line2_gps_lon(1); // including status info
        LCDsetLine(2);LCDprintChar(line2);
      }
      #endif // case 7 : GPS
      break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_9
    case 9:
    case '9':
      LCDsetLine(1);
      lcdprint_int16(debug[0]); LCDprint(' ');
      lcdprint_int16(debug[1]); LCDprint(' ');
      lcdprint_int16(debug[2]); LCDprint(' ');
      lcdprint_int16(debug[3]); LCDprint(' ');
      break;
#endif

#if defined(LOG_VALUES) && defined(DEBUG)
    case 'R':
    //Reset logvalues
    cycleTimeMax = 0;// reset min/max on transition on->off
    cycleTimeMin = 65535;
    telemetry = 0;// no use to repeat this forever
    break;
#endif // case R
#ifdef DEBUG
    case 'F':
    extern unsigned int __bss_end;
    extern unsigned int __heap_start;
    extern void *__brkval;
    int free_memory;
    if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
    free_memory = ((int)&free_memory) - ((int)__brkval);
    strcpy_P(line1,PSTR(" Free ----")); // uint8_t free_memory
    line1[6] = digit1000( free_memory );
    line1[7] = digit100( free_memory );
    line1[8] = digit10( free_memory );
    line1[9] = digit1( free_memory );
    LCDsetLine(1); LCDprintChar(line1);
    break;
#endif // DEBUG
    // WARNING: if you add another case here, you should also add a case: in Serial.pde, so users can access your case via terminal input
  } // end switch (telemetry)
} // end function lcd_telemetry

#endif // DISPLAY_2LINES
/* ------------ DISPLAY_MULTILINE ------------------------------------*/
#ifdef DISPLAY_MULTILINE
#ifndef SUPPRESS_TELEMETRY_PAGE_5
void outputMotorServo(uint8_t i, uint16_t unit) {
  #ifdef HELICOPTER
    static char outputNames[16][3] = {"M1", " 2"," 3", " 4", " 5", " 6", " 7", " 8",
      "S1", "S2","S3", "SN", "SL", "ST", "SR", "SM",};
  #else
    static char outputNames[16][3] = {"M1", " 2"," 3", " 4", " 5", " 6", " 7", " 8",
      "S1", "S2","S3", "S4", "S5", "S6", "S7", "S8",};
  #endif
  LCDprintChar(outputNames[i]);
  LCDprint(' ');
  //unit = servo[j-1]; // [1000 ; 2000]
  LCDprint( digit1000(unit) );
  LCDprint( digit100(unit) );
  LCDprint( digit10(unit) );
  LCDprint( digit1(unit) );
  LCDprint(' ');
  unit = constrain(unit,1000,2000);
  LCDbar(12, (unit-1000)/10 );
  LCDcrlf();
}
#endif

void lcd_telemetry() {
  static uint8_t linenr = 0;
  #ifdef DISPLAY_FONT_DSIZE
    uint8_t offset = 0;
    #define POSSIBLE_OFFSET offset
  #else
    #define POSSIBLE_OFFSET 0
  #endif
  switch (telemetry) { // output telemetry data
    uint16_t unit;
    uint8_t i;
    case '0': // request to turn telemetry off - workaround, cannot enter binary zero \000 into string
      telemetry = 0;
      break;
#ifndef SUPPRESS_TELEMETRY_PAGE_1
  #ifdef DISPLAY_FONT_DSIZE
    case '!':
      { offset = 4; }
      // no break !!
  #endif
    case 1:// overall display
    case '1':
    {
      static uint8_t index = 0;
      linenr = (index++ % 8) + 1;
      LCDsetLine(linenr);
      switch (linenr + POSSIBLE_OFFSET) { // not really linenumbers
        case 1:// V
          output_V();
          break;
        case 2:// mAh
          output_mAh();
          break;
        case 4:// errors or ...
          if (failsafeEvents || (i2c_errors_count>>10)) { // errors
            // ignore i2c==1 because of bma020-init
            LCDalarmAndReverse();
            output_fails();
            LCDattributesOff();
          } else { // ... armed time
            uint16_t ats = armedTime / 1000000;
            #ifdef ARMEDTIMEWARNING
              if (ats > conf.armedtimewarning) { LCDattributesReverse(); }
              LCDbar(7, (ats < conf.armedtimewarning ? (((conf.armedtimewarning-ats+1)*100)/(conf.armedtimewarning+1)) : 0 ));
              LCDattributesOff();
            #endif
            LCDprint(' ');
            print_uptime(ats);
          }
          break;
        case 6:// height
          #if BARO
             {
               int16_t h = alt.EstAlt / 100;
               LCDprint('A'); lcdprint_int16(h); LCDprint('m');
               h = BAROaltMax / 100;
               LCDprintChar(" ("); lcdprint_int16(h);
             }
           #endif
           break;
        case 8:// uptime, eeprom set#
          strcpy_P(line1,PSTR("Up ")); LCDprintChar(line1); print_uptime(millis() / 1000 );
          strcpy_P(line1,PSTR("  Cset -")); line1[7] = digit1(global_conf.currentSet); LCDprintChar(line1);
          break;
        case 3:// Vmin
           output_Vmin();
           break;
        case 7:// A, maxA
          #ifdef POWERMETER_HARD
            fill_line2_AmaxA();
            LCDprintChar(line2);
          #endif
          break;
        case 5: // checkboxstatus
          #ifdef BUZZER
            if (isBuzzerON()) { LCDalarmAndReverse(); } // buzzer on? then add some blink for attention
          #endif
          strcpy_P(line1,PSTR(".   .   .   .   "));
          LCDprintChar(line1);
          LCDsetLine(linenr); // go to beginning of same line again
          output_checkboxitems();
          LCDattributesOff();
          break;
      }
      LCDcrlf();
    break;
    }
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_2
  #ifdef DISPLAY_FONT_DSIZE
    case '@':
      { offset = 3; }
      // no break !!
  #endif
    case 2: // sensor readings
    case '2':
    static char sensorNames[6][3] = {"Gx", "Gy", "Gz", "Ax", "Ay", "Az"};
    i = linenr++ % 6;
    LCDsetLine((i - POSSIBLE_OFFSET)%6 + 1);
    LCDprintChar(sensorNames[i]);
    LCDprint(' ');
    switch (i) {
      case 0:
        lcdprint_int16(imu.gyroData[0]); LCDprint(' ');
        outputSensor(10, imu.gyroData[0], GYROLIMIT);
      break;
      case 1:
        lcdprint_int16(imu.gyroData[1]); LCDprint(' ');
        outputSensor(10, imu.gyroData[1], GYROLIMIT);
      break;
      case 2:
        lcdprint_int16(imu.gyroData[2]); LCDprint(' ');
        outputSensor(10, imu.gyroData[2], GYROLIMIT);
      break;
      case 3:
        lcdprint_int16(imu.accSmooth[0]); LCDprint(' ');
        outputSensor(10, imu.accSmooth[0], ACCLIMIT);
      break;
      case 4:
        lcdprint_int16(imu.accSmooth[1]); LCDprint(' ');
        outputSensor(10, imu.accSmooth[1], ACCLIMIT);
      break;
      case 5:
        lcdprint_int16(imu.accSmooth[2]); LCDprint(' ');
        outputSensor(10, imu.accSmooth[2] - ACC_1G, ACCLIMIT);
      break;
    }
    LCDcrlf();
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_3
  #ifdef DISPLAY_FONT_DSIZE
    case '#':
      { offset = 4; }
    // no break !!
  #endif
    case 3: // checkboxes and modes
    case '3':
    {
      static uint8_t index = 0;
      index %= CHECKBOXITEMS;
      if (index == 0) linenr = 1;
      LCDsetLine((linenr++ + POSSIBLE_OFFSET)%CHECKBOXITEMS);
      LCDprintChar(checkboxitemNames[index]);
      //LCDprintChar((PGM_P)(boxnames[index]));
      LCDprint(' ');
      LCDprint( rcOptions[index] ? 'X' : '.');
      LCDcrlf();
      index++;
      break;
    }
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_4
  #ifdef DISPLAY_FONT_DSIZE
    case '$':
      { offset = 4; }
      // no break !!
  #endif
    case 4: // RX inputs
    case '4':
    static char channelNames[8][4] = {"Ail", "Ele", "Yaw", "Thr", "Ax1", "Ax2", "Ax3", "Ax4"};
    i = linenr++ % 8; // 8 channels
    LCDsetLine((i - POSSIBLE_OFFSET)%8 + 1);
    //strcpy_P(line1,PSTR("-Thr ---- "));
    //                   0123456789.12345
    LCDprint( '0' + i+1);// channel numbering [1;8]
    LCDprint(' ');
    LCDprintChar(channelNames[i]);
    LCDprint(' ');
    unit = rcData[i];
    LCDprint( digit1000(unit) );
    LCDprint( digit100(unit) );
    LCDprint( digit10(unit) );
    LCDprint( digit1(unit) );
    LCDprint(' ');
    unit = constrain(rcData[i],1000,2000);
    LCDbar(10, (unit-1000)/10 );
    LCDcrlf();
    break;
#endif
#ifndef SUPPRESS_TELEMETRY_PAGE_5
  #ifdef DISPLAY_FONT_DSIZE
    case '%':
      { offset = 4; }
      // no break !!
  #endif
    case 5: // outputs motors+servos
    case '5':
    {
//      static char outputNames[16][3] = {"M1", " 2"," 3", " 4", " 5", " 6", " 7", " 8",
//          "S1", "S2","S3", "S4", "S5", "S6", "S7", "S8",};
      static uint8_t index = 0;
      i = index++ % 16;
      if (i == 0) linenr = 0; //vt100 starts linenumbering @1
      LCDsetLine((linenr - POSSIBLE_OFFSET)%8 +1);
      if (i < 8) {
        if (i < NUMBER_MOTOR) {
          outputMotorServo(i, motor[i]);
          linenr++;
        } else {
          index = 8;
        }
      } else {
        uint8_t j = i-7; // [8;15] -> [1;8]
        #if defined(PRI_SERVO_FROM) && defined(SEC_SERVO_FROM)
          if ((PRI_SERVO_FROM <= j && PRI_SERVO_TO >= j) || (SEC_SERVO_FROM <= j && SEC_SERVO_TO >= j))
        #elif defined(PRI_SERVO_FROM)
          if (j < PRI_SERVO_FROM) index = 7 + PRI_SERVO_FROM;
          else if (j > PRI_SERVO_TO) index = 16;
          else // (PRI_SERVO_FROM <= j && PRI_SERVO_TO >= j)
        #endif
        #if defined(PRI_SERVO_FROM) || defined(SEC_SERVO_FROM)
          {
            outputMotorServo(i, servo[j-1]);
            linenr++;
            break;
          }
        #endif

      }
      break;
    }
#endif // page 5

#ifndef SUPPRESS_TELEMETRY_PAGE_7
  #if GPS
    #ifdef DISPLAY_FONT_DSIZE
    case '&':
      { offset = 4; }
      // no break !!
    #endif
    case 7: // GPS
    case '7':
      linenr++;
      linenr %= 6;
      LCDsetLine(linenr+1);
      switch (linenr + POSSIBLE_OFFSET) {
        case 0: // lat
          fill_line1_gps_lat(0); // skip #sat
          LCDprintChar(line1);
          break;
        case 1: // lon
          fill_line2_gps_lon(0);
          LCDprintChar(line2);
          break;
        case 2: // # Sats
          strcpy_P(line1,PSTR("-- Sats"));
          //                   0123456789012345
          line1[0] = digit10(GPS_numSat);
          line1[1] = digit1(GPS_numSat);
          LCDprintChar(line1);
          break;
        case 3: //
          strcpy_P(line1,PSTR("Status "));
          //                   0123456789012345
          LCDprintChar(line1);
          if (GPS_Present)
            LCDprintChar("OK");
          else {
            LCDattributesReverse();
            LCDprintChar("KO");
            LCDattributesOff();
          }
        break;
        case 4: // gps speed
        {
          uint8_t v = (GPS_speed * 0.036f);
          strcpy_P(line1,PSTR("Speed --km/h"));
          //                   0123456789012345
          line1[6] = digit10(v);
          line1[7] = digit1(v);
          LCDprintChar(line1);
          break;
        }
        case 5: // vbat
          output_V();
          break;
      }
      LCDcrlf();
    break;
  #endif // gps
#endif // page 7

#ifndef SUPPRESS_TELEMETRY_PAGE_9
  #ifdef DISPLAY_FONT_DSIZE
      case '(':
      { offset = 4; }
      // no break !!
  #endif
    case 9: // diagnostics
    case '9':
    linenr++;
    linenr %= 8;
    LCDsetLine(linenr+1);
    switch (linenr + POSSIBLE_OFFSET) {
      case 0:// cycle
      fill_line1_cycle();
      LCDprintChar(line1);
      break;
      case 1:// cycle min/max
      fill_line2_cycleMinMax();
      LCDprintChar(line2);
      break;
      case 2:// Fails, i2c
      output_fails();
      break;
      case 3:// annex-overruns
      output_annex();
      break;
#ifdef DEBUG
      case 4:// debug
      LCDprintChar("D1 ");
      lcdprint_int16(debug[0]);
      break;
      case 5:// debug
      LCDprintChar("D2 ");
      lcdprint_int16(debug[1]);
      break;
      case 6:// debug
      LCDprintChar("D3 ");
      lcdprint_int16(debug[2]);
      break;
      case 7:// debug
      LCDprintChar("D4 ");
      lcdprint_int16(debug[3]);
      break;
#endif
    }
    LCDcrlf();
    break;
#endif // page 9
#if defined(LOG_VALUES) || defined(DEBUG)
    case 'R':
    {
      //Reset logvalues
      cycleTimeMax = 0;
      cycleTimeMin = 65535;
      #if BARO
        #if defined(LOG_VALUES)
          BAROaltMax = 0;
        #endif
      #endif
      failsafeEvents = 0; // reset failsafe counter
      i2c_errors_count = 0;
      f.OK_TO_ARM = 1; // allow arming again
      telemetry = 0; // no use to repeat this forever
      break;
    }
#endif // case R
#ifdef DEBUG
    case 'F':
    extern unsigned int __bss_end;
    extern unsigned int __heap_start;
    extern void *__brkval;
    int free_memory;
    if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
    free_memory = ((int)&free_memory) - ((int)__brkval);
    strcpy_P(line1,PSTR(" Free ----")); // uint8_t free_memory
    line1[6] = digit1000( free_memory );
    line1[7] = digit100( free_memory );
    line1[8] = digit10( free_memory );
    line1[9] = digit1( free_memory );
    LCDsetLine(1); LCDprintChar(line1);
    telemetry = 0;// no use to repeat this forever
    break;
#endif // DEBUG
    // WARNING: if you add another case here, you should also add a case: in Serial.pde, so users can access your case via terminal input
  } // end switch (telemetry)
} // end function lcd_telemetry

#endif // DISPLAY_MULTILINE

void toggle_telemetry(uint8_t t) {
  if (telemetry == t) telemetry = 0; 
  else {
     telemetry = t; 
     #ifdef OLED_I2C_128x64
       if (telemetry != 0) i2c_OLED_init();
     #endif
     LCDclear();
  }
}
#endif //  LCD_TELEMETRY

#ifdef LOG_PERMANENT
  void dumpPLog(uint8_t full) {
    #ifdef HAS_LCD
      /*LCDclear();*/ LCDnextline();
      if (full) {
        #ifdef DEBUG
          LCDprintChar("#arm   "); lcdprint_int16(plog.arm); LCDnextline();
          LCDprintChar("#disarm"); lcdprint_int16(plog.disarm); LCDnextline();
          LCDprintChar("last[s]"); lcdprint_int16(plog.armed_time/1000000); LCDnextline();
          LCDprintChar("#fail@dis"); lcdprint_int16(plog.failsafe); LCDnextline();
          LCDprintChar("#i2c@dis "); lcdprint_int16(plog.i2c); LCDnextline();
          //            0123456789012345
        #endif
      }
      LCDprintChar("LastOff   "); LCDprintChar(plog.running ? "KO" : "ok");  LCDnextline();
      LCDprintChar("#On      "); lcdprint_int16(plog.start); LCDnextline();
      LCDprintChar("Life[min]"); lcdprint_int16(plog.lifetime/60); LCDnextline();
      /*strcpy_P(line2,PSTR("Fail --- i2c ---"));
      line2[5] = digit100(plog.failsafe);
      line2[6] = digit10(plog.failsafe);
      line2[7] = digit1(plog.failsafe);
      line2[13] = digit100(plog.i2c);
      line2[14] = digit10(plog.i2c);
      line2[15] = digit1(plog.i2c);
      LCDprintChar(line2); LCDnextline();*/
      delay(4000);
    #endif
    #ifdef LOG_PERMANENT_SERVICE_LIFETIME
      serviceCheckPLog();
    #endif
    #ifdef HAS_LCD
      LCDclear();
    #endif
  }

  void LCDnextline() {
    #if ( defined(DISPLAY_MULTILINE) )
      lnr++;
      if (lnr > (MULTILINE_PRE+MULTILINE_POST)) {
        lnr = 1;
        delay(4000);
        LCDclear();
      }
      LCDsetLine(lnr);
      LCD_FLUSH;
    #elif ( defined(DISPLAY_2LINES))
      #if (! (defined(LCD_TTY)  ) )
        delay(600);
      #endif
      LCDprintChar("\r\n");
    #else
      // no LCD, nothing to do here
    #endif
  }

  #ifdef LOG_PERMANENT_SERVICE_LIFETIME
  void serviceCheckPLog() {
    if ( (!f.ARMED) && (plog.lifetime > LOG_PERMANENT_SERVICE_LIFETIME) ){
      for (uint8_t i = 0; i<max(1, min(9,(plog.lifetime-LOG_PERMANENT_SERVICE_LIFETIME)>>10 )); i++) {
        #ifdef HAS_LCD
          LCDprintChar("SERVICE lifetime"); LCDnextline();
        #endif
        blinkLED(5,200,5);
        delay(5000);
      }
      alarmArray[7] = 3;
    }
  }
  #endif // LOG_PERMANENT_SERVICE_LIFETIME

#endif // LOG_PERMANENT

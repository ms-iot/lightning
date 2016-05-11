#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <inttypes.h>
#include "Print.h"
#include "Lightning.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LiquidCrystal : public Print {
public:
    LIGHTNING_DLL_API LiquidCrystal(uint8_t rs, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    LIGHTNING_DLL_API LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    LIGHTNING_DLL_API LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
    LIGHTNING_DLL_API LiquidCrystal(uint8_t rs, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

    LIGHTNING_DLL_API void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

    LIGHTNING_DLL_API void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

    LIGHTNING_DLL_API void clear();
    LIGHTNING_DLL_API void home();

    LIGHTNING_DLL_API void noDisplay();
    LIGHTNING_DLL_API void display();
    LIGHTNING_DLL_API void noBlink();
    LIGHTNING_DLL_API void blink();
    LIGHTNING_DLL_API void noCursor();
    LIGHTNING_DLL_API void cursor();
    LIGHTNING_DLL_API void scrollDisplayLeft();
    LIGHTNING_DLL_API void scrollDisplayRight();
    LIGHTNING_DLL_API void leftToRight();
    LIGHTNING_DLL_API void rightToLeft();
    LIGHTNING_DLL_API void autoscroll();
    LIGHTNING_DLL_API void noAutoscroll();

    LIGHTNING_DLL_API void setRowOffsets(int row1, int row2, int row3, int row4);
    LIGHTNING_DLL_API void createChar(uint8_t, uint8_t[]);
    LIGHTNING_DLL_API void setCursor(uint8_t, uint8_t);
    LIGHTNING_DLL_API virtual size_t write(uint8_t);
    LIGHTNING_DLL_API void command(uint8_t);

    using Print::write;
private:
    void send(uint8_t, uint8_t);
    void write4bits(uint8_t);
    void write8bits(uint8_t);
    void pulseEnable();

    uint8_t _rs_pin; // LOW: command.  HIGH: character.
    uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
    uint8_t _enable_pin; // activated by a HIGH pulse.
    uint8_t _data_pins[8];

    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;

    uint8_t _initialized;

    uint8_t _numlines;
    uint8_t _row_offsets[4];
};

#endif

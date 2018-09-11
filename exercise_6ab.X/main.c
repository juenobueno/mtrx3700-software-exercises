/* exercise_6 Using the LCD
 *
 * Jeffrey Quach    460 364 634
 * Julian Hamama    460 368 090
 */

#include <p18f452.h>
#include "ConfigRegs.h"
#include <delays.h>

#define MAX_LCD_COL 15
#define MAX_LCD_ROW 1

unsigned int currentLCDCol = 0;
unsigned int currentLCDRow = 0;

void checkBusyFlag() {
    if (PORTDbits.RD3 == 0) {
        return 1; 
    } else {
        return 0; 
    }
}

// Given a 4 bit command in the lower nibble, Bits RD0 - RD3 will be written.
// This command is then clocked to the LCD. pg24
void writeCommandToLCD(unsigned char cmd) {
    // Write mode
    PORTDbits.RD5 = 0;
    
    // Command Mode
    PORTDbits.RD4 = 0;
    
    // Move the command into the lower nibble
    PORTD &= 0xF0; 
    PORTD |= cmd; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
    
    // E High - Start Data Write
    PORTDbits.RD6 = 1; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
    
    // E Low - Bring it back to low, ready for the next read/write
    PORTDbits.RD6 = 0; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
}

// Given a 4 bit data in the lower nibble, Bits RD0 - RD3 will be written.
// This data is then clocked to the LCD. pg24
void writeDataToLCD(unsigned char data) {
    // Write mode
    PORTDbits.RD5 = 0;
    
    // Data Mode
    PORTDbits.RD4 = 1;
    
    // Move the command into the lower nibble
    PORTD &= 0xF0; 
    PORTD |= data; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
    
    // E High - Start Data Write
    PORTDbits.RD6 = 1; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
    
    // E Low - Bring it back to low, ready for the next read/write
    PORTDbits.RD6 = 0; 
    
    // Delay 20 Tcy 
    Delay10TCYx(2);
}

void writeStringToLCDAtLocation(unsigned char *str, int col, int r) {
    // Change to the given location
    unsigned char startAddress = 0; 
    unsigned char lowNibble = 0;
    unsigned char highNibble = 0;
    
    if (r == 0) {
        startAddress = 0;
    } else if (r == 1) {
        startAddress = 0x40; 
    }
    
    startAddress += col;
    
    lowNibble = startAddress & 0b00001111; 
    highNibble = startAddress & 0b1111000; 
    highNibble = highNibble >> 4;
    
    // Set this bit, pg24
    highNibble |= 0b00001000;
    
    writeCommandToLCD(highNibble);
    writeCommandToLCD(lowNibble);
    
    while(*str != '\0') {
        writeDataToLCD((*str & 0b11110000) >> 4);
        writeDataToLCD(*str & 0b00001111);
        
        str++;
    }
    
    
}

void main() {
    unsigned char test[] = "90 for the boys\0";
    
    TRISD = 0;
    PORTD = 0; 
    
    PORTDbits.RD7 = 1;
    
    // Give time for the LCD to Power ON
    Delay10KTCYx(2);
    
    // Set the LCD to 4 bit mode, from now on writes have to occur in pairs
    writeCommandToLCD(0b0010);
    
    // Function Set: 2 Lines, 5x8 Dots, pg24, pg25
    writeCommandToLCD(0b0010);
    writeCommandToLCD(0b1000);
    
    // Display on, Cursor On, pg24
    writeCommandToLCD(0b0000);
    writeCommandToLCD(0b1110);
    
    // Entry Mode, pg24
    writeCommandToLCD(0b0000);
    writeCommandToLCD(0b0110);
    
    writeStringToLCDAtLocation(test, 2, 1); 
 
    while(1);
}
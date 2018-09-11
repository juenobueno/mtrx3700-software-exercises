/* exercise_6c Using the LCD and USART Serial
 *
 * Jeffrey Quach    460 364 634
 * Julian Hamama    460 368 090
 */

#include <p18f452.h>
#include <delays.h>
#include <stdlib.h>
#include "ConfigRegs.h"

#define ROW_BUF_SIZE 2
#define COL_BUF_SIZE 3
#define MSG_BUF_SIZE 33

unsigned char rowBuf[ROW_BUF_SIZE];
unsigned char colBuf[COL_BUF_SIZE];
unsigned char messageBuf[MSG_BUF_SIZE]; 

int rowIndex;
int colIndex = 0; 
int messageIndex = 0;
int rowChosen = 0; 
int colChosen = 0;
int messageDisplayed = 0;
int row = 0;
int col = 0;

int tx232C(unsigned char *str) {
	// Loop until null character
    while(*str != '\0') {
        TXREG = *str; 
        
        // Wait until transmit is finished
        while (TXSTAbits.TRMT == 0);
        
        str++; 
    }
}

void initSerial(void){
    RCSTAbits.SPEN = 1;		// Enable serial port, pg167
    TXSTAbits.SYNC = 0;     // USART in async mode, pg166
    TXSTAbits.BRGH = 1;		// USART in High Speed async mode, pg166 
    SPBRG = 25;        		// Sets the baud rate to 9600, pg171
}

void initSerialReceive() {
    PIE1bits.TXIE = 0;      // Disable USART Transmit Interrupt, pg80
    TXSTAbits.TXEN = 0;		// Disable transmit for USART, pg166
    PIE1bits.RCIE = 1;      // Enable USART Receive Interrupt, pg80
    RCSTAbits.CREN = 1;		// Enables receive for USART, pg167
    
    IPR1bits.RCIP = 1;      // Set USART Receive Interrupt Priority to HIGH, pg82
}

void initSerialTransmit(void) {
    PIE1bits.RCIE = 0;      // Disable USART Receive Interrupt, pg80
    RCSTAbits.CREN = 0;		// Disable receive for USART, pg167
    PIE1bits.TXIE = 0;      // Disable USART Transmit Interrupt, pg80
	TXSTAbits.TXEN = 1;		// Enables transmit for USART, pg166
}

void initInterrupts() {    
    RCONbits.IPEN = 1;      // Enable priority levels on interrupts, pg53
    INTCONbits.GIEH = 1;    // Enables all low priority interrupts, pg75
    INTCONbits.GIEL = 1;    // Enables all high priority interrupts, pg75
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

void initLCD() {
    // PORTD to output and clear PORTD
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
}

void writeStringToLCDAtLocation() {
    // Change to the given location
    unsigned char startAddress = 0; 
    unsigned char lowNibble = 0;
    unsigned char highNibble = 0;
    
    // Clear the display, pg24
    // This command seems to need a delay in order to work properly
    writeCommandToLCD(0b0000);
    writeCommandToLCD(0b0001);
    Delay10KTCYx(2);
    
    // Get the value of the start address
    if (row == 0) {
        startAddress = 0;
    } else if (row == 1) {
        startAddress = 0x40; 
    }
    startAddress += col;
    
    // Separate the start address into high and low nibbles
    lowNibble = startAddress & 0b00001111; 
    highNibble = startAddress & 0b1111000; 
    highNibble = highNibble >> 4;
    
    // Set bit 3 as this is required for DDRAM command, pg24
    highNibble |= 0b00001000;
    
    // Set the new cursor location
    writeCommandToLCD(highNibble);
    writeCommandToLCD(lowNibble);
    

    // Display messageBuf on LCD, messageIndex is reused as an index to 
    // iterate through the message
    messageIndex = 0;
    
    while(messageBuf[messageIndex] != '\0') {
		// Send the current character as high and low nibbles
        writeDataToLCD((messageBuf[messageIndex] & 0b11110000) >> 4);
        writeDataToLCD(messageBuf[messageIndex] & 0b00001111);
        
		messageIndex++;
        
        col++;
        // Check if end of LCD is reached
        if (row == 1 && col == 16) {
            break;
        }
        
        // Check if overflow to second line of LCD is required
        
        if (row == 0 && col == 16) {
            // Set the new cursor location to 0x40 (Beginning of second line)
            writeCommandToLCD(0b1100);
            writeCommandToLCD(0b0000);
            
            row = 1; 
            col = 0;
        }
	}
}

void main() {      
    unsigned char msg1[] = "Select starting row (0 or 1): \0";
    unsigned char msg2[] = "Select starting column (0 - 15): \0";
    unsigned char msg3[] = "Select a message to display on LCD (MAX 32 CHARS): \0";
    unsigned char msg4[] = "\r\n\0";
    
    initLCD();
	initSerial();
    initInterrupts();
    
    // LEDs use for debugging
    TRISB = 0;
    PORTB = 0x0F;
    
    while(1) {
        initSerialTransmit();
        tx232C(msg1);
        initSerialReceive(); 
    
        // Wait until a row has been selected
        while(rowChosen == 0); 
        
        initSerialTransmit();
        tx232C(msg4); 
        tx232C(msg2); 
        initSerialReceive();
        
        // Wait until a column has been selected
        while(colChosen == 0);
        
        initSerialTransmit();
        tx232C(msg4); 
        tx232C(msg3); 
        initSerialReceive();
        
        // Wait until the message is displayed 
        while(messageDisplayed == 0);
        
        initSerialTransmit();
        tx232C(msg4); 
    }  
}

#pragma interrupt highPriorityISR
void highPriorityISR(void) {
	// This High Priority ISR should check any interrupt flags that we're
	// interested in, and then go to some subroutine ideally.

	// Check if USART Receive Interrupt Triggered. This means RCREG is full.
	// Flag should automatically be cleared when RCREG is read. pg78
	if (PIR1bits.RCIF == 1) {
		// Store the character read
        unsigned char temp = RCREG;  
        
        // If 'ENTER' was pressed, do some processing with the result,
        // else load it into the correct buffer
        if (temp == '\r') {
            if (rowChosen == 0) {
                rowBuf[rowIndex] = '\0';
                row = atoi(rowBuf);
                rowChosen = 1;
                messageDisplayed = 0;
            } else if (colChosen == 0) {
                colBuf[colIndex] = '\0';
                col = atoi(colBuf);
                colChosen = 1;
            } else {
                messageBuf[messageIndex] = '\0';
                writeStringToLCDAtLocation(col, row);
                rowChosen = 0;
                colChosen = 0;
                rowIndex = 0;
                colIndex = 0;
                messageIndex = 0;
                messageDisplayed = 1;
            }
        } else {
            // Relay the received character back to the user so they can see it
            initSerialTransmit();
            TXREG = temp; 
            while (TXSTAbits.TRMT == 0);
            initSerialReceive();
            
            // Store the character in the correct buffer
            if (rowChosen == 0) {
                if (rowIndex < ROW_BUF_SIZE) {
                    rowBuf[rowIndex] = temp;
                    rowIndex++;
                }
            } else if (colChosen == 0) {
                if (colIndex < COL_BUF_SIZE) {
                    colBuf[colIndex] = temp; 
                    colIndex++;
                }
            } else {
                if (messageIndex < MSG_BUF_SIZE) {
                    messageBuf[messageIndex] = temp;
                    messageIndex++; 
                }
            }
        }
	}
    
    // Every time interrupt is triggered, lights should swap
    PORTB = ~PORTB;
}

#pragma code highISR = 0x08
void goToHighISR(void) {
    highPriorityISR();
}
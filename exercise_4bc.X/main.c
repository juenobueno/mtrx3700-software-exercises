/* exercise_4b Interrupt Driven Serial Communication
 * 
 * Jeffrey Quach    460 364 634
 * Julian Hamama    460 368 090  
 *
 * So we assume that the program begins in receive mode,
 * then when the character received is 'carriage return'
 * is received, we swap to transmit mode, transmit the
 * received inputs and go back to receive mode. This should
 * all occur in the interrupt.
 * 
 * Something important to note is that only either RECEIVE or TRANSMIT interrupts
 * can be enabled for USART. Otherwise neither of them trigger. 
 */

#include <p18f452.h>
#include <stdlib.h>
#include "ConfigRegs.h"

#define MAX_BUFFER_SIZE 7

typedef struct {
   unsigned char buf[MAX_BUFFER_SIZE];
   int  receiveIndex;   // Location of next character to be put in buffer
   int  transmitIndex;  // Location of next character to be transmitted 
} circBuff ;

circBuff message;

// LRC is referenced as the first character of this string. Initialized to 0 first
unsigned char LRC[] = "\0\0"; 
unsigned char newLine[] = "\r\n\0";
unsigned char msg1[] = "Received: \0";
unsigned char msg2[] = "The decimal for the LRC was: \0";


int tx232C(unsigned char *txPtr) {
	int i = 0;
    
    // Loop until null character
    while(*txPtr != '\0') {
        TXREG = *txPtr; 
        LRC[0] ^= *txPtr;
        
        // Wait until transmit is finished
        while (TXSTAbits.TRMT == 0);
        
        txPtr++; 
        i++;

        // Check for circular buffer overflow 
        if (txPtr == message.buf + MAX_BUFFER_SIZE) {
            txPtr = message.buf;
        }
	}
    
    return i;
}

void initSerial(void){
    RCSTAbits.SPEN = 1;		// Enable serial port, pg167
    TXSTAbits.SYNC = 0;     // USART in async mode, pg166
    TXSTAbits.BRGH = 1;		// USART in High Speed async mode, pg166 
    SPBRG = 25;        		// Sets the baud rate to 9600, pg171
}

void initSerialTransmit(void) {
    PIE1bits.RCIE = 0;      // Disable USART Receive Interrupt, pg80
    RCSTAbits.CREN = 0;		// Disable receive for USART, pg167
    PIE1bits.TXIE = 1;      // Enable USART Transmit Interrupt, pg80
	TXSTAbits.TXEN = 1;		// Enables transmit for USART, pg166
    
    IPR1bits.TXIP = 1;      // Set USART Transmit Interrupt Priority to HIGH, pg82
}


void initSerialReceive() {
    PIE1bits.TXIE = 0;      // Disable USART Transmit Interrupt, pg80
    TXSTAbits.TXEN = 0;		// Disable transmit for USART, pg166
    PIE1bits.RCIE = 1;      // Enable USART Receive Interrupt, pg80
    RCSTAbits.CREN = 1;		// Enables receive for USART, pg167
    
    IPR1bits.RCIP = 1;      // Set USART Receive Interrupt Priority to HIGH, pg82
}

void initInterrupts() {    
    RCONbits.IPEN = 1;      // Enable priority levels on interrupts, pg53
    INTCONbits.GIEH = 1;    // Enables all low priority interrupts, pg75
    INTCONbits.GIEL = 1;    // Enables all high priority interrupts, pg75
}

void main() {
    int i = 0;
    
    // Initialize the circle buffer
    message.receiveIndex = 0;
    message.transmitIndex = 0;
    
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        message.buf[i] = '\0';
    }

	initSerial();
    initSerialReceive();
    initInterrupts();
    
    // LEDs use for debugging
    TRISB = 0;
    PORTB = 0x0F;

    while(1);
}

#pragma interrupt highPriorityISR
void highPriorityISR(void) {
	// This High Priority ISR should check any interrupt flags that we're
	// interested in, and then go to some subroutine ideally.

	// Check if USART Receive Interrupt Triggered. This means RCREG is full.
	// Flag should automatically be cleared when RCREG is read. pg78
	if (PIR1bits.RCIF == 1) {
		unsigned char temp = RCREG; 
        
        if (temp == '\r') {
            // Replace the return carriage with a NULL character
            message.buf[message.receiveIndex] = '\0';
            
            // Go into transmit mode
            initSerialTransmit(); 
        } else {
            // Move RCREG into the message buffer
            message.buf[message.receiveIndex] = temp; 
            message.receiveIndex++;
            
            // Check for buffer overflow
            if (message.receiveIndex == MAX_BUFFER_SIZE) {
                message.receiveIndex = 0; 
            }
         
        }
	}
    
    // Check if USART Transmit Interrupt Triggered. This means TXREG is empty.
	// Flag should automatically be cleared when TXREG is written. pg78
    if (PIR1bits.TXIF == 1) {
        unsigned char LRCStringinDecimal[4];
        int charsReceived = 0;
        
        
        
        tx232C(msg1);
        
        // Reset the LRC. LRC is calculated during transmit to ensure
        // that only transmitted characters are part of the computation.
        LRC[0] = '\0';
        
        charsReceived = tx232C(&(message.buf[message.transmitIndex]));
        itoa((int)LRC[0], LRCStringinDecimal);
        tx232C(LRC);
        
        tx232C(newLine);
        tx232C(msg2);
        tx232C(LRCStringinDecimal);
        tx232C(newLine);
        
        
        
        // Update transmit index
        message.transmitIndex = (message.transmitIndex + charsReceived) % MAX_BUFFER_SIZE; 
        
        initSerialReceive(); 
    }
    
    // Every time interrupt is triggered, lights should swap
    PORTB = ~PORTB;
}

#pragma code highISR = 0x08
void goToHighISR(void) {
    highPriorityISR();
}
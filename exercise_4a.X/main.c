/* exercise_4 - 4
 * 
 * Jeffrey Quach    460 364 634
 * Julian Hamama    460 368 090
 */

#include <p18f452.h>
#include <stdio.h>

int tx232C(unsigned char *txPtr) {
	// Loop until null character
    while (*txPtr != 0) {
		TXREG = *txPtr;
                
		while (TXSTAbits.TRMT == 0) {
            
        }
        
		txPtr++;
	}
}

void initSerial(void) {
    RCSTAbits.SPEN = 1;		// Enable serial port
	SPBRG = 25;        		// Sets the baud rate to 9600           pg171
    TXSTAbits.SYNC = 0;     // USART in async mode                  pg166
    TXSTAbits.BRGH = 1;		// USART in High Speed async mode    
    TXSTAbits.TXEN = 1;		// Enables transmit for USART  
}


int main() {
	unsigned char msg[15] = "Hello Mr Jeff!\0";
	initSerial();

    // Infinite loop of sending the message
    while(1){
        tx232C(msg);
    }
}


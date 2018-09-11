/* exercise_4b Interrupt Driven Serial Communication
 *
 * Jeffrey Quach    460 364 634
 * Julian Hamama    460 368 090
 *
 */

#include <p18f452.h>
#include "ConfigRegs.h"
#include <delays.h>

#define MAX_BUFFER_SIZE 50

// CLK FOR BOTH CASES
// Create a relitvely fast pwm to perform this task


// INTERUPT 30hz FROM TIMER

    // PARRALEL TO SERIAL
    // SSET SH/LD to low to load the parralel data
    // Small tiny delay maybe ?
    // Set SH/LD to high to enable the shifiting
    // Write to SSPBUF to trigger the 8 clock pulses.
	// Something to note about the SN74HC164 chip is that LOW TO HIGH = clock out (puts H --> QH PIN 9)



    // SERIAL TO PARRALEL
    // Write to CLR to clear the current state of the LED ARRAY
    // WRITE TO A To enable the inputs coming in
    // Output the serial data from the board to B ( 8 times )
    // Write TO A To disable the LED from changing
// Repeat




void initInterrupts() {
    RCONbits.IPEN = 1;      // Enable priority levels on interrupts, pg53
    INTCONbits.GIEH = 1;    // Enables all low priority interrupts, pg75
    INTCONbits.GIEL = 1;    // Enables all high priority interrupts, pg75

    PIR1bits.SSPIF = 1;
}

initSPI() {
	SSPCON1bits.CKP = 0; 	// Idle state of the clock is LOW, pg127
	SSPSTATbits.CKE = 1;	// When CKP == 0, transmit data on rising edge of clock, pg126

	SSPSTATbits.SMP = 0;	// Input data sampled at middle of data output time, pg126


	SSPCON1 |= 0b00000010; 	// Sets SPI to MASTER MODE. Uses a clock speed of Fosc / 64, pg127

    SSPCON1bits.SSPEN = 1;	// Enables serial port and configures SCK, SDO, SDI, and SS as serial port pins, pg127



}


setPortC(){
    TRISCbits.TRISC4 = 1;   // Sets the SPI input port, pg125 diagram
    TRISCbits.TRISC5 = 0;   // Sets the SPI output port, pg125 diagram
    TRISCbits.TRISC3 = 0;   // Sets the output port for the SPI clock, pg125 diagram

	TRISCbits.TRISC6 = 1;	// Debug
}

setPortB(){
    TRISBbits.TRISB0 = 0;   // The SH/LD pin from the SN74HC164 Parrallel to Serial Chip is connected to PORTB<0>
							// Writing this to LOW loads parallel data in
							// Writing this to HIGH allows clocking to occur, we should write to SSPBUF after setting this to HIGH
    TRISBbits.TRISB1 = 0;
}

void main() {
	unsigned char TempVar;

	initInterrupts();

    setPortC();
    setPortB();

	initSPI();


	while(1) {
		PORTBbits.RB0 = 0;      // Inputs the parallel data into the parrallel to serial converter.

		Delay10KTCYx(1);	    // Delay by 10ms

	    PORTBbits.RB0 = 1;      // Sets parralel to serial converter to output buffer mode

		TempVar = SSPBUF;        // Clear BF
	    PIR1bits.SSPIF = 0;      // Clear interrupt flag
	  	SSPBUF = 0xF0;           // initiate bus cycle
	  	while ( !SSPSTATbits.BF );  // wait until cycle complete


		TempVar = SSPBUF;        // Clear BF
		PIR1bits.SSPIF = 0;      // Clear interrupt flag
		SSPBUF = TempVar;
		while ( !SSPSTATbits.BF );  // wait until cycle completes
	}

}

#pragma interrupt highPriorityISR
void highPriorityISR(void) {
	// This High Priority ISR should check any interrupt flags that we're
	// interested in, and then go to some subroutine ideally.

    // Every time interrupt is triggered, lights should swap
    PORTB = ~PORTB;
}

#pragma code highISR = 0x08
void goToHighISR(void) {
    //highPriorityISR();

    _asm
    goto highPriorityISR
    _endasm
}

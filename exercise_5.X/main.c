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
#define ever ; ;

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



int writeLedReady = 0;

void initInterrupts() {
    RCONbits.IPEN = 1;      // Enable priority levels on interrupts, pg53
    INTCONbits.GIEH = 1;    // Enables all low priority interrupts, pg75
    INTCONbits.GIEL = 1;    // Enables all high priority interrupts, pg75
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
}

setPortB(){
    TRISBbits.TRISB0 = 0;   // The SH/LD pin from the SN74HC164 Parrallel to Serial Chip is connected to PORTB<0>
							// Writing this to LOW loads parallel data in
							// Writing this to HIGH allows clocking to occur, we should write to SSPBUF after setting this to HIGH

    TRISBbits.TRISB1 = 0;   // Sets teh ability to disable the inputs for the serial to parralel input
}

initTimerInterrupt(){
    CCPR1L = 0x46;          // Sets counter value to accomplish a 30 hz interrupt
    CCPR1H = 0x10;

    T1CON = 0b10110001;     // Bit 7:	16 bit mode
                            // Bit 5-4:	Prescale value of 8
                            // Bit 1:	Use internal clock
                            // Bit 0:	Timer 1 ON

    CCP1CON = 0b00001011;   // Bit 3-0:	Compare Mode. CCPIF is set on successful compare.
                            // TIMER1 is cleared on successful compare. pg1202

    PIE1bits.CCP1IE = 1;    // Enable Interrupts for CCP1, pg80
    PIR1bits.CCP1IF = 0;    //Clear the CCP1 Interrupt Flag, pg78
}

void main() {
	unsigned char TempVar;
    int currentState;

	initInterrupts();
    initTimerInterrupt();
    setPortC();
    setPortB();
	initSPI();


	while (1) {

        if(writeLedReady){
            PORTBbits.RB0 = 0;      // Inputs the parallel data into the parrallel to serial converter.

    		Delay10TCYx(10);	    // Delay by 100us

    	    PORTBbits.RB0 = 1;      // Sets parralel to serial converter to output buffer mode


            PORTBbits.RB1 = 0;          // Disables the serial to parralel chip from chaniging its state
            SSPBUF = 0xFF;              // Sends data in order to read from the buffer
            while ( !SSPSTATbits.BF );  // wait until cycle complete


    		currentState = SSPBUF;        // Reads in from the buffer
    	    // PIR1bits.SSPIF = 0;      // Clear interrupt flag
    	  	// SSPBUF = 0xF0;           // initiate bus cycle

            PORTBbits.RB1 = 1;          // Enables the serial to parralel chip to change its state

    		// PIR1bits.SSPIF = 0;      // Clear interrupt flag
    		SSPBUF = currentState;      // Pushes out the current state onto the LED's
    		while ( !SSPSTATbits.BF );  // wait until cycle completes

            writeLedReady = 0;
    	}
    }


}

#pragma interrupt highPriorityISR
void highPriorityISR(void) {
	// This High Priority ISR should check any interrupt flags that we're
	// interested in, and then go to some subroutine ideally.
    if (PIR1bits.CCP1IF == 1) {
		writeLedReady = 1;
        PIR1bits.CCP1IF = 0;
	}

    // Every time interrupt is triggered, lights should swap
}

#pragma code highISR = 0x08
void goToHighISR(void) {
    //highPriorityISR();

    _asm
    goto highPriorityISR
    _endasm
}

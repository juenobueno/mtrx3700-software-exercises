; exercise_3 - PWM Generated using Interrupts
;
; Jeffrey Quach    460 364 634
; Julian Hamama    460 368 090

    ; List has to be here because ???
    list p=18f452

    include "p18f452.inc"
    include "configReg.inc"

    UDATA_ACS
STATUS_TEMP	res 1
WREG_TEMP	res 1
BSR_TEMP	res 1
SLOW_COUNTER    res 1

    ; Declare a code section at 0x0 named 'RST'. This instruction sends the
    ; program to the actual start of the program.
RST code    0x0000
    GOTO    Setup

    ; Declare a code section at 0x0008 named 'LOW'
LOW code    0x0008
    GOTO    LowPriorityISR

    ; Declare a code section at 0x0030. This is where the actual program logic will start
SRT code    0x0030
Setup:
    BCF     TRISB, 2	    ; Set PORTB<2> to output for the 60Hz PWM
    BCF	    TRISB, 3	    ; SET PORTB<3> to output for the 0.3Hz PWM

    MOVLW   D'200'	    ; Initialise SLOW_COUNTER as D'200'. This will decrement until 0
    MOVWF   SLOW_COUNTER    ; as 0.300Hz is 200 times slower than 60z.

    MOVLW   0x13	    ; Set CCP1's comparator value corresponding to trigger at 120Hz.
    MOVWF   CCPR1L	    ; This number is based on a 1MHz cycle time.
    MOVLW   0x04
    MOVWF   CCPR1H

    ; Configure Timer 1
    MOVLW   B'10110001'	    ; Configure Timer1 to the following:
			    ; Bit 7:	16 bit mode
			    ; Bit 5-4:	Prescale value of 8
			    ; Bit 1:	Use internal clock
			    ; Bit 0:	Timer 1 ON

    MOVWF   T1CON	    ; pg107

    ; Configure CCP1
    MOVLW   B'00001011'	    ; Configrue CCP1 to the following:
			    ; Bit 3-0:	Compare Mode. CCPIF is set on successful compare.
			    ;		TIMER1 is cleared on successful compare. pg120

    MOVWF   CCP1CON	    ; pg117

    ; Configure Interrupts
    BSF	    RCON, IPEN	    ; Enable priority levels on interrupts, pg53

    BSF	    PIE1, CCP1IE    ; Enable Interrupts for CCP1, pg80
    BCF	    PIR1, CCP1IF    ; Clear the CCP1 Interrupt Flag, pg78

    BSF	    INTCON, GIEH    ; Enables all low priority interrupts, pg75
    BSF	    INTCON, GIEL    ; Enables all high priority interrupts, pg75


Main:
    GOTO    Main

PWMSlow:
    BTG	    PORTB, 3	    ; Toggle PORTB<3>

    MOVLW   D'200'	    ; Reset SLOW_COUNTER to 200
    MOVWF   SLOW_COUNTER

    RETURN


LowPriorityISR:
    ; Saves all the registers as we enter the interrupt
    MOVFF   STATUS, STATUS_TEMP
    MOVFF   BSR, BSR_TEMP
    MOVWF   WREG_TEMP

    BTG	    PORTB, 2	    ; Toggle PORTB<2>

    DCFSNZ  SLOW_COUNTER    ; Decrement SLOW_COUNTER, Skip if not 0.
    CALL    PWMSlow	    ; Instruction only runs if SLOW_COUNTER == 0.

    BCF	    PIR1, CCP1IF    ; Clear CCP1 Interrupt Flag

    ; Restores all the registers before we leave the interrupt
    MOVF    WREG_TEMP, W
    MOVFF   BSR_TEMP, BSR
    MOVFF   STATUS_TEMP, STATUS

    RETFIE		    ; Return from Interrupt

end

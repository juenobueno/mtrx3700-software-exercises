; exercise_2 - PWM Generated using CCP2
;
; Jeffrey Quach    460 364 634
; Julian Hamama    460 368 090

    ; List has to be here because ???
    list p=18f452

    ; Include file, change directory if needed
    include "p18f452.inc"

    ; Declare a code section at 0x0 named 'RST'. This instruction sends the
    ; program to the actual start of the program.
RST code    0x0000
    GOTO    Setup

    ; Declare a code section at 0x0030. This is where the actual program logic will start
SRT code    0x0030
Setup:
    ; Configure ADC
    MOVLW   B'01000001'     ; Bit 7-6:	Set AD0 clock to Fosc/8, corresponding to a 2us period
			    ;		This is > than 1.6us specified by data sheet, pg188
			    ; Bit 5-3:	Select Analog Channel 0. This is connected to the potentiometer
			    ; Bit 0:	AD0 on
    MOVWF   ADCON0	    ; pg181

    MOVLW   B'10001110'     ; Bit 7:	Right justified result in ADRESH|ADRESL
			    ; Bit 6:	Set AD0 clock to Fosc/8, corresponding to a 2us period
			    ;		This is > than 1.6us specified by data sheet, pg186
			    ; Bit 3-0:	AN0 is set to analog input. AN1 - AN7 is set to Digital output
			    ;		Reference votlages: Vref+ = VDD, Vref- = VSS
    MOVWF   ADCON1          ; pg182

    ; Configrue Timer 0
    MOVLW   B'11000111'     ; Bit 7:	Timer 0 on
			    ; Bit 6:	8 bit mode
			    ; Bit 2-0:	Prescale value of 256
    MOVWF   T0CON	    ; pg103
    
    ; Configure CCP2 for PWM Operation.
    BCF	    T2CON, T2CKPS1  ; Set the prescaler value of TIMER2 to 4, pg111
    BSF	    T2CON, T2CKPS0

    MOVLW   0xFF	    ; Set the frequency to 10kHz, chosen abritrarily, pg123
    MOVWF   PR2		    ; pg123

    BCF	    TRISC, 1	    ; Set PORTC<1> to output as this is the output pin of CCP2

    BSF	    T2CON, TMR2ON   ; Turn TIMER2 on

    MOVLW   B'00001100'	    ; Set CCP2 to PWM Mode, pg117
    MOVWF   CCP2CON

Main:
    BTFSS   INTCON, TMR0IF  ; Wait for Timer0 to timeout. This gives time for
    GOTO    Main	    ; AD0 capacicitors to discharge.
    BCF     INTCON, TMR0IF  ; Clear TIMER1 Interrupt Flag for the next loop

    BSF     ADCON0, GO      ; Start A/D conversion

WaitForAdConversion:
    BTFSS   PIR1, ADIF      ; Wait for conversion to complete
    GOTO    WaitForAdConversion


    ; Copy the ADC result to the registers which determine the PWM. Since
    ; ADC gives a 10 bit result and duty cycle is determined by a 10 bit number,
    ; this is a 1:1 mapping.
    MOVF    ADRESH, W
    MOVWF   CCPR2L

    GOTO    Main            ; Infinite Loop

end

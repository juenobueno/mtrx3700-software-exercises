; exercise_1 - Reading Potentiometer Value using ADC and Outputting to PORTB
;
; Jeffrey Quach    460 364 634
; Julian Hamama    460 368 090                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   *************************
    
    ; List has to be here because ???
    list p=18f452

    include "p18f452.inc"

    UDATA_ACS
onLEDs		res 1
numberOfShifts	res 1

    ; Declare a code section at 0x0 named 'RST'. This instruction sends the
    ; program to the actual start of the program. 
RST code 0x0000 
    GOTO Setup

    ; Declare a code section at 0x0030. This is where the actual program logic will start
SRT code    0x0030
Setup:
    CLRF    TRISB           ; All pins of PORTB set to output
    CLRF    PORTB           ; Clear PORTB to turn off all lights first

    ; Configure ADC
    MOVLW   B'01000001'     ; Bit 7-6:	Set AD0 clock to Fosc/8, corresponding to a 2us period
			    ;		This is > than 1.6us specified by data sheet, pg188
			    ; Bit 5-3:	Select Analog Channel 0. This is connected to the potentiometer
			    ; Bit 0:	AD0 on
    MOVWF   ADCON0	    ; pg181
			    
    MOVLW   B'00001110'     ; Bit 7:	Left justified result in ADRESH|ADRESL
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

    
Main:
    BTFSS   INTCON, TMR0IF  ; Wait for Timer0 to timeout. This gives time for
    GOTO    Main	    ; AD0 capacicitors to discharge.
    BCF     INTCON, TMR0IF  ; Clear TIMER1 Interrupt Flag for the next loop

    MOVLW   B'11100000'	    ; Reset onLEDs. This value will be left shifted n times.
    MOVWF   onLEDs, ACCESS 
    
    BSF     ADCON0, GO      ; Start A/D conversion

    
WaitForAdConversion:
    ; Wait for conversion to complete
    BTFSS   PIR1, ADIF      
    GOTO    WaitForAdConversion
    
    
    ; Get the two most significant bits form ADRESH. Result in numberOfShifts
    RLNCF   ADRESH
    RLNCF   ADRESH, W
    ANDLW   0x03
    MOVWF   numberOfShifts, ACCESS

    
LeftShift: 
    ; Left shift onLEDs n times, n = numberOfShifts
    RLNCF   onLEDs
    DECFSZ  numberOfShifts
    GOTO    LeftShift
    
    
    ; Write onLEDs to PORTB
    MOVF    onLEDs, W
    ANDLW   0x07
    MOVWF   PORTB
    
    GOTO    Main    ; Infinite Loop

end

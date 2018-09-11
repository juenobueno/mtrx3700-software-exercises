;******************************************************************************
;
;    File Name: configReg.inc
;
;    Include file to program the configuration registers using the
;    'new' type of CONFIG directive.
;
;    The CONFIG directive defines configuration register data from within the
;    .ASM file. Values set here will override those defined in the MPLAB IDE.
;    The labels following the directive are defined in the file P18F452.inc.
;    The PIC18FXX2 Data Sheet explains the functions of the configuration bits.
;
;    This file is suitable for debugging; change it to suit your application.
;
;    Note that the directive __CONFIG is deprecated by the CONFIG directive.
;
;    Usage: #include configReg.inc
;
;    Bugs:  --
;
;    ToDo:  --
;
;    Revision History
;       Vers    Date         Who?       Revision Detail?
;       1.0     05-Apr-2005  DCR        First release
;
;******************************************************************************



; Configuration Register 1H
; Oscillator switch disabled, EC oscillator.
; Use OSC = HS or OSC = HSPLL for the minimal board (10MHz/40MHz)
    CONFIG   OSCS=OFF, OSC=EC       ; for PICDem2 board (4MHz external clock)

; Configuration Register 2L
; Brown-out reset disabled, Brown-out voltage 2.7V, Power-on timer enabled
    CONFIG    BOR=OFF, BORV=27, PWRT=ON

; Program Configuration Register 2H
; Watch-dog Timer disabled, Watch Dog Timer PostScaler count = 1:128
    CONFIG    WDT=OFF, WDTPS=128

; Program Configuration Register 3H
; CCP2 Mux enabled (RC1)
;   CONFIG    CCP2MUX=OFF           ; CCP2 is on RB3 - alternate pin
    CONFIG    CCP2MUX=ON            ; CCP2 is on RC1 - Default

; Configuration Register 4L
; Stack Overflow Reset enabled, Low Voltage Programming disabled, Debug enabled
    CONFIG    STVR=ON, LVP=OFF, DEBUG=ON

; Configuration Register 5L
; Code protection disabled
    CONFIG    CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF

; Configuration Register 5H
; Boot block and EEPROM code protection disabled
    CONFIG    CPB=OFF, CPD=OFF

; Configuration Register 6L
; PROM Write protection off
    CONFIG    WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF

; Configuration Register 6H
; Config Register, Boot block, EEPROM Write protection off
    CONFIG    WRTC=OFF, WRTB=OFF, WRTD=OFF

; Configuration Register 7L
    CONFIG    EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF

; Configuration Register 7H
; Boot block table read protection off
    CONFIG    EBTRB=OFF

;------------------------------------------------------------------------------


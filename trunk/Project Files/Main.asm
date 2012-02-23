;    This file is part of Micro 64's Firmware.

;    Micro 64 is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    Micro 64 is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.

;    You should have received a copy of the GNU General Public License
;    along with Micro 64.  If not, see <http://www.gnu.org/licenses/>.
;
;*******************************************************************************
;*******************************************************************************
;This is the firmware for the Micro 64 controller (http://code.google.com/p/micro-64-controller/).
;The memory pack space is 0x07000 - 0xF000 and an additional 64Kb is used as scratch space from 0xF000 - 0x1F000
;The TOC is stored in RAM from 0x500 to 0xCFF, scratch space for read and write scratch space is from 0x4E0 - 0x4FF
;The 1Kb used for Flash Erase storage is at 
;
;TODO: We'll also need to clear flash scratch.

	include "p18f27j53.inc"		;include the defaults for the chip
        include "Communication.inc"

	config WDTEN = OFF, PLLDIV = 2, CFGPLLEN = ON, STVREN = OFF, XINST = OFF, CP0 = OFF		;config word 1
	config CLKOEC = OFF, SOSCSEL = LOW, OSC = INTOSCPLL, FCMEN = OFF                                ;config word 2
	config DSBOREN = OFF, DSWDTEN = OFF, IOL1WAY = OFF						;config word 3
	config LS48MHZ = SYS48X8, WPDIS = ON								;config word 4

        cblock  0x00
            count                           ;Used in the delay routine.            
            crc                             ;Contains the CRC at the end of calculation
            crctemp                         ;Used in the calculation of the data CRC
            addrcrc                         ;Stores the address CRC until needed
            temp                            ;We need scratch space!
            temp2                           ;That wasn't enough!
            temp3                           ;Now I'm just being greedy
            temp4                           ;One more, but only because you insist
            iterate                         ;counter used for loops
            iterate2                        ;Needed because of these silly things called nested loops
            command                         ;The Last command sent to the N64, set at interrupt
            addr1                           ;Used in read and write routines. addr1 is the
            addr2                           ;high byte and addr2 conversely the low byte.
            rumble                          ;Contains the last byte written to the rumble pack
            scratchaddrlow                  ;contains the address pointing to the next
            scratchaddrhigh                 ;free byte in the flash memory's scratch space
        endc

	org	0x0000
	goto	start

        org     0x0008                          ;Interrupt vector
        movlw   0x03                            ;gives exactly 24 cycle delay to middle of the first bit (most of the time :p)
        call    delay
        call    record8bits

        movff   temp,   command
        call    fourus                  ;29 cycles after start of last command bit (not including stop bit)

	movlw	0x02
	subwf	command,	w       ;read 32 bytes

	btfsc   STATUS, Z
	call	pakread

	movlw	0x03
	subwf	command,	w       ;write 32 bytes

	btfsc   STATUS, Z
	call	pakwrite
        
        movlw   0x14
        call    delay

	movlw	0x00
	subwf	command,	w       ;Identify

	btfsc   STATUS, Z
	call	contstatus

	movlw	0x01
	subwf	command,	w       ;Buttons and joystick

	btfsc   STATUS, Z
	call	contbuttons

        bcf     INTCON3, 0
        retfie

    org 0x100
start   call init

mainloop    goto    mainloop

contstatus	movlw	0x05
	call	send8bits
	movlw	0x00
	call 	send8bits
	movlw	0x01
	call	send8bits
        goto    $+2
        goto    $+2
        call    stopbit

	return

contbuttons	movlw	0x00
	call	send8bits
        movlw	0x00
	call	send8bits
        movlw	0x00
	call	send8bits
        movlw	0x00
	call	send8bits
        goto    $+2
        goto    $+2
        call    stopbit
	return

;Sets inital values for many SFRs
;******************************************************************************
init        movlb   0x0E
        movlw   d'17'
	movwf   RPINR1				;Int1 is set to RC6
        movlb   0x00
	movlw   0xFF
        movwf   TRISA                           ;Port A all inputs
	movwf   TRISB                           ;Port B all outputs
	movwf   TRISC                           ;Port C all inputs
        clrf    PORTC

        movlw   b'00100100'
        movwf   EECON1
        clrf    rumble

        ;call    TOCreset
        ;call    scratchflashinit                ;There might still be stuff left there in the event of unexpected reset
                                                ;It's impossible to recover due to a lack of TOC so down the toilet it goes.
        clrf    INTCON2
        movlw   b'01001001'
	movwf   INTCON3
        movlw   b'11000000'
	movwf   INTCON                  ;external interrupt on INT1 on

        return

;Clears the TOC as GPRs apparently don't reset to clear
;*******************************************************************************
TOCreset    movlw 0x05
    movwf   FSR0H
    clrf    FSR0L

    setf   iterate
ea    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    clrf    POSTINC0
    decfsz iterate
    goto ea
    return

;clear scratch flash
;*******************************************************************************
scratchflashinit    clrf   TBLPTRU
    clrf    TBLPTRL
    movlw   0xF0
    movwf   TBLPTRH
    call    flasherase

    movlw   0x04
    addwf   TBLPTRH
    call    flasherase

    movwf   0x04
    addwf   TBLPTRH
    call    flasherase

    movwf   0x04
    addwf   TBLPTRH
    call    flasherase

    bsf     TBLPTRU,    0
    clrf    TBLPTRH
    movlw   0x3C
    movwf   iterate
flashinit   call    flasherase
    movlw   0x04
    addwf   TBLPTRH
    decfsz  iterate
    goto    flashinit
    return
    

;Clears the block of flash specified in the TBLPTR registers. It turns
;off interrupts so they may need to be turned on again.
;*******************************************************************************
flasherase  bsf EECON1, WREN    ;enable write to memory
    bsf EECON1, FREE            ;enable Erase operation
    bcf INTCON, GIE             ;disable interrupts
    movlw   0x55
    movwf   EECON2              ;write 55h
    movlw   0xAA
    movwf   EECON2              ;write 0AAh
    bsf EECON1, WR              ;start erase (CPU stall)
    bsf INTCON, GIE             ;re-enable interrupts
    return

;Writes the 32 bytes in RAM to the next free location in flash memory. If we reach
;the end of scratch spcace then it automatically consolidates at the end.
;*******************************************************************************
;writetoscratch

;Delays for 5 + 3*n cycles
;*******************************************************************************
delay   movwf   count
delay1    decfsz  count,  f
    goto    delay1
    return

    include "lookup CRC.inc"

    end
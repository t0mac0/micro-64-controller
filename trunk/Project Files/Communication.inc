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

    CODE
    org 0x0300

;Sends one byte from the w register
;**********************************************
send8bits    movwf	temp
    movlw	0x08
    movwf	iterate
    goto    skip

iteratesend     rlncf	temp,	f
    goto    $+2
    goto    $+2
    goto    $+2
    nop
skip        bcf     TRISC,  6
    movlw   0x01
    call    delay
    nop

    btfsc	temp,	7
    bsf	TRISC,	6                   ;has 23 cycles afterwards

    movlw 0x00
    cpfseq  iterate                 ;we check for the last iteration and if so calculate the crc with that byte
    goto normdelay

    call    crcbyteretrieve
    bsf TRISC,  6                     ;back to the program with control byte
    return

normdelay    movlw   0x04
    call    delay
    goto    $+2

    bsf	TRISC,	6
    decfsz	iterate,    f
    goto	iteratesend
    return

;Records one byte and leaves it in temp
;*************************************************
record8bitswait    btfsc   PORTC,  6
    goto    record8bitswait
    movlw   0x01
    call    delay

record8bits clrf    temp            
    movlw	0x08
    movwf	iterate
    goto    skipread

iteraterecord   rlncf	temp,	f
    movlw   0x02
    call    delay

wait    btfss   PORTC,  6           ;
   goto    wait
wait2        btfsc   PORTC,  6
    goto    wait2

    movlw   0x03
    call    delay

skipread    btfsc	PORTC,	6
    bsf temp,	0
    decfsz	iterate
    goto	iteraterecord
    return

;instantly produces a stop bit and leaves with some cycles to spare
;******************************************************************
stopbit        bcf     TRISC,  6    
    movlw   0x02
    call    delay
    bsf     TRISC,  6
    return


;Reads 32 bytes from the software controller pack
;*******************************************************************
pakread call    record8bitswait         ;Gets the two adress bytes and masks the second one
    movff   temp,   addr1
    movlw   0x01
    call    delay
    call    record8bitswait
    movff   temp, addr2
    movlw   b'11100000'
    andwf   addr2,  f           ;masks the address crc bits off

    movff   addr2,  temp2
    movff   addr1,  temp

    movlw   b'00100000'         ;sets up the table reads, needed for CRC as well as normal data
    movwf   EECON1
    movlw   0x00
    movwf   TBLPTRU

    rrcf    temp,   f
    rrcf    temp2,  f
    rrcf    temp,  f
    rrcf    temp2,  f
    rrcf    temp,  f
    rrcf    temp2,  f
    rrcf    temp,  f
    rrcf    temp2,  f
    rrcf    temp,  f
    rrcf    temp2,  f

    btfsc   temp,   7           ;branches of if messing with memory space reserved for rumble
    goto    rumbleloop

mempack    movlw   b'00000011'         ;masks any carry bits that shouldn't be there and gets rid of the rumble bit
    andwf   temp,   f
    movlw   b'01110000'         ;offsets from 0x7000
    iorwf   temp,   f

    movff   temp,   TBLPTRH
    movff   temp2,  TBLPTRL
    
    clrf    TBLPTRH

    tblrd*                      ;Checks the table of contents (0x6000) to see where the latest data is stored
    
    movlw   b'01110000'         ;0x07000   
    
    btfss   TABLAT, 0
    movlw   b'11110000'         ;0x0F000

    btfsc   TABLAT, 1           ;0x17000
    goto    $+6                 
    movlw   0x01
    movwf   TBLPTRH
    movlw   b'01110000'
    movwf   TBLPTRL
    clrf    crc

    movlw   0x19
    movwf   iterate2

    tblrd*+                     ;first byte without CRC
    movf    TABLAT, w
    call    send8bits
    movff   temp,   crc

read32  tblrd*+
    movf    TABLAT, w
    xorwf   crc,    f
    
    call    send8bits
   
    decfsz  iterate2, f
    goto    read32
    goto    retcrc                ;returns and is nearly the end of the read except for stop bit in the case of memory pack

rumbleloop    movlw   0x19
    movwf   iterate2

    movf    rumble, w           ;copy of above pretty much with changes so that this instead reads out the value of the rumble pack
                                ;When looking inside a rumble pack it appeared that when the highest adress bit was set the rumble would
    call    send8bits           ;latch the data byte and then drive the motor with intensity related to that.
    movff   temp,   crc                                   
   
rumble32  movf  rumble, w
    xorwf   crc,    f

    call    send8bits
 
    decfsz  iterate2, f
    goto    rumble32

retcrc    movlw   0x00
    xorwf   crc,    f                   ;augments the message and calculates the final crc

    movf    crc,    w
    call    send8bits
    goto    $+2
    goto    $+2
    call    stopbit

fourus    return


;Writes 32 bytes to flash, not actually written to flash but instead scratch space ready
;for the user to indicate moving the data to flash. Fun and complicated function
;***************************************************************************************
pakwrite    movlb   0x00
    call    record8bitswait
    movff   temp,   addr1
    movlw   0x03
    call    delay
    call    record8bitswait
    movff   temp,   addr2

    movlw   d'31'
    movwf   iterate2

    movlw   b'00001100'                 ;upper byte of indirect address
    movwf   FSR0H
    movlw   b'00000000'                 ;lower byte of indirect address
    movwf   FSR0L

    call    record8bitswait
    movff   temp,   crc
    movff   temp,   POSTINC0            ;stores in indirect address with post increment

    movlw   0x01
    call    delay

write32    call    record8bitswait
    movff   temp,   POSTINC0

    call    crcbyteretrieve
    movf    temp,   w
    xorwf   crc,    f

    decfsz  iterate2
    goto    write32

    call    crcbyteretrieve             ;augments the message and calculates the final crc
    movlw   0x00
    xorwf   crc,    f                   

    movlw   0x0B
    call    delay

    movf    crc,    w
    call    send8bits
    goto    $+2
    goto    $+2
    call    stopbit
   
    return

;Retrieves a control byte for the CRC using the lookup table that I am 
;inordinately pleased with myself for calculating.
;*****************************************************************
crcbyteretrieve movff   TBLPTRU,    temp2
    movff    TBLPTRH,   temp3
    movff    TBLPTRL,   temp4

    clrf    TBLPTRU
    movlw   b'01011111'
    movwf   TBLPTRH
    movff   crc,    TBLPTRL

    tblrd*
    movff   TABLAT, crc
    movff   temp2,   TBLPTRU
    movff   temp3,   TBLPTRH
    movff   temp4,   TBLPTRL

    return
# Introduction #

This will hopefully be a fairly complete description of the N64 controller protocol. There are many sources which describe the protocol and this page aims to combine these with what I've learnt to produce an exhaustive description.


# Physical #

N64 controllers connect via a three core conductor which carries 3.3V, ground and a bidirectional data line. The single data line is tied high on the N64 side (and optionally at the controller) and the controller or console can drive the line low. Its important to remember that the data line is never driven high but instead when both devices are presenting high impedance the line is pulled high automatically. This type of arrangement is commonly known as an open collector circuit and is used most notably in i2c.

To send a 0 bit the data line is pulled low for 3µs and let high for 1µs.
In contrast to send a 1 the data line is pulled low for 1µs and let high for 3µs.

# General Structure #

The Nintendo 64 always initiates communication by sending a command byte. Depending on the command this can be followed by further data from the N64. The commands are as follows:

## 0x00 ##
The controller responds with three bytes to identify itself. For a normal controller the first two bytes are always 0500. The last byte is:
  * 01 if there is a controller pack plugged in.
  * 02 if there is no controller pack.
  * 04 if the previous controller read/write address CRC showed an error.

## 0x01 ##
The controller responds with four bytes of button and joystick data

| Byte |Data|
|:-----|:---|
| 1    | A  | B  | Z  | Start | D up | D down | D left | D right |
| 2    | Joy reset | 0  | L  | R  | C up | C down | C left | C right  |
| 3    | Joystick x ordinate |
| 4    | Joystick y ordinate |

While the joystick data is a signed 8 bit 2s complement we know from Micro that controllers only have 160 steps on them and I've had games which screw up when given the full 8 bit range.

## 0x02 ##
The N64 follows this command with two bytes which indicate the address to be read from and a CRC to verify the address (these are described below). The controller responds with 32 bytes from the indicated address and ends with a data CRC.

## 0x03 ##
Again the N64 follows this command with an address and associated CRC and then 32 bytes to be written to the address. following reception of all the data the controller responds with a CRC as above.

## 0xFF ##
This command is used to reset the N64 controller which importantly includes resetting the calibration of the analog stick. The controller responds with the same sequence as for the 0x00 command.

## Addressing ##
In the read and write commands a 2 byte address and CRC follows the first command byte. The first 11 bits of the two bytes are the address. This allows us 2048 or, in the case of the memory pack which doesn't utilise the last address bit, 1024 possible addresses. This may seem unable to address all of the 32Kb memory pack but as the reads and writes deal with 32 byte blocks of data the least significant 5 bits of the address aren't needed as all the data is aligned on 32 byte boundaries.

In the place of the least significant 5 address bits (which would be 0 in any case) an address CRC is implemented. The CRC is fully described in the accompanying CRC document.
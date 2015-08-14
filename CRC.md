# Introduction #
Complete description of the N64's two CRCs along with some implementations.


# Data CRC #

To discover the N64 Data CRC a message was sent to a controller with the last bit set. From this it was easy to compare the returned CRC and message to find the CRC polynomial:
  * Length: n=8 with an implicit 9th "1" bit
  * Poly: 0x85
  * Initial Value: 0x00
  * Post XOR: with controller pack - 0x00, without - 0xFF

As with all CRCs the message is augmented with a byte of 0x00.

In summary it's a fairly simple 8 bit CRC that wasn't too difficult to work out however calculation of the CRC using the poly is quite computationally expensive, especially when running on a microcontroller. It requires a XOR, shift and most crucially knowledge of the next 7 bits which means it's difficult to calculate while the message is being received/sent. Normally this wouldn't be a problem but the N64 controller requires a prompt answer (within a few µs).

To deal with this an alternative approach is used. Its possible to pre-calculate the effect a byte will have on itself and the next byte and store it in a table with 256 entries. While it was difficult to work out the rewards are significant. Each byte requires merely a table lookup and a XOR.

The algorithm in pseudocode is:
```
while(augmented message not exhausted){
	Control = messagebyte
	messagebyte = next_messagebyte()
	messagebyte = messagebyte XOR table[Control]
}
CRC = messagebyte
```

The table and the C program used to calculate it are included in the repository.

Finally my understanding of CRCs has been greatly helped by a paper written by Ross Williams. I cannot recommend it enough.
http://www.ross.net/crc/


## Address CRC ##
The same approach was reused to find the address CRC:
  * Length: n=5 with an implicit 6th "1" bit
  * Poly: 0x15
  * Initial Value: 0x00
  * Post XOR: 0x00
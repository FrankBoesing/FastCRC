FastCRC
=======

Fast CRC Arduino library for ARM Cortex M4 (e.g. Teensy 3) - uses the on-chip hardware
Up to 400 times faster than crc16.h (_arv_libc)

List of supported CRCs:
-
8 BIT:

SMBUS
MAXIM

16 BIT:

KERMIT (Alias CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT)
CCITT-FALSE
MCRF4XX
MODBUS
XMODEM (Alias ZMODEM, CRC-16/ACORN)
X25 (Alias CRC-16/IBM-SDLC, CRC-16/ISO-HDLC, CRC-B)
and all from _avr_libc

32 BIT:

CRC32, CRC-32/ADCCP, PKZIP, ETHERNET, 802.3
CKSUM, CRC-32/POSIX

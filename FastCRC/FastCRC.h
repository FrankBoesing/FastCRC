/* FastCRC library code is placed under the MIT license
 * Copyright (c) 2014 Frank Bösing
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


//
// See K20P64M72SF1RM.pdf (Kinetis), Pages 638 - 641 for documentation of CRC Device
// See KINETIS_4N30D.pdf for Errata (Errata ID 2776)
//
// Because of Errata above, ALL calculations are done as 32 bit.
// (...and this gives an additional speedup !)
//
// Thanks to:
// Catalogue of parametrised CRC algorithms, CRC RevEng
// http://reveng.sourceforge.net/crc-catalogue/
//

#if !defined(FastCRC_h) && (defined(__MK20DX128__) || defined(__MK20DX256__))
#define FastCRC_h

#include "inttypes.h"



// ================= 8-BIT CRC ===================

class FastCRC8
{
public:
  FastCRC8();
  uint8_t smbus(const uint8_t *data, const uint16_t datalen);      // aka CRC-8
  uint8_t maxim(const uint8_t *data, const uint16_t datalen);      // equivalent to _crc_ibutton_update() in crc16.h from avr_libc

  uint8_t update(const uint8_t *data, const uint16_t datalen);
  uint8_t generic(const uint8_t polyom, const uint8_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen);
};

// ================= 16-BIT CRC ===================

class FastCRC16
{
public:
  FastCRC16();
  uint16_t ccitt(const uint8_t *data, const uint16_t datalen);      // Alias "false CCITT"
  uint16_t mcrf4xx(const uint8_t *data,const uint16_t datalen);     // equivalent to _crc_ccitt_update() in crc16.h from avr_libc
  uint16_t kermit(const uint8_t *data, const uint16_t datalen);     // Alias CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT
  uint16_t modbus(const uint8_t *data, const uint16_t datalen);     // equivalent to _crc_16_update() in crc16.h from avr_libc
  uint16_t xmodem(const uint8_t *data, const uint16_t datalen);     // Alias ZMODEM, CRC-16/ACORN
  uint16_t x25(const uint8_t *data, const uint16_t datalen);        // Alias CRC-16/IBM-SDLC, CRC-16/ISO-HDLC, CRC-B

  uint16_t update(const uint8_t *data, const uint16_t datalen);
  uint16_t generic(const uint16_t polyom, const uint16_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen);
};

// ================= 32-BIT CRC ===================

class FastCRC32
{
public:
  FastCRC32();
  uint32_t crc32(const uint8_t *data, const uint16_t datalen);     // Alias CRC-32/ADCCP, PKZIP, Ethernet, 802.3
  uint32_t cksum(const uint8_t *data, const uint16_t datalen);     // Alias CRC-32/POSIX

  uint32_t update(const uint8_t *data, const uint16_t datalen);
  uint32_t generic(const uint32_t polyom, const uint32_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen);
};

#endif

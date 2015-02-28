/* FastCRC library code is placed under the MIT license
 * Copyright (c) 2014,2015 Frank Bösing
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


// Teensy 3.0, Teensy 3.1:
// See K20P64M72SF1RM.pdf (Kinetis), Pages 638 - 641 for documentation of CRC Device
// See KINETIS_4N30D.pdf for Errata (Errata ID 2776)
//
// So, ALL HW-calculations are done as 32 bit.
//
//
//
// Thanks to:
// - Catalogue of parametrised CRC algorithms, CRC RevEng
// http://reveng.sourceforge.net/crc-catalogue/
//
// - Danjel McGougan (CRC-Table-Generator)
//


#if defined(__MK20DX128__) || defined(__MK20DX256__)

#include "mk20dx128.h"
#include "FastCRC.h"

// ===============================================

#define CRC_CRC8     *(volatile uint8_t  *)0x40032000
#define CRC_CRC8H1   *(volatile uint8_t  *)0x40032003
#define CRC_CRC16    *(volatile uint16_t *)0x40032000
#define CRC_CRC16H   *(volatile uint16_t *)0x40032002

#define CRC_CTRL_WAS   25 // Write CRC Data Register As Seed(1) / Data(0)
#define CRC_CTRL_TCRC  24 // Width of CRC protocol (0=16 BIT, 1=32 BIT)
#define CRC_CTRL_TOTR1 29 // TOTR[1]

#ifndef BITBAND_PERI_REF
  #define BITBAND_PERI_REF  0x40000000
  #define BITBAND_PERI_BASE 0x42000000
  #define BITBAND_PERI(a,b) ((BITBAND_PERI_BASE + (a-BITBAND_PERI_REF)*32 + (b*4))) // Convert PERI address
#endif

#define CRC_CTRL_BIT(x) *((volatile unsigned int *)(BITBAND_PERI(0x40032008,x))) //BITBAND-access to CRC_CTRL

// ================= 8-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC8::FastCRC8(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;
}

/** SMBUS CRC
 * aka CRC-8
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::smbus(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x07 init=0x00 refin=false refout=false xorout=0x00 check=0xf4
  return generic(0x07, 0, CRC_FLAG_NOREFLECT, data, datalen);
}

/** MAXIM 8-Bit CRC
 * equivalent to _crc_ibutton_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::maxim(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x31 init=0x00 refin=true refout=true xorout=0x00  check=0xa1
  return generic(0x31, 0, CRC_FLAG_REFLECT, data, datalen);
}

/** Update
 * Call for subsequent calculations with previous seed
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::update(const uint8_t *data, const uint16_t datalen)
{

  const uint8_t *src = data;
  const uint8_t *target = src + datalen;

  while (((uintptr_t)src & 0x03) != 0) {
    CRC_CRC8H1 = *src++; //Write 8 BIT
  }

  while (src <= target-4) {
    CRC_CRC = *( uint32_t  *)src; //Write 32 BIT
    src += 4;
  }

  while (src < target) {
    CRC_CRC8H1 = *src++; //Write 8 Bit
  }

  if (CRC_CTRL_BIT(CRC_CTRL_TOTR1))
    return CRC_CRC8;
  else
    return CRC_CRC8H1;
}

/** generic function for all 8-Bit CRCs
 * @param polynom Polynom
 * @param seed Seed
 * @param flags Flags
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::generic(const uint8_t polynom, const uint8_t seed, const uint32_t flags, const uint8_t *data,const uint16_t datalen)
{

  CRC_CTRL  = flags | (1<<CRC_CTRL_TCRC) | (1<<CRC_CTRL_WAS);                      // 32Bit Mode, Prepare to write seed(25)
  CRC_GPOLY = ((uint32_t)polynom)<<24;                                             // Set polynom
  CRC_CRC   = ((uint32_t)seed<<24);                                                // Write seed
  CRC_CTRL_BIT(CRC_CTRL_WAS) = 0;                                                  // Clear WAS Bit - prepare to write data

  return update(data, datalen);
}
uint8_t FastCRC8::smbus_upd(const uint8_t *data, uint16_t datalen){return update(data, datalen);}
uint8_t FastCRC8::maxim_upd(const uint8_t *data, uint16_t datalen){return update(data, datalen);}

// ================= 16-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC16::FastCRC16(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;
}

/** CCITT
 * Alias "false CCITT"
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::ccitt(const uint8_t *data,const uint16_t datalen)
{
 // poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  return generic(0x1021, 0XFFFF, CRC_FLAG_NOREFLECT, data, datalen);
}

/** MCRF4XX
 * equivalent to _crc_ccitt_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::mcrf4xx(const uint8_t *data,const uint16_t datalen)
{
 // poly=0x1021 init=0xffff refin=true refout=true xorout=0x0000 check=0x6f91
  return generic(0x1021, 0XFFFF, CRC_FLAG_REFLECT, data, datalen);
}

/** MODBUS
 * equivalent to _crc_16_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::modbus(const uint8_t *data, const uint16_t datalen)
{
 // poly=0x8005 init=0xffff refin=true refout=true xorout=0x0000 check=0x4b37
  return generic(0x8005, 0XFFFF, CRC_FLAG_REFLECT, data, datalen);
}

/** KERMIT
 * Alias CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::kermit(const uint8_t *data, const uint16_t datalen)
{
 // poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189
 // sometimes byteswapped presentation of result
  return generic(0x1021, 0x00, CRC_FLAG_REFLECT, data, datalen);
}

/** XMODEM
 * Alias ZMODEM, CRC-16/ACORN
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::xmodem(const uint8_t *data, const uint16_t datalen)
{
  //width=16 poly=0x1021 init=0x0000 refin=false refout=false xorout=0x0000 check=0x31c3
  return generic(0x1021, 0, CRC_FLAG_NOREFLECT, data, datalen);
}

/** X25
 * Alias CRC-16/IBM-SDLC, CRC-16/ISO-HDLC, CRC-B
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::x25(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x1021 init=0xffff refin=true refout=true xorout=0xffff check=0x906e
  return generic(0x1021, 0XFFFF, CRC_FLAG_REFLECT | CRC_FLAG_XOR, data, datalen);
}

/** Update
 * Call for subsequent calculations with previous seed
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::update(const uint8_t *data, const uint16_t datalen)
{
  const uint8_t *src = data;
  const uint8_t *target = src + datalen;

  while (((uintptr_t)src & 0x03) !=0)  {
    CRC_CRC8H1 = *src++; //Write 8 BIT
  }

  while (src <= target-4) {
    CRC_CRC = *( uint32_t  *)src; //Write 32 BIT
    src += 4;
  }

  while (src < target) {
    CRC_CRC8H1 = *src++; //Write 8 Bit
  }

  if (CRC_CTRL_BIT(CRC_CTRL_TOTR1))
    return CRC_CRC16;
  else
    return CRC_CRC16H;
}

/** generic function for all 16-Bit CRCs
 * @param polynom Polynom
 * @param seed Seed
 * @param flags Flags
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::generic(const uint16_t polynom, const uint16_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen)
{

  CRC_CTRL  = flags | (1<<CRC_CTRL_TCRC) | (1<<CRC_CTRL_WAS);// 32-Bit Mode, prepare to write seed(25)
  CRC_GPOLY = ((uint32_t)polynom)<<16;                       // set polynom
  CRC_CRC   = ((uint32_t)seed<<16);                          // this is the seed
  CRC_CTRL_BIT(CRC_CTRL_WAS) = 0;                            // Clear WAS Bit - prepare to write data

  return update(data, datalen);
}

uint16_t FastCRC16::ccitt_upd(const uint8_t *data, uint16_t len)  {return update(data, len);}
uint16_t FastCRC16::mcrf4xx_upd(const uint8_t *data, uint16_t len){return update(data, len);}
uint16_t FastCRC16::kermit_upd(const uint8_t *data, uint16_t len) {return update(data, len);}
uint16_t FastCRC16::modbus_upd(const uint8_t *data, uint16_t len) {return update(data, len);}
uint16_t FastCRC16::xmodem_upd(const uint8_t *data, uint16_t len) {return update(data, len);}
uint16_t FastCRC16::x25_upd(const uint8_t *data, uint16_t len)    {return update(data, len);}



// ================= 32-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC32::FastCRC32(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;
}

/** CRC32
 * Alias CRC-32/ADCCP, PKZIP, Ethernet, 802.3
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint32_t FastCRC32::crc32(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
  return generic(0x04C11DB7L, 0XFFFFFFFFL, CRC_FLAG_REFLECT | CRC_FLAG_XOR, data, datalen);
}

/** CKSUM
 * Alias CRC-32/POSIX
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint32_t FastCRC32::cksum(const uint8_t *data, const uint16_t datalen)
{
  // width=32 poly=0x04c11db7 init=0x00000000 refin=false refout=false xorout=0xffffffff check=0x765e7680
  return generic(0x04C11DB7L, 0, CRC_FLAG_NOREFLECT | CRC_FLAG_XOR, data, datalen);
}

/** Update
 * Call for subsequent calculations with previous seed
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
//#pragma GCC diagnostic ignored "-Wpointer-arith"
uint32_t FastCRC32::update(const uint8_t *data, const uint16_t datalen)
{

  const uint8_t *src = data;
  const uint8_t *target = src + datalen;

  while (((uintptr_t)src & 0x03) != 0 ) {
    CRC_CRC8H1 = *src++; //Write 8 BIT
  }

  while (src <= target-4) {
    CRC_CRC = *( uint32_t  *)src; //Write 32 BIT
    src += 4;
  }

  while (src < target) {
    CRC_CRC8H1 = *src++; //Write 8 Bit
  }

  return CRC_CRC;
}

/** generic function for all 32-Bit CRCs
 * @param polynom Polynom
 * @param seed Seed
 * @param flags Flags
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint32_t FastCRC32::generic(const uint32_t polynom, const uint32_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen)
{

  CRC_CTRL  = flags | (1<<CRC_CTRL_TCRC) | (1<<CRC_CTRL_WAS); // 32Bit Mode, prepare to write seed(25)
  CRC_GPOLY = polynom;                                        // Set polynom
  CRC_CRC   = seed;                                           // This is the seed
  CRC_CTRL_BIT(CRC_CTRL_WAS) = 0;                             // Clear WAS Bit - prepare to write data

  return update(data, datalen);
}

uint32_t FastCRC32::crc32_upd(const uint8_t *data, uint16_t len){return update(data, len);}
uint32_t FastCRC32::cksum_upd(const uint8_t *data, uint16_t len){return update(data, len);}

#endif // __MK20DX128__ || __MK20DX256__
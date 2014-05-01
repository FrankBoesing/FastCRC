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

#if defined(__MK20DX128__) || defined(__MK20DX256__)

#include "mk20dx128.h"
#include "FastCRC.h"

#define CRC_CRC8     *(volatile uint8_t  *)0x40032000
#define CRC_CRC8H1   *(volatile uint8_t  *)0x40032003
#define CRC_CRC16    *(volatile uint16_t *)0x40032000
#define CRC_CRC16H   *(volatile uint16_t *)0x40032002
#define CRC_GPOLY16  *(volatile uint16_t *)0x40032004


// ================= 8-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC8::FastCRC8(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;
}

/** SMBUS CRC
 * aka CRC-8
 * @return CRC value
 */
uint8_t FastCRC8::smbus(const uint8_t *data, const uint16_t datalen)
{
  return generic(0x07, 0, CRC_FLAG_NONE, data, datalen);
}
/** MAXIM 8-Bit CRC
 * equivalent to _crc_ibutton_update() in crc16.h from avr_libc
 * @return CRC value
 */
uint8_t FastCRC8::maxim(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x31 init=0x00 refin=true refout=true xorout=0x00 
  return generic(0x31, 0, CRC_FLAG_REFLECT, data, datalen);
}

/** Update
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::update(const uint8_t *data, const uint16_t datalen)
{
  uint32_t *d32 = (uint32_t *) data;
  uint16_t *d16 = (uint16_t *) data;
  uint8_t *d8   = (uint8_t *) data;  

  int32_t lengthWords = datalen>>2;
  int32_t i=0;
  
  while ( i < lengthWords ) CRC_CRC = d32[i++];
  
  i = (i<<2);
   
  if (datalen-i>1) {CRC_CRC16 = d16[i>>1]; i+=2;} 
  if (datalen-i>0) {CRC_CRC8H1 = d8[i];}
  
  return CRC_CRC;  
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
  uint32_t f = flags | (1<<24); //32-Bit Mode
  
  CRC_CTRL  = f | (1<<25); // prepare to write seed(25)
  CRC_GPOLY = ((uint32_t)polynom)<<24;
  CRC_CRC   = ((uint32_t)seed<<24)|((uint32_t)seed<<16)|((uint32_t)seed<<8)|seed;  //write seed
  CRC_CTRL  = f;
  
  return update(data, datalen);
}

// ================= 16-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC16::FastCRC16(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;     // enable crc clock
}

/** ccit
 * Alias "false CCITT"
 * equivalent to _crc_ccitt_update() in crc16.h from avr_libc
 * @return CRC value
 */
uint16_t FastCRC16::ccit(const uint8_t *data,const uint16_t datalen)
{
 // poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000
  return generic(0x1021, 0XFFFF, CRC_FLAG_REFLECT, data, datalen);
}


uint16_t FastCRC16::modbus(const uint8_t *data, const uint16_t datalen)
{ 
 // poly=0x8005 init=0xffff refin=true refout=true xorout=0x0000
  return generic(0x8005, 0XFFFF, CRC_FLAG_REFLECT, data, datalen);
}


uint16_t FastCRC16::kermit(const uint8_t *data, const uint16_t datalen)
{ 
 // poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 Swap Result
  return generic(0x1021, 0x00, CRC_FLAG_REFLECT_SWAP, data, datalen);
}


uint16_t FastCRC16::xmodem(const uint8_t *data, const uint16_t datalen)
{
  //width=16 poly=0x1021 init=0x0000 refin=false refout=false xorout=0x0000 
  return generic(0x1021, 0, ((1<<31) | (1<<30) | (0<<29) | (0<<28)), data, datalen);
}

uint16_t FastCRC16::x25(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x1021 init=0xffff refin=true refout=true xorout=0xffff
  return generic(0x1021, 0XFFFF, CRC_FLAG_REFLECT | CRC_FLAG_XOR, data, datalen);
}

 uint16_t FastCRC16::update(const uint8_t *data, const uint16_t datalen) 
{
  uint32_t *d32 = (uint32_t *) data;
  uint16_t *d16 = (uint16_t *) data;
  uint8_t *d8   = (uint8_t *) data;  

  int32_t lengthWords = datalen>>2;
  int32_t i=0;
  
  while ( i < lengthWords ) CRC_CRC = d32[i++];
  
  i = (i<<2);
   
  if (datalen-i>1) {CRC_CRC16 = d16[i>>1]; i+=2;} 
  if (datalen-i>0) {CRC_CRC8H1 = d8[i];}
  
  if (~CRC_CTRL & (1<<29))
    return CRC_CRC>>16;
  else   
    return CRC_CRC;
}


uint16_t FastCRC16::generic(const uint16_t polynom, const uint16_t seed, const uint32_t flags, const uint8_t *data, const uint16_t datalen)
{
  uint32_t f = flags | (1<<24); //32-Bit Mode
  
  CRC_CTRL  = f | (1<<25); // prepare to write seed(25)
  CRC_GPOLY = ((uint32_t)polynom)<<16;
  CRC_CRC   = ((uint32_t)seed<<24)|((uint32_t)seed<<16);  //write seed
  CRC_CTRL  = f;
  
  return update(data, datalen);
}




// ================= 32-BIT CRC ===================

/** Constructor
 * Enables CRC-clock
 */
FastCRC32::FastCRC32(){
  SIM_SCGC6 |= SIM_SCGC6_CRC;     // enable crc clock
}

uint32_t FastCRC32::crc32(const uint32_t *data, const uint16_t datalen)
{
  return generic(0x04C11DB7L, 0XFFFFFFFFL, (CRC_FLAG_REFLECT | CRC_FLAG_XOR), &data[0], datalen);
}

uint32_t FastCRC32::update(const uint32_t *data, const uint16_t datalen)
{
  for (uint16_t i=0; i<datalen; i++) {
    CRC_CRC = data[i];
  }
  return CRC_CRC;
}

uint32_t FastCRC32::generic(const uint32_t polynom, const uint32_t seed, const uint32_t flags, const uint32_t *data, const uint16_t datalen)
{
  CRC_CTRL  = (1<<25) | (1<<24);  // Set 32 BIT, prepare to write seed(25)
  CRC_GPOLY = polynom;            // set polynom
  CRC_CRC   = seed;               // this is the seed
  CRC_CTRL  = flags | (1<<24);    // prepare to write data
  
  return update(data, datalen);
}


#endif // __MK20DX128__ || __MK20DX256__
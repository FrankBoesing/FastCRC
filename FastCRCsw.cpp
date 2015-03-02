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


#if !defined(__MK20DX128__) && !defined(__MK20DX256__)

#include "FastCRC.h"
#include "FastCRC_cpu.h"
#include "FastCRC_tables.h"


// ================= 8-BIT CRC ===================

/** Constructor
 */
FastCRC8::FastCRC8(){}

/** SMBUS CRC
 * aka CRC-8
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::smbus_upd(const uint8_t *data, uint16_t datalen)
{
	uint8_t crc = seed;
	if (datalen) do {
		crc = pgm_read_byte(&crc_table_smbus[crc ^ *data]);
		data++;
	} while (--datalen);
	seed = crc;
	return crc;
}

uint8_t FastCRC8::smbus(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x07 init=0x00 refin=false refout=false xorout=0x00 check=0xf4
  seed = 0x00;
  return smbus_upd(data, datalen);
}

/** MAXIM 8-Bit CRC
 * equivalent to _crc_ibutton_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint8_t FastCRC8::maxim_upd(const uint8_t *data, uint16_t datalen)
{
	uint8_t crc = seed;
	if (datalen) do {
		crc = pgm_read_byte(&crc_table_maxim[crc ^ *data]);
		data++;
	} while (--datalen);
	seed = crc;
	return crc;
}
uint8_t FastCRC8::maxim(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x31 init=0x00 refin=true refout=true xorout=0x00  check=0xa1
  seed = 0x00;
  return maxim_upd(data, datalen);
}

// ================= 16-BIT CRC ===================
/** Constructor
 */
FastCRC16::FastCRC16(){}

#define crc_n4(crc, data, table) crc ^= data; \
	crc = pgm_read_word(&table[(crc & 0xff) + 0x300]) ^		\
	pgm_read_word(&table[((crc >> 8) & 0xff) + 0x200]) ^	\
	pgm_read_word(&table[((data >> 16) & 0xff) + 0x100]) ^	\
	pgm_read_word(&table[data >> 24]);

/** CCITT
 * Alias "false CCITT"
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::ccitt_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ pgm_read_byte(&crc_table_ccitt[(crc & 0xff) ^ *data++]);
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_ccitt);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_ccitt);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_ccitt);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_ccitt);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ pgm_read_word(&crc_table_ccitt[(crc & 0xff) ^ *data++]);
	}

	crc = REV16(crc);

	seed = crc;
	return crc;
}
uint16_t FastCRC16::ccitt(const uint8_t *data,const uint16_t datalen)
{
 // poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
  seed = 0xffff;
  return ccitt_upd(data, datalen);
}

/** MCRF4XX
 * equivalent to _crc_ccitt_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */

uint16_t FastCRC16::mcrf4xx_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_mcrf4xx[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_mcrf4xx);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_mcrf4xx);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_mcrf4xx[(crc & 0xff) ^ *data++];
	}

	seed = crc;
	return crc;
}

uint16_t FastCRC16::mcrf4xx(const uint8_t *data,const uint16_t datalen)
{
 // poly=0x1021 init=0xffff refin=true refout=true xorout=0x0000 check=0x6f91
  seed = 0xffff;
  return mcrf4xx_upd(data, datalen);
}

/** MODBUS
 * equivalent to _crc_16_update() in crc16.h from avr_libc
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::modbus_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_modbus[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_modbus);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_modbus);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_modbus);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_modbus);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_modbus[(crc & 0xff) ^ *data++];
	}

	seed = crc;
	return crc;
}

uint16_t FastCRC16::modbus(const uint8_t *data, const uint16_t datalen)
{
 // poly=0x8005 init=0xffff refin=true refout=true xorout=0x0000 check=0x4b37
  seed = 0xffff;
  return modbus_upd(data, datalen);
}

/** KERMIT
 * Alias CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::kermit_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_kermit[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_kermit);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_kermit);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_kermit);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_kermit);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_kermit[(crc & 0xff) ^ *data++];
	}

	seed = crc;
	return crc;
}

uint16_t FastCRC16::kermit(const uint8_t *data, const uint16_t datalen)
{
 // poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189
 // sometimes byteswapped presentation of result
  seed = 0x0000;
  return kermit_upd(data, datalen);
}

/** XMODEM
 * Alias ZMODEM, CRC-16/ACORN
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::xmodem_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_xmodem[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_xmodem);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_xmodem);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_xmodem);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_xmodem);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_xmodem[(crc & 0xff) ^ *data++];
	}

	crc = REV16(crc);

	seed = crc;
	return crc;
}

uint16_t FastCRC16::xmodem(const uint8_t *data, const uint16_t datalen)
{
  //width=16 poly=0x1021 init=0x0000 refin=false refout=false xorout=0x0000 check=0x31c3
  seed = 0x0000;
  return xmodem_upd(data, datalen);
}

/** X25
 * Alias CRC-16/IBM-SDLC, CRC-16/ISO-HDLC, CRC-B
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint16_t FastCRC16::x25_upd(const uint8_t *data, uint16_t len)
{

	uint16_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ crc_table_x25[(crc & 0xff) ^ *data++];
		len--;
	}

	while (len >= 16) {
		len -= 16;
		crc_n4(crc, ((uint32_t *)data)[0], crc_table_x25);
		crc_n4(crc, ((uint32_t *)data)[1], crc_table_x25);
		crc_n4(crc, ((uint32_t *)data)[2], crc_table_x25);
		crc_n4(crc, ((uint32_t *)data)[3], crc_table_x25);
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ crc_table_x25[(crc & 0xff) ^ *data++];
	}

	crc = ~crc;

	seed = crc;
	return crc;
}

uint16_t FastCRC16::x25(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x1021 init=0xffff refin=true refout=true xorout=0xffff check=0x906e
  seed = 0xffff;
  return x25_upd(data, datalen);
}





// ================= 32-BIT CRC ===================
/** Constructor
 */
FastCRC32::FastCRC32(){}

#define crc_n4d(crc, data, table) crc ^= data; \
	crc = pgm_read_dword(&table[(crc & 0xff) + 0x300]) ^	\
	pgm_read_dword(&table[((crc >> 8) & 0xff) + 0x200]) ^	\
	pgm_read_dword(&table[((data >> 16) & 0xff) + 0x100]) ^	\
	pgm_read_dword(&table[data >> 24]);
	
#define crcsm_n4d(crc, data, table) crc ^= data; \
	crc = (crc >> 8) ^ pgm_read_dword(&table[crc & 0xff]); \
	crc = (crc >> 8) ^ pgm_read_dword(&table[crc & 0xff]); \
	crc = (crc >> 8) ^ pgm_read_dword(&table[crc & 0xff]); \
	crc = (crc >> 8) ^ pgm_read_dword(&table[crc & 0xff]);

/** CRC32
 * Alias CRC-32/ADCCP, PKZIP, Ethernet, 802.3
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */

#define CRC_TABLE_CRC32 crc_table_crc32

uint32_t FastCRC32::crc32_upd(const uint8_t *data, uint16_t len)
{

	uint32_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ pgm_read_dword(&crc_table_crc32[(crc & 0xff) ^ *data++]);
		len--;
	}

	while (len >= 16) {
		len -= 16;
		#if CRC_BIGTABLES
		crc_n4d(crc, ((uint32_t *)data)[0], crc_table_crc32_big);
		crc_n4d(crc, ((uint32_t *)data)[1], crc_table_crc32_big);
		crc_n4d(crc, ((uint32_t *)data)[2], crc_table_crc32_big);
		crc_n4d(crc, ((uint32_t *)data)[3], crc_table_crc32_big);
		#else
		crcsm_n4d(crc, ((uint32_t *)data)[0], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[1], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[2], crc_table_crc32);
		crcsm_n4d(crc, ((uint32_t *)data)[3], crc_table_crc32);
		#endif
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ pgm_read_dword(&crc_table_crc32[(crc & 0xff) ^ *data++]);
	}

	crc = ~crc;

	seed = crc;
	return crc;
}

uint32_t FastCRC32::crc32(const uint8_t *data, const uint16_t datalen)
{
  // poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
  seed = 0xffffffff;
  return crc32_upd(data, datalen);
}

/** CKSUM
 * Alias CRC-32/POSIX
 * @param data Pointer to Data
 * @param datalen Length of Data
 * @return CRC value
 */
uint32_t FastCRC32::cksum_upd(const uint8_t *data, uint16_t len)
{

	uint32_t crc = seed;

	while (((uintptr_t)data & 3) && len) {
		crc = (crc >> 8) ^ pgm_read_dword(&crc_table_cksum[(crc & 0xff) ^ *data++]);
		len--;
	}

	while (len >= 16) {
		len -= 16;
		#if CRC_BIGTABLES
		crc_n4d(crc, ((uint32_t *)data)[0], crc_table_cksum_big);
		crc_n4d(crc, ((uint32_t *)data)[1], crc_table_cksum_big);
		crc_n4d(crc, ((uint32_t *)data)[2], crc_table_cksum_big);
		crc_n4d(crc, ((uint32_t *)data)[3], crc_table_cksum_big);
		#else
		crcsm_n4d(crc, ((uint32_t *)data)[0], crc_table_cksum);
		crcsm_n4d(crc, ((uint32_t *)data)[1], crc_table_cksum);
		crcsm_n4d(crc, ((uint32_t *)data)[2], crc_table_cksum);
		crcsm_n4d(crc, ((uint32_t *)data)[3], crc_table_cksum);
		#endif
		data += 16;
	}

	while (len--) {
		crc = (crc >> 8) ^ pgm_read_dword(&crc_table_cksum[(crc & 0xff) ^ *data++]);
	}

	crc = ~REV32(crc);

	seed = crc;
	return crc;
}

uint32_t FastCRC32::cksum(const uint8_t *data, const uint16_t datalen)
{
  // width=32 poly=0x04c11db7 init=0x00000000 refin=false refout=false xorout=0xffffffff check=0x765e7680
  seed = 0x00;
  return cksum_upd(data, datalen);
}

#endif // __MK20DX128__ || __MK20DX256__
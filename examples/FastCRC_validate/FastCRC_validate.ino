//FastCRC
//Validate computed CRCs
//
//(c) Frank Boesing 2014

#include <util/crc16.h>
#include <FastCRC.h>

#define Ser Serial



FastCRC8 CRC8;
FastCRC16 CRC16;
FastCRC32 CRC32;


uint8_t buf[9] = {'1','2','3','4','5','6','7','8','9'};


void setup() {
uint32_t crc;

  Ser.begin(115200);
  while (!Ser) {};

  Ser.printf("\r\n\r\nCRC Validation\r\n");

  crc = CRC8.smbus(buf, sizeof(buf));
  Ser.printf("\r\nSMBUS: %s",( crc==0xf4 ) ? "OK":"FALSE!!!!");

  crc = CRC8.maxim(buf, sizeof(buf));
  Ser.printf("\r\nMaxim: %s",(crc==0xa1) ? "OK":"FALSE!!!!");

  crc = CRC16.ccitt(buf, sizeof(buf));
  Ser.printf("\r\nCCITT 'false': %s",(crc==0x29b1) ? "OK":"FALSE!!!!");

  crc = CRC16.mcrf4xx(buf, sizeof(buf));
  Ser.printf("\r\nMCRF4XX: %s",(crc==0x6f91) ? "OK":"FALSE!!!!");

  crc = CRC16.modbus(buf, sizeof(buf));
  Ser.printf("\r\nMODBUS: %s",(crc==0x4b37) ? "OK":"FALSE!!!!");

  crc = CRC16.kermit(buf, sizeof(buf));
  Ser.printf("\r\nKERMIT: %s",(crc==0x2189) ? "OK":"FALSE!!!!");

  crc = CRC16.xmodem(buf, sizeof(buf));
  Ser.printf("\r\nXMODEM: %s",(crc==0x31c3) ? "OK":"FALSE!!!!");

  crc = CRC16.x25(buf, sizeof(buf));
  Ser.printf("\r\nX.25: %s",(crc==0x906e) ? "OK":"FALSE!!!!");

  crc = CRC32.crc32(buf, sizeof(buf));
  Ser.printf("\r\nCRC32: %s",(crc==0xcbf43926) ? "OK":"FALSE!!!!");

  crc = CRC32.cksum(buf, sizeof(buf));
  Ser.printf("\r\nCKSUM: %s",(crc==0x765e7680) ? "OK":"FALSE!!!!");
 //  Ser.println(CRC_CRC,HEX);
}


void loop() {
  delay(1000);
}

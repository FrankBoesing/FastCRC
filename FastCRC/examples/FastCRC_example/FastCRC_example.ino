
//FastCRC
//Quick'n dirty Benchmark
//
//(c) Frank Boesing 2014

#include <util/crc16.h>
#include <FastCRC.h>

#define Ser Serial
#define BUFSIZE 16384


FastCRC8 CRC8;
FastCRC16 CRC16;
FastCRC32 CRC32;

uint8_t buf[BUFSIZE];


// Supporting functions for Software CRC

inline uint16_t softcrc(uint16_t seed, uint8_t *data, uint16_t datalen) {
    for (uint16_t i=0; i<datalen; i++) {
        seed = _crc16_update(seed,  data[i]);
    }
    return seed;
}

inline uint16_t softcrcIbutton(uint16_t seed, uint8_t *data, uint16_t datalen) {
    for (uint16_t i=0; i<datalen; i++) {
        seed = _crc_ibutton_update(seed,  data[i]);
    }
    return seed;
}

inline uint16_t softcrcCCIT(uint16_t seed, uint8_t *data, uint16_t datalen) {
    for (uint16_t i=0; i<datalen; i++) {
        seed = _crc_ccitt_update(seed,  data[i]);
    }
    return seed;
}

inline uint16_t softcrcXMODEM(uint16_t seed, uint8_t *data, uint16_t datalen) {
    for (uint16_t i=0; i<datalen; i++) {
        seed = _crc_xmodem_update(seed,  data[i]);
    }
    return seed;
}


void setup() {
int time;
uint32_t crc;


  Ser.begin(115200);
 // while (!Ser) {};
  Ser.printf("\r\n\r\nCRC Benchmark\r\n");
  Ser.printf("Length: %u Bytes\r\n\r\n", sizeof(buf));

  //Fill array with data
  for (int i=0; i<BUFSIZE; i++) {
    buf[i] = (i+1) & 0xff;
  }

  Ser.printf("\r\nMaxim (iButton) 8-Bit CRC:\r\n");
  Ser.flush();

  time = micros();
  crc = CRC8.maxim(buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC : Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();


  time = micros();
  crc = softcrcIbutton(0, buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("Software: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();



  Ser.println("\r\nMODBUS 16-Bit CRC:");
  Ser.flush();

  time = micros();
  crc = CRC16.modbus(buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();

  time = micros();
  crc = softcrc(0xffff, buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("Software: Value 0x%X in %uus\r\n", crc, time);




  Ser.println("\r\nXMODEM 16-Bit CRC:");
  Ser.flush();

  time = micros();
  crc = CRC16.xmodem(buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();

  time = micros();
  crc = softcrcXMODEM(0, buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("Software: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();


  Ser.println("\r\nMCRF4XX 16-Bit CRC:");
  Ser.flush();

  time = micros();
  crc = CRC16.mcrf4xx(buf,BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();


  time = micros();
  crc = softcrcCCIT(0xffff, buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("Software: Value 0x%X in %uus\r\n", crc, time);
  Ser.flush();


  Ser.println("\r\nKERMIT 16-Bit CRC:");
  Ser.flush();

  time = micros();
  crc = CRC16.kermit(buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC: Value 0x%X in %uus\r\n", crc, time);


  Ser.println("\r\nEthernet 32-Bit CRC:");
  Ser.flush();

  time = micros();
  crc = CRC32.crc32(buf, BUFSIZE);
  time = micros() - time;
  Ser.printf("FastCRC: Value 0x%X in %uus\r\n", crc, time);

}

void loop() {
  delay(1000);
}

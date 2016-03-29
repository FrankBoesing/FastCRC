//FastCRC
//Benchmark
//
//(c) Frank Boesing 2014

#include <util/crc16.h>
#include <FastCRC.h>

#define BUFSIZE 16384


FastCRC8 CRC8;
FastCRC16 CRC16;
FastCRC32 CRC32;

uint8_t buf[BUFSIZE] __attribute__((aligned(4)));


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


void printVals(char * name, uint32_t crc, uint32_t time) {
	Serial.print(name);
	Serial.print(" Value:0x");
	Serial.print(crc, HEX);
	Serial.print(", Time: ");
	Serial.print(time);
	Serial.println(" us.");
}

void setup() {
int time;
uint32_t crc;

  delay(1500);

  Serial.begin(115200);

  Serial.println("CRC Benchmark");
  Serial.print("Length: ");
  Serial.print(sizeof(buf));
  Serial.println(" Bytes");
  Serial.println();

  //Fill array with data
  for (int i=0; i<BUFSIZE; i++) {
    buf[i] = (i+1) & 0xff;
  }


  noInterrupts();
  time = micros();
  crc = CRC8.maxim(buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("Maxim (iButton) FastCRC:",crc,time);

  noInterrupts();
  time = micros();
  crc = softcrcIbutton(0, buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("Maxim (iButton) builtin:",crc,time);




  noInterrupts();
  time = micros();
  crc = CRC16.modbus(buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("MODBUS FastCRC:",crc,time);

  noInterrupts();
  time = micros();
  crc = softcrc(0xffff, buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("MODBUS builtin:",crc,time);




  noInterrupts();
  time = micros();
  crc = CRC16.xmodem(buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("XMODEM FastCRC:",crc,time);

  noInterrupts();
  time = micros();
  crc = softcrcXMODEM(0, buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("XMODEM builtin:",crc,time);




  noInterrupts();
  time = micros();
  crc = CRC16.mcrf4xx(buf,BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("MCRF4XX FastCRC:",crc,time);

  noInterrupts();
  time = micros();
  crc = softcrcCCIT(0xffff, buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("MCRF4XX builtin:",crc,time);




  noInterrupts();
  time = micros();
  crc = CRC16.kermit(buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("KERMIT FastCRC:",crc,time);




  noInterrupts();
  time = micros();
  crc = CRC32.crc32(buf, BUFSIZE);
  time = micros() - time;
  interrupts();
  printVals("Ethernet FastCRC:",crc,time);

}

void loop() {
}

#include "pxt.h"
#include "mbed.h"
#include "MicroBitPin.h"
#include "MicroBitSerial.h"

#define PXT_PACKET_START 	0xFD
#define PXT_PACKET_END   	0xFE
#define PXT_CMD_STREAMOFF	0x7A
#define PXT_CMD_STREAMON_CB 0x7B
#define PXT_CMD_QUERY		0x7C

#define PXT_RET_CAM_SUCCESS	0xE0
#define PXT_RET_CAM_ERROR	0xE1

enum PixSerialPin {
    P0 = 0,
    P1 = 1,
    P2 = 2,
    P8 = 8,
    P12 = 12,
    P13 = 13,
    P14 = 14,
    P15 = 15,
    P16 = 16
};

using namespace pxt;

//% color=#A88002 weight=111 icon="\uf192"
namespace pixetto {

	MicroBitSerial *serial = nullptr;
	uint8_t data_buf[10] = {0xFF};
	
    bool getPinName(PixSerialPin p, PinName& name) {
      switch(p) {
        case P0: name = MICROBIT_PIN_P0;  return true;
        case P1: name = MICROBIT_PIN_P1;  return true;
        case P2: name = MICROBIT_PIN_P2;  return true;
        case P8: name = MICROBIT_PIN_P8;  return true;
        case P12: name = MICROBIT_PIN_P12; return true;
        case P13: name = MICROBIT_PIN_P13; return true;
        case P14: name = MICROBIT_PIN_P14; return true;
        case P15: name = MICROBIT_PIN_P15; return true;
        case P16: name = MICROBIT_PIN_P16; return true;
      }
      return false;
    }
	
	bool verifyChecksum(uint8_t *buf, int len)
	{
		uint8_t sum = 0;
		
		for (uint8_t i=1; i<len-2; i++)
			sum += buf[i];
		
		if (sum == PXT_PACKET_START || sum == PXT_PACKET_END)
			sum = 0xAA;
		
		return (sum == buf[8]);
	}

    //% 
    bool begin(PixSerialPin rx, PixSerialPin tx){
		PinName txn, rxn;
		uBit.sleep(8000);
		if (getPinName(tx, txn) && getPinName(rx, rxn))
		{
			serial = new MicroBitSerial(txn, rxn);
			serial->baud(38400);
			//serial->setRxBufferSize(64);
			//serial->setTxBufferSize(32);
			uBit.sleep(100);
			
			int try_streamon = 0;
			do {
				uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_STREAMON_CB, 0, PXT_PACKET_END};
				serial->send(cmd_buf, 5);
				
				int read_len = 0;
			
				uint8_t code_buf[5] = {0xFF};
				do {
					read_len = serial->read(code_buf, 1, ASYNC);
				} while (code_buf[0] != PXT_PACKET_START);
		
				read_len = serial->read(&code_buf[1], 4);

				if (code_buf[0] == PXT_PACKET_START &&
					code_buf[4] == PXT_PACKET_END &&
					code_buf[2] == PXT_RET_CAM_SUCCESS)
					return true;
				try_streamon++;
				uBit.sleep(500);
			} while (try_streamon < 4);
		}
		return false;
    }
    
    //%
    bool isDetected(){
		int read_len = 0;
		for (int a=0; a<10; a++)
			data_buf[a] = 0xFF;
	
		uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_QUERY, 0, PXT_PACKET_END};
		serial->send(cmd_buf, 5);

		do {
			read_len = serial->read(data_buf, 1, ASYNC);
		} while (data_buf[0] != PXT_PACKET_START);

		read_len = serial->read(&data_buf[1], 9);
		
		if (read_len != 9) return false;
		if (data_buf[9] != PXT_PACKET_END) return false;
		if (!verifyChecksum(data_buf, 10)) return false;
		if (data_buf[2] == 0) return false;
		return true;
	}
	//%
	int getFuncID(){
		return data_buf[2];
	}
	//%
	int getTypeID() {
		return data_buf[3];
	}
	//%
	int getPosX() {
		return data_buf[4];
	}
	//%
	int getPosY() {
		return data_buf[5];
	}
	//%
	int getWidth() {
		return data_buf[6];
	}
	//%
	int getHeight() {
		return data_buf[7];
	}
}

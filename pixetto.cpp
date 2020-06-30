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

enum PixFunction {
        //% block="Color Detection"
        COLOR_DETECTION=1,
        //% block="Color Label Detection"
        COLOR_LABEL=2,
        //% block="Shape Detection"
        SHAPE_DETECTION=3,
        //% block="Circle Detection"
        CIRCLE_DETECTION=4,
        //% block="Template Matching"
        TEMPLATE=6,
        //% block="Keypoint"
        KEYPOINT=8,
        //% block="Neural Network"
        NEURAL_NETWORK=15,
        //% block="Face Detection"
        FACE_DETECTION=11,
        //% block="Traffic Sign Detection"
        TRAFFIC_SIGN_DETECTION=12,
        //% block="Handwriting Digits Detection"
        HANDWRITING_DIGITS_DETECTION=13,
        //% block="Handwriting Letters Detection"
        HANDWRITING_LETTERS_DETECTION=14,
        //% block="AprilTag(16h5)"
        APRILTAG=10
};
    

using namespace pxt;

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
				int loop = 0;
				uint8_t code_buf[5] = {0xFF};
				do {
					read_len = serial->read(code_buf, 1, ASYNC);
					
					if (read_len == 0 || read_len == MICROBIT_NO_DATA) {
						loop++;
					}
				} while (code_buf[0] != PXT_PACKET_START && loop < 50000);
				
				if (read_len == 0) break;
					
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
	bool getFuncID(int func_id){
		return (data_buf[2] == func_id);
	}
	
    //% 
    bool get_colordetect_color(int color) {
		if (data_buf[2] == COLOR_DETECTION && data_buf[3] == color)
			return true;
        return false;
    }
    
    //% 
    bool get_shapedetect_shape(int shape) {
		if (data_buf[2] == SHAPE_DETECTION && data_buf[3] == shape)
			return true;
        return false;
    }

    //% 
    bool get_circledetect_color(int color) {
		if (data_buf[2] == CIRCLE_DETECTION && data_buf[3] == color)
			return true;
        return false;
    }

    //%
    bool get_nn_id(int id) {
		if (data_buf[2] == NEURAL_NETWORK && data_buf[3] == id)
			return true;
        return false;
    }

    //%
    bool get_traffic_sign(int sign) {
		if (data_buf[2] == TRAFFIC_SIGN_DETECTION && data_buf[3] == sign)
			return true;
        return false;
    }

    //%
    bool get_hand_digit(int digit) {
		if (data_buf[2] == HANDWRITING_DIGITS_DETECTION && data_buf[3] == digit)
			return true;
        return false;
    }

    //%
    bool get_hand_letter(int letter) {
		if (data_buf[2] == HANDWRITING_LETTERS_DETECTION && data_buf[3] == letter)
			return true;
        return false;
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

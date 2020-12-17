#include "pxt.h"
#include "MicroBit.h"

#ifdef CODAL_CONFIG_H
#define MICROBIT_CODAL 1
#else
#define MICROBIT_CODAL 0
#define target_panic(n) microbit_panic(n)
#define target_wait(n) wait_ms(n)
#endif

#define PXT_PACKET_START 	0xFD
#define PXT_PACKET_END   	0xFE
#define PXT_CMD_STREAMOFF	0x7A
#define PXT_CMD_STREAMON_CB 0x7B
#define PXT_CMD_QUERY		0x7C
#define PXT_CMD_RESET		0x53 //(83)

#define PXT_RET_CAM_SUCCESS	0xE0
#define PXT_RET_CAM_ERROR	0xE1

#define SERIAL_BUF_SIZE		64
#define DATA_SIZE			33

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
        //% block="Color Codes Detection"
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
        NEURAL_NETWORK=9,
        //% block="AprilTag(16h5)"
        APRILTAG=10,
        //% block="Face Detection"
        FACE_DETECTION=11,
        //% block="Traffic Sign Detection"
        TRAFFIC_SIGN_DETECTION=12,
        //% block="Handwriting Digits Detection"
        HANDWRITING_DIGITS_DETECTION=13,
        //% block="Handwriting Letters Detection"
        HANDWRITING_LETTERS_DETECTION=14,
        /*
        //% block="Remote Computing"
        REMOTE_COMPUTING=15,
        //% block="Lanes Detection"
        LANES_DETECTION=16,
        */
        //% block="Digits Operation"
        DIGITS_OPERATION=17
        /*
        //% block="Simple Classifier"
        SIMPLE_CLASSIFIER=18,
        //% block="Voice Commands"
        VOICE_COMMANDS=19
        */
};
    

//using namespace pxt;

namespace pixetto {

	MicroBitSerial *serial = nullptr;
	uint8_t data_buf[DATA_SIZE] = {0xFF};
	int data_len = 0;
	int m_x = 0;
	int m_y = 0;
	int m_w = 0;
	int m_h = 0;
	int m_eqLen = 0;
	float m_eqAnswer = 0;
	char m_eqExpr[17] = {0};
	
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
	
	bool ssflush()
	{
		uint8_t a;
		
		int read_len = 0;
		do {
			read_len = serial->read(&a, 1, ASYNC);
		} while (read_len > 0 && read_len != MICROBIT_NO_DATA);
		
		return true;
	}
	
	int ssread(uint8_t *buf, int len, int wait_loop)
	{
		int read_len = 0;
		int read_idx = 0;
		int loop = 0;
		do {
			read_len = serial->read(&buf[read_idx], 1, ASYNC);
			
			if (read_len == 0 || read_len == MICROBIT_NO_DATA)
				loop++;
			else
				read_idx++;
		} while (read_idx < len && loop < wait_loop);
		
		if (read_len == 0 || read_len == MICROBIT_NO_DATA)
			read_idx = read_len;

		return read_idx;
	}

	bool verifyChecksum(uint8_t *buf, int len)
	{
		uint8_t sum = 0;
		
		for (uint8_t i=1; i<len-2; i++)
			sum += buf[i];
		
		if (sum == PXT_PACKET_START || sum == PXT_PACKET_END || sum == 0xFF)
			sum = 0xAA;
		
		return (sum == buf[len-2]);
	}

	bool opencam(bool reset) 
	{
		if (reset)
			uBit.sleep(6000);
			
		int try_streamon = 0;
		do {
			ssflush();
			
			uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_STREAMON_CB, 0, PXT_PACKET_END};
			serial->send(cmd_buf, 5, ASYNC);
			
			int read_len = 0;
			uint8_t code_buf[5] = {0xFF};
			
			read_len = ssread(code_buf, 1, 50000);			
			if (read_len == 0 || read_len == MICROBIT_NO_DATA) return false;
			
			read_len = ssread(&code_buf[1], 4, 50000);

			if (read_len == 4 &&
			    code_buf[0] == PXT_PACKET_START &&
				code_buf[4] == PXT_PACKET_END &&
				code_buf[2] == PXT_RET_CAM_SUCCESS)
				return true;

			try_streamon++;
			uBit.sleep(500);
		} while (try_streamon < 4);
		
		return false;
	}
    //% 
    bool begin(PixSerialPin rx, PixSerialPin tx){
		int ret = false;
		PinName txn, rxn;
		uBit.sleep(8000);
		if (getPinName(tx, txn) && getPinName(rx, rxn))
		{
			serial = new MicroBitSerial(txn, rxn, 64, 20);

			#if MICROBIT_CODAL
			serial->setBaudrate(38400);
			#else
			serial->baud(38400);
			#endif
			//serial->setRxBufferSize(64);
			//serial->setTxBufferSize(32);
			uBit.sleep(100);
			
			ret = opencam(false);
		}
		return ret;
    }
	//%
    bool isDetected(){
		int read_len = 0;
		int loop = 0;
		int a = 0;
		for (a=0; a<DATA_SIZE; a++)
			data_buf[a] = 0xFF;
	
		ssflush();
		uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_QUERY, 0, PXT_PACKET_END};
		serial->send(cmd_buf, 5, ASYNC);

		do {
			read_len = ssread(data_buf, 1, 50000);
			loop++;
		} while (data_buf[0] != PXT_PACKET_START && loop < 300000);
		
		if (read_len == 0 || read_len == MICROBIT_NO_DATA) {
			uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_RESET, 0, PXT_PACKET_END};
			serial->send(cmd_buf, 5, ASYNC);
			opencam(true);
			return false;
		}

		read_len = ssread(&data_buf[1], 2, 50000);
		data_len = data_buf[1];
		if (data_len > 3)
			read_len = ssread(&data_buf[3], data_len - 3, 50000);
		else
			return false;

		if (read_len != (data_len-3)) return false;
		if (data_buf[data_len-1] != PXT_PACKET_END) return false;
		if (!verifyChecksum(data_buf, data_len)) return false;
		if (data_buf[2] == 0) return false; // num == 0
		
		if (data_buf[2] == DIGITS_OPERATION) {
			m_x = data_buf[3];
			m_y = data_buf[4];
			m_w = data_buf[5];
			m_h = data_buf[6];
			
			m_eqAnswer = 0;
			for (a=8; a<=14; a++) 
				m_eqAnswer = m_eqAnswer * 10 + data_buf[a];

			m_eqAnswer /= 100;
			if (data_buf[7] == 0) m_eqAnswer = 0 - m_eqAnswer;
			
			memset(m_eqExpr, 0, sizeof(m_eqExpr));
			m_eqLen = data_len - 17;
			for (a=0; a<m_eqLen; a++)
				m_eqExpr[a] = (char)data_buf[a+15];
		}
		/*
		else if (data_buf[2] == LANES_DETECTION) {
		}
		else if (data_buf[2] == APRILTAG) {
		}
		*/
		else {
			m_x = data_buf[4];
			m_y = data_buf[5];
			m_w = data_buf[6];
			m_h = data_buf[7];
		}
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
		return m_x;
	}
	//%
	int getPosY() {
		return m_y;
	}
	//%
	int getWidth() {
		return m_w;
	}
	//%
	int getHeight() {
		return m_h;
	}

	//%
	float getEquationAnswer() {
		return m_eqAnswer;
	}

	//%
	String getEquationExpr() {
		ManagedString s = m_eqExpr;
		return PSTR(s);
	}
	
}

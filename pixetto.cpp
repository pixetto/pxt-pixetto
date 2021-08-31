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
#define PXT_CMD_ENABLEFUNC	0x7D
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
        //% block="Sphere Detection"
        SPHERE_DETECTION=4,
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
        //% block="Handwritten Digits Detection"
        HANDWRITTEN_DIGITS_DETECTION=13,
        //% block="Handwritten Letters Detection"
        HANDWRITTEN_LETTERS_DETECTION=14,
        //% block="Remote Computing"
        REMOTE_COMPUTING=15,
        //% block="Lanes Detection"
        LANES_DETECTION=16,
        //% block="Digits Operation"
        DIGITS_OPERATION=17,
        //% block="Simple Classifier"
        SIMPLE_CLASSIFIER=18,
        //% block="Voice Commands"
        VOICE_COMMANDS=19,
        //% block="Lanes and Traffic Sign Detection"
        LANE_AND_SIGN=20        
};
    
enum PixApriltagField {
        //% block="position x"
        APRILTAG_POS_X=1,
        //% block="position y"
        APRILTAG_POS_Y,
        //% block="position z"
        APRILTAG_POS_Z,
        //% block="rotation x"
        APRILTAG_ROT_X,
        //% block="rotation y"
        APRILTAG_ROT_Y,
        //% block="rotation z"
        APRILTAG_ROT_Z,
        //% block="center x"
        APRILTAG_CENTER_X,
        //% block="center y"
        APRILTAG_CENTER_Y
};

enum PixLanesField {
        //% block="Left X1"
        LANES_LX1=1,
        //% block="Left Y1"
        LANES_LY1,
        //% block="Left X2"
        LANES_LX2,
        //% block="Left Y2"
        LANES_LY2,
        //% block="Right X1"
        LANES_RX1,
        //% block="Right Y1"
        LANES_RY1,
        //% block="Right X2"
        LANES_RX2,
        //% block="Right Y2"
        LANES_RY2
};

//using namespace pxt;
//using namespace codal;

//extern MicroBit uBit;

namespace pixetto {
	MicroBitSerial *serial = nullptr;
	uint8_t data_buf[DATA_SIZE] = {0xFF};
	int data_len = 0;
	int m_type = 0;
	int m_x = 0;
	int m_y = 0;
	int m_w = 0;
	int m_h = 0;
	int m_eqLen = 0;
	float m_eqAnswer = 0;
	char m_eqExpr[17] = {0};
	float m_posx=0, m_posy=0, m_posz=0, m_rotx=0, m_roty=0, m_rotz=0, m_centerx=0, m_centery=0;
	bool bOnStarting = false;
	
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
		
		sum %= 256;
		
		//if (sum == PXT_PACKET_START || sum == PXT_PACKET_END || sum == 0xFF)
		//	sum = 0xAA;
		
		return (sum == buf[len-2]);
	}
	
	bool checkcam()
	{
		ssflush();
		
		uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_STREAMON_CB, 0, PXT_PACKET_END};
		serial->send(cmd_buf, 5, ASYNC);
			
		int read_len = 0;
		int loop = 0;
		uint8_t code_buf[5] = {0xFF};
		
		do {
			read_len = serial->read(code_buf, 1, ASYNC);
			
			if (read_len == 0 || read_len == MICROBIT_NO_DATA) {
				loop++;
			}
		} while (code_buf[0] != PXT_PACKET_START && loop < 50000);
		
		if (read_len == 0 || read_len == MICROBIT_NO_DATA) return false;
			
		read_len = serial->read(&code_buf[1], 4);

		if (code_buf[0] == PXT_PACKET_START &&
			code_buf[4] == PXT_PACKET_END &&
			code_buf[2] == PXT_RET_CAM_SUCCESS)
			return true;
		return false;
	}
	
	bool opencam(bool reset) 
	{
		if (reset)
			uBit.sleep(8000);
			
		int try_streamon = 0;
		do {
			ssflush();
			
			uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_STREAMON_CB, 0, PXT_PACKET_END};
			serial->send(cmd_buf, 5, ASYNC);
			
			int read_len = 0;
			uint8_t code_buf[5] = {0xFF};
			int loop = 0;
			
			do {
				read_len = serial->read(code_buf, 1, ASYNC);
				loop++;
			} while (code_buf[0] != PXT_PACKET_START && loop < 300000);

			if (read_len == 0 || read_len == MICROBIT_NO_DATA) return false;
			
			read_len = serial->read(&code_buf[1], 4);

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
		bOnStarting = true;
		
		bool ret = false;
		PinName txn, rxn;
		uBit.sleep(6000);
		
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
			uBit.sleep(1000);
			
			ret = opencam(false);
		}
		if (ret)
			bOnStarting = false;
			
		return ret;
    }
    
	//%
	void enableFunc(int func_id){
		uint8_t cmd_buf[6] = {PXT_PACKET_START, 0x06, PXT_CMD_ENABLEFUNC, func_id, 0, PXT_PACKET_END};
		serial->send(cmd_buf, 6, ASYNC);
		return;
	}
	
	//%
    bool isDetected(){
		if (bOnStarting) 
			return false;
		
		int a = 0;
		for (a=0; a<DATA_SIZE; a++)
			data_buf[a] = 0xFF;
	
		ssflush();
		uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_QUERY, 0, PXT_PACKET_END};
		serial->send(cmd_buf, 5, ASYNC);

		int read_len = 0;
		int loop = 0;

		do {
			read_len = serial->read(data_buf, 1, ASYNC);
			loop++;
		} while (data_buf[0] != PXT_PACKET_START && loop < 300000);
		
		if (read_len == 0 || read_len == MICROBIT_NO_DATA) {
			if (!checkcam()) {
				uint8_t cmd_buf[5] = {PXT_PACKET_START, 0x05, PXT_CMD_RESET, 0, PXT_PACKET_END};
				serial->send(cmd_buf, 5, ASYNC);
				opencam(true);
			}
			return false;
		}

		read_len = serial->read(&data_buf[1], 2);
		data_len = data_buf[1];
		if (data_len > 3) {
			read_len = serial->read(&data_buf[3], data_len - 3);
		}
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
		else if (data_buf[2] == LANES_DETECTION) {
			m_x = data_buf[3];
			m_y = data_buf[4];
		}
		else if (data_buf[2] == SIMPLE_CLASSIFIER) {
			m_type = data_buf[3] * 256 + data_buf[4];
			m_x = data_buf[5];
			m_y = data_buf[6];
			m_w = data_buf[7];
			m_h = data_buf[8];
		}
		else if (data_buf[2] == LANE_AND_SIGN) {
			if (data_buf[18] == 0 || data_buf[18] == 2) {
				m_x = -1; m_y = -1;
			}
			else {
				m_x = data_buf[3];
				m_y = data_buf[4];
			}

			if (data_buf[18] == 0 || data_buf[18] == 1)
				m_type = -1;
			else
				m_type = data_buf[13];
		}
		else {
			m_type = data_buf[3];
			m_x = data_buf[4];
			m_y = data_buf[5];
			m_w = data_buf[6];
			m_h = data_buf[7];
			
			if (data_buf[2] == APRILTAG) {
				int value = 0;
				value = (short)(data_buf[8] * 256 + data_buf[9]);
				m_posx = (float)value / 100.0;
				value = (short)(data_buf[10] * 256 + data_buf[11]);
				m_posy = (float)value / 100.0;
				value = (short)(data_buf[12] * 256 + data_buf[13]);
				m_posz = (float)value / 100.0;
				
				m_rotx = (short)(data_buf[14] * 256 + data_buf[15]);
				m_roty = (short)(data_buf[16] * 256 + data_buf[17]);
				m_rotz = (short)(data_buf[18] * 256 + data_buf[19]);
				
				m_centerx = (short)(data_buf[20] * 256 + data_buf[21]);
				m_centery = (short)(data_buf[22] * 256 + data_buf[23]);
			}
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
    bool get_spheredetect_color(int color) {
		if (data_buf[2] == SPHERE_DETECTION && data_buf[3] == color)
			return true;
        return false;
    }

    //%
    bool get_template_id(int id) {
		if (data_buf[2] == TEMPLATE && data_buf[3] == id)
			return true;
        return false;
    }

    //%
    bool get_keypoint_id(int id) {
		if (data_buf[2] == KEYPOINT && data_buf[3] == id)
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
		if (data_buf[2] == LANE_AND_SIGN && m_type == sign)
			return true;
       return false;
    }

    //%
    bool get_hand_digit(int digit) {
		if (data_buf[2] == HANDWRITTEN_DIGITS_DETECTION && data_buf[3] == digit)
			return true;
        return false;
    }

    //%
    bool get_hand_letter(int letter) {
		if (data_buf[2] == HANDWRITTEN_LETTERS_DETECTION && data_buf[3] == letter)
			return true;
        return false;
    }	

    //%
    bool get_remote_computing(int id) {
		if (data_buf[2] == REMOTE_COMPUTING && data_buf[3] == id)
			return true;
        return false;
    }
	
    //%
    bool get_simple_classifier(int id) {
		if (data_buf[2] == SIMPLE_CLASSIFIER && data_buf[3] == id)
			return true;
        return false;
    }

    //%
    bool get_voice_command(int vcmd) {
		if (data_buf[2] == VOICE_COMMANDS && data_buf[3] == vcmd)
			return true;
        return false;
    }

	//%
	int getTypeID() {
		return m_type;
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
	
	//%
	float getApriltagField(int field) {
		switch(field) {
			case APRILTAG_POS_X:
				return m_posx;
			case APRILTAG_POS_Y:
				return m_posy;
			case APRILTAG_POS_Z:
				return m_posz;
			case APRILTAG_ROT_X:
				return m_rotx;
			case APRILTAG_ROT_Y:
				return m_roty;
			case APRILTAG_ROT_Z:
				return m_rotz;
			case APRILTAG_CENTER_X:
				return m_centerx;
			case APRILTAG_CENTER_Y:
				return m_centery;
			default:
				return 0;
		}
	}
		
	//%
	int getLanesField(int field) {
		switch(field) {
			case LANES_LX1:
				return data_buf[5];
			case LANES_LY1:
				return data_buf[6];
			case LANES_LX2:
				return data_buf[7];
			case LANES_LY2:
				return data_buf[8];
			case LANES_RX1:
				return data_buf[9];
			case LANES_RY1:
				return data_buf[10];
			case LANES_RX2:
				return data_buf[11];
			case LANES_RY2:
				return data_buf[12];
			default:
				return 0;
		}
	}
}

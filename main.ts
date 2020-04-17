//% weight=0 color=#A88002 icon="\uf0ad" block="Pixetto"
namespace pixetto {

    export enum PixColorType {
        //% block="red"
        RED=1,
        //% block="yellow"
        YELLOW,
        //% block="green"
        GREEN,
        //% block="blue"
        BLUE,
        //% block="purple"
        PURPLE,
        //% block="black"
        BLACK
    }
    
    export enum PixShapeType {
        //% block="circle"
        CIRCLE=1,
        //% block="rectangle"
        RECTANGLE,
        //% block="triangle"
        TRIANGLE,
        //% block="pentagon"
        PENTAGON,
    }

    export enum PixTrafficSign {
        //% block="No entre"
        SIGN_NO_ENTRE = 0,
        //% block="No left turn"
		SIGN_NO_LEFT_TURN,
		//% block="No right turn"
		SIGN_NO_RIGHT_TURN,
		//% block="Wrong way"
		SIGN_WRONG_WAY,
		//% block="No U turn"
		SIGN_NO_U_TURN,
		//% block="Maximum speed"
		SIGN_MAX_SPEED,
		//% block="One-way traffic"
		SIGN_ONEWAY_TRAFFIC,
		//% block="Left turn"
		SIGN_LEFT_TURN,
		//% block="Right turn"
		SIGN_RIGHT_TURN,
		//% block="Minimum speed"
		SIGN_MIN_SPEED,
		//% block="U turn"
		SIGN_U_TURN,
		//% block="Tunnel ahead"
		SIGN_TUNNEL_AHEAD,
		//% block="Beware of children"
		SIGN_BEWARE_OF_CHILDREN,
		//% block="Roundabout"
		SIGN_ROUNDABOUT,
		//% block="Yield to pedestrian"
		SIGN_YIELD_TO_PEDESTRIAN,
		//% block="Red light"
		SIGN_RED_LIGHT,
		//% block="Green light"
		SIGN_GREEN_LIGHT
    }
  
    export enum PixHandDigit {
        //% block="0"
        Digit_0,
        //% block="1"
        Digit_1,
        //% block="2"
        Digit_2,
        //% block="3"
        Digit_3,
        //% block="4"
        Digit_4,
        //% block="5"
        Digit_5,
        //% block="6"
        Digit_6,
        //% block="7"
        Digit_7,
        //% block="8"
        Digit_8,
        //% block="9"
        Digit_9
    }
    
    export enum PixHandLetter {
		//% block="Aa"
		A=0,
		//% block="Bb"
		B,
		//% block="Cc"
		C,
		//% block="Dd"
		D,
		//% block="Ee"
		E,
		//% block="Ff"
		F,
		//% block="Gg"
		G,
		//% block="Hh"
		H,
		//% block="Ii"
		I,
		//% block="Jj"
		J,
		//% block="Kk"
		K,
		//% block="Ll"
		L,
		//% block="Mm"
		M,
		//% block="Nn"
		N,
		//% block="Oo"
		O,
		//% block="Pp"
		P,
		//% block="Qq"
		Q,
		//% block="Rr"
		R,
		//% block="Ss"
		S,
		//% block="Tt"
		T,
		//% block="Uu"
		U,
		//% block="Vv"
		V,
		//% block="Ww"
		W,
		//% block="Xx"
		X,
		//% block="Yy"
		Y,
		//% block="Zz"
		Z
    }
      
	//% block="initialize vision sensor RX %rx TX %tx"
	//% shim=pixetto::begin
	export function begin(rx: PixSerialPin, tx: PixSerialPin): boolean {
		return false
    }
    
    //% block="object is detected"
    //% shim=pixetto::isDetected
    export function isDetected(): boolean {
		return false;
    }
    
    //% block="current function is %func_id"
    //% shim=pixetto::getFuncID
    export function getFuncID(func_id: PixFunction): boolean {
        return false;
    }
    
    //% block="Color Detection detects color %color"
    //% shim=pixetto::get_colordetect_color
    export function get_colordetect_color(color: PixColorType): boolean {
        return false;
    }
    
    //% block="Shape Detection detects shape %shape"
    //% shim=pixetto::get_shapedetect_shape
    export function get_shapedetect_shape(shape: PixShapeType): boolean {
        return false;
    }

    //% block="Circle Detection detects color %color"
    //% shim=pixetto::get_circledetect_color
    export function get_circledetect_color(color: PixColorType): boolean {
        return false;
    }

    //% block="Neural Network detects %id"
    //% shim=pixetto::get_nn_id
    export function get_nn_id(id: number): boolean {
        return false;
    }

    //% block="Traffic Sign Detection detects %sign"
    //% shim=pixetto::get_traffic_sign
    export function get_traffic_sign(sign: PixTrafficSign): boolean {
        return false;
    }

    //% block="Handwriting Digits Detection detects %digit"
    //% shim=pixetto::get_hand_digit
    export function get_hand_digit(digit: PixHandDigit): boolean {
        return false;
    }

    //% block="Handwriting Lettes Detection detects %letter"
    //% shim=pixetto::get_hand_letter
    export function get_hand_letter(letter: PixHandLetter): boolean {
        return false;
    }

    //% block="object position x"
    //% shim=pixetto::getPosX
    export function getPosX(): number {
        return 0;
    }
    
    //% block="object position y"
    //% shim=pixetto::getPosY
    export function getPosY(): number {
        return 0;
    }

    //% block="object width"
    //% shim=pixetto::getWidth
    export function getWidth(): number {
        return 0;
    }

    //% block="object height"
    //% shim=pixetto::getHeight
    export function getHeight(): number {
        return 0;
    }
}

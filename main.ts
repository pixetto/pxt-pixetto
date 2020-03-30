//% weight=0 color=#A88002 icon="\uf0ad" block="Pixetto"
namespace pixetto {
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
    
    //% block="current function"
    //% shim=pixetto::getFuncID
    export function getFuncID(): number {
        return 0;
    }
    
    //% block="object type"
    //% shim=pixetto::getTypeID
    export function getTypeID(): number {
        return 0;
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

//% weight=0 color=#A88002 icon="\uf0ad" block="Pixetto"
namespace pixetto {
	//% block="初始化視覺感測器 RX %rx TX %tx"
	//% shim=pixetto::begin
	export function begin(rx: PixSerialPin, tx: PixSerialPin): boolean {
		return false
    }
    
    //% block="識別到物體"
    //% shim=pixetto::isDetected
    export function isDetected(): boolean {
		return false
    }
    
    //% block="目前功能"
    //% shim=pixetto::getFuncID
    export function getFuncID(): number {
        return 0;
    }
    
    //% block="物體類別"
    //% shim=pixetto::getTypeID
    export function getTypeID(): number {
        return 0;
    }
    
    //% block="物體座標X"
    //% shim=pixetto::getPosX
    export function getPosX(): number {
        return 0;
    }
    
    //% block="物體座標Y"
    //% shim=pixetto::getPosY
    export function getPosY(): number {
        return 0;
    }

    //% block="物體寬度"
    //% shim=pixetto::getWidth
    export function getWidth(): number {
        return 0;
    }

    //% block="物體高度"
    //% shim=pixetto::getHeight
    export function getHeight(): number {
        return 0;
    }
}

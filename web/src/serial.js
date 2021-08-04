
class Serial {
    constructor() {
    }

    static requestPort() {
        return navigator.serial.requestPort().then(
            device => {
                return device;
            }
        );
        
    }
    getDevice() {
        let device = null;
        this.ready = false;
        return new Promise((resolve, reject) => {
            Serial.requestPort().then(dev => {
                console.log("Opening device...");
                device = dev;
                this.device = device;
                return this.device.open({baudRate: 115200});
            }).then(() => {
                console.log("Open!");
                this.ready = true;
                console.log(this.device);
                resolve();
            });
        });
    }

    read() {
        return new Promise((resolve, reject) => {
            const reader = this.device.readable.getReader();
            reader.read().then(({done, value}) => {
                if(done) {
                    // console.log("READER DONE");
                } else {
                    // console.log("READER");
                    // console.log(value);
                }

                reader.releaseLock();
                // console.log(value);
                // console.log(done)
                var string = new TextDecoder().decode(value);
                resolve(string);
            });
        }); 
    }
}

export { Serial };

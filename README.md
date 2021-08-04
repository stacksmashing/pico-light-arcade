# Pico Light Arcade

This is the code for the Pico Light Arcade game that can be seen here:
https://twitter.com/ghidraninja/status/1422900329369178113

## Hardware setup

The LEDs and buttons are connected in a 3x7 matrix. Pictures will follow shortly :)

### LEDS

The anode is the Y axis, and gets connected to IO 0, 1 and 2.

The cathode is the X axis of the matrix and gets connected to IOs 3-9.

### Buttons

The buttons' matrix Y axis is connected to IO 10-12, the X axis to IO 13-19.


## Building the firmware

```
export  PICO_SDK_PATH=path-to-sdk
mkdir build
cd build
cmake ..
make
```

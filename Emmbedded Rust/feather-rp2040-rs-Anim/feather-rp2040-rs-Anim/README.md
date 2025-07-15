Embedded Rust Animation Project for Feather RP2040
=================================================

main.rs:
Configures the rp2040, i2c, usb_manager, animations
Also it configures the accelerometer to tell its x, y, and z acceleration
and that is printed to the serial.

This repo contains 4 different Animations depending on the way the device is oriented in embedded rust for the Adafruit Feather RP2040.
This is done through using the Accelorameter and checking if +x, +y, -x, or -y is the orientation of the device. If it is in that way, it will play an animation based on that orientation

usb_manager.rs:
Code that will configure the USB peripheral as a serial port to allow for printing of formatted strings via the `write!` macro. Additionally, panic messages are sent to the serial port, and will show up when properly connected to a utility such as minicom, nRF terminal, or putty.

animations.rs:
Creates structs for 4 different animations
1. Color Cycle Pulse
2. Sprial that changes color each thim it finishes
3. Waterfall row animation
4. Bounceing a row and colum back and forth

How to run the project:
1. unizp the folder downloaded from canvas
2. Run cargo clean
3. Plug in rp2040 featherlight and its battery
4. Hold the bootloader and reset button once the device is plugged in
5. Run Cargo run flash the device by holding the bootloader and reset
8. Project should be loaded onto the board


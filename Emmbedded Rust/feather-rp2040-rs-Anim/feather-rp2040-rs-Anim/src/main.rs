#![no_std]
#![no_main]

/*** Low-level imports ***/
use core::panic::PanicInfo;
//use panic_halt as _;
use core::fmt::Write;
use cortex_m::prelude::*;
use cortex_m_rt::entry;
use embedded_hal::{
    blocking::i2c, digital::v2::{InputPin, OutputPin}, timer::CountDown
};
use embedded_time::rate::*;

/*** Board-specific imports ***/
use adafruit_feather_rp2040::hal::{self as hal, timer};
use adafruit_feather_rp2040::{
    hal::{
        clocks::{init_clocks_and_plls, Clock},
        pac,
        watchdog::Watchdog,
        Sio,
        gpio::{FunctionUart, FunctionSpi, FunctionI2C, FunctionPio0, Pin, PullDown},
        pac::interrupt,
        uart,
        I2C,
        pio::PIOExt,
        timer::{Timer},
    },
    Pins, XOSC_CRYSTAL_FREQ,
};

/*** External devices ***/
use fugit::{RateExtU32, ExtU32};
use ws2812_pio::Ws2812;
use lis3dh::{accelerometer::orientation, Lis3dh, SlaveAddr};
use lis3dh::accelerometer::{RawAccelerometer, Tracker};
use smart_leds::{RGB8};
use smart_leds_trait::SmartLedsWrite; // Ensure the trait is imported

// USB Device Support
// USB Device support
use usb_device::class_prelude::*;
// USB Communications Class Device support
mod usb_manager;
use usb_manager::UsbManager;
// Global USB objects & interrupt
static mut USB_BUS: Option<UsbBusAllocator<hal::usb::UsbBus>> = None;
static mut USB_MANAGER: Option<UsbManager> = None;

mod animations;
use animations::{nmPulse, nmRGB, nmRGB2, nmSpiral};

#[allow(non_snake_case)]
#[interrupt]
unsafe fn USBCTRL_IRQ() {
    match USB_MANAGER.as_mut() {
        Some(manager) => manager.interrupt(),
        None => (),
    };
}
#[panic_handler]
fn panic(panic_info: &PanicInfo) -> ! {
    if let Some(usb) = unsafe { USB_MANAGER.as_mut() } {
        writeln!(usb, "{}", panic_info).ok();
    }
    loop {}
}

#[entry]
fn main() -> ! {
    // Grab the Singleton objects
    let mut pac = pac::Peripherals::take().unwrap(); // SPI, I2C
    let core = pac::CorePeripherals::take().unwrap(); // Clock, clock div
    // Init the watchdog timer, to pass into the clock init
    let mut watchdog = Watchdog::new(pac.WATCHDOG); // Specific hardware on the chip

    let clocks = init_clocks_and_plls(
        XOSC_CRYSTAL_FREQ,
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut watchdog,
    ).ok().unwrap();

    // Init the GPIO (General Purpose IO)
    let sio = Sio::new(pac.SIO);
    //pins
    let pins = Pins::new(
        pac.IO_BANK0,
        pac.PADS_BANK0,
        sio.gpio_bank0,
        &mut pac.RESETS,
    );

    // Assuming `clocks.system_clock` is the correct value (frequency in Hz)
    let mut delay_timer = cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz());

    // Setup the Power Enable Pin
    let mut pwr_pin = pins.d10.into_push_pull_output();
    pwr_pin.set_high().unwrap();

    // Timer setup
    let timer = Timer::new(pac.TIMER, &mut pac.RESETS, &clocks);

    let (mut pio, sm0, _, _, _) = pac.PIO0.split(&mut pac.RESETS);
    let pin_d5: Pin<_, FunctionPio0, PullDown> = pins.d5.into_mode(); // Correct pin configuration

    // Initialize the WS2812 NeoPixel driver
    let mut neopixels = Ws2812::new(
        pin_d5,                        // Correctly configured pin (PIO function)         
        &mut pio,                      // PIO peripheral
        sm0,                           // State machine 0
        clocks.peripheral_clock.freq(),// Clock frequency for PIO
        timer.count_down(),            // CountDown timer
    );

    //Init the Lis3dh Accel driver
    //let mut accel: = new Lis3dh();
    // Init the Variables from the struct in Anim
    let mut nm_spiral = nmSpiral::new(RGB8::new(0, 0, 255));
    let mut nm_pulse = nmPulse::new(RGB8::new(255, 0, 0));
    let mut nm_rgb = nmRGB::new(RGB8::new(100, 100, 0));
    let mut nm_rgb2 = nmRGB2::new(RGB8::new(100, 100, 50));

    //USB DRVERS
      // Setup USB
      let usb = unsafe {
        USB_BUS = Some(UsbBusAllocator::new(hal::usb::UsbBus::new(
            pac.USBCTRL_REGS,
            pac.USBCTRL_DPRAM,
            clocks.usb_clock,
            true,
            &mut pac.RESETS,
        )));
        USB_MANAGER = Some(UsbManager::new(USB_BUS.as_ref().unwrap()));
        // Enable the USB interrupt
        pac::NVIC::unmask(hal::pac::Interrupt::USBCTRL_IRQ);
        USB_MANAGER.as_mut().unwrap()
    };


    //I2C Setup for ls3dh
    let sda_pin: Pin<_, FunctionI2C, _> = pins.sda.reconfigure();
    let scl_pin: Pin<_, FunctionI2C, _> = pins.scl.reconfigure();
    let mut i2c = hal::I2C::i2c1(
        pac.I2C1,
        sda_pin,
        scl_pin, // Try `not_an_scl_pin` here
        RateExtU32::kHz(400),
        &mut pac.RESETS,
        &clocks.system_clock,
    );
    let mut lis3dh = Lis3dh::new_i2c(i2c, SlaveAddr::Default).unwrap();
    //lis3dh.set_range(lis3dh::Range::G8).unwrap();
    //let mut tracker = Tracker::new(3700.0);
    
    let mut mode: u8 = 0;
    use lis3dh::accelerometer::Accelerometer;
    loop {
        // Your logic for controlling NeoPixels or other tasks
        let accel = lis3dh.accel_norm().unwrap();

        if accel.x > 0.9f32 {
            mode = 1;
        }else if accel.x < -0.9f32 {
            mode = 0;
        }else if accel.y > 0.9f32 {
            mode = 2;
        }else if accel.y < -0.9f32 {
            mode = 3;
        }
    
        nm_spiral.next();
        nm_pulse.next();
        nm_rgb.next();
        nm_rgb2.next();
        let ds: [RGB8; animations::NUM_PX] = match mode {
            0 => nm_spiral.to_list(),
            1 => nm_pulse.to_list(),
            2 => nm_rgb.to_list(),
            3 => nm_rgb2.to_list(),
            _ => [RGB8::new(0,0,0); animations::NUM_PX],
        };

        neopixels.write(ds.iter().cloned()).unwrap(); // Corrected write call


        write!(usb, "Accel {:?}\r\n", accel);

        delay_timer.delay_ms(100 as u32);
    }
}



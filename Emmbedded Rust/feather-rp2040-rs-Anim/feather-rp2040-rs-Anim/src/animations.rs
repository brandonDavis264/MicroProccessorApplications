/*Neomatrix.rs
    A Collection of Anmations for the display
*/

use smart_leds::{hsv::hsv2rgb, RGB8};
use smart_leds::hsv;
pub const WIDTH: usize = 8;
pub const HEIGHT: usize = 8;
pub const NUM_PX: usize = WIDTH*HEIGHT;

pub struct nmRGB2{
    strip: [RGB8; WIDTH*HEIGHT], //List Array
    color: RGB8,
    delta: bool,
    row: usize,
}

impl nmRGB2 {
    pub fn new(color: RGB8) -> nmRGB2 {
        Self {
            strip: [RGB8::new(0,0,0); WIDTH*HEIGHT],
            color: hsv2rgb(hsv::Hsv { hue: (255), sat: (255), val: (10) }),
            delta: true,
            row: 0,  
        }
    }

    pub fn set(&mut self){
        for(idx, px) in self.strip.iter_mut().enumerate(){
            if (idx)%8 == self.row  {
                *px = self.color;
            }else{
                *px = hsv2rgb(hsv::Hsv { hue: (140), sat: (255), val: (10) });
            }
        }
    }

    pub fn to_list(&self) -> [RGB8; WIDTH*HEIGHT]{
        self.strip
    }

    pub fn next(&mut self){
        //Bounce Row Value
        self.row += 1;

        if self.row == WIDTH {
            self.row = 0;
        }

        self.set();
    }
}



//---------------------------------------
//Color Cycle
pub struct nmRGB{
    strip: [RGB8; WIDTH*HEIGHT], //List Array
    color: RGB8,
    deltaR: bool,
    deltaC: bool,
    row: usize,
    px_counter: u8,
    descending: bool,
    col: usize,
}

impl nmRGB {
    pub fn new(color: RGB8) -> nmRGB {
        Self {
            strip: [RGB8::new(0,0,0); WIDTH*HEIGHT],
            color: color,
            deltaR: true,
            deltaC: true,
            row: 0,
            px_counter: 0, 
            descending: false,
            col: 0, 
        }
    }

    pub fn set(&mut self, color:RGB8){
        for(idx, px) in self.strip.iter_mut().enumerate(){
            if (idx)/8 == self.row || (idx%8) == (self.col) {
                *px = color;
            }else{
                *px = self.color;
            }
        }

        self.color = hsv2rgb(hsv::Hsv { hue: (100), sat: (40), val: (10) });
    }

    pub fn to_list(&self) -> [RGB8; WIDTH*HEIGHT]{
        self.strip
    }

    pub fn next(&mut self){
        if self.row == WIDTH-1 {
            self.deltaR = false;
        }else if self.row == 0 {
            self.deltaR = true;
        }
        
        if self.deltaR {
            self.row += 1;
        }else{
            self.row -= 1;
        }

        if self.col == WIDTH - 1 {
            self.deltaC = false; // Change direction when hitting the rightmost
        } else if self.col == 0 {
            self.deltaC = true; // Change direction when hitting the leftmost
        }
    
        if self.deltaC {
            self.col += 1; // Move right
        } else {
            self.col -= 1; // Move left
        }


                // Adjust px_counter to change the Value (V)
                if self.px_counter >= 50 {
                    self.descending = true;  // Start decreasing the Value (V)
                } else if self.px_counter <= 0 {
                    self.descending = false;  // Start increasing the Value (V)
                }
        
                // Adjust the px_counter to move through the cycle (up or down)
                if self.descending {
                    self.px_counter = self.px_counter.saturating_sub(5); // Decrease slowly
                } else {
                    self.px_counter = self.px_counter.saturating_add(5); // Increase slowly
                }
        let color = hsv2rgb(hsv::Hsv { hue: 255, sat: (100), val: 50 });
        self.set(color);
    }
}

//Pulsing Light Block
pub struct nmPulse{
    strip: [RGB8; WIDTH*HEIGHT], //List Array
    color: RGB8,
    px_counter: u8,
    descending: bool,
}

impl nmPulse{
    pub fn new(color: RGB8) -> nmPulse {
        Self {
            strip: [RGB8::new(0,0,0); WIDTH * HEIGHT],
            color: color,
            px_counter: 0,
            descending: false,
        }
    }

    pub fn clear(&mut self){
        for px in self.strip.iter_mut(){
            *px = RGB8::new(0,0,0)
        }
    }

    pub fn set(&mut self, color: RGB8){
        for px in self.strip.iter_mut() {
            *px = color;
        }
    }

    pub fn to_list(&self) -> [RGB8; WIDTH*HEIGHT]{
        self.strip
    }

    pub fn next(&mut self){
        // Set fixed Hue and Saturation values
        let saturation: u8 = 255; // Full saturation

        // Adjust px_counter to change the Value (V)
        if self.px_counter >= 255 {
            self.descending = true;  // Start decreasing the Value (V)
        } else if self.px_counter <= 0 {
            self.descending = false;  // Start increasing the Value (V)
        }

        // Adjust the px_counter to move through the cycle (up or down)
        if self.descending {
            self.px_counter = self.px_counter.saturating_sub(5); // Decrease slowly
        } else {
            self.px_counter = self.px_counter.saturating_add(5); // Increase slowly
        }

        // Convert the HSV values (with changing V) to RGB
        let color = hsv2rgb(hsv::Hsv { hue: (self.px_counter), sat: (saturation), val: (10) });

        // Set the new color to the strip
        self.set(color);
    
    }
}

//A Spiraling Pixel
pub struct nmSpiral{
    strip: [RGB8; WIDTH*HEIGHT],
    color: RGB8, //List Array
    color1: RGB8,
    color2: RGB8,
    row: usize,
    col: usize,
    switchColor: bool,
}

impl nmSpiral{
    pub fn new(color: RGB8) -> nmSpiral{
        Self{
            strip: [RGB8::new(0,0,0); WIDTH*HEIGHT],
            color: hsv2rgb(hsv::Hsv { hue: (150), sat: (255), val: (10)}),
            color2: hsv2rgb(hsv::Hsv { hue: (150), sat: (255), val: (10)}),
            color1: hsv2rgb(hsv::Hsv { hue: (100), sat: (255), val: (10) }),
            switchColor: false,
            row: 0,
            col: 0,
        }
    }

    //Changr the instance of the struct
    pub fn set(&mut self){
        let index = self.row * WIDTH + self.col;

        for(idx, px) in self.strip.iter_mut().enumerate(){
            if idx == index {
                *px = self.color;
            }else if idx < index{
                *px = self.color;
            }else{
                *px = self.color1;
            }
        }
    }

    pub fn to_list(&self) -> [RGB8; WIDTH*HEIGHT]{
        self.strip
    }

    pub fn next(&mut self){
        if self.col + 1 < WIDTH {
            self.col += 1; // Move to the next pixel in the row
        } else {
            self.col = 0; // Reset the column to the beginning of the row
            if self.row + 1 < HEIGHT {
                self.row += 1; // Move to the next row
            } else {
                self.row = 0;
                self.switchColor = true; // Reset to the first row if we reach the end
            }
        }

        if self.switchColor{
            self.color = self.color1;
            self.color1 = self.color2;
            self.color2 = self.color;
            self.switchColor = false;
        } 
        self.set();
    }
}
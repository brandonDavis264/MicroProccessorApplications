import serial
import time
import random

# Open the UART4 serial port
ser = serial.Serial('/dev/ttyO4', 9600, timeout=1)  # UART4 on BeagleBone

# Wait for the serial port to initialize
time.sleep(2)

# Define screen dimensions
X_Max = 240
Y_Max = 280

# Generate random values for the box
random_X = random.randint(0, X_Max - 1)  # X position, ensuring the box fits within the screen width
random_Y = random.randint(0, Y_Max - 1)  # Y position, ensuring the box fits within the screen height

# Random width and height, ensuring the box fits within the screen
random_W = random.randint(1, X_Max - random_X)  # Width, ensuring the box fits
random_H = random.randint(1, Y_Max - random_Y)  # Height, ensuring the box fits

# Random color selection: 1=red, 2=green, 3=blue
random_Color = random.randint(1, 3)

# Prepare the data to be sent via UART
# Here, we will send the position (x, y), size (width, height), and color

# Split Y-coordinate into two bytes (high and low)
random_Y_high = random_Y >> 8  # High byte (shift right 8 bits)
random_Y_low = random_Y & 0xFF  # Low byte (mask with 0xFF)

# Split Height into two bytes (high and low)
random_H_high = random_H >> 8  # High byte (shift right 8 bits)
random_H_low = random_H & 0xFF  # Low byte (mask with 0xFF)

# Prepare the data array (5 bytes for box and 2 bytes for Y, 2 bytes for height)
data = bytearray([
    random_X,               # Byte 1: X position
    random_Y_high,          # Byte 2: Y high byte
    random_Y_low,           # Byte 3: Y low byte
    random_W,               # Byte 4: Width
    random_H_high,          # Byte 5: Height high byte
    random_H_low,           # Byte 6: Height low byte
    random_Color            # Byte 7: Color (1 = red, 2 = green, 3 = blue)
])

# Send the data via UART
ser.write(data)

# Wait for the data to be transmitted
time.sleep(1)

# Close the serial port
ser.close()

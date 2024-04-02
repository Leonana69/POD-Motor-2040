import board
import neopixel_write
import digitalio
import busio
import time

uart = busio.UART(board.TX, board.RX, baudrate=115200)

pin = digitalio.DigitalInOut(board.NEOPIXEL)
pin.direction = digitalio.Direction.OUTPUT
pixel_color = [bytearray([0, 0, 0]), bytearray([255, 0, 0])]
count = 0

while True:
    data = uart.read(32)  # read up to 32 bytes
    if data is not None:
        # convert bytearray to string
        data_string = ''.join([chr(b) for b in data])
        print(data_string, end="")


    neopixel_write.neopixel_write(pin, pixel_color[count % 2])
    count += 1
    time.sleep(0.5)
    print(time.time())
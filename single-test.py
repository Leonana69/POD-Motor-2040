# SPDX-License-Identifier: MIT

import time
import board
import digitalio
from pod_motor.motor import Wheel

UPDATE_RATE = 100
 
# Create a digitalinout object for the user switch
user_sw = digitalio.DigitalInOut(board.USER_SW)
user_sw.direction = digitalio.Direction.INPUT
user_sw.pull = digitalio.Pull.UP

def button_pressed():
    return not user_sw.value

speed = 0.1
count = 0

button_state = button_pressed()

wheel = Wheel('A', 25000, UPDATE_RATE, 380)

# Run until the user switch is pressed
while True:
    if button_state and not button_pressed():
        speed = 0.1 - speed
    button_state = button_pressed()

    wheel.set_speed(speed)
    wheel.update()
    time.sleep(1 / UPDATE_RATE)
    if count % 50 == 0:
        print(f"Speed: {wheel.get_speed()}")
    count += 1
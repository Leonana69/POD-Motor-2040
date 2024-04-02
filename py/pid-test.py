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

speed = -0.4
count = 0

button_state = button_pressed()

wheel = [
    Wheel('A', 25000, UPDATE_RATE, 50),
    Wheel('B', 25000, UPDATE_RATE, 50),
    Wheel('C', 25000, UPDATE_RATE, 50),
    Wheel('D', 25000, UPDATE_RATE, 50)
]

# Run until the user switch is pressed
while True:
    if button_state and not button_pressed():
        speed = -0.5 - speed
    button_state = button_pressed()

    for i in range(0, 4):
        wheel[i].set_speed(speed)
        wheel[i].update()
    time.sleep(1 / UPDATE_RATE)
    if count % 50 == 0:
        print(f"Speed: {wheel[0].get_speed()}, {wheel[0].mot.throttle}")
    count += 1
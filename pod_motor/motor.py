from adafruit_motor import motor
import board
import pwmio
import rotaryio
from .pid import PID

VEL_KP = 30.0
VEL_KI = 2.0
VEL_KD = 1.0
INTEGRAL_LIMIT = 0.0
OUTPUT_LIMIT = 50.0

MOTOR_PINS = {
    'A': [board.MOTOR_A_P, board.MOTOR_A_N, board.ENCODER_A_A, board.ENCODER_A_B],
    'B': [board.MOTOR_B_P, board.MOTOR_B_N, board.ENCODER_B_A, board.ENCODER_B_B],
    'C': [board.MOTOR_C_P, board.MOTOR_C_N, board.ENCODER_C_A, board.ENCODER_C_B],
    'D': [board.MOTOR_D_P, board.MOTOR_D_N, board.ENCODER_D_A, board.ENCODER_D_B]
}

class Wheel:
    def __init__(self, channel: str, freq: int, update_rate = 100, gear_ratio = 50, mode = motor.SLOW_DECAY):
        self.speed = 0
        self.gear_ratio = gear_ratio
        # Create the pwm and objects
        pins = MOTOR_PINS[channel]
        pwm_p = pwmio.PWMOut(pins[0], frequency=freq)
        pwm_n = pwmio.PWMOut(pins[1], frequency=freq)
        self.mot = motor.DCMotor(pwm_p, pwm_n)
        self.mot.decay_mode = mode
        self.mot.throttle = 0
        self.update_rate = update_rate
        # Create the encoder object
        self.encoder = rotaryio.IncrementalEncoder(pins[2], pins[3], divisor=1)
        self.pid = PID(VEL_KP, VEL_KI, VEL_KD, update_rate, INTEGRAL_LIMIT, OUTPUT_LIMIT)
        self.counts_per_rev = 24 * gear_ratio
        self.last_revs = 0

    def set_speed(self, speed):
        self.speed = speed

    def get_speed(self):
        return self.speed

    def stop(self):
        self.speed = 0

    def update(self):
        revs = self.encoder.position / self.counts_per_rev
        vel = (revs - self.last_revs) * self.update_rate
        self.last_revs = revs
        accel = self.pid.update(self.speed, vel)
        self.mot.throttle = max(min(self.mot.throttle + ((accel / self.update_rate)), 1.0), -1.0)
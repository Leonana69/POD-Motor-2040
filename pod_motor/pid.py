class PID:
    def __init__(self, kp, ki, kd, sample_rate, max_integral = 0, max_output = 0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.integral = 0
        self.last_error = 0
        self.rate = sample_rate
        self.max_integral = max_integral
        self.max_output = max_output

    def update(self, target, value):
        error = target - value
        self.integral += error / self.rate
        if self.max_integral != 0:
            self.integral = max(min(self.integral, self.max_integral), -self.max_integral)
        derivative = (error - self.last_error) * self.rate
        self.last_error = error
        output = self.kp * error + self.ki * self.integral + self.kd * derivative
        if self.max_output != 0:
            output = max(min(output, self.max_output), -self.max_output)
        return output
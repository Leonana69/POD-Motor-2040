#include <cstdio>
#include "pico/stdlib.h"

#include "motor2040.hpp"
#include "button.hpp"
#include "pid.hpp"

#include "hardware/uart.h"
#include "hardware/irq.h"
/*
A demonstration of driving all four of Motor 2040's motor outputs through a
sequence of velocities, with the help of their attached encoders and PID control.

Press "Boot" to exit the program.
*/

using namespace motor;
using namespace encoder;

enum Wheels {
	FL = 3, FR = 0,
	RL = 2, RR = 1,
};

// The gear ratio of the motor
constexpr float GEAR_RATIO = 110.0f;

// The counts per revolution of the motor's output shaft
constexpr float COUNTS_PER_REV = MMME_CPR * GEAR_RATIO;

// The scaling to apply to the motor's speed to match its real-world speed
constexpr float SPEED_SCALE = 5.4f;

// How many times to update the motor per second
const uint UPDATES_RATE = 100;
constexpr float UPDATE_TIME = 1.0f / (float)UPDATES_RATE;

// The time to travel between each random value
constexpr float TIME_FOR_EACH_MOVE = 2.0f;
const uint UPDATES_PER_MOVE = TIME_FOR_EACH_MOVE * UPDATES_RATE;

// How many of the updates should be printed (i.e. 2 would be every other update)
const uint PRINT_DIVIDER = 20;

// The speed to drive the wheels at
constexpr float DRIVING_SPEED = 0.2f;

// PID values
constexpr float VEL_KP = 30.0f;   // Velocity proportional (P) gain
constexpr float VEL_KI = 2.0f;    // Velocity integral (I) gain
constexpr float VEL_KD = 0.5f;    // Velocity derivative (D) gain

// Create an array of motor pointers
const pin_pair motor_pins[] = { motor2040::MOTOR_A, motor2040::MOTOR_B,
                            	motor2040::MOTOR_C, motor2040::MOTOR_D };
const uint NUM_MOTORS = count_of(motor_pins);
Motor *motors[NUM_MOTORS];

// Create an array of encoder pointers
const pin_pair encoder_pins[] = { motor2040::ENCODER_A, motor2040::ENCODER_B,
                                  motor2040::ENCODER_C, motor2040::ENCODER_D };
const char* ENCODER_NAMES[] = {"RR", "RL", "FL", "FR"};
const uint NUM_ENCODERS = count_of(encoder_pins);
Encoder *encoders[NUM_ENCODERS];

// Create the user button
Button user_sw(motor2040::USER_SW);

// Create an array of PID pointers
PID vel_pids[NUM_MOTORS];

#define UART_ID uart0
#define UART_IRQ UART0_IRQ
void uart_init() {
    uart_init(UART_ID, 115200); // Initialize UART0 with a baud rate of 115200
    gpio_set_function(motor2040::TX_TRIG, GPIO_FUNC_UART);
    gpio_set_function(motor2040::RX_ECHO, GPIO_FUNC_UART);
}

#include "link.h"
#include "podtp.h"
static PodtpPacket packet = { 0 };
unsigned int packet_count = 0;
void UART_IRQ_handler() {
	// Check if the interrupt is for UART
	while (uart_is_readable(UART_ID)) {
		// Read the data from the UART
		char data = uart_getc(UART_ID);
		if (linkBufferPutChar(data)) {
			linkGetPacket(&packet);
			packet_count += 1;
		}
	}
}

void uart_interrupt_init() {
    // Enable UART receive interrupt
    uart_set_irq_enables(UART_ID, true, false); // Enable receive interrupt, disable transmit interrupt
    // Set the UART interrupt handler
    irq_set_exclusive_handler(UART_IRQ, UART_IRQ_handler);
    // Enable the UART interrupt
    irq_set_enabled(UART_IRQ, true);
}

int main() {
	stdio_init_all();
	uart_init();
	uart_interrupt_init();

	// Fill the arrays of motors, encoders, and pids, and initialise them
	for (auto i = 0u; i < NUM_MOTORS; i++) {
		motors[i] = new Motor(motor_pins[i], NORMAL_DIR, SPEED_SCALE);
		motors[i]->init();

		encoders[i] = new Encoder(pio0, i, encoder_pins[i], PIN_UNUSED, NORMAL_DIR, COUNTS_PER_REV, true);
		encoders[i]->init();

		vel_pids[i] = PID(VEL_KP, VEL_KI, VEL_KD, UPDATE_TIME);
	}

	// Reverse the direction of the B and D motors and encoders
	motors[FL]->direction(REVERSED_DIR);
	motors[RL]->direction(REVERSED_DIR);
	encoders[FL]->direction(REVERSED_DIR);
	encoders[RL]->direction(REVERSED_DIR);

	// Enable all motors
	for(auto i = 0u; i < NUM_MOTORS; i++) {
		motors[i]->enable();
	}

	Encoder::Capture captures[NUM_MOTORS];
	int missing_packet_count = 0;
	while (true) {
		if (packet_count > 0) {
			packet_count -= 1;
			motor_2040_control_t *vel = (motor_2040_control_t *) packet.data;
			for (auto i = 0u; i < NUM_MOTORS; i++) {
				// set the setpoint of the PID to the speed received from the STM32
				vel_pids[i].setpoint = vel->speed[i] / 1000.0f;
			}
		} else {
			missing_packet_count += 1;
			if (missing_packet_count > 5) {
				for (auto i = 0u; i < NUM_MOTORS; i++) {
					vel_pids[i].setpoint = 0.0f;
				}
			}
		}

		// Capture the state of all the encoders
		for (auto i = 0u; i < NUM_MOTORS; i++) {
			captures[i] = encoders[i]->capture();
		}

		motor_encoder_t encoder_data = {
			.count = {
				captures[FR].count(),
				captures[RR].count(),
				captures[RL].count(),
				captures[FL].count(),
			}
		};
		uint8_t length = sizeof(encoder_data);
		uint8_t *buffer = linkPackData((uint8_t *) &encoder_data, &length);
		uart_write_blocking(UART_ID, buffer, length);

		for (auto i = 0u; i < NUM_MOTORS; i++) {
			// Calculate the acceleration to apply to the motor to move it closer to the velocity setpoint
			float accel = vel_pids[i].calculate(captures[i].revolutions_per_second());
			// Accelerate or decelerate the motor
			if (vel_pids[i].setpoint == 0 && fabs(motors[i]->speed()) < 0.3f) {
				motors[i]->speed(0.0f);
			} else {
				motors[i]->speed(motors[i]->speed() + (accel * UPDATE_TIME));
			}
		}

		sleep_ms(1000 / UPDATES_RATE);
	}
}

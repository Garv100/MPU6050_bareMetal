# MPU6050 Bare-Metal Interface for ATmega328P

This project demonstrates a bare-metal implementation for interfacing the MPU6050 6-axis IMU sensor with the ATmega328P microcontroller using I2C communication. It includes reading raw accelerometer and gyroscope data, calculating roll, pitch, and yaw angles, and sending the results over UART for monitoring.

---

## Features

- Direct register-level I2C communication (no external libraries)
- MPU6050 initialization and configuration
- Reading raw accelerometer and gyroscope data
- Computing orientation angles (roll, pitch, yaw)
- UART output of angle data at 9600 baud
- Minimalistic and optimized for AVR microcontrollers

---

## Hardware Setup

- **Microcontroller:** ATmega328P (e.g., Arduino Uno)
- **Sensor:** MPU6050 connected via I2C (SCL and SDA pins)
- **UART:** Serial connection for data output (TX pin connected to serial monitor)

Make sure pull-up resistors (4.7kΩ to 10kΩ) are present on the I2C lines (SCL and SDA).

---

## Usage

1. Clone this repository:

   ```bash
   git clone https://github.com/Garv100/MPU6050_bareMetal.git
   cd MPU6050_bareMetal

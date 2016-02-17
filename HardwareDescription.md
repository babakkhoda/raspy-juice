# Introduction #

GOTO NEWER PAGE  **http://code.google.com/p/raspy-juice/wiki/1_Hardware_Description**

Raspy Juice is an experimental expansion board to supply a Raspberry Pi (**RPi**) host computer with a regulated +5V from a wide-range voltage source. The board also contains a real-time clock, an RS232-level translator for the host console serial port, and an expansion AVR ATmega168A  microcontroller (MCU). This MCU provides the service of controlling 4 channels of RC servo outputs, an RS485 interface, and a half-duplex software-based RS232 interface. In addition, the spare pins of the MCU are brought to an expansion header which may be used for other purposes. The MCU itself is interfaced to the host computer through an I2C/TWI interface, in which the former is a slave device.

# 1 Details #

### 1.1 Main Features ###
  * 6-23V input to 5V, 3A high-efficiency (>80%) buck regulator with input polarity protection.
The main feature of this expansion board is to supply the RPi host computer with a regulated +5V through the GPIO header. With the wide voltage input range, the RPi can be powered from a wide variety of external sources such as batteries, 12V power adapters, solar battery sources, etc. Additional +5V power outputs are also available at the pins of the JST servo ports connectors.
  * RS232-level translated RPi console port with either 2.5mm stereo jack or JST connector.
  * RTC based on NXP PCF8523 with backup power capacitor.
### 1.2 Secondary Features ###
  * Atmel ATmega168A AVR microcontroller, running at +3.3V, 14.7456MHz and as a slave I2C device to the RPi host computer.
  * Secondary +3.3 450mA high-efficiency buck regulator
  * 4-channel RC servo port with JST connector
  * RS485 interface to USART0 of the AVR microcontroller.
  * RS232 interface to the programmable pins of the AVR microcontroller.
  * Expansion header of unused programmable pins of the microcontroller.



# 2 Module Conections #
![https://raspy-juice.googlecode.com/svn/wiki/juice-r1-conndiag.png](https://raspy-juice.googlecode.com/svn/wiki/juice-r1-conndiag.png)

### 2.1 RPi GPIO header ###
See description on http://elinux.org/Rpi_Low-level_peripherals

Raspy Juice supplies the host Raspberry Pi with regulated +5V through this GPIO header using a 2A poly-resettable (PTC) fuse. **Warning: do not connect a +5V supply through the Raspberry Pi micro-USB connector when used with Raspy Juice.**


### 2.2 DC Power connector ###
| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | GND          | Ground connection |
| 2           | +VIN         | 6 - 23V positive input, reverse-polarity protected. |


### 2.3 Console RS232 port and jack ###
The two connectors are wired in parallel and only one of these connectors may be used at any time. These ports provide for an RS232 level-translated of the Raspberry Pi console serial port signals (from the above GPIO header), to connect to a laptop, computer or other embedded devices.

##### 2.3.1 JST connector #####
| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | GND          | Ground connection |
| 2           | CON-TX       | RS232-level RPi console TX output signal. |
| 3           | CON-RX       | RS232-level RPi console RX input signal. |

##### 2.3.2 Stereo 2.5mm jack #####
| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| Ring        | GND          | Ground connection |
| Tip         | CON-TX       | RS232-level RPi console TX output signal. |
| Middle      | CON-RX       | RS232-level RPi console RX input signal. |


### 2.4 AVR RS232 port ###
From the expansion AVR microcontroller using a software-emulated UART, this port provides an RS232-level interface for connection to a computer, or other embedded devices. The transmission and reception of the data is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the I2C interface of  the GPIO header.

| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | GND          | Ground connection |
| 2           | AVR-232TX    | RS232-level AVR TX output signal. Connected and translated from AVR pin PD3. |
| 3           | AVR-232RX    | RS232-level AVR RX input signal. Connected and translated to AVR pin PD2/INT0. |

### 2.5 AVR RS485 port ###
From the expansion AVR microcontroller using the USART0, this port provides an RS485-level half-duplex interface for connection to other RS485-capable devices. No RS485 termination is provided at the PCB board level (ie., pullups/pulldowns/impedance-control) -- these may be configured at the last-mile connector. The transmission and reception of the data is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the GPIO header I2C interface. The AVR pin PD4 controls the transmission-enable signal of the RS485 transceiver interface.

| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | GND          | Ground connection |
| 2           | AVR-485A     | RS485-level A input/output signal. |
| 3           | AVR-485B     | RS485-level B input/output signal. |

### 2.6 Servo [1..4] Ports ###
From the expansion AVR microcontroller PC[0..3] pins using TIMER1, these ports provide PWM signals of 1 - 2ms pulses, to connect to standard RC servos. The control of the individual PWM is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the GPIO header I2C interface. A total of +5V 2A supplies these ports through a poly-resettable (PTC) fuse. Alternatively, these ports can be used to supply +5V to other external circuitry, or embedded devices.

| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | GND          | Ground connection |
| 2           | +5V          | +5V servo power supply output |
| 3           | S[1..4]      | Servo control signal at +3.3V logic levels |


### 2.7 AVR ICSP programming port ###
This is the standard Atmel(c) AVR 6-pin programming header.

| **Description** | **Pin** | **Pin** | **Description** |
|:----------------|:--------|:--------|:----------------|
| MISO - communications to an AVR In-System Programmer | 1       | 2       | VCC - Target power |
| SCK  - communications to an AVR In-System Programmer | 3       | 4       | MOSI - communications to an AVR In-System Programmer |
| RESET - Target AVR microcontroller reset signal      | 5       | 6       | GND - Ground connection |

### 2.8 I2C Debug header ###
Originally intended for debugging, this unstuffed header may be used for I2C expansion. The +3.3V output power source pin is supplied by the Raspy Juice on-board +3.3V regulator, and a total of 450mA (for the AVR microcontroller, AVR expansion header +3.3V power output pin and this pin) is available.
| **Pin No.** | **Pin Name** | **Description** |
|:------------|:-------------|:----------------|
| 1           | +3V3 output  | +3V3 power output source. (Total of 450mA available, //see text//). |
| 2           | RPI-SDA      | Directly connected to the above RPi GPIO header (pin 3) SDA signal. |
| 3           | RPI-SCL      | Directly connected to the above RPi GPIO header (pin 5) SCL signal. |
| 4           | RPI-GPIO4    | Directly connected to the above RPi GPIO header (pin 7) GPIO4 signal. |
| 5           | GND          | Ground connection.  |

### 2.9 AVR Expansion header ###
Unused or spare pins of AVR microcontroller are routed here.
FixMe:
  * All signals here are series-terminated with 470ohm resistors.
  * PB[0..3] and PD[5..6]  are +5v over-voltage protected with diodes.
  * ADC[6..7] are +5V tolerant.
  * sub-schematics here.

| **Description** | **Pin** | **Pin** | **Description** |
|:----------------|:--------|:--------|:----------------|
| GND             | 1       | 2       | +3V3            |
| ADC6            | 3       | 4       | PB3             |
| ADC7            | 5       | 6       | PB2             |
| PD6             | 7       | 8       | PB1             |
| PD5             | 9       | 10      | PB0             |
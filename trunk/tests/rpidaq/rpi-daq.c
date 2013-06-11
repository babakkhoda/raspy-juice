#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c-userspace.h"
#include <linux/spi/spidev.h>

int rpi_rev;

int fd_i2cbus = 0;
int fd_dac1;
int fd_dac2;
int fd_adc;
int i2caddr_dac1 = 0x60;
int i2caddr_adc = 0x69;
int i2cbus_open(const char *devbusname);
int i2cdev_open(int fd, int i2caddr);

uint8_t spi_outbuf[16];
uint8_t spi_inbuf[16];
int fd_spi0;
int fd_spi1;
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
int spi_open(const char *devbusname);
void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx);

#define DAQ_SPISUB_CFG 1
#define DAQ_SPISUB_LCD 2

static int cfg_data;
static int lcd_data;

#define CFG_MASK_RELAY 0x000f
#define CFG_MASK_LEDS  0x00f0
#define CFG_MASK_I2C_B 0x0100
#define CFG_MASK_SPI_B 0x0200
#define CFG_MASK_AVRST 0x0400

int  lcd_init(int rows, int cols, int bits);
void lcd_home(void);
void lcd_clear(void);
void lcdSendCommand(uint8_t command);
void lcd_pos(int row, int col);
void lcd_putchar(uint8_t data);
void lcd_puts(char *string);
void lcd_printf(char *message, ...);

int i2cbus_open(const char *devbusname)
{
    int rval;
    unsigned char i2c_buffer[16];
    
    /* test bus */
    fd_i2cbus = open(devbusname, O_RDWR);
    if (fd_i2cbus < 0)
	return -1;

    /* setup ADC device as slave*/
    fd_adc = ioctl(fd_i2cbus, I2C_SLAVE, i2caddr_adc);
    if (fd_adc < 0)
       return -2;
    
    /* read ADC device */
    rval = read(fd_i2cbus, i2c_buffer, 4);
    if (rval < 0)
	return -3;
    
    return 0;
}
    
int spi_open(const char *devbusname)
{
    int ret, fd;
    
    fd = open(devbusname, O_RDWR);
    if (fd < 0) {
	printf("spi_open: can't open device\n");
	return -1;
    }
    
    /* spi mode */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
	printf("can't set spi mode\n");
	return -2;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
	printf("spi_open: can't get spi mode\n");
	return -3;
    }
    
    /* bits per word */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
	printf("spi_open: can't set bits per word\n");
	return -4;
    }
    
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
	printf("spi_open: can't get bits per word\n");
	return -5;
    }
    
    /* max speed hz */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
	printf("spi_open: can't set max speed hz\n");
	return -6;
    }
    
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
	printf("spi_open: can't get max speed hz");
	return -7;
    }
    
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    
    return fd;
}

void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx)
{
    int ret;
    
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = len,
	.delay_usecs = delay,
	.speed_hz = speed,
	.bits_per_word = bits,
    };
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
	printf("spi_xfer: can't send spi message");
#if 0
    for (ret = 0; ret < len; ret++) {
	if (!(ret % 6))
	    puts("");
	printf("%.2X ", rx[ret]);
	}
    puts("");
#endif
}


int daq_xfer(int subsystem, int dout)
{
    spi_outbuf[0] = subsystem >> 8;
    spi_outbuf[1] = subsystem;
    spi_xfer(fd_spi1, 2, spi_outbuf, spi_inbuf);
    
    spi_outbuf[0] = dout >> 8;
    spi_outbuf[1] = dout;
    spi_xfer(fd_spi0, 2, spi_outbuf, spi_inbuf);
    
    return (spi_inbuf[1] << 8) | (spi_inbuf[0] & 0xff);
}

int  daq_lcd_data(int data)
{
    lcd_data = lcd_data & 0xff00;
    lcd_data = lcd_data | (data & 0xff);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    return 0;
}

void daq_lcd_regsel(int addr)
{
    lcd_data = lcd_data & (~0x0100);
    lcd_data = lcd_data | (addr ? 0x0100 : 0);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
}

void daq_lcd_strobe(void)
{
    /* lcd strobe E# is inverted in CPLD implementation */
    /* assert */
    lcd_data = lcd_data & (~0x0400);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);

    /* unassert */
    lcd_data = lcd_data | (0x0400);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);
#if 0
    printf("lcd strobed\n");
#endif
}



void daq_set_relay(int relay, int onoff)
{
    int mask;
    if ((relay < 0) || (relay > 3))
	return;
    mask = 1 << relay;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);    
}

void daq_set_led(int led, int onoff)
{
    int mask;
    if ((led < 0) || (led > 3))
	return;
    mask = 1 << (led + 4);
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);    
}

void daq_set_buffered_i2c(int onoff)
{
    int mask;
    mask = CFG_MASK_I2C_B;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);
}

void daq_set_buffered_spi(int onoff)
{
    int mask;
    mask = CFG_MASK_SPI_B;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);
}

void daq_set_avr_reset(int onoff)
{
    int mask;
    mask = CFG_MASK_AVRST;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);
}




int main(int argc, char *argv[])
{
    int err, numbytes, i, desired, fd1;
    unsigned char i2c_buffer[16];
    
    printf("Hello, world!\n\n");
    
    rpi_rev = 1;
    err = i2cbus_open("/dev/i2c-0");

    if (err < 0) {
	rpi_rev = 2;
	err = i2cbus_open("/dev/i2c-1");
    }
    
    if (err < 0) {
	printf("i2cbus_open: /dev/i2c-x unsuccessful, rpidaq not found.\n");
	exit(1);
    }
    
    printf("i2cbus_open: successful, rpi_rev = %d\n", rpi_rev);

    fd_spi0 = spi_open("/dev/spidev0.0");
    if (fd_spi0 < 0) {
	printf("spi_open: /dev/spidev0.0 unsuccessful.\n");
	exit(1);
    }

    fd_spi1 = spi_open("/dev/spidev0.1");
    if (fd_spi1 < 0) {
	printf("spi_open: /dev/spidev0.1 unsuccessful.\n");
	exit(1);
    }
    
    cfg_data = 0;
    lcd_data = 0;
    
    /* UI-header and LCD init */
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    daq_set_avr_reset(0);
    daq_set_buffered_spi(0);
    daq_set_buffered_i2c(0);
    
    for (i = 0; i < 4; i++) {
	daq_set_led(i, 0);
	daq_set_relay(i, 0);
    }
    
    if (argc < 2)
	exit(0);
    
    desired = atoi(argv[1]);

    /* switch on relay 0 */
    daq_set_relay(0, 1);
    sleep(2);
    daq_set_buffered_i2c(1);

#if 1
    lcd_init(2, 20, 4);
    printf("Got out of lcdInit()\n");
    sleep(1);
    lcd_pos(0, 0);
    lcd_puts("Dave Appleton") ;
    lcd_pos(1, 0);
    lcd_puts("-------------") ;
    printf("Got out of lcd_main()\n");
#endif

    if (desired == 1) {
	printf("state = 1, setting and leaving buffered SPI-AVR interface on\n");
	daq_set_buffered_spi(1);
	daq_set_avr_reset(1);
	daq_xfer(3, cfg_data);
	exit(0);
    }
    

    /* turn off everything */
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    for (i = 0; i < 4; i++) {
	daq_set_led(i, 0);
	daq_set_relay(i, 0);
    }
    daq_set_avr_reset(0);
    daq_set_buffered_spi(0);
    daq_set_buffered_i2c(0);
    
    return 0;
}


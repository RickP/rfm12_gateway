/*
 * spidev.c
 *
 *  Created on: 17.05.2012
 *      Author: rick
 */

#include "spi.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


static const char *device = "/dev/spidev1.0";
static uint8_t mode = 0;
static uint8_t bits = 16;
static uint32_t speed = 100000000;
static uint16_t delay = 0;
static int fd;

static void pabort(const char *s)
{
	perror(s);
	abort();
}


void spiinit() {
	fd = open(device, O_RDWR);

	if (fd < 0)
		pabort("can't open device");

	int ret = 0;

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	// close(fd);

}

uint16_t spitransfer(uint16_t in)
{
	// fd = open(device, O_RDWR);

	if (fd < 0)
		pabort("can't open device");

	uint16_t rx = 0;

	struct spi_ioc_transfer tr = {
		.tx_buf = (__u64) &in,
		.rx_buf = (__u64) &rx,
		.len = 2,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret == 1)
		pabort("can't send spi message");

	// close(fd);

	return rx;
}

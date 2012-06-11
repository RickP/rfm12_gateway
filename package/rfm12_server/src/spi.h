/*
 * spidev.h
 *
 *  Created on: 17.05.2012
 *      Author: rick
 */

#include <stdint.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>


#ifndef SPI_H_
#define SPI_H_

void spiinit();
uint16_t spitransfer(uint16_t input);

#endif /* SPI_H_ */

/*
 * gpio_int.c
 *
 *  Created on: 07.06.2012
 *      Author: rick
 */

#include "gpio_int.h"
#include <linux/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/gpio_dev.h>

 /****************************************************************
 * Static vars
 ****************************************************************/

static int gpio_fd;
static unsigned int current_gpio = 0;


/****************************************************************
 * public functions
 ****************************************************************/

void gpio_init(int gpio)
{
	current_gpio = gpio;

	if ((gpio_fd = open(SYSFS_GPIO_DIR, O_RDWR)) < 0)
	{
		printf("Error whilst opening /dev/gpio\n");
	}

	ioctl(gpio_fd, GPIO_DIR_IN, current_gpio);
}

int gpio_poll() {
	int ret = ioctl(gpio_fd, GPIO_GET, current_gpio);
	return !ret;
}

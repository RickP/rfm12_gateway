/*
 * gpio_int.c
 *
 *  Created on: 07.06.2012
 *      Author: rick
 */
#ifndef GPIO_INT_H_
#define GPIO_INT_H_



 /****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/dev/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

 /****************************************************************
 * Functions
 ****************************************************************/

void gpio_init(int gpio);
int gpio_poll();

#endif /* GPIO_INT_H_ */

/*
 * rfm12.h
 *
 *  Created on: 22.05.2012
 *      Author: rick
 */

#ifndef RF12M_H_
#define RF12M_H_

#include "spi.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#define rf12_grp        rf12_buf[0]
#define rf12_hdr        rf12_buf[1]
#define rf12_len        rf12_buf[2]
#define rf12_data       (rf12_buf + 3)

#define RF12_HDR_CTL    0x80
#define RF12_HDR_DST    0x40
#define RF12_HDR_ACK    0x20
#define RF12_HDR_MASK   0x1F

#define RF12_MAXDATA    66

#define RF12_433MHZ     1
#define RF12_868MHZ     2
#define RF12_915MHZ     3

// shorthand to simplify sending out the proper ACK when requested
#define RF12_WANTS_ACK ((rf12_hdr & RF12_HDR_ACK) && !(rf12_hdr & RF12_HDR_CTL))
#define RF12_ACK_REPLY (rf12_hdr & RF12_HDR_DST ? RF12_HDR_CTL : \
            RF12_HDR_CTL | RF12_HDR_DST | (rf12_hdr & RF12_HDR_MASK))

extern volatile uint16_t rf12_crc;  // running crc value, should be zero at end
extern volatile uint8_t rf12_buf[]; // recv/xmit buf including hdr & crc bytes
extern long rf12_seq;               // seq number of encrypted packet (or -1)


// call this once with the node ID, frequency band, and group
uint8_t rf12_initialize(uint8_t id, uint8_t band, uint8_t group);

// talk to the module
void rf12_communicate();

void rf12_recvStart ();

uint8_t rf12_recvDone(void);

// call this frequently, returns true if a packet has been received
uint8_t rf12_recvDone(void);

// call this to check whether a new transmission can be started
// returns true when a new transmission may be started with rf12_sendStart()
uint8_t rf12_canSend();

// call this only when rf12_recvDone() or rf12_
void rf12_sendStart_simple(uint8_t hdr);
void rf12_sendStart(uint8_t hdr, const void* ptr, uint8_t len);

// wait for send to finish, sleep mode: 0=none, 1=idle, 2=standby, 3=powerdown
void rf12_sendWait(uint8_t mode);

// this simulates OOK by turning the transmitter on and off via SPI commands
// use this only when the radio was initialized with a fake zero node ID
void rf12_onOff(uint8_t value);

// power off the RF12, ms > 0 sets watchdog to wake up again after N * 32 ms
// note: once off, calling this with -1 can be used to bring the RF12 back up
void rf12_sleep(char n);

// returns true of the supply voltage is below 3.1V
char rf12_lowbat(void);

// set up the easy tranmission mode, arg is number of seconds between packets
void rf12_easyInit(uint8_t secs);

// call this often to keep the easy transmission mode going
char rf12_easyPoll(void);

// send new data using the easy transmission mode, buffer gets copied to driver
char rf12_easySend(const void* data, uint8_t size);

// enable encryption (null arg disables it again)
void rf12_encrypt(uint32_t key[4]);

// low-level control of the RFM12B via direct register access
// http://tools.jeelabs.org/rfm12b is useful for calculating these
uint16_t rf12_control(uint16_t cmd);

// See http://blog.strobotics.com.au/2009/07/27/rfm12-tutorial-part-3a/
// Transmissions are packetized, don't assume you can sustain these speeds!
//
// Note - data rates are approximate. For higher data rates you may need to
// alter receiver radio bandwidth and transmitter modulator bandwidth.
// Note that bit 7 is a prescaler - don't just interpolate rates between
// RF12_DATA_RATE_3 and RF12_DATA_RATE_2.
enum rf12DataRates {
    RF12_DATA_RATE_CMD = 0xC600,
    RF12_DATA_RATE_9 = RF12_DATA_RATE_CMD | 0x02,  // Approx 115200 bps
    RF12_DATA_RATE_8 = RF12_DATA_RATE_CMD | 0x05,  // Approx  57600 bps
    RF12_DATA_RATE_7 = RF12_DATA_RATE_CMD | 0x06,  // Approx  49200 bps
    RF12_DATA_RATE_6 = RF12_DATA_RATE_CMD | 0x08,  // Approx  38400 bps
    RF12_DATA_RATE_5 = RF12_DATA_RATE_CMD | 0x11,  // Approx  19200 bps
    RF12_DATA_RATE_4 = RF12_DATA_RATE_CMD | 0x23,  // Approx   9600 bps
    RF12_DATA_RATE_3 = RF12_DATA_RATE_CMD | 0x47,  // Approx   4800 bps
    RF12_DATA_RATE_2 = RF12_DATA_RATE_CMD | 0x91,  // Approx   2400 bps
    RF12_DATA_RATE_1 = RF12_DATA_RATE_CMD | 0x9E,  // Approx   1200 bps
    RF12_DATA_RATE_DEFAULT = RF12_DATA_RATE_7,
};


#endif /* RF12M_H_ */

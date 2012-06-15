/*
 *
 */

#include "rfm12.h"


#define RF_MAX   (RF12_MAXDATA + 5)

// RF12 command codes
#define RF_RECEIVER_ON  0x82DD
#define RF_XMITTER_ON   0x823D
#define RF_IDLE_MODE    0x820D
#define RF_SLEEP_MODE   0x8205
#define RF_WAKEUP_MODE  0x8207
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

// RF12 status bits
#define RF_LBD_BIT      0x0400
#define RF_RSSI_BIT     0x0100

// bits in the node id configuration byte
#define NODE_BAND       0xC0        // frequency band
#define NODE_ACKANY     0x20        // ack on broadcast packets if set
#define NODE_ID         0x1F        // id of this node, as A..Z or 1..31

// transceiver states, these determine what to do with each interrupt
enum {
    TXCRC1, TXCRC2, TXTAIL, TXDONE, TXIDLE,
    TXRECV,
    TXPRE1, TXPRE2, TXPRE3, TXSYN1, TXSYN2,
};

static uint8_t nodeid;              // address of this node
static uint8_t group;               // network group
static volatile uint8_t rxfill;     // number of data bytes in rf12_buf
static volatile int8_t rxstate;     // current transceiver state

#define RETRIES     8               // stop retrying after 8 times
#define RETRY_MS    1000            // resend packet every second until ack'ed

volatile uint16_t rf12_crc;         // running crc value
volatile uint8_t rf12_buf[RF_MAX];  // recv/xmit buf, including hdr & crc bytes

pthread_mutex_t mutex;

static uint16_t _crc16_update(uint16_t crc, uint8_t a)
{
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}

	return crc;
}


/*!
  Call this once with the node ID (0-31), frequency band (0-3), and
  optional group (0-255 for RF12B, only 212 allowed for RF12).
*/
uint8_t rf12_initialize (uint8_t id, uint8_t band, uint8_t g) {
	pthread_mutex_lock (&mutex);
	spiinit();

    nodeid = id;
    group = g;
    spitransfer(0x0000);
    spitransfer(RF_SLEEP_MODE);

    // wait until RFM12B is out of power-up reset, this takes several *seconds*
    spitransfer(RF_TXREG_WRITE); // in case we're still in OOK mode

    sleep(3);

    spitransfer(0x80C7 | (band << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
    spitransfer(0xA640); // 868MHz
    spitransfer(0xC691); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
    spitransfer(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
    spitransfer(0xC2AC); // AL,!ml,DIG,DQD4

    if (group != 0) {
        spitransfer(0xCA83); // FIFO8,2-SYNC,!ff,DR
        spitransfer(0xCE00 | group); // SYNC=2DXX；
    } else {
        spitransfer(0xCA8B); // FIFO8,1-SYNC,!ff,DR
        spitransfer(0xCE2D); // SYNC=2D；
    }
    spitransfer(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
    spitransfer(0x9850); // !mp,90kHz,MAX OUT
    spitransfer(0xCC77); // OB1，OB0, LPX,！ddy，DDIT，BW0
    spitransfer(0xE000); // NOT USE
    spitransfer(0xC800); // NOT USE
    spitransfer(0xC049); // 1.66MHz,3.1V

    rxstate = TXIDLE;
    pthread_mutex_unlock (&mutex);


    return nodeid;
}

void rf12_recvStart () {
	pthread_mutex_lock (&mutex);
    rxfill = rf12_len = 0;
    rf12_crc = ~0;
    if (group != 0)
        rf12_crc = _crc16_update(~0, group);
    rxstate = TXRECV;
    spitransfer(RF_RECEIVER_ON);
    pthread_mutex_unlock (&mutex);
}

uint8_t rf12_canSend() {
    // no need to test with interrupts disabled: state TXRECV is only reached
    // outside of ISR and we don't care if rxfill jumps from 0 to 1 here
    if (rxstate == TXRECV && rxfill == 0) {
        spitransfer(RF_IDLE_MODE); // stop receiver
        rxstate = TXIDLE;
        return 1;
    }
    return 0;
}

void rf12_communicate() {
	pthread_mutex_lock (&mutex);
	spitransfer(0x0000);

    if (rxstate == TXRECV) {

		uint8_t in = spitransfer(RF_RX_FIFO_READ);

		if (rxfill == 0 && group != 0)
			rf12_buf[rxfill++] = group;

		rf12_buf[rxfill++] = in;
		rf12_crc = _crc16_update(rf12_crc, in);

		if (rxfill >= rf12_len + 5 || rxfill >= RF_MAX)
			spitransfer(RF_IDLE_MODE);


    } else {
        uint8_t out;

        if (rxstate < 0) {
            uint8_t pos = 3 + rf12_len + rxstate++;
            out = rf12_buf[pos];
            rf12_crc = _crc16_update(rf12_crc, out);
        } else
            switch (rxstate++) {
                case TXSYN1: out = 0x2D; break;
                case TXSYN2: out = group; rxstate = - (2 + rf12_len); break;
                case TXCRC1: out = rf12_crc; break;
                case TXCRC2: out = rf12_crc >> 8; break;
                case TXDONE: spitransfer(RF_IDLE_MODE);
                /* no break */
                default:     out = 0xAA;
                /* no break */
            }

        spitransfer(RF_TXREG_WRITE + out);
    }
    pthread_mutex_unlock (&mutex);
}

void rf12_sendStart_simple (uint8_t hdr) {
	pthread_mutex_lock (&mutex);
	rf12_hdr = hdr & RF12_HDR_DST ? hdr : (hdr & ~RF12_HDR_MASK) + (nodeid & NODE_ID);

	rf12_crc = ~0;
	rf12_crc = _crc16_update(rf12_crc, group);
	rxstate = TXPRE1;
	spitransfer(RF_XMITTER_ON); // bytes will be fed via interrupts
    pthread_mutex_unlock (&mutex);
}

void rf12_sendStart (uint8_t hdr, const void* ptr, uint8_t len) {
	pthread_mutex_lock (&mutex);
    rf12_len = len;
    memcpy ((void*) rf12_data, ptr, len);

    rf12_hdr = hdr & RF12_HDR_DST ? hdr :
                    (hdr & ~RF12_HDR_MASK) + (nodeid & NODE_ID);

	rf12_crc = ~0;
	rf12_crc = _crc16_update(rf12_crc, group);
	rxstate = TXPRE1;
	spitransfer(RF_XMITTER_ON); // bytes will be fed via interrupts

    pthread_mutex_unlock (&mutex);
}

uint8_t rf12_recvDone () {

    if (rxstate == TXRECV && (rxfill >= rf12_len + 5 || rxfill >= RF_MAX)) {
    	pthread_mutex_lock (&mutex);
        rxstate = TXIDLE;
        if (rf12_len > RF12_MAXDATA)
            rf12_crc = 1; // force bad crc if packet length is invalid
        if (!(rf12_hdr & RF12_HDR_DST) || (nodeid & NODE_ID) == 31 ||
                (rf12_hdr & RF12_HDR_MASK) == (nodeid & NODE_ID)) {

            pthread_mutex_unlock (&mutex);
            return 1; // it's a broadcast packet or it's addressed to this node
        }
        pthread_mutex_unlock (&mutex);
    }
    if (rxstate == TXIDLE) {
        rf12_recvStart();
    }

    return 0;
}




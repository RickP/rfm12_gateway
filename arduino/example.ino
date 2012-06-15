#include <EEPROM.h>

// Send out a radio packet every minute, consuming as little power as possible
// 2010-08-29 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <JeeLib.h> 

MilliTimer receiveTimer;

// this must be added since we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

// TONE Frequency
#define TONE  440

// SETUP ============================================
// Set up speaker on a PWM pin (digital 9, 10 or 11)
int speakerOut = 8;
int numBeeps = 3;
long seq = 0;

int failCount = 0;
boolean failed = false;
RF12_tx_power_t currentTransmitPower = RF12_TX_21DB;

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration  = 0;

struct {
    char  command[5];
    char parameter[RF12_MAXDATA-5];
} payload;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
    cli();
    CLKPR = bit(CLKPCE);
    CLKPR = 0; // div 1, i.e. speed up to 8 MHz
    sei();
    rf12_initialize(2, RF12_868MHZ, 1);
    rf12_config(0xC040); // set low-battery level to 2.2V i.s.o. 3.1V
    rf12_txPower(currentTransmitPower);
}

void loop() {
    
    while (!rf12_canSend())
        rf12_recvDone();
    
    strcpy(payload.command, "cosm");
    
    String startString = "zwo,";
    startString.concat(String(seq++));
    startString.toCharArray(payload.parameter, sizeof payload.parameter);
    
    rf12_sendStart(1, &payload, sizeof payload);
    
    // set the sync mode to 2 if the fuses are still the Arduino default
    // mode 3 (full powerdown) can only be used with 258 CK startup fuses
    rf12_sendWait(2);

    Sleepy::loseSomeTime(20000);
    
    rf12_sleep(RF12_WAKEUP);
}

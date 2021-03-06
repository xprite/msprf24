#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

volatile unsigned int user;

void main()
{
	char addr[4];
	char buf[32];

	WDTCTL = WDTHOLD | WDTPW;
	DCOCTL = CALDCO_1MHZ;
	BCSCTL1 = CALBC1_1MHZ;
	BCSCTL2 = DIVS_0;  // SMCLK = DCOCLK/1
	// SPI (USI) uses SMCLK, prefer SMCLK=DCO (no clock division)
	user = 0xFE;

	/* Initial values for nRF24L01+ library config variables */
	rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
	rf_addr_width      = 4;
	rf_speed_power     = RF24_SPEED_MIN | RF24_POWER_MIN;
	rf_channel         = 119;
	msprf24_init();
	msprf24_set_pipe_packetsize(0, 15);
	msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst for receiving Auto-ACKs

	// Transmit to 0xBEEFBEEF
	msprf24_standby();
	user = msprf24_current_state();
	addr[0] = 0xBE; addr[1] = 0xEF; addr[2] = 0xBE; addr[3] = 0xEF;
	w_tx_addr(addr);
	w_rx_addr(0, addr);  // Pipe#0 receives auto-ack's
	buf[0] = '0';
	buf[1] = '\0';
	w_tx_payload(15, buf);
	msprf24_activate_tx();
	LPM4;

	if (rf_irq & RF24_IRQ_FLAGGED) {
		user = ~(msprf24_get_irq_reason());
	}
}

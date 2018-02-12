#include <xc.h>
#include "app.h"
#include "osc.h"

void ConfigureOscillator (void)
{
   // Configure Internal Oscillator
	OSCCON = (unsigned char)OSCCON | 0x70;				// Internal 16 MHZ Source on 18F26K22
	while (!OSCCONbits.IOFS);

	// Enable PLL & wait for stable
	OSCTUNEbits.PLLEN = 1;
	while (!OSCCON2bits.PLLRDY);
}

//  Texas Instruments, Inc
//  January 2006
//  Built with IAR Embedded Workbench Version: 3.40A
//******************************************************************************

#include "msp430.h"

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  P1DIR |= 0x01;                        // Set P1.0 to output direction

  for (;;)
  {
    volatile unsigned int i;            // volatile to prevent optimization

    P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR

    i = 10000;                          // SW Delay
    do i--;
    while (i != 0);
  }
}

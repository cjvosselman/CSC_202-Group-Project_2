//-----------------------------------------------------------------------------
// Loads standard C include files
//-----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Loads MSP launchpad board support macros and definitions
//-----------------------------------------------------------------------------
#include "LaunchPad.h"
#include "clock.h"
#include "timer.h"
#include <ti/devices/msp/msp.h>
#include <ti/devices/msp/peripherals/hw_iomux.h>

volatile uint8_t ones_seconds = 0;
volatile uint8_t tens_seconds = 0;

void timerA_config(uint32_t load_value, uint32_t compare_value) {

  IOMUX->SECCFG.PINCM[IOMUX_PINCM57] =
      IOMUX_PINCM57_PF_TIMA0_CCP3 | IOMUX_PINCM_PC_CONNECTED;

  // Reset TIMA0
  TIMA0->GPRCM.RSTCTL =
      (GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETSTKYCLR_CLR |
       GPTIMER_RSTCTL_RESETASSERT_ASSERT);

  // Enable power to TIMA0
  TIMA0->GPRCM.PWREN =
      (GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE);

  // Wait for 24 bus cycles
  clock_delay(24);

  // Selects LFCLK as clock source
  TIMA0->CLKSEL =
      (GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE | GPTIMER_CLKSEL_MFCLK_SEL_DISABLE |
       GPTIMER_CLKSEL_LFCLK_SEL_DISABLE);

  // Configures Timer Clock
  TIMA0->CLKDIV = GPTIMER_CLKDIV_RATIO_DIV_BY_8;

  TIMA0->COMMONREGS.CPS = GPTIMER_CPS_PCNT_MASK & TIMER_CPS_PCNT;

  TIMA0->COUNTERREGS.LOAD = GPTIMER_LOAD_LD_MASK & (load_value - 1);

  TIMA0->COMMONREGS.CCLKCTL = GPTIMER_CCLKCTL_CLKEN_ENABLED;

  // Sets TIMA0_C3 as output
  TIMA0->COMMONREGS.CCPD =
      (GPTIMER_CCPD_C0CCP3_OUTPUT | GPTIMER_CCPD_C0CCP2_INPUT |
       GPTIMER_CCPD_C0CCP1_INPUT | GPTIMER_CCPD_C0CCP0_INPUT);

  TIMA0->CPU_INT.IMASK = 0x00000000;

  // When enabled, timer starts at 0, counts up, and then stops

  TIMA0->COUNTERREGS.CTRCTL =
      GPTIMER_CTRCTL_CVAE_ZEROVAL | GPTIMER_CTRCTL_CZC_CCCTL2_ZCOND |
      GPTIMER_CTRCTL_REPEAT_REPEAT_1 | GPTIMER_CTRCTL_CM_UP;

  // Set timer reload value
  TIMA0->COUNTERREGS.LOAD = GPTIMER_LOAD_LD_MASK & (load_value - 1);

  // Set timer compare value
  TIMA0->COUNTERREGS.CC_23[1] = GPTIMER_CC_23_CCVAL_MASK & compare_value;

  // Set compare control for PWM function with output initially low
  TIMA0->COUNTERREGS.OCTL_23[1] =
      (GPTIMER_OCTL_23_CCPIV_LOW | GPTIMER_OCTL_23_CCPOINV_NOINV |
       GPTIMER_OCTL_23_CCPO_ZERO);

  TIMA0->COUNTERREGS.CCCTL_23[1] =
      GPTIMER_CCCTL_23_COC_COMPARE | GPTIMER_CCCTL_23_CCUPD_IMMEDIATELY;
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//    This function enables the timer to start counting.
//
// INPUT PARAMETERS:
//    none
//
// OUTPUT PARAMETERS:
//    none
//
// RETURN:
//    none
// -----------------------------------------------------------------------------
void timerA_enable(void) {
  TIMA0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);
} /* timerA_enable */
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DESCRIPTION:
//    This function disable the timer to start counting.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
// -----------------------------------------------------------------------------
void timerA_disable(void) {
  TIMA0->COUNTERREGS.CTRCTL &= ~(GPTIMER_CTRCTL_EN_ENABLED);
} /* timerA_disable */

//-----------------------------------------------------------------------------
// DESCRIPTION:
//    This function enables interrupts for Timer A0 by first clearing any
//    pending interrupts and then unmasking specific interrupt conditions.
//    It also sets the priority for the Timer A0 interrupt and enables the
//    interrupt in the NVIC (Nested Vectored Interrupt Controller).
//
//    NOTE: ADJUST INTERRUPTS AS NEEDED
//
// INPUT PARAMETERS:
//     None
//
// OUTPUT PARAMETERS:
//     None
//
// RETURN:
//     None
//-----------------------------------------------------------------------------
void timerA_enable_interrupt(void) {
  // Clear all pre-existing interrupts that might be set
  TIMA0->CPU_INT.ICLR =
      GPTIMER_CPU_INT_ICLR_DC_CLR | GPTIMER_CPU_INT_ICLR_REPC_CLR |
      GPTIMER_CPU_INT_ICLR_TOV_CLR | GPTIMER_CPU_INT_ICLR_F_CLR |
      GPTIMER_CPU_INT_ICLR_CCD0_CLR | GPTIMER_CPU_INT_ICLR_CCD1_CLR |
      GPTIMER_CPU_INT_ICLR_CCD2_CLR | GPTIMER_CPU_INT_ICLR_CCD3_CLR |
      GPTIMER_CPU_INT_ICLR_CCD4_CLR | GPTIMER_CPU_INT_ICLR_CCD5_CLR |
      GPTIMER_CPU_INT_ICLR_CCU0_CLR | GPTIMER_CPU_INT_ICLR_CCU1_CLR |
      GPTIMER_CPU_INT_ICLR_CCU2_CLR | GPTIMER_CPU_INT_ICLR_CCU3_CLR |
      GPTIMER_CPU_INT_ICLR_CCU4_CLR | GPTIMER_CPU_INT_ICLR_CCU5_CLR |
      GPTIMER_CPU_INT_ICLR_Z_CLR | GPTIMER_CPU_INT_ICLR_L_CLR;

  // Unmask conditions to allow interrupt
  TIMA0->CPU_INT.IMASK = GPTIMER_CPU_INT_IMASK_L_SET;

  // Set priority and enable
  NVIC_SetPriority(TIMA0_INT_IRQn, 2);
  NVIC_EnableIRQ(TIMA0_INT_IRQn);
} /* timerA_enable_interrupt */

// DESCRIPTION:
//    This is the interrupt handler for Timer A0 (TIMA0). It checks the
//    interrupt index register (IIDX) to identify the type of interrupt event
//    that occurred. Based on the interrupt event, the function processes the
//    event accordingly:
//     - Zero event (Z): Counts the number of time the ISR is called. After
//                       the appropriate delay, the count value is decremented
//                       and sent to the seven-segment display.
//
//    The function continues checking for interrupts until all events are
//    cleared.
//
//    NOTE: ADJUST PROCESSING AS NEEDED
//
// INPUT PARAMETERS:
//     None
//
// OUTPUT PARAMETERS:
//     None
//
// RETURN:
//     None
//-----------------------------------------------------------------------------
void TIMA0_IRQHandler(void) {
  uint32_t timer_iidx;
  static uint16_t isr_call_count = MAX_ISR_COUNT_DELAY;

  

  do {
    timer_iidx = TIMA0->CPU_INT.IIDX;
    switch (timer_iidx) {
    // Check if overflow event
    case (GPTIMER_CPU_INT_IIDX_STAT_TOV):
      break;

    // Check if load event
    case (GPTIMER_CPU_INT_IIDX_STAT_L):
      // isr_call_count++;
      // if (isr_call_count >= 1000)
      // {
      //   isr_call_count = 0;

      // }
      ones_seconds++;
      if (ones_seconds > 9)
      {
        tens_seconds++;
        ones_seconds = 0;
      }
      if (tens_seconds > 9)
      {
        tens_seconds = 0;
      }
      
      

      break;

    // Check if CC event on CCP2 (ECHO)
    case (GPTIMER_CPU_INT_IIDX_STAT_CCU2):
      break;

    // Check if zero event
    case (GPTIMER_CPU_INT_IIDX_STAT_Z):

      break;

    // Check if CC event on CCP3
    case (GPTIMER_CPU_INT_IIDX_STAT_CCU3):
      break;

    // Check if unexpected event
    default:
      break;

    } /* switch */

  } while (timer_iidx != 0);

} /* TIMA0_IRQHandler */

//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//  DESIGNER NAME:  TBD
//
//       LAB NAME:  TBD
//
//      FILE NAME:  main.c
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//    This project runs on the LP_MSPM0G3507 LaunchPad board interfacing to
//    the CSC202 Expansion board.
//
//    This code ... *** COMPLETE THIS BASED ON LAB REQUIREMENTS ***
//
//*****************************************************************************
//*****************************************************************************

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
#include <ti/devices/msp/msp.h>
#include "lcd1602.h"
#include "spi.h"
#include "uart.h"
#include "adc.h"

//-----------------------------------------------------------------------------
// Define function prototypes used by the program
//-----------------------------------------------------------------------------
void light_read_display();
void TIMA0_C0_init(void);
void TIMA0_C0_pwm_init(uint32_t load_value, uint32_t compare_value);
void TIMA0_C0_pwm_enable(void);
void TIMA0_C0_set_pwm_dc(uint8_t duty_cycle);
uint32_t ADC1_in(uint8_t channel);
void ADC1_init(uint32_t reference);
//-----------------------------------------------------------------------------
// Define symbolic constants used by the program
//-----------------------------------------------------------------------------
#define step_size                                                           450
#define light_threshold                                                     1800
#define dark_threshold                                                      1200
#define optimal_value                                                       1600
//-----------------------------------------------------------------------------
// Define global variables and structures here.
// NOTE: when possible avoid using global variables
//-----------------------------------------------------------------------------

// Define a structure to hold different data types

int main(void)
{
  // Configure the LaunchPad board
  clock_init_40mhz();
  launchpad_gpio_init();
  I2C_mstr_init(); 
  lcd1602_init();
  leds_init();
  leds_enable();
  ADC1_init(ADC12_MEMCTL_VRSEL_INTREF_VSSA);

  TIMA0_C0_init();
  TIMA0_C0_pwm_init(4000, 0);
  TIMA0_C0_pwm_enable();
  TIMG8_C0_init();
  TIMG8_C0_CNT_init(20000,0);
  lcd_clear();
  light_read_display();

  leds_off();
   
 // Endless loop to prevent program from ending
 while (1);

} /* main */

void light_read_display()
{
  bool done = false;
  uint16_t light_sensor_adc = 0;  
  uint8_t led_max = 6 ;
  uint8_t led_idx = 0;
  uint8_t led_value = 0;
  char adc_text[] = "ADC = ";
  char light_text[] = "Light = ";
  static uint8_t increment = 1;
  uint8_t duty_cycle = 0;

  while (!done)
  {
  light_sensor_adc = ADC1_in(6);
  led_value = (light_sensor_adc/step_size);
  leds_off();


  lcd_set_ddram_addr(LCD_LINE1_ADDR);
  lcd_write_string(adc_text);
  lcd_set_ddram_addr(LCD_LINE1_ADDR + LCD_CHAR_POSITION_8);
  lcd_write_doublebyte(light_sensor_adc);
  lcd_set_ddram_addr(LCD_LINE2_ADDR);
  lcd_write_string(light_text);
  lcd_set_ddram_addr(LCD_LINE2_ADDR + LCD_CHAR_POSITION_13);
  
  
  
  if (light_sensor_adc > light_threshold)
  {
    if(duty_cycle >= increment)
    {
    duty_cycle += increment;
    TIMA0_C0_set_pwm_dc(duty_cycle);
    TIMA0_C0_pwm_enable();
    msec_delay(200);
    light_sensor_adc = ADC1_in(6);
    lcd_set_ddram_addr(LCD_LINE1_ADDR + LCD_CHAR_POSITION_8);
    lcd_write_doublebyte(light_sensor_adc);
    }
  }

   else if (light_sensor_adc < dark_threshold)
  {
    if(duty_cycle <= increment)
    {
    duty_cycle -= increment;
    TIMA0_C0_set_pwm_dc(duty_cycle);
    TIMA0_C0_pwm_enable();
    msec_delay(200);
    light_sensor_adc = ADC1_in(6);
    lcd_set_ddram_addr(LCD_LINE1_ADDR + LCD_CHAR_POSITION_8);
    lcd_write_doublebyte(light_sensor_adc);
    }
  }
    else {
    TIMA0_C0_set_pwm_dc(duty_cycle);
    TIMA0_C0_pwm_enable();
    }
  }

  for (led_idx = 0; led_idx < led_value; led_idx++)
  {
  led_on(led_idx);
  }

}


void TIMA0_C0_init(void)
{
  // Set PA28 (LD0) for TIMA0_C0
  IOMUX->SECCFG.PINCM[IOMUX_PINCM19] = IOMUX_PINCM19_PF_TIMA0_CCP0 | 
                                    IOMUX_PINCM_PC_CONNECTED;

} /* TimA0_C0_init */

void TIMA0_C0_pwm_init(uint32_t load_value, uint32_t compare_value)
{
  // Reset TIMA0
  TIMA0->GPRCM.RSTCTL = (GPTIMER_RSTCTL_KEY_UNLOCK_W | 
        GPTIMER_RSTCTL_RESETSTKYCLR_CLR | GPTIMER_RSTCTL_RESETASSERT_ASSERT);

  // Enable power to TIMA0
  TIMA0->GPRCM.PWREN = (GPTIMER_PWREN_KEY_UNLOCK_W | 
        GPTIMER_PWREN_ENABLE_ENABLE);

  clock_delay(24);

  TIMA0->CLKSEL = (GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE | 
        GPTIMER_CLKSEL_MFCLK_SEL_DISABLE | GPTIMER_CLKSEL_LFCLK_SEL_DISABLE);

  TIMA0->CLKDIV = GPTIMER_CLKDIV_RATIO_DIV_BY_8;

  // Set the pre-scale count value that divides selected clock by PCNT+1
  // TimerClock = BusCock / (DIVIDER * (PRESCALER))
  // 200,000 Hz = 40,000,000 Hz / (8 * (24 + 1))
  TIMA0->COMMONREGS.CPS = GPTIMER_CPS_PCNT_MASK & 0x18;

  // Set C3 action for compare 
  // On Zero, set output HIGH; On Compares up, set output LOW
  TIMA0->COUNTERREGS.CCACT_01[0] = (GPTIMER_CCACT_01_FENACT_DISABLED  | 
        GPTIMER_CCACT_01_CC2UACT_DISABLED | GPTIMER_CCACT_01_CC2DACT_DISABLED |
        GPTIMER_CCACT_01_CUACT_CCP_LOW | GPTIMER_CCACT_01_CDACT_DISABLED | 
        GPTIMER_CCACT_01_LACT_DISABLED | GPTIMER_CCACT_01_ZACT_CCP_HIGH);

  // Set timer reload value
  TIMA0->COUNTERREGS.LOAD = GPTIMER_LOAD_LD_MASK & (load_value - 1);

  // Set timer compare value
  TIMA0->COUNTERREGS.CC_01[0] = GPTIMER_CC_01_CCVAL_MASK & compare_value;

  // Set compare control for PWM func with output initially low
  TIMA0->COUNTERREGS.OCTL_01[0] = (GPTIMER_OCTL_01_CCPIV_LOW | 
        GPTIMER_OCTL_01_CCPOINV_NOINV | GPTIMER_OCTL_01_CCPO_FUNCVAL);
  
  // Set to capture mode with writes to CC register has immediate effect 
  TIMA0->COUNTERREGS.CCCTL_01[0] = (GPTIMER_CCCTL_01_CCUPD_IMMEDIATELY |
        GPTIMER_CCCTL_01_COC_COMPARE | 
        GPTIMER_CCCTL_01_ZCOND_CC_TRIG_NO_EFFECT |
        GPTIMER_CCCTL_01_LCOND_CC_TRIG_NO_EFFECT |
        GPTIMER_CCCTL_01_ACOND_TIMCLK | GPTIMER_CCCTL_01_CCOND_NOCAPTURE);

  // When enabled counter is 0, set counter counts up
  TIMA0->COUNTERREGS.CTRCTL = (GPTIMER_CTRCTL_CVAE_ZEROVAL | 
        GPTIMER_CTRCTL_PLEN_DISABLED | GPTIMER_CTRCTL_SLZERCNEZ_DISABLED |
        GPTIMER_CTRCTL_CM_UP | GPTIMER_CTRCTL_REPEAT_REPEAT_1);

  // Enable the clock
  TIMA0->COMMONREGS.CCLKCTL = GPTIMER_CCLKCTL_CLKEN_ENABLED;

  // No interrupt is required
  TIMA0->CPU_INT.IMASK = (GPTIMER_CPU_INT_IMASK_Z_CLR | 
        GPTIMER_CPU_INT_IMASK_L_CLR | GPTIMER_CPU_INT_IMASK_CCD0_CLR |
        GPTIMER_CPU_INT_IMASK_CCD1_CLR | GPTIMER_CPU_INT_IMASK_CCU0_CLR |
        GPTIMER_CPU_INT_IMASK_CCU1_CLR | GPTIMER_CPU_INT_IMASK_F_CLR |
        GPTIMER_CPU_INT_IMASK_TOV_CLR | GPTIMER_CPU_INT_IMASK_DC_CLR | 
        GPTIMER_CPU_INT_IMASK_QEIERR_CLR | GPTIMER_CPU_INT_IMASK_CCD2_CLR |
        GPTIMER_CPU_INT_IMASK_CCD3_CLR | GPTIMER_CPU_INT_IMASK_CCU2_CLR |
        GPTIMER_CPU_INT_IMASK_CCU3_CLR | GPTIMER_CPU_INT_IMASK_CCD4_CLR |
        GPTIMER_CPU_INT_IMASK_CCD5_CLR | GPTIMER_CPU_INT_IMASK_CCU4_CLR |
        GPTIMER_CPU_INT_IMASK_CCU5_CLR | GPTIMER_CPU_INT_IMASK_REPC_CLR);

  // Set TIMA0_C0 as output
  TIMA0->COMMONREGS.CCPD =(GPTIMER_CCPD_C0CCP0_OUTPUT | 
         GPTIMER_CCPD_C0CCP2_INPUT | GPTIMER_CCPD_C0CCP1_INPUT |  
         GPTIMER_CCPD_C0CCP0_INPUT);

} /* TIMA0_C0_pwm_init */

void TIMA0_C0_pwm_enable(void)
{
    TIMA0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);
} /*TIMA0_C0_pwm_enable */

void TIMA0_C0_set_pwm_dc(uint8_t duty_cycle)
{
  uint32_t threshold = (TIMA0->COUNTERREGS.LOAD * duty_cycle) / 100;

  TIMA0->COUNTERREGS.CC_01[0] = GPTIMER_CC_01_CCVAL_MASK & threshold;
} /* TIMA0_C0_set_pwm */



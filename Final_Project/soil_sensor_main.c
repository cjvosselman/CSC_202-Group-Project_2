//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//  DESIGNER NAME:  Casey J Vosselman
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
#include "adc.h"



//-----------------------------------------------------------------------------
// Define function prototypes used by the program
//-----------------------------------------------------------------------------
void wait_for_pb_released(uint8_t PB_IDX);
void read_display_soil();
void average_adc_values();

//-----------------------------------------------------------------------------
// Define symbolic constants used by the program
//-----------------------------------------------------------------------------
#define adc12_max                                                           4096
#define soil_threshold                                                      2200 // Dry is above | Wet is below
#define adc_to_tens(adc_val) ((adc_val) * (10 / adc12_max))
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
  ADC0_init(ADC12_MEMCTL_VRSEL_VDDA_VSSA);
  
  average_adc_values();

  read_display_soil();

  
  

  // Endless loop to prevent program from ending
  while (1)
    ;

} /* main */

void read_display_soil()
{

  bool     done      = false;
  uint16_t soil_read = 0;  
  uint16_t soil_value = 0;
  uint16_t dry_soil_value = 0;
  
  lcd_clear();

  while (!done)
  {
    soil_read = ADC0_in(5);
    soil_value = soil_read;
    dry_soil_value = soil_threshold;
    
    lcd_set_ddram_addr(LCD_LINE1_ADDR);

    if (soil_value < dry_soil_value )
    {
      lcd_write_string("Status: Soil Wet");
    }  
    else if (soil_value > dry_soil_value)
    {
      lcd_write_string("Status: Soil Dry");
    }
    


    lcd_write_string("ADC:");

    lcd_set_ddram_addr(LCD_LINE2_ADDR);

    lcd_write_doublebyte(soil_value);

    if (is_pb_down(PB1_IDX))
    {
      wait_for_pb_released(PB1_IDX);
      done = true;
    }

  }
  lcd_clear();
  lcd_write_string("Program Stopped");
}

//  ----------------------------------------------------------------------------
// DESCRIPTION:
//  Accepts a pushbutton press an input. While the button is down, nothing
//  happens.
//
//  INPUT PARAMETERS:
//  uint8_t value representing a pushbutton
//
//  OUTPUT PARAMETERS:
//  none
//
//  RETURN:
//  none
//  ----------------------------------------------------------------------------
void wait_for_pb_released(uint8_t PB_IDX)
{
  while (is_pb_down(PB_IDX))
    ;
  msec_delay(10);
}


void average_adc_values()
{
ADC0->ULLMEM.MEMCTL[0] =  ADC12_MEMCTL_AVGEN_ENABLE;
ADC0->ULLMEM.CTL1 = ADC12_CTL1_AVGN_AVG_16 | ADC12_CTL1_AVGD_SHIFT4;
}

/*void enable_wincomp()
{
  ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_WINCOMP_ENABLE;
  ADC0->ULLMEM.WCHIGH = 
}

void ADC0_IRQHandler(void)
{
  uint32_t ADC0_iidx_status;
  uint32_t ADC0_mis;

  do
  {
    group_iidx_status = CPUSS->INT_GROUP[1].IIDX;

    gpio_mis = GPIOA->CPU_INT.MIS;

        if ((gpio_mis & GPIO_CPU_INT_MIS_DIO15_MASK) ==
            GPIO_CPU_INT_MIS_DIO15_SET)
        {
          g_pb2_pressed       = true;
          GPIOA->CPU_INT.ICLR = GPIO_CPU_INT_ICLR_DIO15_CLR;
        }
    }
  } while (group_iidx_status != 0);


void GROUP1_IRQHandler(void)

{
  uint32_t group_iidx_status;
  uint32_t gpio_mis;

  do
  {
    group_iidx_status = CPUSS->INT_GROUP[1].IIDX;

    switch (group_iidx_status)
    {
      case (CPUSS_INT_GROUP_IIDX_STAT_INT0):
        gpio_mis = GPIOA->CPU_INT.MIS;

        if ((gpio_mis & GPIO_CPU_INT_MIS_DIO15_MASK) ==
            GPIO_CPU_INT_MIS_DIO15_SET)
        {
          g_pb2_pressed       = true;
          GPIOA->CPU_INT.ICLR = GPIO_CPU_INT_ICLR_DIO15_CLR;
        }

        break;

      case (CPUSS_INT_GROUP_IIDX_STAT_INT1):
        gpio_mis = GPIOB->CPU_INT.MIS;

        if ((gpio_mis & GPIO_CPU_INT_MIS_DIO18_MASK) ==
            GPIO_CPU_INT_MIS_DIO18_SET)
        {
          g_pb1_pressed       = true;
          GPIOB->CPU_INT.ICLR = GPIO_CPU_INT_ICLR_DIO18_CLR;
        }
    }
  } while (group_iidx_status != 0);*/
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
#include "adc.h"
#include "clock.h"
#include "spi.h"
#include <ti/devices/msp/msp.h>
#include "lcd1602.h"

//-----------------------------------------------------------------------------
// Define function prototypes used by the program
//-----------------------------------------------------------------------------
uint8_t days_since_watered();
void update_seg7(bool increment);
void timer_testing();
void half_sec_delay(void);
//-----------------------------------------------------------------------------
// Define symbolic constants used by the program
//-----------------------------------------------------------------------------
#define tick_to_sec(tick_counter) (tick_counter / 10)
//-----------------------------------------------------------------------------
// Define global variables and structures here.
// NOTE: when possible avoid using global variables
//-----------------------------------------------------------------------------
volatile uint32_t tick_counter = 0;
/* bool increase_seg7 = false;*/
// Define a structure to hold different data types


int main(void) {
  // Configure the LaunchPad board
  clock_init_40mhz();
  launchpad_gpio_init();
  dipsw_init();
  I2C_mstr_init();
  lcd1602_init();
  seg7_init();
  sys_tick_init(409600); // set the systick to call every 10.24 seconds


  timer_testing();


  // Endless loop to prevent program from ending
  while (1)
    ;

}

void timer_testing() {

  bool done = false;
  uint8_t day_count = 0;
  update_seg7(false);
  while (!done)

 {
  update_seg7(increase_seg7);
  }
}

void update_seg7 (bool increment)
{
static uint8_t ones_dig = 0;
static uint8_t tens_dig = 0;

if (increment == true)
{
  ones_dig++;
      if (ones_dig > 9)
    {
      ones_dig = 0;
      tens_dig++;
    }
  }

seg7_hex(ones_dig, SEG7_DIG3_ENABLE_IDX);
msec_delay(5);
seg7_hex(tens_dig, SEG7_DIG2_ENABLE_IDX);
msec_delay(5);
}

/*bool days_since_watered()
{
  uint8_t sec_count = 0;
  uint8_t hour_count = 0;
  uint8_t minute_count = 0;
  uint8_t day_count = 0;
  uint32_t current_tick = 0;
  bool increment = false;

  if (tick_counter == 10)
  {
    sec_count++;
    return increment = true;
    if (seg7_counter > 9)
    {
      seg7_counter = 0;
    } 
    if (sec_count == 59) {
      minute_count++;
      sec_count = 0;
      if (minute_count == 59) {
        hour_count++;
        minute_count = 0;
        if (hour_count == 24) {
          day_count++;
          hour_count = 0;
        }
      }
    }
  } 
}*/


void SyTick_handler(void)
{
uint8_t tick_counter = 0;

tick_counter++;
lcd_write_byte(tick_counter);
if (tick_counter < 9)
{
 increase_seg7 = true;
} 
}

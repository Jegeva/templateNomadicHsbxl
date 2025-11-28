#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/exti.h>
#include "projspeeds.h"
#include "freqs.h"

// wraps around every 12h
volatile uint32_t system_millis;
void sys_tick_handler(void)
{  
  system_millis+=5;	
  if((system_millis % 10) == 0){
    curr_freq_rnk++;
    if(curr_freq_rnk>(FRQS_SIZE-1)){
      curr_freq_rnk=0;    //TIM2_CNT=1;
    }
    if(freqs[curr_freq_rnk]>10   )
    timer_set_oc_value(TIM2, TIM_OC1,(freqs[curr_freq_rnk]));
  }
}



const struct rcc_clock_scale * curr_clockscale;
void clock_setup(){
  /* this is within the spec */
  /* this is overclocking the pll beyond the chip specs */
  // curr_clockscale = & turbodontdothis;
  rcc_clock_setup_pll(rccp);
}

void systick_setup(){
  systick_set_reload( curr_clockscale->ahb_frequency / 200 ); //5ms
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();
  systick_interrupt_enable();
}

void gpio_setup(){
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_AFIO); // for extis

  // integrated led
  gpio_set_mode 	(
			 GPIOC,
			 GPIO_MODE_OUTPUT_2_MHZ,
			 GPIO_CNF_OUTPUT_PUSHPULL,
			 GPIO13);

}

int main(){  
  clock_setup();
  gpio_setup();

   while(1){
     GPIOC_ODR ^= GPIO13;

   }

}

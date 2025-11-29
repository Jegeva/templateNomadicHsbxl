#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/exti.h>
#include "projspeeds.h"
#include "freqs.h"

volatile uint32_t curr_freq_rnk=00;

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
    timer_set_oc_value(TIM1, TIM_OC1,  freqs[curr_freq_rnk]  );
    timer_set_oc_value(TIM1, TIM_OC2,  freqs[(curr_freq_rnk+300)%FRQS_SIZE]  );
    timer_set_oc_value(TIM1, TIM_OC3,  freqs[(curr_freq_rnk+600)%FRQS_SIZE]  );
    
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
			 GPIO13
			 );

  // let setup the AF for TIM1_CH1-4 cause why not
  // channels > PA8-PA11
  AFIO_MAPR|=  AFIO_MAPR_USART1_REMAP;
  AFIO_MAPR &= ~AFIO_MAPR_TIM1_REMAP_FULL_REMAP;
  gpio_set_mode 	(
			 GPIOA,
			 GPIO_MODE_OUTPUT_2_MHZ,
			 GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
			 GPIO8|GPIO9|GPIO10|GPIO11
			 );
  

}

void tim_setup(){
  rcc_periph_clock_enable(RCC_TIM1); // TIM1 is a bit more annoying an capable so let's use it
  //nvic_enable_irq(NVIC_TIM2_IRQ); // cause we need to reset some flags at overflow and so we can manage the changes in output compare
                                  // when it overflows and not in the middle of counting like little piggies
  rcc_periph_reset_pulse(RST_TIM1);
  // counts from 0 to ARR then overflow and restarts at 0 
  timer_set_mode(        TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_oc_mode(     TIM1, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(     TIM1, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(     TIM1, TIM_OC3, TIM_OCM_PWM1);

 // only for TIM1
  timer_enable_break_main_output(TIM1);
  
  timer_enable_oc_output(TIM1, TIM_OC1);
  timer_enable_oc_output(TIM1, TIM_OC2);
  timer_enable_oc_output(TIM1, TIM_OC3);
  timer_enable_update_event(TIM1);
 
  


  timer_set_oc_value(TIM1, TIM_OC1,  0xffff>> 1   ); // 75% we don't have floating point math and non 2^x division is the ennemy :)
  timer_set_oc_value(TIM1, TIM_OC2,  0xffff>> 1   ); // 50%
  timer_set_oc_value(TIM1, TIM_OC3,  0xffff>> 1   ); // 50%

  timer_set_period(TIM1, 0xffff); // use the full 16 bits
    
  timer_continuous_mode(TIM1);
  
  TIM1_CNT = TIM2_CNT = TIM3_CNT = TIM4_CNT = 0;
  //  TIM1_PSC = 100;
  //TIM1_EGR |= TIM_EGR_UG;
  timer_enable_counter(TIM1);
}


int main(){
  clock_setup();
  gpio_setup();
  GPIOC_ODR=GPIO13;
  systick_setup();
  tim_setup();

  while(1){
    for(unsigned int i = 1000000;i;i--)
      asm("NOP");
    GPIOC_ODR ^= GPIO13;

  }

}

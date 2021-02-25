#include <stdio.h>
#include "pico/stdlib.h"
// For ADC input:
#include "hardware/adc.h"
#include "hardware/dma.h"
// For resistor DAC output:
#include "pico/multicore.h"
#include "hardware/irq.h"


#define CAPTURE_CHANNEL 0
#define CAPTURE_DEPTH 132300
#define FLAG_VALUE1 123
#define FLAG_VALUE2 321

uint8_t capture_buf[CAPTURE_DEPTH];
static int core0_rx_val = 0, core1_rx_val = 0;
static uint16_t adc_value = 0;
static uint16_t adc_value_core1 = 0;

void core1_main();

bool read_adc_timer(struct repeating_timer *t) {
    adc_value = adc_read();
    if (multicore_fifo_wready){
        multicore_fifo_push_blocking(adc_value);
    }
    return true;
}

int main() {
    stdio_init_all();
    multicore_launch_core1(core1_main);

    adc_gpio_init(26 + CAPTURE_CHANNEL);
    adc_init();
    adc_select_input(CAPTURE_CHANNEL);
    adc_set_clkdiv(0);

    struct repeating_timer timer;
    
    add_repeating_timer_ms(0.01, read_adc_timer, NULL, &timer);

    while (true) {
     //   printf("Hello from core 0 \n");
        
      //  sleep_ms(1000);
    }
}

void core1_main() {
    
    while (true) {
       if (multicore_fifo_rvalid){
            adc_value_core1 = multicore_fifo_pop_blocking();
            printf("%-3d\n", adc_value_core1);
       }

        

      //  sleep_ms(1000);
    }
}
#include <stdio.h>
#include "pico/stdlib.h"
// For ADC input:
#include "hardware/adc.h"
#include "hardware/dma.h"
// For resistor DAC output:
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"  // pwm 
#include "hardware/sync.h" // wait for interrupt 


#define CAPTURE_CHANNEL 0
#define AUDIO_PIN 0

// #define CAPTURE_DEPTH 132300

// uint8_t capture_buf[CAPTURE_DEPTH];
const uint LED_PIN = 16;

void core1_main();



bool read_adc_timer(struct repeating_timer *t) {
    uint16_t result = adc_read();
    if (multicore_fifo_wready){
        multicore_fifo_push_blocking(result);
    }
    return true;
}

int main() {
    stdio_init_all();
    set_sys_clock_khz(176000, true); 
    multicore_launch_core1(core1_main);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_gpio_init(26 + CAPTURE_CHANNEL);
    adc_init();
    adc_select_input(CAPTURE_CHANNEL);
    adc_set_clkdiv(0);

    struct repeating_timer timer;
    add_repeating_timer_ms(0.01, read_adc_timer, NULL, &timer);

    while (true) {
     tight_loop_contents();
    }
}


void pwm_interrupt_handler() {
    
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN)); 

    while (multicore_fifo_rvalid()){
         
         uint16_t raw = multicore_fifo_pop_blocking();
         pwm_set_gpio_level(AUDIO_PIN, raw);  
    }   
}

void core1_main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Setup PWM for audio output
    pwm_config config = pwm_get_default_config();
    /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
     * to set the interrupt rate. 
     * 
     * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
     * 
     * 
     * So clkdiv should be as follows for given sample rate
     *  8.0f for 11 KHz
     *  4.0f for 22 KHz
     *  2.0f for 44 KHz etc
     */
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    // Infinte While Loop to wait for interrupt
    while (1){
        tight_loop_contents();
    }
}


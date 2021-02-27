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
#define NSAMP 1
#define CLOCK_DIV 2000
//44000 ADC


const uint LED_PIN = 25;

void core1_main();
uint8_t capture_buf[NSAMP];

int main() {
    stdio_init_all();

    set_sys_clock_khz(176000, true); 

    multicore_launch_core1(core1_main);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    adc_gpio_init(26 + CAPTURE_CHANNEL);

    adc_init();
    adc_select_input(CAPTURE_CHANNEL);
    adc_fifo_setup(
        true,    // Write each completed conversion to the sample FIFO
        true,    // Enable DMA data request (DREQ)
        1,       // DREQ (and IRQ) asserted when at least 1 sample present
        false,   // We won't see the ERR bit because of 8 bit reads; disable.
        true     // Shift each sample to 8 bits when pushing to FIFO
    );

    // set sample rate
    adc_set_clkdiv(CLOCK_DIV);

    sleep_ms(1000);
    // Set up the DMA to start transferring data as soon as it appears in FIFO
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);

    while (true) {
      adc_fifo_drain();
      adc_run(false);
      
      dma_channel_configure(dma_chan, &cfg,
        capture_buf,    // dst
        &adc_hw->fifo,  // src
        NSAMP,          // transfer count
        true            // start immediately
      );

      adc_run(true);

      uint64_t start_time = time_us_64();
      dma_channel_wait_for_finish_blocking(dma_chan);
      uint64_t end_time = time_us_64();
      
      uint64_t time_diff_us = end_time-start_time;
      float sample_time = time_diff_us/1e6;
      
    //printf("us: %llu | Total Time: %f s\n", time_diff_us, sample_time);
    //("Sample Rate: %0.1f Hz\n", NSAMP/(sample_time));
    for(int i=0; i < NSAMP; i++){
        if (multicore_fifo_wready){
            multicore_fifo_push_blocking(capture_buf[i]);
        }
    }
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
    pwm_config_set_clkdiv(&config, 16.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    // Infinte While Loop to wait for interrupt
    while (1){
        tight_loop_contents();
    }
}



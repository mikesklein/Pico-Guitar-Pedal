#include <stdio.h>
#include "pico/stdlib.h"
// For ADC input:
#include "hardware/adc.h"
#include "hardware/dma.h"
// For resistor DAC output:
#include "pico/multicore.h"
#include "hardware/irq.h"


#define CAPTURE_CHANNEL 0
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
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    multicore_launch_core1(core1_main);

    adc_gpio_init(26 + CAPTURE_CHANNEL);
    adc_init();
    adc_select_input(CAPTURE_CHANNEL);
    adc_set_clkdiv(0);

    struct repeating_timer timer;
    add_repeating_timer_ms(1, read_adc_timer, NULL, &timer);

    while (true) {
     tight_loop_contents();
    }
}

void core1_interrupt_handler() {

    // Receive Raw Value, Convert and Print Temperature Value
    while (multicore_fifo_rvalid()){
        uint16_t raw = multicore_fifo_pop_blocking();
       // const float conversion_factor = 3.3f / (1 << 12);
        gpio_put(LED_PIN, 1);
        printf("%-3d\n", raw);
        gpio_put(LED_PIN, 0);

    }

    multicore_fifo_clear_irq(); // Clear interrupt
}

void core1_main() {
// Configure Core 1 Interrupt
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    // Infinte While Loop to wait for interrupt
    while (1){
        tight_loop_contents();
    }
}


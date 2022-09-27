#include "stdio.h"
#include "stdlib.h"

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"
#include "pwm.pio.h"

const uint16_t period = 96u;

#define LEDS_COUNT  9

struct _rgb_led {
    uint8_t cmn;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} leds[LEDS_COUNT] = {
    {15, 7, 8, 9},
    {14, 7, 8, 9},
    {13, 7, 8, 9},
    {12, 7, 8, 9},
    {11, 7, 8, 9},
    {10, 13, 14, 15},
    {9, 13, 14, 15},
    {8, 13, 14, 15},
    {7, 13, 14, 15}
};

struct _led_brightness {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} leds_brightness[LEDS_COUNT] = {
    {31, 0, 0},
    {0, 31, 0},
    {0, 0, 31},
    {1, 0, 1},
    {31, 31, 31},
    {0, 0, 0},
    {24, 0, 0},
    {2, 2, 2},
    {0, 0 , 12}
};

static volatile int position = 0;
static PIO pio = pio0;
static int sm = 0;


void pio_pwm_set_levels(PIO pio, uint sm, uint32_t cmn, uint32_t red, uint32_t green, uint32_t blue) {
    uint32_t levels = ((cmn - 10) & 0x1F) | (((cmn - 10) & 0x1F) << 5) | (red << 10) | (green << 15) | (blue << 20) | ((period - (red + green + blue)) << 25);
    pio_sm_put_blocking(pio, sm, levels);
}

void gpio_callback(uint gpio, uint32_t events) {
    printf("Button pushed, reboot to bootloader.\n");
    reset_usb_boot(0, 0);
}

void draw_next_led(){
    
    if (++position == LEDS_COUNT)
        position = 0;
    
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_set_set_pins(pio, sm, leds[position].red, 3);
    pio_sm_put_blocking(pio, sm, (1 << (leds[position].cmn)));
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_pwm_set_levels(pio, sm, leds[position].cmn, leds_brightness[position].red,
        leds_brightness[position].green, leds_brightness[position].blue);
    pio_sm_set_enabled(pio, sm, true);
};

bool refresh_callback(struct repeating_timer *t) {
    draw_next_led();
    return true;
}

int main(){
    stdio_init_all();

    gpio_set_pulls(0, true, false);
    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true, &gpio_callback);

    uint offset = pio_add_program(pio, &pwmrgb_program);

    pwm_program_init(pio, sm, offset);
    for (uint8_t c = 0; c < LEDS_COUNT; c++){
        gpio_set_pulls(leds[c].cmn, false, false);
        gpio_set_function(leds[c].cmn, GPIO_FUNC_PIO0);
    };
    pio_sm_set_out_pins(pio, sm, 0, 32);


    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
    
    bool state = true;

    
    struct repeating_timer timer;
    add_repeating_timer_us(-2000, refresh_callback, NULL, &timer);
    uint8_t c = 0;
    while (true) {
        leds_brightness[c].red = 0;
        leds_brightness[c].blue = 0;
        leds_brightness[c].green = 0;        
        if (++c == LEDS_COUNT)
            c = 0;
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].red = ++leds_brightness[c].red;
            sleep_ms(10);
        };
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].red = --leds_brightness[c].red;
            sleep_ms(10);
        };
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].green = ++leds_brightness[c].green;
            sleep_ms(10);
        };
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].green = --leds_brightness[c].green;
            sleep_ms(10);
        };
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].blue = ++leds_brightness[c].blue;
            sleep_ms(10);
        };
        for (uint8_t i = 0; i < 31; i++){
            leds_brightness[c].blue = --leds_brightness[c].blue;
            sleep_ms(10);
        };
        gpio_put(PICO_DEFAULT_LED_PIN, state);
        state = !state;
        sleep_ms(100);
    }
	
}
